// What a settings section provides: one per component that has settings, shown by two front ends, the web setup page
// (SetupServer: write_form, then apply) and the serial console (SetupConsole: fields and write_values, then apply with
// the name=value pairs typed). Both go through the same apply(), so the same rules and messages hold for both; a field
// that isn't given keeps its saved value (the page sends every field, the console maybe only one). The
// app builds its sections (main/application.hpp) and hands the same list to both, which show them in that order.
// Sections run on the web server's task, not the main loop's: they may only touch what is safe from any task (Settings,
// and model fields that carry their own lock); they get those models through their constructor. The serial console runs
// them on its own task, under the same rule.
#pragma once

#include <concepts>
#include <span>
#include <string_view>

#include "avela_setup/apply_result.hpp"
#include "avela_setup/form_fields.hpp"
#include "avela_setup/html_writer.hpp"
#include "avela_setup/setup_field.hpp"
#include "avela_setup/value_writer.hpp"

namespace avela {

template <typename T>
concept SetupSectionConcept = requires(T& section, HtmlWriter& writer, const FormFields& fields, ValueWriter& values) {
    { T::id } -> std::convertible_to<std::string_view>;      // form id, letters and '_' only
    { T::title } -> std::convertible_to<std::string_view>;   // section heading
    { section.write_form(writer) } -> std::same_as<void>;    // the form's fields (the page adds form, id and button)
    { section.apply(fields) } -> std::same_as<ApplyResult>;  // validate and save a submitted form
    { std::span<const SetupField>{T::fields} };              // the fields apply() takes, for the console's help
    { section.write_values(values) } -> std::same_as<void>;  // every field's saved value (empty if unset), and notes
};

}  // namespace avela
