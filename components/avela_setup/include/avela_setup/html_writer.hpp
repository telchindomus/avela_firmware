// Writes an HTML response in chunks through a small buffer (no heap, no page-sized string). text() escapes,
// raw() doesn't: everything that comes from a user, a setting or the air (network names) goes through text().
#pragma once

#include <array>
#include <cstddef>
#include <string_view>

#include "esp_http_server.h"

namespace avela {

class HtmlWriter {
public:
    explicit HtmlWriter(httpd_req_t* request) : m_request_(request) {}

    // Markup written as is.
    HtmlWriter& raw(std::string_view markup);

    // Text or attribute value, with & < > " ' escaped.
    HtmlWriter& text(std::string_view value);

    // Sends what is buffered and ends the response; true if every chunk went out.
    bool finish();

private:
    static constexpr std::size_t buffer_size = 512;

    void put(char character);
    void flush();

    httpd_req_t* m_request_;
    std::array<char, buffer_size> m_buffer_{};
    std::size_t m_length_ = 0;
    bool m_failed_ = false;
};

}  // namespace avela
