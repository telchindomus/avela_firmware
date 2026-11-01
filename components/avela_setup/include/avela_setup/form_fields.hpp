// A submitted form in a fixed buffer, from the setup page or the serial console. Setup page (application/x-www-form-
// urlencoded): the body is read into buffer(), then parse() decodes it in place (+ and %XX). Serial console: clear(),
// then add() each name=value it was given. find() returns a field's value. No heap: the buffer is its owner's (each
// frontend sizes its own); longer bodies are refused by the server before they get here.
#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <span>
#include <string_view>

namespace avela {

class FormFields {
public:
    static constexpr std::size_t max_fields = 8;

    explicit FormFields(std::span<char> buffer) : m_buffer_(buffer) {}

    // Where the body is read to, before parse().
    [[nodiscard]] std::span<char> buffer() { return m_buffer_; }

    // Decodes the first `length` bytes of the buffer into fields; fields beyond max_fields are dropped.
    void parse(std::size_t length);

    // Empties the fields, for add().
    void clear();
    // Adds a field, copying its name and value into the buffer; false if there is no room for it.
    bool add(std::string_view name, std::string_view value);

    [[nodiscard]] std::size_t size() const { return m_count_; }
    // The name of the field at `index` (< size()).
    [[nodiscard]] std::string_view name(std::size_t index) const { return m_fields_.at(index).m_name; }

    // The decoded value of the first field with this name.
    [[nodiscard]] std::optional<std::string_view> find(std::string_view name) const;

private:
    struct Field {
        std::string_view m_name;
        std::string_view m_value;
    };

    std::span<char> m_buffer_;
    std::array<Field, max_fields> m_fields_{};
    std::size_t m_count_ = 0;
    std::size_t m_used_ = 0;  // bytes of the buffer add() has filled
};

}  // namespace avela
