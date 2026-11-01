// Settings that survive a restart, as fixed-size records in NVS (the flash's key-value store). No heap: a record is a
// trivially copyable struct stored as one blob, together with its version; a record of another size or version
// reads as missing, so the caller falls back to its defaults (no migrations yet).
// A record type may also be kept in numbered slots (one blob each, keyed Record::key + the number: "station1"), for
// lists such as the stations.
// All static, like SystemClock: there is one NVS per device, and the web server's task writes records while the main
// loop's services read them. revision<Record>() counts the saves since power-on, so a service sees that a record has
// changed by comparing one number per frame. NVS itself is safe from any task; the saves also share one lock, so
// update() can read, change and write a record with no other save in between.
#pragma once

#include <atomic>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <type_traits>

#include "avela_settings/settings_record_concept.hpp"
#include "avela_strings/fixed_string.hpp"

namespace avela {

class Settings {
public:
    // Opens the NVS partition (erases it if it is full or from a newer NVS format). Once, before any load or save.
    static void init();

    // Reads the stored record; false (and `record` untouched) if there is none or it has another size or version.
    template <SettingsRecordConcept Record>
    [[nodiscard]] static bool load(Record& record)
    {
        return load_from(Record::key, record);
    }

    // Stores the record; false if NVS refused (logged).
    template <SettingsRecordConcept Record>
    static bool save(const Record& record)
    {
        const Lock lock;
        return save_to(Record::key, record);
    }

    // Reads the record (a default one if there is none), lets `change` edit it and stores it if `change` returns
    // true, with no other save in between: for a task that changes part of a record another task may save too.
    // False if `change` declined or NVS refused. `change` must not save (save() or update()): the lock isn't
    // recursive, so that would never return.
    template <SettingsRecordConcept Record, std::predicate<Record&> Change>
    static bool update(Change change)
    {
        const Lock lock;
        Record record{};
        if (!load_from(Record::key, record)) {
            record = Record{};
        }
        return change(record) && save_to(Record::key, record);
    }

    // The same for the record in numbered slot `slot` (Record::key is then at most 13 characters).
    template <SettingsRecordConcept Record>
    [[nodiscard]] static bool load(Record& record, std::size_t slot)
    {
        return load_from(slot_key(Record::key, slot).view(), record);
    }
    template <SettingsRecordConcept Record>
    static bool save(const Record& record, std::size_t slot)
    {
        const Lock lock;
        return save_to(slot_key(Record::key, slot).view(), record);
    }

    // Number of saves of this record (in any slot) since power-on.
    template <SettingsRecordConcept Record>
    [[nodiscard]] static uint32_t revision()
    {
        return m_revision_<Record>.load();
    }

private:
    static constexpr std::size_t key_capacity = 15 + 1;  // NVS keys: up to 15 characters

    // Holds the saves' lock while it lives (settings.cpp owns the mutex).
    class Lock {
    public:
        Lock();
        ~Lock();
        Lock(const Lock&) = delete;
        Lock& operator=(const Lock&) = delete;
        Lock(Lock&&) = delete;
        Lock& operator=(Lock&&) = delete;
    };

    template <typename Record>
    struct Stored {
        uint16_t m_version = 0;
        Record m_record{};
    };

    template <SettingsRecordConcept Record>
    static bool load_from(std::string_view key, Record& record)
    {
        Stored<Record> stored{};
        if (!read_blob(key, std::as_writable_bytes(std::span{&stored, 1})) || stored.m_version != Record::version) {
            return false;
        }
        record = stored.m_record;
        return true;
    }

    template <SettingsRecordConcept Record>
    static bool save_to(std::string_view key, const Record& record)
    {
        const Stored<Record> stored{.m_version = Record::version, .m_record = record};
        if (!write_blob(key, std::as_bytes(std::span{&stored, 1}))) {
            return false;
        }
        m_revision_<Record>.fetch_add(1);
        return true;
    }

    static FixedString<key_capacity> slot_key(std::string_view key, std::size_t slot)
    {
        FixedString<key_capacity> text;
        text.append(key).append_number(slot);
        return text;
    }

    // false unless a blob of exactly bytes.size() is stored under the key
    static bool read_blob(std::string_view key, std::span<std::byte> bytes);
    static bool write_blob(std::string_view key, std::span<const std::byte> bytes);

    template <typename Record>
    static inline std::atomic<uint32_t> m_revision_{0};
};

}  // namespace avela
