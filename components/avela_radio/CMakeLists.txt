idf_component_register(
    SRCS "src/model/station_list.cpp" "src/model/carry_out.cpp" "src/model/player_status.cpp"
         "src/service/simulated_radio_service.cpp" "src/service/streaming_radio_service.cpp"
         "src/service/stream_receiver.cpp" "src/service/mp3_player.cpp" "src/service/radio_storage.cpp"
         "src/setup/station_fields.cpp" "src/setup/stations_section.cpp" "src/setup/player_section.cpp"
         "src/stream/icy_metadata_reader.cpp"
         "src/audio/mp3_decoder.cpp" "src/audio/spectrum_analyzer.cpp" "src/audio/spectrum_bands.cpp"
         "src/audio/volume.cpp"
    INCLUDE_DIRS "include"
    REQUIRES avela_strings avela_messaging avela_ui avela_hal avela_network avela_setup avela_settings freertos esp_http_client
             esp_app_format
             espressif__esp_audio_codec
    PRIV_REQUIRES espressif__esp-dsp esp-tls mbedtls log)
