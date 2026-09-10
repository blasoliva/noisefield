# Noisefield — Backlog

Status: `[ ]` pending · `[~]` in progress · `[x]` done
Size: **S** ≤ 1 day · **M** 2–4 days · **L** 1–2 weeks

See context and technical rationale in [`plan.md`](plan.md).

## Milestones

| # | Name | Goal | Phase |
|---|---|---|---|
| **M1** | Scaffolding and skeleton | The project builds, passes CI, and opens a window | Phase 0 — ✅ done |
| **M2** | Audible MVP | Generate a frequency + white noise and mix them | Phase 1 — ✅ code done |
| **M3** | Noisefield | Multi-layer, all noise colors, band filter, presets | Phase 2 — 🚧 in progress |
| **M4** | Tools and distribution | Analysis, recording, timer, modulation, AppImage/Flatpak | Phases 3–4 |

---

## M1 — Scaffolding and skeleton — ✅ done (2026-09-10)

**Exit criterion:** `cmake --build` produces an executable that opens an empty window;
CI green; formatting and linter configured. — met locally: build exit 0, 7/7 tests pass,
no warnings from our code.

- [x] **NF-001** (S) Initialize git repository, `.gitignore`, license, and initial `README`.
- [x] **NF-002** (S) Root `CMakeLists.txt` with C++20 and strict warning options.
- [x] **NF-003** (M) Integrate JUCE 8 via `FetchContent` pinned to a tag; `juce_add_gui_app` target.
- [x] **NF-004** (S) `src/app` skeleton: `Main.cpp`, `MainComponent`, resizable window with dark theme.
- [x] **NF-005** (S) Configure `clang-format` and `clang-tidy` + a local check script.
- [x] **NF-006** (M) GitHub Actions: build + tests on Ubuntu LTS; dependency caching.
- [x] **NF-007** (S) Integrate Catch2 v3 and a trivial test that runs in CI.
- [x] **NF-008** (S) Document system dependencies and build steps in `docs/`.
- [x] **NF-009** (S) Create the folder structure (`engine/ dsp/ model/ gui/ io/`) with placeholders and CMake targets.
- [x] **NF-010** (S) `.desktop` file and a provisional icon.

> Note: CI is written but not yet observed green on GitHub (no remote/push). The `format`
> job needs `clang-format-18`, which is not installed in this dev environment.

## M2 — Audible MVP — ✅ code done (2026-09-10), pending on-device listening test

**Exit criterion:** open the app, pick a device, set a tone to an exact frequency, add white
noise, adjust the mix and the master, and listen for 10 min with no clicks or xruns.

- [x] **NF-020** (M) `AudioEngine`: open `AudioDeviceManager`, callback, sample rate/buffer handling.
- [x] **NF-021** (S) UI for audio device, sample rate, and buffer size selection; xrun indicator (status bar).
- [x] **NF-022** (M) `SineOscillator` with smoothed frequency (30 ms ramp); clamped to 20 Hz–20 kHz.
- [x] **NF-023** (S) Frequency control: logarithmic rotary slider + editable Hz text box (double-click to type).
- [x] **NF-024** (M) `WhiteNoise` with a seedable `xoshiro256++` PRNG; level via the mixer; re-seed button.
- [x] **NF-025** (M) Mixer: tone gain + noise gain + master gain, all `dsp::ParamSmoother`-ramped.
- [x] **NF-026** (S) `dsp::SoftLimiter` (tanh knee) on the master bus + `juce::ScopedNoDenormals`.
- [x] **NF-027** (S) Transport: play / stop; master mute.
- [x] **NF-028** (S) Master level meter (peak-hold + RMS) via a lock-free `compare_exchange` on `std::atomic<float>`.
- [x] **NF-029** (M) RT-safety audit + [`realtime-rules.md`](realtime-rules.md).
- [x] **NF-030** (S) Persist frequency, levels, enables, limiter and the audio-device state (`juce::PropertiesFile`).
- [x] **NF-031** (M) Tests: oscillator (range/RMS/frequency/clamp/ramp), white noise (bounds/mean/variance/flatness), smoother, limiter, gain, level detector — 23 tests, all passing.
- [x] **NF-032** (S) Detached **Settings** window (`gui::SettingsComponent` + `gui::DetachedWindow`): moves the soft limiter and the whole audio-device selector (output / sample rate / buffer / channels / advanced) out of the main window; main window keeps only a **Settings** button.
- [x] **NF-033** (M) In-app **Guide** window: `docs/guide.md` embedded via `juce_add_binary_data`, rendered by `gui::GuideView` (small Markdown subset → `AttributedString`/`TextLayout`, scrollable). Opened from a **Guide** button, non-modal so it stays open while using the app.

> Not yet verified here: actual sound output and the 10-minute no-xrun listening test (no
> audio device in the build environment). Run the app on a real machine to close the milestone.

## M3 — Noisefield

**Exit criterion:** create a project with several layers (oscillators and noise of any color),
filter each layer by band, mix with pan/mute/solo, and save/load the project and presets.

- [ ] **NF-040** (M) `Project` / `Layer` model + JSON serialization with a schema version.
- [ ] **NF-041** (M) Layer management in the engine: add/remove/reorder without glitches (crossfade).
- [ ] **NF-042** (M) Layer-list GUI: create, delete, reorder, select; per-layer parameter editing.
- [x] **NF-043** (M) Pink noise (Kellett filter) — `dsp::NoiseTint`, RMS-matched, spectral-tilt test.
- [x] **NF-044** (S) Brown noise (leaky integrator) — `dsp::NoiseTint` + test.
- [x] **NF-045** (S) Blue and violet noise (differentiation) — `dsp::NoiseTint` + tests.
- [x] **NF-046** (M) Grey noise (approximate inverse equal-loudness shelving) — `dsp::NoiseTint` + test.
  All 6 colours are selectable per-source in the GUI (Noise → Colour) and level-matched;
  `test_noise_tint.cpp` checks bounds, RMS match, the dark→bright ordering and determinism.
- [ ] **NF-047** (M) Per-layer SVF (TPT) band filter: LP/HP/BP/Notch/off modes, smoothed cutoff and Q.
- [ ] **NF-048** (M) Full mixer: fader, pan, mute, solo per layer + per-layer meters.
- [ ] **NF-049** (M) Additional waveforms with PolyBLEP (triangle/square/saw) + aliasing test.
- [ ] **NF-050** (M) Save/open project (file dialogs) and autosave.
- [ ] **NF-051** (M) Preset system: factory presets in `resources/`, user save, A/B comparison.
- [ ] **NF-052** (S) Performance: 8 layers < 25% of one core at 48 kHz (measurement and profiling).

## M4 — Tools and distribution

**Exit criterion:** working spectrum analyzer and meters; offline recording and export;
session timer with fades; LFO/ADSR/sweep modulation; installable AppImage and Flatpak.

- [ ] **NF-060** (M) FFT spectrum analyzer (window, averaging, log scale) fed by a FIFO.
- [x] **NF-061** (S) Oscilloscope with rising-zero-crossing trigger, collapsible in the main
  window (Scope button; window grows/shrinks to fit; state persists). Engine feeds it via
  `dsp::ScopeBuffer` (lock-free SPSC ring); `test_scope_buffer.cpp`.
- [~] **NF-062** (S) Master meter: dBFS scale ticks, peak hold, clip latch, peak-dBFS readout
  in the status line. Per-layer meters wait for the M3 layer rework (NF-041/048).
- [ ] **NF-063** (M) WAV/FLAC recorder of the master bus (streaming to disk from a separate thread).
- [ ] **NF-064** (M) Fixed-duration offline export, faster than real time, with TPDF dither.
- [ ] **NF-065** (M) Session timer: duration, fade-in/out, automatic stop.
- [ ] **NF-066** (M) LFO (per-layer and global) assignable to frequency/cutoff/gain/pan.
- [ ] **NF-067** (M) ADSR envelope and per-layer fade in/out.
- [ ] **NF-068** (M) Frequency sweep (linear/exponential) and glissando.
- [ ] **NF-069** (M) Basic modulation matrix (source→destination, amount) + UI.
- [ ] **NF-070** (S) Stereo width / noise decorrelation between channels.
- [x] **NF-071** (M) AppImage packaging with `linuxdeploy` (via the release workflow):
  `cmake --install --component noisefield` → `AppDir` → `linuxdeploy --output appimage` →
  `Noisefield-<version>-x86_64.AppImage`. Verified locally (builds, runs). (`.desktop` and
  icon done since M1; "final icon" is still the provisional one.)
- [ ] **NF-072** (M) Flatpak manifest and local publishing + instructions.
- [ ] **NF-073** (S) Explicit JACK support: port names, reconnection.
- [ ] **NF-074** (M) CLI/headless mode: render a project to a file without the GUI.
- [ ] **NF-075** (M) Accessibility: full keyboard navigation, labels, contrast.
- [ ] **NF-076** (M) User manual in `docs/` and release notes.
- [ ] **NF-077** (M) Final RT-safety and stability audit (1-hour session with no xruns).

### Distribution — installers and releases

- [x] **NF-081** (S) **Install rules + FHS layout** in the `noisefield` install component
  (`bin/noisefield`, `share/applications/`, `share/icons/hicolor/{128,256,512}/apps/`) — a
  component so it picks up only our files, not JUCE's own `install()` rules.
- [x] **NF-082** (M) **`.deb` via CPack.** `Release` + `CPACK_STRIP_FILES`, deps via
  `CPACK_DEBIAN_PACKAGE_SHLIBDEPS`, `sound` section, maintainer / homepage. `CPACK_RPM_*`
  set too. Verified: `noisefield_<version>_amd64.deb` with the right layout and metadata.
- [x] **NF-083** (M) **Release workflow** (`.github/workflows/release.yml`): on a `v*` tag,
  `Release` build on `ubuntu-22.04` → `.deb` (NF-082) + AppImage (NF-071) + plugin
  `.tar.gz` (VST3/LV2/CLAP) → attached to the GitHub Release
  (`softprops/action-gh-release`, `generate_release_notes`). A tag containing `-` is marked
  pre-release. `NOISEFIELD_VERSION` (from the tag) is stamped into the binary and packages.
- [ ] **NF-084** (S) `CHANGELOG.md` and per-release notes (feeds NF-076; the workflow uses
  GitHub's auto-generated notes for now).
- [ ] **NF-085** (S) Ship the plugins in the `.deb` too (install to `/usr/lib/{vst3,lv2,clap}`
  behind an option), or a separate `noisefield-plugins.deb`.

---

## Icebox (post-M4, no milestone)

- [x] **NF-090** Plugin: **VST3 + LV2 + CLAP** from the shared `engine::SignalGraph`
  (`src/plugin/`, `-DNOISEFIELD_BUILD_PLUGIN=ON`). `NoisefieldAudioProcessor` exposes the
  tone/noise/master params via an APVTS for host automation. Engine was refactored so the
  standalone `AudioEngine` and the plugin drive the same graph. Follow-up: the plugin uses a
  generic editor — the custom UI is not shared with it yet.
- [ ] **NF-091** MIDI input: play the oscillator, map CC to parameters.
- [x] **NF-092** Use-case presets: 8 built-in presets (tinnitus mask ±tone, focus, sleep,
  deep rain, white wash, 1 kHz test tone, silence) selectable from a **Preset** combo that
  sets every control at once (`src/app/Presets.h`). Not persisted itself — the resulting
  control values are. File-based user presets remain M3's NF-051.
- [ ] **NF-093** Timeline automation.
- [ ] **NF-094** Internationalization ES/EN.
- [ ] **NF-095** EQ profiles / filter-response import.
- [ ] **NF-096** LFO sync to BPM / tap tempo.

---

## Bugs to resolve

- [x] **BUG-001** — Encoding artefacts in the Guide / Settings window titles
  (they read `Noisefield â Guide` instead of `Noisefield — Guide`).
  - **Cause:** the titles were `const char*` literals with a UTF-8 em dash;
    `juce::String(const char*)` decodes as `CharPointer_ASCII` and mangled it. A second,
    milder failure surfaced under headless test — Xlib's
    `Xutf8TextListToTextProperty` needs a UTF-8 locale to write the title property.
  - **Fix** (`src/app/MainComponent.cpp`, `src/app/Main.cpp`): a local `uiString()`
    helper wrapping `juce::String::fromUTF8` for the two titles, plus
    `std::setlocale(LC_ALL, "")` at startup. Verified via `xprop`: `WM_NAME` now
    carries `... 20 e2 80 94 20 ...` (a real U+2014).
  - Convention: user-facing literals with non-ASCII characters go through
    `juce::String::fromUTF8` (or `uiString`).

---

## Task dependencies (summary)

- **NF-003** blocks essentially all of M2 onward.
- **NF-020** blocks NF-021 … NF-028.
- **NF-040** blocks NF-041, NF-042, NF-050, NF-051.
- **NF-047** depends on NF-041 (layers in the engine).
- **NF-060 / NF-061 / NF-062** depend on NF-028 (analysis FIFO infrastructure).
- **NF-064** depends on NF-063.
- **NF-081** (install rules) was the prerequisite for **NF-071** (AppImage) and **NF-082**
  (.deb); **NF-083** (release workflow) ties all three together. All done.
