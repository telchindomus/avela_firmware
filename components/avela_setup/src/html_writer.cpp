#include "avela_setup/html_writer.hpp"

namespace avela {

//--------------------------------------------------------------------------------------------------

HtmlWriter& HtmlWriter::raw(std::string_view markup)
{
    for (const char character : markup) {
        put(character);
    }
    return *this;
}

//--------------------------------------------------------------------------------------------------

HtmlWriter& HtmlWriter::text(std::string_view value)
{
    for (const char character : value) {
        switch (character) {
        case '&':
            raw("&amp;");
            break;
        case '<':
            raw("&lt;");
            break;
        case '>':
            raw("&gt;");
            break;
        case '"':
            raw("&quot;");
            break;
        case '\'':
            raw("&#39;");
            break;
        default:
            put(character);
        }
    }
    return *this;
}

//--------------------------------------------------------------------------------------------------

bool HtmlWriter::finish()
{
    flush();
    if (!m_failed_ && httpd_resp_send_chunk(m_request_, nullptr, 0) != ESP_OK) {
        m_failed_ = true;
    }
    return !m_failed_;
}

//--------------------------------------------------------------------------------------------------

void HtmlWriter::put(char character)
{
    if (m_length_ == m_buffer_.size()) {
        flush();
    }
    m_buffer_.at(m_length_) = character;
    ++m_length_;
}

//--------------------------------------------------------------------------------------------------

void HtmlWriter::flush()
{
    if (m_length_ > 0 && !m_failed_) {
        m_failed_ = httpd_resp_send_chunk(m_request_, m_buffer_.data(), static_cast<ssize_t>(m_length_)) != ESP_OK;
    }
    m_length_ = 0;
}

}  // namespace avela
