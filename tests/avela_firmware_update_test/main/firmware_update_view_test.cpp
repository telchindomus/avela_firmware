#include "avela_firmware_update/ui/firmware_update_view.hpp"

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <string_view>

#include "avela_ui/view_concept.hpp"

namespace avela {
namespace {

// Keeps what the view handed over last.
struct RecordingSkin {
    std::string m_running_version;
    std::string m_page_by_host_name;
    std::string m_page_by_address;
    FirmwareUpdateState m_state = FirmwareUpdateState::waiting;
    int m_percent = -1;
    std::string m_failure;
    int m_draws = 0;
};

// The skin the view owns, writing into a RecordingSkin the test reads.
class FakeSkin {
public:
    explicit FakeSkin(RecordingSkin& record) : m_record_(&record) {}

    void set_running_version(std::string_view version) { m_record_->m_running_version = version; }
    void set_update_page(std::string_view by_host_name, std::string_view by_address)
    {
        m_record_->m_page_by_host_name = by_host_name;
        m_record_->m_page_by_address = by_address;
    }
    void set_progress(FirmwareUpdateState state, int percent, std::string_view failure)
    {
        m_record_->m_state = state;
        m_record_->m_percent = percent;
        m_record_->m_failure = failure;
    }
    void draw() { ++m_record_->m_draws; }

private:
    RecordingSkin* m_record_;
};

static_assert(FirmwareUpdateSkinConcept<FakeSkin>);
static_assert(ViewConcept<FirmwareUpdateView<FakeSkin>>);

NetworkModel& network_model()
{
    static NetworkModel network;
    return network;
}

TEST_CASE("the skin gets the version, the page by host name and the progress")
{
    FirmwareUpdateModel model;
    model.m_running_version = "0.1.0";
    model.m_state = FirmwareUpdateState::receiving;
    model.m_total_bytes = 1000;
    model.m_received_bytes = 250;
    NetworkModel& network = network_model();
    network.m_host_name.clear().append("kitchen-radio");
    RecordingSkin record;
    FirmwareUpdateView<FakeSkin> view{model, network, FakeSkin{record}};
    view.draw();
    CHECK(record.m_draws == 1);
    CHECK(record.m_running_version == "0.1.0");
    CHECK(record.m_page_by_host_name == "http://kitchen-radio.local/update");
    CHECK(record.m_state == FirmwareUpdateState::receiving);
    CHECK(record.m_percent == 25);
}

TEST_CASE("the page by IP address only while connected")
{
    const FirmwareUpdateModel model;
    NetworkModel& network = network_model();
    network.m_address.clear();
    network.m_address.append("192.168.1.23");
    RecordingSkin record;
    FirmwareUpdateView<FakeSkin> view{model, network, FakeSkin{record}};

    network.m_status = WifiStatus::connecting;
    view.refresh();
    CHECK(record.m_page_by_address.empty());

    network.m_status = WifiStatus::connected;
    view.refresh();
    CHECK(record.m_page_by_address == "http://192.168.1.23/update");
}

TEST_CASE("a failure reaches the skin with its reason")
{
    FirmwareUpdateModel model;
    model.m_state = FirmwareUpdateState::failed;
    model.m_failure.append("This isn't an Avela firmware.");
    RecordingSkin record;
    FirmwareUpdateView<FakeSkin> view{model, network_model(), FakeSkin{record}};
    view.refresh();
    CHECK(record.m_state == FirmwareUpdateState::failed);
    CHECK(record.m_failure == "This isn't an Avela firmware.");
}

TEST_CASE("percent_received is 0 before an upload starts and whole at the end")
{
    FirmwareUpdateModel model;
    CHECK(model.percent_received() == 0);
    model.m_total_bytes = 1'339'264;
    model.m_received_bytes = 1'339'264;
    CHECK(model.percent_received() == 100);
    model.m_received_bytes = 1'339'263;
    CHECK(model.percent_received() == 99);
}

}  // namespace
}  // namespace avela
