// What a settings record is (each component owns its records: WifiCredentials, StationRecord, ...): plain data of a
// fixed size, with its NVS key and a version to raise whenever the struct changes (Settings refuses a stored record
// of another version or size).
#pragma once

#include <concepts>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace avela {

template <typename T>
concept SettingsRecordConcept = std::is_trivially_copyable_v<T> && std::default_initializable<T> && requires {
    { T::key } -> std::convertible_to<std::string_view>;  // NVS key, at most 15 characters
    { T::version } -> std::convertible_to<uint16_t>;      // raise it when the struct changes
};

}  // namespace avela
