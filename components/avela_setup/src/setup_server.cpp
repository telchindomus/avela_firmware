#include "avela_setup/setup_server.hpp"

#include <array>
#include <cstring>
#include <optional>

#include "avela_settings/settings.hpp"
#include "avela_setup/page_password.hpp"
#include "avela_setup/setup_network_client.hpp"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "lwip/sockets.h"

namespace avela::setup_page {
namespace {

constexpr const char* log_tag = "setup";
constexpr uint16_t max_open_sockets = 4;    // phones open several at once
constexpr std::size_t query_capacity = 64;  // "tab=..." and whatever else an address carries; longer ones are ignored
// "Basic " and the base64 of a user name and the password; a longer header is refused
constexpr std::size_t authorization_capacity = 128;

constexpr std::string_view style = R"(<style>
:root{color-scheme:dark}
body{margin:0;padding:16px;background:#0F4248;color:#F5F2E9;font:16px/1.4 system-ui,sans-serif}
main{max-width:480px;margin:auto}
h1{font-size:22px;margin:4px 0 16px}h1 span{color:#D2AF6D}
section{background:#14505A;border-radius:12px;padding:14px 16px;margin-bottom:16px}
h2{font-size:17px;margin:0 0 10px}h3{font-size:15px;margin:0;color:#D2AF6D}
form+form{border-top:1px solid #2C6A6C;margin-top:16px;padding-top:12px}
label{display:block;color:#A1A3CB;font-size:14px;margin:10px 0 4px}
input,select{box-sizing:border-box;width:100%;padding:10px;border-radius:8px;border:1px solid #2C6A6C;
background:#0F4248;color:#F5F2E9;font-size:16px}
button{margin-top:14px;padding:10px 18px;border:0;border-radius:8px;background:#D2AF6D;color:#0F4248;
font-weight:700;font-size:16px}
p{color:#7DA499}a{color:#D2AF6D}
nav{display:flex;flex-wrap:wrap;gap:6px;margin-bottom:16px}
nav a{padding:8px 12px;border-radius:8px;background:#14505A;color:#F5F2E9;text-decoration:none;font-size:15px}
nav a.shown{background:#D2AF6D;color:#0F4248;font-weight:700}
.result{margin:0 0 16px;padding:10px 16px;border-radius:12px;background:#14505A}
.result.done{color:#D2AF6D}.result.failed{color:#E8A07A}
.row{display:flex;gap:8px;align-items:center}.row button{margin-top:0}.row input{flex:1}.row .lead{order:-1}
</style>)";

//--------------------------------------------------------------------------------------------------

// The radio's address and netmask on one of its networks ("WIFI_AP_DEF": the setup network, "WIFI_STA_DEF": the home
// network); nothing while that network is down or has no address yet.
std::optional<Ipv4Network> network(const char* interface_key)
{
    wifi_mode_t mode = WIFI_MODE_NULL;
    if (esp_wifi_get_mode(&mode) != ESP_OK || mode == WIFI_MODE_NULL) {
        return std::nullopt;
    }
    const bool setup = std::string_view{interface_key} == "WIFI_AP_DEF";
    if (setup && mode != WIFI_MODE_AP && mode != WIFI_MODE_APSTA) {
        return std::nullopt;
    }
    esp_netif_t* interface = esp_netif_get_handle_from_ifkey(interface_key);
    esp_netif_ip_info_t info{};
    if (interface == nullptr || esp_netif_get_ip_info(interface, &info) != ESP_OK || info.ip.addr == 0) {
        return std::nullopt;
    }
    return Ipv4Network{.m_address = info.ip.addr, .m_netmask = info.netmask.addr};
}

//--------------------------------------------------------------------------------------------------

// The IPv4 address of a socket's end: the client's (getpeername) or ours (getsockname). The server's socket is IPv6
// (lwIP with IPv6), so an IPv4 client comes as ::ffff:a.b.c.d; nothing for native IPv6 or a failed query.
using SocketQuery = int (*)(int, sockaddr*, socklen_t*);
std::optional<uint32_t> socket_ipv4(int socket, SocketQuery query)
{
    sockaddr_storage end{};
    socklen_t size = sizeof(end);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): the BSD socket API takes sockaddr*
    if (query(socket, reinterpret_cast<sockaddr*>(&end), &size) != 0) {
        return std::nullopt;
    }
    if (end.ss_family == AF_INET6) {
        sockaddr_in6 end6{};
        std::memcpy(&end6, &end, sizeof(end6));
        std::array<uint8_t, sizeof(end6.sin6_addr)> bytes{};
        std::memcpy(bytes.data(), &end6.sin6_addr, bytes.size());
        return setup_network_client::mapped_ipv4(bytes);
    }
    if (end.ss_family == AF_INET) {
        sockaddr_in end4{};
        std::memcpy(&end4, &end, sizeof(end4));
        return end4.sin_addr.s_addr;
    }
    return std::nullopt;
}

//--------------------------------------------------------------------------------------------------

// setup_network_client.hpp for this request's connection.
bool from_setup_network(httpd_req_t* request)
{
    const std::optional<Ipv4Network> setup_network = network("WIFI_AP_DEF");
    if (!setup_network) {
        return false;
    }
    const int socket = httpd_req_to_sockfd(request);
    const std::optional<uint32_t> client = socket_ipv4(socket, &getpeername);
    const std::optional<uint32_t> connected_to = socket_ipv4(socket, &getsockname);
    return client && connected_to &&
           setup_network_client::is_setup_network_client(*client, *connected_to, *setup_network,
                                                         network("WIFI_STA_DEF"));
}

//--------------------------------------------------------------------------------------------------

// Unknown addresses: from the setup network (captive-portal checks of phones: /generate_204, /hotspot-detect.html
// ...) a redirect to the page, elsewhere a plain 404.
esp_err_t not_found(httpd_req_t* request, httpd_err_code_t /*error*/)
{
    const std::optional<Ipv4Network> setup_network = network("WIFI_AP_DEF");
    if (!setup_network || !from_setup_network(request)) {
        return httpd_resp_send_err(request, HTTPD_404_NOT_FOUND, "Not found");
    }
    std::array<char, capacity_for("255.255.255.255")> address{};
    const esp_ip4_addr_t ip{.addr = setup_network->m_address};
    esp_ip4addr_ntoa(&ip, address.data(), static_cast<int>(address.size()));
    FixedString<capacity_for("http://255.255.255.255/")> url;
    url.append("http://").append(address.data()).append("/");
    httpd_resp_set_status(request, "302 Found");
    httpd_resp_set_hdr(request, "Location", url.c_str());
    return httpd_resp_send(request, nullptr, 0);
}

//--------------------------------------------------------------------------------------------------

// A refusal as a page in the setup page's style: a heading and the steps.
void send_refusal(httpd_req_t* request, std::string_view title, std::string_view text)
{
    httpd_resp_set_type(request, "text/html; charset=utf-8");
    HtmlWriter writer{request};
    write_head(writer);
    write_section_start(writer, title);
    writer.raw("<p>").text(text).raw("</p>");
    writer.raw("<ol><li>On the radio open Info and hold the knob: Wi-Fi setup opens its setup network.</li>"
               "<li>Join that network with the name and password on the radio's screen.</li>"
               "<li>On the page that opens, set a password on the Access tab.</li></ol>"
               "<p>Or over USB: <code>avela-tuner set access password=...</code></p>");
    write_section_end(writer);
    write_tail(writer);
    writer.finish();
}

//--------------------------------------------------------------------------------------------------

// Logs once how long the first password check took (PBKDF2 on every request; PagePassword::iterations).
void log_check_time(int64_t started_us)
{
    static bool logged = false;  // only the server's task checks
    if (!logged) {
        logged = true;
        constexpr int64_t microseconds_per_millisecond = 1000;
        ESP_LOGI(log_tag, "page password checked in %lld ms",
                 (esp_timer_get_time() - started_us) / microseconds_per_millisecond);
    }
}

}  // namespace

//--------------------------------------------------------------------------------------------------

httpd_handle_t start(void* context, Handler page, Handler save)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = server_stack_bytes;
    config.max_open_sockets = max_open_sockets;
    config.lru_purge_enable = true;
    httpd_handle_t server = nullptr;
    if (const esp_err_t result = httpd_start(&server, &config); result != ESP_OK) {
        ESP_LOGE(log_tag, "web server not started: %s", esp_err_to_name(result));
        return nullptr;
    }
    const httpd_uri_t page_uri{.uri = "/", .method = HTTP_GET, .handler = page, .user_ctx = context};
    const httpd_uri_t save_uri{.uri = "/save", .method = HTTP_POST, .handler = save, .user_ctx = context};
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &page_uri));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &save_uri));
    ESP_ERROR_CHECK(httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, &not_found));
    return server;
}

//--------------------------------------------------------------------------------------------------

bool allowed(httpd_req_t* request)
{
    if (from_setup_network(request)) {
        return true;
    }
    PagePassword saved{};
    if (!Settings::load(saved)) {
        httpd_resp_set_status(request, "403 Forbidden");
        send_refusal(request, "No page password yet",
                     "On your home network this page opens only with a password, and none is set yet.");
        return false;
    }
    std::array<char, authorization_capacity> authorization{};
    if (httpd_req_get_hdr_value_str(request, "Authorization", authorization.data(), authorization.size()) == ESP_OK) {
        const int64_t started_us = esp_timer_get_time();
        const bool authorized = page_password::authorizes(authorization.data(), saved);
        log_check_time(started_us);
        if (authorized) {
            return true;
        }
    }
    httpd_resp_set_status(request, "401 Unauthorized");
    httpd_resp_set_hdr(request, "WWW-Authenticate", R"(Basic realm="Avela setup, user avela-admin", charset="UTF-8")");
    send_refusal(request, "Page password",
                 "This page needs the user name avela-admin and the page password. Forgotten: set a new one from "
                 "the radio's setup network.");
    return false;
}

//--------------------------------------------------------------------------------------------------

esp_err_t after_refusal(httpd_req_t* request)
{
    // with a body still coming, closing beats reading it all (an upload is megabytes, and a slow client would hold
    // the server's one task); ESP_FAIL makes the server close the connection
    return request->content_len > 0 ? ESP_FAIL : ESP_OK;
}

//--------------------------------------------------------------------------------------------------

bool page_password_set()
{
    PagePassword saved{};
    return Settings::load(saved);
}

//--------------------------------------------------------------------------------------------------

TabId requested_tab(httpd_req_t* request)
{
    std::array<char, query_capacity> query{};
    std::array<char, tab_capacity + 1> value{};
    TabId tab;
    if (httpd_req_get_url_query_str(request, query.data(), query.size()) == ESP_OK &&
        httpd_query_key_value(query.data(), "tab", value.data(), value.size()) == ESP_OK) {
        tab.append(value.data());
    }
    return tab;
}

//--------------------------------------------------------------------------------------------------

bool setup_network_up()
{
    return network("WIFI_AP_DEF").has_value();
}

//--------------------------------------------------------------------------------------------------

void write_head(HtmlWriter& writer)
{
    writer
        .raw(R"(<!doctype html><html lang="en"><head><meta charset="utf-8">)"
             R"(<meta name="viewport" content="width=device-width,initial-scale=1"><title>Avela setup</title>)")
        .raw(style)
        .raw("</head><body><main><h1><span>Avela</span> setup</h1>");
}

//--------------------------------------------------------------------------------------------------

void write_tabs_start(HtmlWriter& writer)
{
    writer.raw("<nav>");
}

//--------------------------------------------------------------------------------------------------

void write_tab(HtmlWriter& writer, std::string_view id, std::string_view title, bool shown)
{
    writer.raw(R"(<a href="/?tab=)").text(id).raw(shown ? R"(" class="shown">)" : R"(">)").text(title).raw("</a>");
}

//--------------------------------------------------------------------------------------------------

void write_tabs_end(HtmlWriter& writer)
{
    writer.raw("</nav>");
}

//--------------------------------------------------------------------------------------------------

void write_section_start(HtmlWriter& writer, std::string_view title)
{
    writer.raw("<section><h2>").text(title).raw("</h2>");
}

//--------------------------------------------------------------------------------------------------

void write_section_end(HtmlWriter& writer)
{
    writer.raw("</section>");
}

//--------------------------------------------------------------------------------------------------

void write_form_start(HtmlWriter& writer, std::string_view id)
{
    writer.raw(R"(<form method="post" action="/save"><input type="hidden" name="section" value=")").text(id);
    writer.raw(R"(">)");
}

//--------------------------------------------------------------------------------------------------

void write_form_end(HtmlWriter& writer)
{
    writer.raw(R"(<button type="submit">Save</button></form>)");
}

//--------------------------------------------------------------------------------------------------

void write_tail(HtmlWriter& writer)
{
    writer.raw("</main></body></html>");
}

//--------------------------------------------------------------------------------------------------

bool read_form(httpd_req_t* request, FormFields& fields)
{
    const std::span<char> buffer = fields.buffer();
    if (request->content_len > buffer.size()) {
        httpd_resp_send_err(request, HTTPD_400_BAD_REQUEST, "Form too long");
        return false;
    }
    std::size_t received = 0;
    while (received < request->content_len) {
        const int count = receive(request, buffer.subspan(received, request->content_len - received));
        if (count <= 0) {
            return false;  // the client is gone or silent; nothing to answer
        }
        received += static_cast<std::size_t>(count);
    }
    fields.parse(received);
    return true;
}

//--------------------------------------------------------------------------------------------------

int receive(httpd_req_t* request, std::span<char> buffer)
{
    for (int timeouts = 0;;) {
        const int count = httpd_req_recv(request, buffer.data(), buffer.size());
        if (count != HTTPD_SOCK_ERR_TIMEOUT || ++timeouts == max_receive_timeouts) {
            return count;
        }
    }
}

//--------------------------------------------------------------------------------------------------

void write_result(HtmlWriter& writer, const ApplyResult& result)
{
    writer.raw(result.m_saved ? R"(<p class="result done">)" : R"(<p class="result failed">)");
    writer.text(result.m_message.view()).raw("</p>");
}

}  // namespace avela::setup_page
