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

## Plugin (VST3 / LV2 / CLAP)

```sh
cmake -B build -G Ninja -DNOISEFIELD_BUILD_PLUGIN=ON
cmake --build build
```

Artefacts land in `build/src/plugin/NoisefieldPlugin_artefacts/<config>/`:
`VST3/Noisefield.vst3`, `LV2/Noisefield.lv2`, `CLAP/Noisefield.clap`. Copy them to your
plugin folders (`~/.vst3`, `~/.lv2`, `~/.clap`).

The CLAP build fetches `clap-juce-extensions` (with its CLAP SDK submodules) on the first
configure. Skip it with `-DNOISEFIELD_PLUGIN_CLAP=OFF` if you only want VST3 + LV2.

The plugin currently uses a generic parameter editor; the full custom UI is not shared with
it yet.

## Packaging

Pre-built downloads (AppImage, `.deb`, plugin tarball) are attached to each
[GitHub release](https://github.com/blasoliva/noisefield/releases). To build them yourself:

```sh
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DNOISEFIELD_BUILD_PLUGIN=ON
cmake --build build

# .deb (Debian/Ubuntu)
( cd build && cpack -G DEB )        # -> build/noisefield_<version>_amd64.deb

# install into an AppDir, then run linuxdeploy over it (see .github/workflows/release.yml)
cmake --install build --prefix AppDir/usr
```

`cmake --install` lays the app out under the prefix following the FHS (`bin/noisefield`,
`share/applications/`, `share/icons/hicolor/`). Pass `-DNOISEFIELD_VERSION=x.y.z-tag` to stamp
a specific version into the binary and the packages (the release workflow sets it from the
git tag).

Releases: push a tag matching `project(VERSION ...)`, e.g. `git tag v0.1.0-alpha.1 && git push
origin v0.1.0-alpha.1`. `.github/workflows/release.yml` then builds everything and publishes
it; a tag containing `-` is marked as a pre-release.

## Options

| Option | Default | Effect |
|---|---|---|
| `NOISEFIELD_BUILD_TESTS` | `OFF` | Build the Catch2 test suite and register it with CTest |
| `NOISEFIELD_BUILD_PLUGIN` | `OFF` | Build the VST3/LV2/CLAP plugin |
| `NOISEFIELD_PLUGIN_CLAP` | `ON` | When the plugin is built, also build a CLAP (fetches `clap-juce-extensions`) |
| `NOISEFIELD_VERSION` | `project()` version | Full version string stamped into the binary and packages |
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
