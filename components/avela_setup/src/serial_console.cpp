#include "avela_setup/serial_console.hpp"

#include <cstdio>

#include "driver/uart.h"
#include "driver/uart_vfs.h"
#include "sdkconfig.h"

namespace avela {
namespace {

constexpr uart_port_t console_port = static_cast<uart_port_t>(CONFIG_ESP_CONSOLE_UART_NUM);
constexpr int receive_buffer_bytes = 512;  // the driver's minimum is twice the hardware FIFO (128)
constexpr char backspace = '\b';
constexpr char delete_key = 0x7F;
constexpr char carriage_return = '\r';
constexpr char line_feed = '\n';
constexpr char first_printable = ' ';
constexpr char last_printable = '~';
constexpr unsigned char first_utf8_byte = 0x80;

}  // namespace

//--------------------------------------------------------------------------------------------------

void SerialConsole::init()
{
    // the driver for input; stdout through it too, so the log's and the console's output don't cut into each other
    uart_driver_install(console_port, receive_buffer_bytes, 0, 0, nullptr, 0);
    uart_vfs_dev_use_driver(console_port);
    constexpr UBaseType_t priority = 1;
    xTaskCreateStatic(&SerialConsole::run, task_name, m_stack_.size(), this, priority, m_stack_.data(), &m_task_);
}

//--------------------------------------------------------------------------------------------------

void SerialConsole::prompt()
{
    write("avela> ");
}

//--------------------------------------------------------------------------------------------------

void SerialConsole::write(std::string_view text)
{
    if (!text.empty()) {
        static_cast<void>(std::fwrite(text.data(), 1, text.size(), stdout));
    }
    static_cast<void>(std::fflush(stdout));
}

//--------------------------------------------------------------------------------------------------

void SerialConsole::run(void* self)
{
    static_cast<SerialConsole*>(self)->run();
}

//--------------------------------------------------------------------------------------------------

void SerialConsole::run()
{
    write("\nAvela console: type help\n");
    prompt();
    while (true) {
        uint8_t byte = 0;
        if (uart_read_bytes(console_port, &byte, 1, portMAX_DELAY) == 1) {
            take(static_cast<char>(byte));
        }
    }
}

//--------------------------------------------------------------------------------------------------

void SerialConsole::take(char character)
{
    const bool line_feed_after_return = character == line_feed && m_after_carriage_return_;
    m_after_carriage_return_ = character == carriage_return;
    if (line_feed_after_return) {
        return;  // "\r\n" ends one line, not two
    }
    if (character == carriage_return || character == line_feed) {
        write("\n");
        m_handler_(m_context_, std::string_view{m_line_.data(), m_length_});
        m_length_ = 0;
        prompt();
        return;
    }
    if (character == backspace || character == delete_key) {
        if (m_length_ > 0) {
            --m_length_;
            write("\b \b");
        }
        return;
    }
    // printable ASCII, and the bytes of UTF-8 characters (place names like Kraków)
    const bool printable = (character >= first_printable && character <= last_printable) ||
                           static_cast<unsigned char>(character) >= first_utf8_byte;
    if (printable && m_length_ < m_line_.size()) {
        m_line_.at(m_length_) = character;
        ++m_length_;
        write(std::string_view{&character, 1});
    }
}

}  // namespace avela
