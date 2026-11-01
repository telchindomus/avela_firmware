// The web setup page: an HTTP server on port 80 of every interface (the home network once connected, the radio's own
// setup network while that is up). The sections are tabs: GET /?tab=<section id> shows that section's form (no tab, or
// an unknown one: the landing tab); POST /save hands the form to its section and shows its tab again with the result
// above it. While the setup network is up, every unknown address is redirected to the page, so a phone that joins it
// opens the page by itself (captive portal; the network service answers every DNS query with our address), and the
// page opens on the setup network's landing tab.
// On the home network every page asks for the page password (page_password.hpp; allowed()); on the setup network none
// does: whoever joined it has its password, and it is the way back in when the page password is forgotten.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <tuple>

#include "avela_setup/apply_lock.hpp"
#include "avela_setup/apply_result.hpp"
#include "avela_setup/form_fields.hpp"
#include "avela_setup/html_writer.hpp"
#include "avela_setup/setup_field.hpp"
#include "avela_setup/setup_section_concept.hpp"
#include "avela_setup/writes_own_forms_concept.hpp"
#include "avela_strings/fixed_string.hpp"
#include "esp_http_server.h"

namespace avela {

namespace setup_page {

inline constexpr const char* server_task_name = "httpd";  // ESP-IDF's name for the server's task
inline constexpr uint32_t server_stack_bytes = 6144;      // HtmlWriter buffer + the sections' work
// The server has one task: a client that stops sending in the middle of a body would block every page, so a body
// read gives up after this many receive timeouts in a row (the server's 5 s each).
inline constexpr int max_receive_timeouts = 3;
// Bytes of a form's body as the browser sends it: URL-encoded, up to three bytes (%XX) for each one typed. A section
// whose longest form could be longer checks that it fits (the stations: static_assert).
inline constexpr std::size_t form_capacity = 1024;

inline constexpr std::size_t tab_capacity = 32;  // a section id in the address
using TabId = FixedString<tab_capacity>;

// Plain functions shared by every SetupServer instantiation (src/setup_server.cpp).
using Handler = esp_err_t (*)(httpd_req_t*);
// Starts the server and registers the handlers; the server, or nullptr if it didn't start.
httpd_handle_t start(void* context, Handler page, Handler save);
// Whether a request may be answered: it came from the setup network (setup_network_client.hpp), or it carries the
// page password. If not, it sends the refusal page (401, which makes the browser ask for the password; 403 while none
// is set) and returns false; the handler then returns after_refusal().
bool allowed(httpd_req_t* request);
// What a handler returns after a refusal: ESP_FAIL for a request with a body, so the server closes the connection
// instead of reading the body first; ESP_OK otherwise.
esp_err_t after_refusal(httpd_req_t* request);
// Whether a page password is saved (the Wi-Fi tab reminds to set one before the setup network closes).
bool page_password_set();
// The tab the address asks for (?tab=...), empty if none.
TabId requested_tab(httpd_req_t* request);
bool setup_network_up();
void write_head(HtmlWriter& writer);  // document start, style, title
void write_tabs_start(HtmlWriter& writer);
void write_tab(HtmlWriter& writer, std::string_view id, std::string_view title, bool shown);
void write_tabs_end(HtmlWriter& writer);
void write_result(HtmlWriter& writer, const ApplyResult& result);
void write_section_start(HtmlWriter& writer, std::string_view title);
void write_section_end(HtmlWriter& writer);
// A form of section `id`: its fields go between these two (the end adds the Save button).
void write_form_start(HtmlWriter& writer, std::string_view id);
void write_form_end(HtmlWriter& writer);
void write_tail(HtmlWriter& writer);
bool read_form(httpd_req_t* request, FormFields& fields);  // false (and an error response sent) if it can't
// Receives up to buffer.size() bytes of the request's body: how many came, or 0 or less when the client has gone or
// stayed silent for max_receive_timeouts receive timeouts.
int receive(httpd_req_t* request, std::span<char> buffer);

}  // namespace setup_page

// The tab the page opens on when the address names none: section ids.
struct SetupLanding {
    std::string_view m_tab;                // on the home network
    std::string_view m_setup_network_tab;  // while the setup network is up (the first setup: Wi-Fi)
};

// The tabs are the sections, in the order given here.
template <SetupSectionConcept... Sections>
class SetupServer {
public:
    SetupServer(SetupLanding landing, Sections... sections) : m_landing_(landing), m_sections_{sections...} {}

    // After the network stack is up (the network service's init()). Once.
    void init() { m_http_server_ = setup_page::start(this, &page, &save); }

    // The HTTP server, for a component that serves pages of its own on it (the firmware update); nullptr before init()
    // or if it didn't start.
    [[nodiscard]] httpd_handle_t http_server() const { return m_http_server_; }

private:
    static esp_err_t page(httpd_req_t* request)
    {
        if (!setup_page::allowed(request)) {
            return setup_page::after_refusal(request);
        }
        auto& server = *static_cast<SetupServer*>(request->user_ctx);
        const setup_page::TabId tab = setup_page::requested_tab(request);
        return server.write_page(request, server.shown_tab(tab.view()), nullptr);
    }

    static esp_err_t save(httpd_req_t* request)
    {
        if (!setup_page::allowed(request)) {
            return setup_page::after_refusal(request);
        }
        auto& server = *static_cast<SetupServer*>(request->user_ctx);
        // only the server's one task handles requests; too big for its stack
        static std::array<char, setup_page::form_capacity> body{};
        static FormFields fields{body};
        if (!setup_page::read_form(request, fields)) {
            return ESP_OK;  // the error response is sent
        }
        const std::string_view id = fields.find("section").value_or("");
        ApplyResult result;
        result.m_message.append("Unknown section.");
        std::apply(
            [&](auto&... section) {
                (
                    [&] {
                        using SectionType = std::remove_reference_t<decltype(section)>;
                        if (id == SectionType::id) {
                            const ApplyLock lock;
                            result = section.apply(fields);
                        }
                    }(),
                    ...);
            },
            server.m_sections_);
        return server.write_page(request, server.shown_tab(id), &result);
    }

    // `tab` if it is a section's id, else the landing tab.
    [[nodiscard]] std::string_view shown_tab(std::string_view tab) const
    {
        if (((tab == Sections::id) || ...)) {
            return tab;
        }
        return setup_page::setup_network_up() ? m_landing_.m_setup_network_tab : m_landing_.m_tab;
    }

    // The tabs, the result of a save if there is one, the shown tab's section.
    esp_err_t write_page(httpd_req_t* request, std::string_view tab, const ApplyResult* result)
    {
        httpd_resp_set_type(request, "text/html; charset=utf-8");
        HtmlWriter writer{request};
        setup_page::write_head(writer);
        setup_page::write_tabs_start(writer);
        (setup_page::write_tab(writer, Sections::id, Sections::title, tab == Sections::id), ...);
        setup_page::write_tabs_end(writer);
        if (result != nullptr) {
            setup_page::write_result(writer, *result);
        }
        std::apply(
            [&](auto&... section) {
                (
                    [&] {
                        using SectionType = std::remove_reference_t<decltype(section)>;
                        if (tab != SectionType::id) {
                            return;
                        }
                        setup_page::write_section_start(writer, SectionType::title);
                        if constexpr (WritesOwnFormsConcept<SectionType>) {
                            section.write_form(writer);
                        } else {
                            setup_page::write_form_start(writer, SectionType::id);
                            section.write_form(writer);
                            setup_page::write_form_end(writer);
                        }
                        setup_page::write_section_end(writer);
                    }(),
                    ...);
            },
            m_sections_);
        setup_page::write_tail(writer);
        return writer.finish() ? ESP_OK : ESP_FAIL;
    }

    SetupLanding m_landing_;
    std::tuple<Sections...> m_sections_;
    httpd_handle_t m_http_server_ = nullptr;
};

}  // namespace avela
