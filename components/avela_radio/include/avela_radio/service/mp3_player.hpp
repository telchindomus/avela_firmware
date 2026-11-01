// The player task: takes the stream receiver's chunks of the current session, decodes them (Mp3Decoder), sets their
// volume and plays them (I2sOutput, whose pace is the sample rate's), and measures the spectrum of every frame
// (SpectrumAnalyzer). A new session starts afresh: the decoder forgets the old stream, and the player waits until
// half the queue is filled before the first sound, so that the network's hiccups don't stop it. When no chunk has come
// for silence_after, the output stops (the amplifier shuts down) until the next frame.
//
// Between the tasks: m_latest_playback_, this task -> main loop, a latest-value queue (one item, a newer one
// overwrites it): whether sound comes out, the format and the spectrum (PlaybackReport). The volume comes the other way
// as one number, m_volume_gain_ (atomic); the dropout count goes out as one, m_dropouts_ (atomic).
#pragma once

#include <array>
#include <atomic>
#include <cstdint>

#include "avela_hal/audio/i2s_output.hpp"
#include "avela_radio/audio/mp3_decoder.hpp"
#include "avela_radio/audio/spectrum_analyzer.hpp"
#include "avela_radio/model/radio_model.hpp"
#include "avela_radio/service/playback_report.hpp"
#include "avela_radio/service/stream_receiver.hpp"
#include "avela_strings/fixed_string.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

namespace avela {

class Mp3Player {
public:
    // output: where the sound goes (the board's amplifier); receiver: where the chunks come from
    Mp3Player(I2sOutput& output, const StreamReceiver& receiver) : m_output_(output), m_receiver_(receiver) {}
    Mp3Player(const Mp3Player&) = delete;
    Mp3Player& operator=(const Mp3Player&) = delete;
    Mp3Player(Mp3Player&&) = delete;
    Mp3Player& operator=(Mp3Player&&) = delete;
    ~Mp3Player() = default;

    // Opens the decoder and the analyzer, creates the queue and starts the task. After the receiver's init().
    void init();

    // Main loop: RadioModel::volume_min..volume_max.
    void set_volume(int volume);
    // Main loop: the newest report, if one came since the last call.
    bool take_report(PlaybackReport& report);

    // For the diagnostics log: how often the sound stopped while the station was still wanted (the queue ran dry
    // for silence_after), and the fewest bytes the task's stack has had left.
    [[nodiscard]] uint32_t dropouts() const { return m_dropouts_.load(); }
    [[nodiscard]] uint32_t stack_left() const;

    static constexpr const char* task_name = "player";
    static constexpr uint32_t stack_bytes = 4096;  // measured on the board: 1.3 KB used, the rest for logging

private:
    static constexpr TickType_t silence_after = pdMS_TO_TICKS(200);  // no chunk this long: no sound

    static void run(void* self);
    [[noreturn]] void run();
    // A new session: the decoder starts afresh, and the queue fills up first.
    void start_session(uint32_t session);
    // Decodes the chunk and plays its frames; a chunk may end in the middle of a frame (the decoder keeps that part).
    void play_chunk();
    void play_frame(const Mp3Decoder::Frame& frame);
    void publish();

    I2sOutput& m_output_;
    const StreamReceiver& m_receiver_;
    std::atomic<int32_t> m_volume_gain_{0};  // silent until the first set_volume()
    std::atomic<uint32_t> m_dropouts_{0};

    // the task's state
    Mp3Decoder m_decoder_;
    SpectrumAnalyzer m_analyzer_;
    EncodedAudioChunk m_chunk_;
    PlaybackReport m_report_;
    uint32_t m_kilobits_per_second_ = 0;  // the bitrate in m_report_.m_format

    StaticQueue_t m_latest_playback_storage_{};
    std::array<uint8_t, sizeof(PlaybackReport)> m_latest_playback_items_{};
    QueueHandle_t m_latest_playback_ = nullptr;

    StaticTask_t m_task_{};
    TaskHandle_t m_task_handle_ = nullptr;
    std::array<StackType_t, stack_bytes / sizeof(StackType_t)> m_stack_{};
};

}  // namespace avela
