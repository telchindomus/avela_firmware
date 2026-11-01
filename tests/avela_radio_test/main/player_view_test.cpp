#include "avela_radio/ui/player_view.hpp"

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <string_view>

#include "avela_ui/view_concept.hpp"

namespace avela {
namespace {

// Keeps what the view handed over last.
struct RecordingSkin {
    std::string m_station;
    std::string m_title;
    bool m_playing = false;
    PlaybackState m_state = PlaybackState::stopped;
};

// The skin the view owns, writing into a RecordingSkin the test reads.
class FakeSkin {
public:
    explicit FakeSkin(RecordingSkin& record) : m_record_(&record) {}

    void set_station(std::string_view name, std::string_view /*genre*/) { m_record_->m_station = name; }
    void set_thumbnail(const Thumbnail& /*thumbnail*/) {}
    void set_stream(std::string_view title, std::string_view /*format*/) { m_record_->m_title = title; }
    void set_playing(bool playing) { m_record_->m_playing = playing; }
    void set_playback_state(PlaybackState state) { m_record_->m_state = state; }
    void set_spectrum(std::span<const uint8_t> /*spectrum*/) {}
    void draw() {}

private:
    RecordingSkin* m_record_;
};

static_assert(PlayerSkinConcept<FakeSkin>);
static_assert(ViewConcept<PlayerView<FakeSkin>>);

TEST_CASE("the skin gets the request and what the radio does about it")
{
    RadioModel model;
    RecordingSkin record;
    PlayerView view{model, FakeSkin{record}};
    model.m_playing = true;
    model.m_playback_state = PlaybackState::connecting;
    view.refresh();
    CHECK(record.m_playing);
    CHECK(record.m_state == PlaybackState::connecting);
    CHECK(record.m_station == model.station().m_name);

    model.m_playback_state = PlaybackState::playing;
    model.m_stream_title.clear().append("Bonobo - Kerala");
    view.refresh();
    CHECK(record.m_state == PlaybackState::playing);
    CHECK(record.m_title == "Bonobo - Kerala");
}

}  // namespace
}  // namespace avela
