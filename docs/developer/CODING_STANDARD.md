# Avela firmware — C++ coding standard (ESP32, ESP-IDF)

This standard applies to our code in `main/`, `components/avela_*`, and the test projects. Third-party code is
outside its scope; ESP-IDF, FreeRTOS, LovyanGFX, and managed-component headers are treated as system headers.

Rules marked **[tool]** are enforced by the build or `tools/check_code.sh`. Other rules are checked in review. The
gate combines strict compiler warnings, clang-tidy, and clang-format. It does not use MISRA: that standard requires
paid rules/checkers, and MISRA C++:2023 targets C++17 while this firmware uses C++20 concepts.

## 1. Toolchain and gate

Use ESP-IDF v5.5.5 and its GCC toolchain. The language baseline is C++20; newer features depend on the compiler's
support. Used facilities include concepts, designated initializers, `std::span`, and the `std::chrono` calendar.

Exceptions and RTTI are disabled. Do not use `throw`, `try`, `dynamic_cast`, or `typeid`.

**[tool] Strict warnings are errors.** `cmake/strict_warnings.cmake` applies the following to main and each
`avela_*` component, including sources compiled by the test projects:

```text
-Wall -Wextra -Werror -Wconversion -Wsign-conversion -Wdouble-promotion
-Wshadow -Wold-style-cast -Wcast-align -Wcast-qual -Wnon-virtual-dtor
-Woverloaded-virtual -Wnull-dereference -Wformat=2 -Wimplicit-fallthrough
-Wmisleading-indentation
```

GCC also enables `-Wuseless-cast`, `-Wduplicated-cond`, `-Wduplicated-branches`, and `-Wlogical-op`.

**[tool] clang-format defines formatting.** Apply `.clang-format` to every `.cpp` and `.hpp` under main, components,
and tests. It uses LLVM style, 120 columns, four-space indentation, function braces on their own line, mandatory
braces, left-aligned pointer declarations such as `int* pointer`, and no namespace indentation. The tool version is
clang-format 19 from esp-clang.

**[tool] clang-tidy findings are errors.** `.clang-tidy` enables C++ Core Guidelines, SEI CERT C++, bugprone,
clang-analyzer, misc, modernize, performance, portability, and readability checks. Disabled checks have reasons in
that file. Test-specific exceptions are documented in `tests/.clang-tidy` and cover Catch2 macros and literal test
data.

**[tool] Test platform-free code in separate projects.** `tests/<component>_test/` is an ESP-IDF project for
`IDF_TARGET=linux`, using `espressif/catch2` and the FreeRTOS mock. Its main `CMakeLists.txt` compiles the tested
sources directly and names their include folders, using a variable for each component path. It loads no production
`avela_*` component. Production component CMake must not branch for host tests. A Kconfig value needed by a test is
supplied with `target_compile_definitions`.

Use an include-folder variable for each tested component, for example
`set(avela_hal "${firmware}/components/avela_hal")`.

Before every commit, `idf.py build` must finish without warnings and `tools/check_code.sh` must report clean. The
gate also rejects a production `.cpp` absent from the applicable source lists.
[Build and flash](BUILD.md#checks-before-a-commit) gives the commands, prerequisites, host-test tooling, and
skin/build coverage.

## 2. Exceptions to the rules

Fix findings rather than silencing them. A deliberate local exception must name its check and give a reason at the
affected code:

```cpp
// NOLINT(check-name): reason
// NOLINTNEXTLINE(check-name): reason
```

Never use a bare NOLINT. A project-wide exception belongs in the reasoned list in `.clang-tidy` and requires
agreement first.

## 3. Memory: no heap in our code

Never allocate in our code. Do not use `new`, `delete`, `malloc`, `std::vector`, `std::string`, `std::function`,
`std::map`, or another allocating facility. Storage sizes are fixed at compile time.

Use `FixedString<N>` for constructed text, rather than `std::string` or `snprintf`. Derive capacities from
`capacity_for("23:59")`, `number_capacity<int>`, or an owning named limit such as `avela_skin::LabelText`. The
[string reference](../../components/avela_strings/docs/developer/avela_strings.md) defines byte capacity,
termination, and truncation.

Use `std::array` for owned arrays and `std::span` for views; do not use C arrays. Access embedded assets through
their generated functions, such as `assets::<name>()`.

Long-lived application objects belong to the function-local static `Application` in `app_main`. Do not put them on
the main task's stack or expose them as namespace globals. Pass references to their users. Display-buffer storage is
defined by the [UI renderer](../../components/avela_ui/docs/developer/avela_ui.md#drawing).

Library-owned allocation has a separate lifetime from our fixed application storage. Display font tables and PCNT
handles are allocated during initialization. HTTP clients, TLS, Wi-Fi, and decoder internals also own runtime
memory; clients and request resources must be released according to their library APIs. The relevant service
reference describes those lifetimes. This does not permit allocating application containers or per-frame drawing
buffers. Never use `createSprite` or a framebuffer.

### Stack sizing and memory checks

Declare each owned task's `task_name` and `stack_bytes` as public constants; use the corresponding ESP-IDF setting
for framework-owned tasks. `Application` lists task sizes for the system information service's measurements.

Reduce a stack only from measured usage after exercising every relevant path: screens, setup page, tuner, firmware
update, and network operations. Check the lowest free heap after changes that add RAM, including the Weather TLS
handshake and HTTPS radio connection. Do not infer safe sizes from a single idle screen.

The [Memory](../../components/avela_system_info/docs/developer/avela_system_info.md#memory-and-stack-diagnostics)
explains stack high-water marks, heap measurements, and log fields. It owns the diagnostic examples; this section
owns the sizing requirement.

## 4. Polymorphism: compile time only

Do not use virtual functions or inheritance for polymorphism. Interfaces are concepts, including `ViewConcept`,
`ControllerConcept`, `ScreenConcept`, `SetupSectionConcept`, service concepts, `InputConcept`,
`BoardHardwareConcept`, and `AvelaBoardConcept`. Every implementing header must end with the applicable
`static_assert(Concept<Type>);`.

A concept describes the names and types an implementation must provide. A type does not inherit or declare
membership in it; the compiler checks conformance. The explicit `static_assert` makes that conformance visible. A
concept can refine another, such as `AvelaBoardConcept = BoardHardwareConcept<T> && requires ...`, and participates
in constraint ordering.

Each concept holds one concern. Put common requirements in the base concept and user-specific requirements in a
named refinement. A board without Avela's controls can still satisfy `BoardHardwareConcept`. Keep each concept in
its own header.

An empty non-copyable base can identify a sender independently of its concrete type. Button and Encoder use this
pattern; behavior stays in `InputConcept`. Never add virtual functions to an identity base.

Use `std::variant` for runtime choice between concrete types. `Ui` constructs its active screen in place. Input
events, commands, and requests also use variants, but have separate roles: an event reports an input action and
sender, a command expresses application input for a screen, and a request asks a service to act. Do not introduce
runtime subscriber lists or callbacks to dispatch those variants.

The application chooses service implementations and wires models/services by name. Screens are built through its
`Ui` switch and `UiModels`. Do not add self-registration or a compile-time component list. Keep optional-component
integration explicit and follow [Skin architecture](SKIN_ARCHITECTURE.md#rules) for switches and dependencies.

## 5. Structure

Components live in `components/avela_<name>/`, with public headers under `include/avela_<name>/` and implementations
under `src/`. Includes use `"avela_<name>/<file>.hpp"`. The [architecture](ARCHITECTURE.md#source-layout) and
component READMEs explain ownership; component-specific facts stay in their own references.

Dependencies must follow the architecture's downward direction. Components with screens must not depend on one
another, except to read the lower-level network model. Only main knows all screens and the selected board.
Components must not depend on a skin. Do not create a general core component: name a shared component after its
concern, and keep a helper with its sole user until a second user needs it.

### Component parts

A feature component separates `model/`, `service/`, and `ui/`; `setup/` holds its settings section when needed.
Create a folder with its first implementation.

Shared data models contain state, including requested and reported fields. Views and controllers receive them const.
Services carry out queued requests and update reports on the main loop. Requested fields are changed by the owning
request-handling path, such as `carry_out(PlayerRequest, RadioModel&)`.

A screen has a view, stateless controller, and a page model only when it needs screen-local state. The controller
alone changes that page model. The view passes model values to the skin; it must not draw them itself or keep a
second model state.

Requests are plain trivially copyable data in `avela_messaging`, not in a receiving feature component. The owning
service provides its request queue and publishes status for clients that run on other tasks. Configuration sections
save `Settings` records instead. A controller or frontend must not mutate another component's data model directly.

### Cross-task access

Radio worker tasks exchange requests, audio items, and reports through statically allocated FreeRTOS queues. Encoded
audio uses an item queue; latest-value worker requests and reports use one-item overwritten queues. A scalar may use
`std::atomic`.

Other implemented service handoffs use atomics or a report/failure buffer protected by a static FreeRTOS mutex.
Weather copies its published report into the model in `update()`; firmware update copies HTTP-task state into its
model. Keep such synchronization inside the owning service, and leave the model owned by the main loop. Use
`xSemaphoreCreateMutexStatic` and `xTaskCreateStatic` for application-owned mutexes and task storage.

Other tasks may access only their explicit synchronized interfaces, `Settings`, and immutable values whose lifetimes
cover the access. A whole-value model may carry a revision so readers detect a change without caching each field. Do
not rely on a plain data model becoming safe merely because one field is const at a call site.

Readers poll models at their own pace. Event queues are for events; service requests use the explicit request queues
described above. A reader is passed the models it needs through its constructor, with `UiModels` supplying screen
references.

A device-wide resource reached by a C callback may use an all-static class, as `SystemClock` does. Prefer a class
over a namespace where it is supplied as a template parameter for tests.

### Skins and component lifecycle

Skin-facing headers and their value types must remain free of ESP-IDF and FreeRTOS runtime headers; LovyanGFX is
allowed. This includes the page-skin concepts, `Station`, `Thumbnail`, `WifiStatus`, `LocalTime`, model ranges/text
limits, shared UI/string types, and HAL screen-size/input types. Split a value into its own header when adjacent
implementation is platform-specific. This boundary supports PC rendering by the skin toolbox.

A component with a skin-facing interface must document it in its own `docs/developer/SKIN.md`. Update that reference
and SKIN_CHANGES in the same commit as a skin-facing change. If skins must add or change something, advance the
supported API lower bound as defined in
[Versions and skin compatibility](SKIN_ARCHITECTURE.md#versions-and-skin-compatibility).

Do not remove a `REQUIRES` entry to disable a component. Its Kconfig switch is the only optional-component switch;
source and alias guards follow it, while `REQUIRES` stays unconditional. Retire a component in one commit, removing
its folder, main integration, and official-skin pages, and documenting what outside skins may delete.

## 6. No magic numbers

**[tool]** `readability-magic-numbers` is enabled with no exemptions. Give timings, limits, and capacities named
constants in their owner.

Hardware pins, bus clocks, panel geometry, and backlight configuration belong in board descriptions as constexpr
designated initializers of HAL configuration structs. Layout coordinates, sizes, and gaps belong to the skin's
layout structs and per-screen designated initializers; never put literal layout coordinates in drawing code.

Use palette roles such as `palette::accent` and `palette::text_secondary`, and font roles such as `fonts.m_title`
and `fonts.m_caption`. Drawing code must not choose colors or fonts by hue, point size, or weight. Per-screen role
definitions and generated assets are explained in [Writing a skin](SKINS.md#assets).

Adding a screen size requires its board Kconfig symbols, a `SKINS` entry in `tools/generate_assets.py`, and
corresponding layout folders in each applicable skin. Use `std::chrono` calendar facilities for dates, rather than
hand-written calendar arithmetic.

## 7. Initialization and types

**[tool]** Use designated initializers for aggregates. Give every field a default member initializer, such as
`int m_volume = default_volume;` or `std::array<float, N> m_levels{};`.

Do not rely on implicit int/float or signed/unsigned conversions. Use explicit `static_cast`; C-style casts are
errors. Use const wherever possible, constexpr for compile-time values, and nodiscard for getters or results the
caller must inspect.

Plain data with helper methods may be a struct with public fields. A class with invariants keeps its fields private.
Apply the member naming rules below to both.

## 8. Naming

**[tool]** Non-static data members begin with `m_`. Private and protected members also end in `_`, such as
`m_selected_`; aggregate fields use names such as `Rect::m_x`. Static constexpr constants are not fields and use
plain names such as `volume_max` or `Board::screen`. Review static non-constant members by hand; they use the same
member prefix/suffix convention.

Use full words rather than abbreviations: `bounding_box` and `station_index` rather than `bb` and `idx`. Short names
are allowed where they are natural, such as x, y, and dx in drawing code.

Model, view, controller, skin, screen, and request retain the meanings in
[Design rules](ARCHITECTURE.md#design-rules). A page's types share its name, such as StationsModel, StationsView,
StationsController, and `StationsSkin`. Never call a skin a view.

Requests end in Request and commands end in Command. Input events use distinct nouns such as `Click` and `Turn`
without a suffix; do not call them gestures. Concepts end in Concept and have their own header, such as
PlayerSkinConcept in `player_skin_concept.hpp`. Implementing types keep the plain name.

Types and concepts use PascalCase. Functions, variables, constants, files, and folders use snake_case. Everything
belongs in namespace avela, with component/skin sub-namespaces where appropriate. File-local helpers belong in an
anonymous namespace.

## 9. Files and comments

Headers use pragma once. Keep one type per file, named after the class, struct, enum, or concept in snake_case.
Nested types stay with their owner; a type that exists only as part of another is nested there. Functions and
constants stay with the type they serve. A function-only header is named for its concern, such as
`controller_calls.hpp`. Group files by device or topic and mirror that layout in src.

One-line forwarders and accessors may be defined inside the class. Put functions that do work in the cpp file.

In cpp files, precede every function definition, including free functions, anonymous-namespace helpers, and
app_main, with a separator: two slashes followed by dashes through column 100. Leave one empty line before and after
it. Function comments follow the separator; NOLINTNEXTLINE stays directly above the signature.

```cpp
}

//--------------------------------------------------------------------------------------------------

// Bar colours run from spectrum_low over spectrum_middle to spectrum_high, left to right.
uint32_t spectrum_color(int bar, int count)
{
```

Start each code file with a short explanation of its purpose and the constraints that are not obvious from reading
it. `CMakeLists.txt` is the exception: comments explain CMake-specific reasons, such as unconditional dependencies
or top-level skin linking, rather than describing the component.

Comments state what the code does and why; they must not contain development history, dates, decision quotes, shop
names, private paths, or references to private design material. Follow
[Public by default](../WRITING_STANDARD.md#public-by-default). Explain units and ranges where a type alone does not
convey them.

Generated font and logo assets are committed; the firmware build uses them directly. Asset generation is a separate
developer operation, described in [Writing a skin](SKINS.md#assets).

Never commit home-network credentials. They are supplied through the setup frontends and stored in `Settings`. The
device's setup-network configuration is a separate concern, described in the
[network reference](../../components/avela_network/docs/developer/avela_network.md).
