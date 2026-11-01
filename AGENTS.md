# avela_firmware: working here

> **Created by humans, extendable by AI.**
>
> We build the architecture; anyone with proper AI tools can extend Avela or adjust it to their own needs. So every
> change has one obvious place (a component = model + service + screens, a view per skin, a section on the setup page),
> the rules are written down (`docs/`), and the checks run by themselves (strict warnings, `tools/check_code.sh`).

Avela is internet radio firmware for ESP32 chips (ESP-IDF, C++20). This file holds the rules for working in the
repository; what the firmware is and why lives in `docs/`, and this file only points there.

## Read first

[The documentation index](docs/README.md) identifies the references for each change; read those before changing
anything.

A component with its own rules has a `components/<component>/AGENTS.md`; read it before changing that component.

## Rules

- **Before every commit:** `idf.py build` without warnings and `tools/check_code.sh` clean (`--fix` reformats).
  Fix a finding; don't silence it (`docs/developer/CODING_STANDARD.md`, Exceptions to the rules).
- **Writing docs** (`docs/`, a component's `README.md` or `docs/`, an `AGENTS.md`): `docs/WRITING_STANDARD.md`, which
  also says where each doc goes (Where the docs live). Follow `docs/AGENTS.md` for the editing workflow,
  including component-local docs. Code comments:
  `docs/developer/CODING_STANDARD.md`, section 9.
- **A USB update writes otadata and the app** (`idf.py flash`, or 0xd000 + 0x10000), which keeps the settings; the
  merged full image at 0x0 erases them, and the app at 0x10000 alone may never start (`docs/developer/BUILD.md`).
- **Ask the person you work for before** changing the architecture's rules, adding a dependency between components, or adding a third-party
  component.

## When you change the code, update the docs

| Change | Update |
|---|---|
| a new component, or a dependency between components | `docs/developer/ARCHITECTURE.md`: Source layout and Dependency order |
| a design rule | `docs/developer/ARCHITECTURE.md`: Design rules |
| the controls (inputs, commands, screen navigation, buttons) | `docs/developer/FEATURES.md`: Controls |
| a coding rule or a check | `docs/developer/CODING_STANDARD.md`, and `docs/developer/BUILD.md` if running it changes |
| the version or release procedure | `docs/developer/RELEASE.md` |
| a writing rule | `docs/WRITING_STANDARD.md` |
| a component's design or behaviour (radio, weather, update, its setup section), or its first doc | its `components/<component>/docs/developer/<component>.md` (and a brief `README.md` with the first doc) |
| the HAL's parts or inputs | `components/avela_hal/docs/developer/avela_hal.md` |
| a board, its pins or its wiring | `components/avela_boards/docs/developer/avela_boards.md` |
| anything a skin sees (a skin concept, a setter, a shared skin piece) | the component's `docs/developer/SKIN.md`, and `docs/developer/SKIN_CHANGES.md` with the version (`tools/check_skin_api.sh` checks it) |
| a file or folder named in a doc (moved, renamed, deleted) | every doc naming it (`tools/check_docs.sh` finds them) |
