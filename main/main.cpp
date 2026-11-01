// Entry point: the one Application (application.hpp) in static storage, started and run forever.
#include "application.hpp"

extern "C" void app_main()
{
    static avela::Application application;
    application.init();
    application.run();
}
