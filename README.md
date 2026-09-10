# Noisefield

A native Linux desktop app to generate tones across a frequency range and mix different
types of noise (white, pink, brown, blue, violet, grey) into layered sound fields.

I have tinnitus. Playing certain tones and noise blends through this app relieves the
sensation for me at times — that is why it exists. This app is a personal tool, not a
medical device.

Noisefield is built 100% AI-assisted.

> Status: **milestone M2 (audible MVP)**. Tone + white noise + mixer + master limiter, with
> a level meter and device selection. See [`docs/plan.md`](docs/plan.md) for the full plan
> and [`docs/backlog.md`](docs/backlog.md) for tasks.

## Tech stack

- **C++20** + **[JUCE 8](https://juce.com/)** (fetched automatically by CMake)
- **CMake ≥ 3.22** build
- Linux audio via **ALSA** (native) and **JACK** (optional at runtime); PipeWire-compatible
- Tests with **Catch2 v3**

## Building

See [`docs/building.md`](docs/building.md) for the full list of system dependencies.

```sh
# Debian/Ubuntu system dependencies
sudo apt install build-essential cmake pkg-config \
  libasound2-dev libjack-jackd2-dev libfreetype6-dev libfontconfig1-dev \
  libx11-dev libxext-dev libxrandr-dev libxinerama-dev libxcursor-dev \
  libgl1-mesa-dev libcurl4-openssl-dev

# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Run
./build/src/Noisefield_artefacts/Release/Noisefield

# Tests
cmake -B build -DNOISEFIELD_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Repository layout

| Path | Purpose |
|---|---|
| `src/app/` | Application entry point and main window |
| `src/engine/` | Real-time audio engine, graph, mixer, master bus |
| `src/dsp/` | Oscillators, noise generators, filters, modulators |
| `src/model/` | Project/Layer data model and JSON serialization |
| `src/gui/` | GUI components |
| `src/io/` | Recorder, exporter, headless CLI |
| `tests/` | Catch2 tests |
| `cmake/` | Build helpers (JUCE fetch) |
| `resources/` | Icons, factory presets, `.desktop` file |
| `packaging/` | AppImage / Flatpak packaging |

## License

[GPL-3.0-or-later](LICENSE). This choice keeps the project compatible with JUCE 8's
open-source (AGPL/GPL) licensing terms; revisit if a commercial JUCE license is acquired.
