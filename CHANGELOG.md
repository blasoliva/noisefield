# Changelog

All notable changes to Noisefield are documented here. This file is maintained automatically
by [release-please](https://github.com/googleapis/release-please) from the Conventional Commit
messages on `main`; see [`docs/releasing.md`](docs/releasing.md).

## [0.2.1](https://github.com/blasoliva/noisefield/compare/v0.2.0...v0.2.1) (2026-09-11)


### Documentation

* remove personal email from CLAUDE.md and rework README intro ([1a07052](https://github.com/blasoliva/noisefield/commit/1a0705243a1aa5dd5814f79bdf04174c6e6b5c9b))
* remove personal email from CLAUDE.md and rework README intro ([f43704f](https://github.com/blasoliva/noisefield/commit/f43704f2fcd53f0285ba0628c9b4dd25ca23082e))

## [0.2.0](https://github.com/blasoliva/noisefield/compare/v0.1.0...v0.2.0) (2026-09-10)


### Features

* **engine:** render a pool of layer voices with crossfades (NF-041) ([e6ff64a](https://github.com/blasoliva/noisefield/commit/e6ff64a26dd9dc5df66dd4f10e14cf4315874382))
* preset model, JSON serialisation and user save/load (NF-040, NF-051) ([7c67e92](https://github.com/blasoliva/noisefield/commit/7c67e9272ce774d1bfbdd5978f7319646b053f98))


### Documentation

* add preset-menu screenshot and fix README repo layout table ([fa74e1c](https://github.com/blasoliva/noisefield/commit/fa74e1ca9d5a0c99e087490a8d3f9c5b019edf08))
* add Release, License and Platform badges to the README ([7666c96](https://github.com/blasoliva/noisefield/commit/7666c96e08f81a606b30e1677fd959bf3a625528))
* document the automated release flow ([9d5d01e](https://github.com/blasoliva/noisefield/commit/9d5d01ee7051817595096eb2cacf6d169c05cc96))
* document the engine layer pool (NF-041) ([078c18f](https://github.com/blasoliva/noisefield/commit/078c18fa924f36e9a32b67cdf1e44080858dddd6))
* drop A/B comparison from NF-051 ([98c3e04](https://github.com/blasoliva/noisefield/commit/98c3e04791d3eeb6212ad5b5bcbe9e9b379c3874))
* fix the noise re-seed row in realtime-rules for the layer model (NF-041) ([c826aa8](https://github.com/blasoliva/noisefield/commit/c826aa856ad5e9037f60bd9847103b9d5ebb28f4))
* refresh screenshots and docs for presets / colours / scope / plugin ([661a52a](https://github.com/blasoliva/noisefield/commit/661a52a587140245b0448a72d5876eac41a8647d))

## 0.1.0-alpha.1 (2026-09-10)

First preview release. Tone generator (20 Hz – 20 kHz) plus six noise colours (white, pink,
brown, blue, violet, grey), a mixer with a soft-limited master bus, a dBFS level meter, a
collapsible oscilloscope, factory and user presets, and a VST3 / LV2 / CLAP plugin built from
the same engine. Shipped as an AppImage and a `.deb`.
