# `avela_settings`: persistent records

`Settings` stores fixed-size component records as NVS blobs. It is an all-static class because the device has one
settings store. Each component owns its record type, key, version, and defaults.

## Record format and lifetime

A record satisfies `SettingsRecordConcept` and is trivially copyable. The stored blob includes its version. A load
with a different size or version returns false, leaving the destination unchanged; the caller chooses defaults.
There is no record migration mechanism.

`Settings::init()` opens NVS before any load or save. Its initialization may erase NVS if the partition is full or
has a newer NVS format. This recovery behavior differs from an ordinary record failing to load.

## Reading and saving

| Operation | Contract |
|---|---|
| `load(record)` | Read a matching record; false if missing or incompatible |
| `save(record)` | Write one record; false if NVS refuses |
| `update<Record>(change)` | Read the current record or its default, apply change under the save lock, and save if change returns true |
| `revision<Record>()` | Count successful saves since power-on, across slots for that type |

Services compare a record's revision to notice configuration changes. A revision is a runtime change counter, not
the stored record-format version.

## Atomic partial updates

Saves and updates share one static mutex. Use `update()` when changing part of a record that another task may save,
such as Weather's coordinates and user-selected units. The callback sees the currently stored record rather than a
snapshot prepared earlier.

The callback must not call `save()` or `update()`: the lock is not recursive, so that would deadlock. A false result
can mean the callback declined to change the record or NVS refused the save; callers must interpret that distinction
where needed.

## Numbered records

`load(record, slot)` and `save(record, slot)` use the record key plus the slot number. `Station` records use this
mechanism. NVS keys hold up to 15 characters, and the numbered-record key prefix is limited to 13 characters.

The [radio reference](../../../avela_radio/docs/developer/avela_radio.md#stations-and-saved-playback) owns station
slots and playback memory. The
[Weather reference](../../../avela_weather/docs/developer/avela_weather.md#settings-changed-during-a-request)
explains its concurrent-save merge.
