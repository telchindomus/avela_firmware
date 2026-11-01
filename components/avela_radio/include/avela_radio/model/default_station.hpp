// The stations a radio starts with: the radio stores them in the station slots 1.. once, when no slot holds a
// station; from then on the stations are the user's (the setup page's Stations section). MP3 streams at 128 kbit/s.
#pragma once

#include <array>

namespace avela {

struct DefaultStation {
    const char* m_name;
    const char* m_genre;
    const char* m_url;  // its MP3 stream (http:// or https://)
};

inline constexpr std::array default_stations{
    DefaultStation{
        .m_name = "SomaFM Lush", .m_genre = "Chillout / Vocals", .m_url = "http://ice2.somafm.com/lush-128-mp3"},
    DefaultStation{
        .m_name = "Radio Paradise", .m_genre = "Eclectic / Rock", .m_url = "http://stream.radioparadise.com/mp3-128"},
    DefaultStation{.m_name = "FIP Jazz", .m_genre = "Jazz", .m_url = "http://icecast.radiofrance.fr/fipjazz-midfi.mp3"},
    DefaultStation{
        .m_name = "Radio Swiss Jazz", .m_genre = "Jazz / Soul", .m_url = "http://stream.srg-ssr.ch/m/rsj/mp3_128"},
    DefaultStation{.m_name = "Groove Salad",
                   .m_genre = "Ambient / Downtempo",
                   .m_url = "http://ice2.somafm.com/groovesalad-128-mp3"},
    // Lush again, over HTTPS: what a TLS station costs in RAM (the stream's connection log)
    DefaultStation{
        .m_name = "Lush HTTPS", .m_genre = "Chillout / Vocals", .m_url = "https://ice2.somafm.com/lush-128-mp3"},
};

}  // namespace avela
