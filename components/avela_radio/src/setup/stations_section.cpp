#include "avela_radio/setup/stations_section.hpp"

#include <bitset>

#include "avela_radio/model/station_record.hpp"
#include "avela_settings/settings.hpp"
#include "avela_setup/setup_server.hpp"

namespace avela {
namespace {

using Slots = std::bitset<StationRecord::slot_count + 1>;  // by slot number; bit 0 unused
using Message = FixedString<ApplyResult::message_capacity>;

// A station's form as the browser sends it, every byte typed encoded as %XX, must fit the setup page's body.
constexpr std::size_t encoded_bytes_per_byte = 3;
static_assert(capacity_for("section=stations&16.name=&16.genre=&16.url=") +
                      encoded_bytes_per_byte *
                          ((StationRecord::name_capacity - 1) + (StationRecord::genre_capacity - 1) +
                           (StationRecord::url_capacity - 1)) <=
                  setup_page::form_capacity,
              "a station's longest form fits the setup page's form buffer");

StationRecord stored(std::size_t slot)
{
    StationRecord record;
    if (!Settings::load(record, slot)) {
        record = StationRecord{};
    }
    return record;
}

// The given fields' slots.
Slots slots_given(const FormFields& submitted)
{
    Slots slots;
    for (std::size_t index = 0; index < submitted.size(); ++index) {
        if (const std::optional<station_fields::Field> field = station_fields::parse(submitted.name(index))) {
            slots.set(field->m_slot);
        }
    }
    return slots;
}

// A slot's values after the change: the given ones, else the saved ones (skins into `submitted` or `saved`).
struct StationValues {
    std::string_view m_name;
    std::string_view m_genre;
    std::string_view m_url;  // empty: the slot is freed
};

StationValues values_after(std::size_t slot, const FormFields& submitted, const StationRecord& saved)
{
    using station_fields::Part;
    return StationValues{
        .m_name = submitted.find(station_fields::name(slot, Part::name)).value_or(saved.m_name.view()),
        .m_genre = submitted.find(station_fields::name(slot, Part::genre)).value_or(saved.m_genre.view()),
        .m_url = submitted.find(station_fields::name(slot, Part::url)).value_or(saved.m_url.view()),
    };
}

StationRecord record_of(const StationValues& values)
{
    StationRecord record;
    if (!values.m_url.empty()) {
        record.m_name.append(values.m_name);
        record.m_genre.append(values.m_genre);
        record.m_url.append(values.m_url);
    }
    return record;
}

// Why the slot's new values can't be saved, or nothing.
std::optional<ApplyResult> invalid(std::size_t slot, const StationValues& values)
{
    using station_fields::Part;
    const auto [name, genre, url] = values;
    if (url.empty()) {
        return std::nullopt;
    }
    if (!url.starts_with("http://") && !url.starts_with("https://")) {
        return ApplyResult::invalid(station_fields::name(slot, Part::url),
                                    "A stream address starts with http:// or https://.");
    }
    if (url.size() > StationRecord::url_capacity - 1) {
        return ApplyResult::invalid(station_fields::name(slot, Part::url),
                                    "A stream address may have up to 200 characters.");
    }
    if (name.empty()) {
        return ApplyResult::invalid(station_fields::name(slot, Part::name), "A station needs a name.");
    }
    if (name.size() > StationRecord::name_capacity - 1) {
        return ApplyResult::invalid(station_fields::name(slot, Part::name), "A name may have up to 32 characters.");
    }
    if (genre.size() > StationRecord::genre_capacity - 1) {
        return ApplyResult::invalid(station_fields::name(slot, Part::genre), "A genre may have up to 32 characters.");
    }
    return std::nullopt;
}

void write_input(HtmlWriter& writer, std::size_t slot, station_fields::Part part, std::string_view label,
                 std::string_view value, std::string_view placeholder)
{
    const std::string_view name = station_fields::name(slot, part);
    writer.raw(R"(<label for=")").text(name).raw(R"(">)").text(label).raw("</label>");
    writer.raw(R"(<input id=")").text(name).raw(R"(" name=")").text(name).raw(R"(" placeholder=")").text(placeholder);
    writer.raw(R"(" value=")").text(value).raw(R"(">)");
}

}  // namespace

//--------------------------------------------------------------------------------------------------

// The used slots and the first free one (to add a station), each its own form.
void StationsSection::write_form(HtmlWriter& writer)
{
    bool free_slot_shown = false;
    for (std::size_t slot = 1; slot <= StationRecord::slot_count; ++slot) {
        const StationRecord record = stored(slot);
        const bool used = !record.m_url.empty();
        if (!used && free_slot_shown) {
            continue;
        }
        free_slot_shown = free_slot_shown || !used;
        setup_page::write_form_start(writer, id);
        FixedString<capacity_for("Station 16")> heading;
        heading.append(used ? "Station " : "New station in slot ").append_number(slot);
        writer.raw("<h3>").text(heading.view()).raw("</h3>");
        write_input(writer, slot, station_fields::Part::name, "Name", record.m_name.view(), "e.g. Radio Paradise");
        write_input(writer, slot, station_fields::Part::genre, "Genre", record.m_genre.view(), "e.g. Eclectic");
        write_input(writer, slot, station_fields::Part::url, "Stream address", record.m_url.view(),
                    "http://... (MP3); empty: remove the station");
        setup_page::write_form_end(writer);
    }
}

//--------------------------------------------------------------------------------------------------

void StationsSection::write_values(ValueWriter& values)
{
    std::size_t used = 0;
    for (std::size_t slot = 1; slot <= StationRecord::slot_count; ++slot) {
        const StationRecord record = stored(slot);
        values.value(station_fields::name(slot, station_fields::Part::name), record.m_name.view());
        values.value(station_fields::name(slot, station_fields::Part::genre), record.m_genre.view());
        values.value(station_fields::name(slot, station_fields::Part::url), record.m_url.view());
        used += record.m_url.empty() ? 0U : 1U;
    }
    Message note;
    note.append_number(used).append(" of ").append_number(StationRecord::slot_count).append(" slots hold a station");
    values.note(note.view());
}

//--------------------------------------------------------------------------------------------------

// Checks every changed slot first and saves only if all are fine and a station is left; one record in memory at a
// time (the tasks' stacks are small).
ApplyResult StationsSection::apply(const FormFields& submitted)
{
    const Slots given = slots_given(submitted);
    if (given.none()) {
        return ApplyResult::invalid("", "No station field given (e.g. 3.name, 3.url).");
    }
    std::size_t stations_left = 0;
    for (std::size_t slot = 1; slot <= StationRecord::slot_count; ++slot) {
        if (!given.test(slot)) {
            stations_left += stored(slot).m_url.empty() ? 0U : 1U;
            continue;
        }
        const StationRecord saved = stored(slot);
        const StationValues values = values_after(slot, submitted, saved);
        if (const std::optional<ApplyResult> error = invalid(slot, values)) {
            return *error;
        }
        stations_left += values.m_url.empty() ? 0U : 1U;
    }
    if (stations_left == 0) {
        return ApplyResult::invalid("", "The radio needs at least one station.");
    }

    Message done;
    for (std::size_t slot = 1; slot <= StationRecord::slot_count; ++slot) {
        if (!given.test(slot)) {
            continue;
        }
        const StationRecord saved = stored(slot);
        const StationValues values = values_after(slot, submitted, saved);
        if (!Settings::save(record_of(values), slot)) {
            return ApplyResult::storage_failed();
        }
        done.append(done.empty() ? "" : ", ").append_number(slot).append(values.m_url.empty() ? " (removed)" : "");
    }
    ApplyResult result;
    result.m_saved = true;
    result.m_message.append(given.count() == 1 ? "Saved station " : "Saved stations ").append(done.view()).append(".");
    return result;
}

}  // namespace avela
