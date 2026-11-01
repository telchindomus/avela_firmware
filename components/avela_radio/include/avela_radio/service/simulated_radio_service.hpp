// Stands in for the real radio until it exists, and later for UI work without a network: reports a made-up
// stream title and format per station and a spectrum that looks like music, following volume and play/pause.
#pragma once

#include <cstddef>
#include <limits>
#include <random>

#include "avela_radio/service/player_queues.hpp"
#include "avela_radio/service/radio_service_concept.hpp"

namespace avela {

// NOLINTNEXTLINE(cert-msc32-c,cert-msc51-cpp): default-seeded engine on purpose, see m_random_engine_
class SimulatedRadioService {
public:
    void update(RadioModel& model);
    // The queues its clients send requests to and read the status from.
    PlayerQueues& queues() { return m_queues_; }

private:
    // Spectrum, in thousandths (integers only): each band gets a fresh random level every frame, shaped like music
    // (strong mids); louder volume = higher levels; nothing while paused. How the levels move on screen (rising
    // fast, falling slowly) is the skin's.
    static constexpr int per_mille = 1000;
    static constexpr int shape_peak = 450;          // position of the highest bands (0 = lowest, per_mille = highest)
    static constexpr int shape_curvature = 1600;    // how fast levels drop away from the peak
    static constexpr int minimum_randomness = 350;  // a band's level never drops below this share of its shape
    static constexpr int quiet_scale = 250;         // levels at the lowest volume, relative to the highest

    void report_stream(RadioModel& model);
    void report_spectrum(RadioModel& model);

    PlayerQueues m_queues_;
    std::size_t m_reported_station_ = std::numeric_limits<std::size_t>::max();  // none yet
    std::minstd_rand m_random_engine_;  // default seed: the same sequence on every start is fine for a simulation
    std::uniform_int_distribution<int> m_random_per_mille_{0, per_mille};
};

static_assert(RadioServiceConcept<SimulatedRadioService>);

}  // namespace avela
