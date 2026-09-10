# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Noisefield is a native Linux desktop app (C++20 + JUCE 8) that generates a tone and white
noise, mixes them, and sends the result to an audio output. It is a personal tinnitus-relief
tool. Current state: milestone **M2** (audible MVP). The roadmap, milestones (M1–M4), the
unscheduled/mobile work and the bug list all live in `docs/backlog.md`; the design rationale
is in `docs/plan.md`.

## Commands

```sh
# Configure (first run fetches JUCE + Catch2 from GitHub — needs network)
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DNOISEFIELD_BUILD_TESTS=ON

# Build everything
cmake --build build

# Run the app  (binary path carries the build type)
./build/src/Noisefield_artefacts/RelWithDebInfo/Noisefield

# All tests
ctest --test-dir build --output-on-failure

# A single test (Catch2 test-case name or tag)
./build/tests/noisefield_tests "SineOscillator produces the requested frequency"
./build/tests/noisefield_tests "[dsp]"

# Strict build (adds -Werror -Wconversion on our code only)
cmake -B build -DNOISEFIELD_WERROR=ON && cmake --build build

# Plugin (VST3 / LV2 / CLAP). CLAP fetches clap-juce-extensions; -DNOISEFIELD_PLUGIN_CLAP=OFF skips it.
cmake -B build -DNOISEFIELD_BUILD_PLUGIN=ON && cmake --build build
# artefacts under build/src/plugin/NoisefieldPlugin_artefacts/<config>/{VST3,LV2,CLAP}/

# Formatting  (CI and local both pin clang-format 23.1.0: `pip install "clang-format==23.1.0"`)
scripts/check-format.sh          # check
scripts/check-format.sh --fix    # rewrite

# clang-tidy (uses build/compile_commands.json; HeaderFilterRegex limits it to src/)
run-clang-tidy -p build src/
```

There is no display in most sandboxes; the app needs X11/Wayland to open its window, so use
`xvfb-run` for a smoke test and rely on `ctest` for logic. Actual sound output and the
10-minute no-xrun listening test can only be verified on a real machine.

Dependencies and per-distro package lists: `docs/building.md`.

## Build architecture (important — do not casually restructure)

The CMake target layout keeps the JUCE modules compiled **once per final artefact** and never
inside a shared library (that would cause duplicate-symbol link errors):

- **`noisefield_dsp`** (INTERFACE, `src/dsp/` + `src/model/`) — header-only, **no JUCE**.
  Pure `std` so it is fast to unit-test. This is what `tests/` builds against.
- **`noisefield_core`** (STATIC, `src/core/`) — tiny compiled bits with no JUCE dependency
  (build-info string from the generated `noisefield/Config.h`). Links `noisefield_dsp`.
- **`noisefield_engine`** (INTERFACE) — the real-time `engine::SignalGraph` (`SignalGraph.cpp`),
  **framework-free** (no JUCE; denormal protection is the caller's job). Both the app and the
  plugin link it, so `SignalGraph.cpp` compiles once into each. Unit tests link it too.
- **`Noisefield`** (`juce_add_gui_app`, `src/app/` + `src/gui/` + `engine/AudioEngine.cpp`) —
  standalone app. Compiles JUCE + `SignalGraph.cpp`. `AudioEngine` is a thin device wrapper
  around `SignalGraph`, standalone-only.
- **`NoisefieldPlugin`** (`juce_add_plugin`, `src/plugin/`, opt-in `-DNOISEFIELD_BUILD_PLUGIN=ON`)
  — VST3/LV2/CLAP. `NoisefieldAudioProcessor` wraps the same `SignalGraph`; host automation
  goes through an APVTS mirrored into the graph's atomics each block. Editor is currently
  `juce::GenericAudioProcessorEditor` (sharing the real UI is a follow-up).
- **`noisefield_assets`** (`juce_add_binary_data`) — embeds `docs/guide.md` as `BinaryData.h`.

Anything that needs JUCE goes in one of the final targets (app / plugin / tests), never in a
static lib they share.

Warnings: `noisefield_warnings` (INTERFACE) is applied to `noisefield_core`/tests directly and
to our own `.cpp` files **per-source-file** (`set_source_files_properties` in
`src/CMakeLists.txt` and `src/plugin/CMakeLists.txt`), never to a whole JUCE-compiling target —
that would flag JUCE's own unity `.cpp`. JUCE module targets are marked `SYSTEM ON` in
`cmake/FetchJUCE.cmake` so
their headers don't pollute our warning output. Versions are pinned in
`cmake/FetchJUCE.cmake` (JUCE 8.0.15) and `cmake/FetchCatch2.cmake` (Catch2 v3.7.1).

## Audio architecture

`engine::SignalGraph` is the whole real-time signal path and owns the parameter block. Per
sample: `SineOscillator` + tinted `WhiteNoise` (`dsp::NoiseTint`) → smoothed per-source gains
→ sum → smoothed master gain → `SoftLimiter` → every output channel; then lock-free taps for
the meter and the oscilloscope (`dsp::ScopeBuffer`). It is host-agnostic — the standalone
`engine::AudioEngine` feeds it from an `AudioIODeviceCallback`; the plugin feeds it from
`processBlock`.

- **controls → audio** go through `engine::EngineParameters` — a struct of `std::atomic` read
  with `std::memory_order_relaxed` in `process()`. The standalone GUI writes it directly; the
  plugin mirrors its APVTS into it once per block. No other channel; no locks.
- **audio → GUI**: the meter is an `std::atomic<float>` peak-hold updated with
  `compare_exchange`; the scope is a lock-free SPSC ring. Both read on a 30 Hz GUI timer.
- Every audible parameter is ramped: gains via `dsp::ParamSmoother`, frequency inside
  `SineOscillator`. The scratch buffer is sized once in `prepare()`.
- `SignalGraph::process()` must stay allocation-free, lock-free, I/O-free, exception-free. The
  rules and the patterns are in `docs/realtime-rules.md` — read it before touching it.

`src/dsp/` stays framework-free on purpose. If a DSP primitive needs JUCE, that is a sign it
belongs in `src/engine/` instead.

## GUI structure

`app::MainComponent` is the main window (transport, tone, noise, master, meter). Two
secondary windows are opened on demand via `gui::DetachedWindow` (non-modal, owner nulls its
`unique_ptr` on close):

- `gui::SettingsComponent` — soft-limiter toggle + `juce::AudioDeviceSelectorComponent`.
- `gui::GuideView` — renders the embedded `docs/guide.md` with a small hand-rolled Markdown
  subset renderer (`AttributedString` + `TextLayout`). `docs/guide.md` is the source of
  truth; editing it changes the in-app guide (after a rebuild re-embeds it).

Control values and the audio-device state persist via `juce::PropertiesFile`
(`~/.config/Noisefield/`), written on clean exit.

## Conventions

- **All `.md` files are written in English**, regardless of the conversation language.
- **Git commits**: end the message with `Co-Authored-By: Claude Sonnet 5 <noreply@anthropic.com>`
  and nothing else — no `Claude-Session:` line. Commits are grouped by task type
  (`chore:` / `docs:` / `feat(dsp):` / `feat(engine):` / `feat(gui):` / `test:` / `assets:`).
- **Do not `git push` without explicit confirmation.** Local commits are fine.
- Default branch is `main`.

## Known issues / constraints

- **BUG-001** (`docs/backlog.md`): the Guide and Settings window titles show mojibake because
  they are `const char*` literals with a UTF-8 em dash passed to `juce::String`, which decodes
  `const char*` as ASCII. Keep user-facing literals ASCII, or use `String::fromUTF8(...)`.
- The project is GPL-3.0-or-later, kept compatible with JUCE 8's open-source terms. A closed
  distribution or a store with extra restrictions would need a commercial JUCE license and a
  relicensing decision first.
