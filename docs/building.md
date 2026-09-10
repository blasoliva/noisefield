# Building Noisefield

## Requirements

- A C++20 compiler (GCC ≥ 11 or Clang ≥ 14)
- CMake ≥ 3.22 and a build tool (Ninja or Make)
- Git (JUCE and Catch2 are fetched at configure time)
- Network access on the first configure (to clone JUCE / Catch2)

### System libraries (Debian / Ubuntu)

```sh
sudo apt install \
  build-essential cmake ninja-build pkg-config git \
  libasound2-dev libjack-jackd2-dev \
  libfreetype6-dev libfontconfig1-dev \
  libx11-dev libxext-dev libxrandr-dev libxinerama-dev libxcursor-dev \
  libgl1-mesa-dev libcurl4-openssl-dev
```

### System libraries (Fedora)

```sh
sudo dnf install \
  gcc-c++ cmake ninja-build pkgconf-pkg-config git \
  alsa-lib-devel jack-audio-connection-kit-devel \
  freetype-devel fontconfig-devel \
  libX11-devel libXext-devel libXrandr-devel libXinerama-devel libXcursor-devel \
  mesa-libGL-devel libcurl-devel
```

`libjack-jackd2-dev` / `jack-audio-connection-kit-devel` is optional — it only enables the
JACK backend. ALSA is enough to build and run.

## Configure and build

```sh
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The application binary is at `build/Noisefield_artefacts/Release/Noisefield`.

The first configure downloads JUCE (~pinned tag `8.0.15`) into `build/_deps/`. Subsequent
configures reuse it.

## Tests

```sh
cmake -B build -G Ninja -DNOISEFIELD_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Options

| Option | Default | Effect |
|---|---|---|
| `NOISEFIELD_BUILD_TESTS` | `OFF` | Build the Catch2 test suite and register it with CTest |
| `NOISEFIELD_WERROR` | `OFF` | `-Werror` plus `-Wconversion` / `-Wsign-conversion` on our own code |
| `NOISEFIELD_JUCE_TAG` | `8.0.15` | JUCE git tag to build against |
| `NOISEFIELD_CATCH2_TAG` | `v3.7.1` | Catch2 git tag (only used when tests are on) |

## Formatting and static analysis

```sh
# check formatting (CI pins clang-format 23.1.0: pip install "clang-format==23.1.0")
scripts/check-format.sh
scripts/check-format.sh --fix

# clang-tidy uses build/compile_commands.json
run-clang-tidy -p build src/
```
