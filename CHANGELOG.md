# Changelog

All notable changes to Noisefield are documented here. This file is maintained automatically
by [release-please](https://github.com/googleapis/release-please) from the Conventional Commit
messages on `main`; see [`docs/releasing.md`](docs/releasing.md).

## [0.4.1](https://github.com/blasoliva/noisefield/compare/v0.4.0...v0.4.1) (2026-09-12)


### Bug Fixes

* **model:** serialize preset numbers locale-independently ([#14](https://github.com/blasoliva/noisefield/issues/14)) ([08e43d0](https://github.com/blasoliva/noisefield/commit/08e43d02bb249c75af082cf8e3f367d3e558c558))

## [0.4.0](https://github.com/blasoliva/noisefield/compare/v0.3.0...v0.4.0) (2026-09-11)


### Features

* **app:** draw icons for the Scope/Timer/Guide/Settings buttons ([c36931c](https://github.com/blasoliva/noisefield/commit/c36931cb8f4abb58337a3f5fbe3aa036cb41d2f6))
* **app:** group the main window into cards, drop the frequency knob ([db33dd1](https://github.com/blasoliva/noisefield/commit/db33dd108913bcec9dbaa27b55e1453e38a32a03))
* card-based redesign of the main window ([7cec2bf](https://github.com/blasoliva/noisefield/commit/7cec2bf8b1a98c794768b342456d875788cf76c4))


### Bug Fixes

* **app:** active/hover states for transport buttons, drop Master highlight ([7c60bba](https://github.com/blasoliva/noisefield/commit/7c60bba5958c208072e85124df5f35ea2d483592))
* **app:** align Tone's Enabled checkbox under its heading like Noise's ([2cb7d72](https://github.com/blasoliva/noisefield/commit/2cb7d72b5fe8ec6c55ad4e883be8f89e7e02a720))
* **app:** give LinearHorizontal sliders a fixed-width track ([e50d679](https://github.com/blasoliva/noisefield/commit/e50d6795654047ca6a23542b98456cd74b9491ea))
* **app:** spread the Monitor card's status row left/centre/right ([22a84ba](https://github.com/blasoliva/noisefield/commit/22a84ba8d830b67c42045e5d0a02a9d886291420))
* **ci:** grant issues:write so release-please can label its PR ([b1b749b](https://github.com/blasoliva/noisefield/commit/b1b749b522f69fb3d0568cdbfec2e7236c4f8d7a))
* **ci:** grant issues:write so release-please can label its PR ([3a957f7](https://github.com/blasoliva/noisefield/commit/3a957f734876652f58bede4dd28f193fec302ab6))


### Documentation

* refresh screenshots and wording for the card-based redesign ([56bd237](https://github.com/blasoliva/noisefield/commit/56bd237554be50189b42c51c2cec4272aace33fd))

## [0.3.0](https://github.com/blasoliva/noisefield/compare/v0.2.1...v0.3.0) (2026-09-11)


### Features

* **app:** add a session timer with fade in/out (NF-065) ([37afcd3](https://github.com/blasoliva/noisefield/commit/37afcd30d6d333fbaaccd045e2daea3d4e0594f6))
* **engine:** add explicit JACK support with auto-reconnect (NF-073) ([b050b62](https://github.com/blasoliva/noisefield/commit/b050b6250c86243bcac5f99c5d15da98c76b917f))


### Documentation

* cover the session timer and JACK reconnect in the user manual (NF-076) ([ad90f5c](https://github.com/blasoliva/noisefield/commit/ad90f5c3c189f9261d49bdbfd7c39ebbb912b0a5))
* trim M3/M4 backlog to what's actually planned, move the rest to icebox ([331ce94](https://github.com/blasoliva/noisefield/commit/331ce945abb10758d06d91a00235d61e075d4299))

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
