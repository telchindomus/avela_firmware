// The Avela app: wires the board and the skin (both included by ui.hpp), the UI and the components together: it owns
// everything, hands the models to the Ui and the services' queues to their clients, and turns the board's input events
// into commands for the shown screen (Ui). The board owns its hardware (display, inputs) and the canvas draws on its
// display; the components' models and services are members by name (a data model is written only by its service, on
// the main loop; a service carries out the requests its clients queue: avela_messaging); the Ui builds the screens
// from the models (UiModels, read only) and sends their controllers' requests to the services through route(); the
// setup page gets the components' sections. run() polls the inputs every 5 ms and every 33 ms lets the services take
// their requests and update the models, then the shown screen redraw what changed. app_main holds the one Application
// in static storage (no heap; too big for the main task's stack: the canvas alone holds 12.5 KB of band buffers).
#pragma once

#include <array>
#include <cstdint>
#include <optional>

#include "avela_boards/avela_board_concept.hpp"
#include "avela_boards/setup/board_section.hpp"
#include "avela_clock/setup/clock_section.hpp"
#include "avela_firmware_update/model/firmware_update_model.hpp"
#include "avela_firmware_update/service/esp_firmware_update_service.hpp"
#include "avela_hal/input/events/input_event.hpp"
#include "avela_messaging/messages/request.hpp"
#include "avela_network/model/network_model.hpp"
#include "avela_network/service/esp_wifi_service.hpp"
#include "avela_network/setup/wifi_section.hpp"
#include "avela_radio/model/radio_model.hpp"
#include "avela_radio/service/radio_storage.hpp"
#include "avela_radio/service/streaming_radio_service.hpp"
#include "avela_radio/setup/player_section.hpp"
#include "avela_radio/setup/stations_section.hpp"
#include "avela_setup/access_section.hpp"
#include "avela_setup/setup_console.hpp"
#include "avela_setup/setup_server.hpp"
#include "avela_system_info/model/system_info_model.hpp"
#include "avela_system_info/service/esp_system_info_service.hpp"
#include "avela_system_info/service/task_stack.hpp"
#include "avela_ui/canvas.hpp"
#include "avela_ui/command/command.hpp"
#include "sdkconfig.h"
#include "ui.hpp"
#if CONFIG_AVELA_WEATHER
#include "avela_weather/model/weather_model.hpp"
#include "avela_weather/service/open_meteo_weather_service.hpp"
#include "avela_weather/setup/weather_section.hpp"
#endif

namespace avela {

// The application's command table and the radio need Avela's controls and sound.
static_assert(AvelaBoardConcept<Board>,
              "the chosen board (menuconfig: Avela board) must be an Avela board (AvelaBoard)");

class Application {
public:
    Application() = default;
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;
    ~Application() = default;

    // Once: the board (display, inputs), settings, the brightness, fonts, the serial console, clock, network and setup
    // page; then the first frame.
    void init();
    // The loop, forever: the inputs every 5 ms, a frame every 33 ms.
    [[noreturn]] void run();

private:
    // The board's input events as commands to the shown screen.
    void handle_inputs(uint32_t now_ms);
    // A controller's request, into the queue of the service it is for.
    void route(const Request& request);
    // What the Ui hands its requests to: route(), as one type, so the Ui's templates are built once.
    struct RequestRoute {
        Application* m_application;
        void operator()(const Request& request) const { m_application->route(request); }
    };
    void deliver(const PlayerRequest& request);
    void deliver(const SetupNetworkRequest& request);
    void deliver(const AcceptUploadsRequest& request);
#if CONFIG_AVELA_WEATHER
    void deliver(const WeatherUpdateRequest& request);
#endif
    // What an input event means, the same on every screen: the knob turned = IncrementCommand / DecrementCommand (a
    // step per detent), its click = OkCommand, double click = MenuCommand, long press = OptionsCommand; the back
    // button's click = BackCommand. Nothing for anything else.
    [[nodiscard]] std::optional<Command> command_for(const Turn& turn) const;
    [[nodiscard]] std::optional<Command> command_for(const Click& click) const;
    [[nodiscard]] std::optional<Command> command_for(const DoubleClick& double_click) const;
    [[nodiscard]] std::optional<Command> command_for(const LongPress& long_press) const;
    // Each service writes its model; the clock and the board follow their saved settings.
    void update_models();
    // The saved brightness (BoardSettings) to the display.
    void apply_board_settings();
    // The requests, the models, the radio's status, then the shown screen redraws what changed.
    void draw_frame();

    // Every task's stack as declared, for the system info service's log: ours, then ESP-IDF's whose size is a
    // setting (lwIP's task is "tiT").
    static constexpr std::array task_stacks{
        TaskStack{.m_name = "main", .m_bytes = CONFIG_ESP_MAIN_TASK_STACK_SIZE},
        TaskStack{.m_name = StreamReceiver::task_name, .m_bytes = StreamReceiver::stack_bytes},
        TaskStack{.m_name = Mp3Player::task_name, .m_bytes = Mp3Player::stack_bytes},
#if CONFIG_AVELA_WEATHER
        TaskStack{.m_name = OpenMeteoWeatherService::task_name, .m_bytes = OpenMeteoWeatherService::stack_bytes},
#endif
        TaskStack{.m_name = SerialConsole::task_name, .m_bytes = SerialConsole::stack_bytes},
        TaskStack{.m_name = CaptiveDns::task_name, .m_bytes = CaptiveDns::stack_bytes},
        TaskStack{.m_name = setup_page::server_task_name, .m_bytes = setup_page::server_stack_bytes},
        TaskStack{.m_name = "mdns", .m_bytes = CONFIG_MDNS_TASK_STACK_SIZE},
        TaskStack{.m_name = "tiT", .m_bytes = CONFIG_LWIP_TCPIP_TASK_STACK_SIZE},
        TaskStack{.m_name = "sys_evt", .m_bytes = CONFIG_ESP_SYSTEM_EVENT_TASK_STACK_SIZE},
        TaskStack{.m_name = "esp_timer", .m_bytes = CONFIG_ESP_TIMER_TASK_STACK_SIZE},
        TaskStack{.m_name = "Tmr Svc", .m_bytes = CONFIG_FREERTOS_TIMER_TASK_STACK_DEPTH},
    };

    // Members are built in this order, and later ones refer to earlier ones: the canvas draws on the board's display,
    // the Ui and the setup page read the models.
    Board m_board_;
    BandBuffers<Board::screen.m_width> m_band_buffers_;
    Canvas m_canvas_{m_board_.display(), m_band_buffers_};
    Skin::Fonts m_fonts_;
    SystemInfoModel m_system_info_;
    EspSystemInfoService m_system_info_service_{task_stacks};
    NetworkModel m_network_;
    EspWifiService m_network_service_;
    RadioModel m_radio_;
    RadioStorage m_radio_storage_;
    StreamingRadioService m_radio_service_{m_network_, m_board_.audio_output()};
    FirmwareUpdateModel m_firmware_update_;
    EspFirmwareUpdateService m_firmware_update_service_;
#if CONFIG_AVELA_WEATHER
    WeatherModel m_weather_;
    OpenMeteoWeatherService m_weather_service_{m_network_};
#endif
    Ui m_ui_{Skin::Surface{.m_canvas = m_canvas_, .m_fonts = m_fonts_}, UiModels{
                                                                            .m_radio = m_radio_,
                                                                            .m_system_info = m_system_info_,
                                                                            .m_network = m_network_,
                                                                            .m_firmware_update = m_firmware_update_,
#if CONFIG_AVELA_WEATHER
                                                                            .m_weather = m_weather_,
#endif
                                                                        }};
    // the setup page opens on the player, on the setup network (the first setup) on Wi-Fi
    static constexpr SetupLanding setup_landing{.m_tab = PlayerSection::id, .m_setup_network_tab = WifiSection::id};
    // the settings sections, in order, on the web setup page (its tabs) and the USB serial console alike
#if CONFIG_AVELA_WEATHER
    SetupServer<WifiSection, AccessSection, StationsSection, PlayerSection, WeatherSection, ClockSection, BoardSection>
        m_setup_server_{setup_landing,
                        WifiSection{m_network_},
                        AccessSection{},
                        StationsSection{},
                        PlayerSection{m_radio_service_.queues().requests(), m_radio_service_.queues().status()},
                        WeatherSection{},
                        ClockSection{},
                        BoardSection{}};
    SetupConsole<WifiSection, AccessSection, StationsSection, PlayerSection, WeatherSection, ClockSection, BoardSection>
        m_setup_console_{WifiSection{m_network_},
                         AccessSection{},
                         StationsSection{},
                         PlayerSection{m_radio_service_.queues().requests(), m_radio_service_.queues().status()},
                         WeatherSection{},
                         ClockSection{},
                         BoardSection{}};
#else
    SetupServer<WifiSection, AccessSection, StationsSection, PlayerSection, ClockSection, BoardSection> m_setup_server_{
        setup_landing,
        WifiSection{m_network_},
        AccessSection{},
        StationsSection{},
        PlayerSection{m_radio_service_.queues().requests(), m_radio_service_.queues().status()},
        ClockSection{},
        BoardSection{}};
    SetupConsole<WifiSection, AccessSection, StationsSection, PlayerSection, ClockSection, BoardSection>
        m_setup_console_{WifiSection{m_network_},
                         AccessSection{},
                         StationsSection{},
                         PlayerSection{m_radio_service_.queues().requests(), m_radio_service_.queues().status()},
                         ClockSection{},
                         BoardSection{}};
#endif
    RequestRoute m_route_{this};
    uint32_t m_next_frame_ms_ = 0;
    uint32_t m_board_settings_revision_ = 0;  // of the BoardSettings applied
};

}  // namespace avela
