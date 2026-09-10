# Real-time audio rules

The audio callback runs on a high-priority thread with a hard deadline (one buffer period —
e.g. 5.3 ms at 256 frames / 48 kHz). Missing the deadline produces an xrun (an audible click).
Everything reachable from `AudioEngine::audioDeviceIOCallbackWithContext` must obey these
rules.

## Forbidden on the audio thread

- **Allocation / deallocation** — no `new`/`delete`, no `malloc`/`free`, no container growth
  (`push_back`, `resize`, `std::string` ops, `juce::Array` growth), no `std::function` that
  captures by value into heap, no `shared_ptr` copies.
- **Locks** — no `std::mutex`, no `juce::CriticalSection`, no `juce::MessageManagerLock`,
  nothing that can block on another thread.
- **I/O** — no file access, no logging, no `printf`/`std::cout`, no sockets.
- **System calls that may block or page** — no `sleep`, no time-zone-aware clock calls in hot
  paths.
- **Exceptions** — no `throw`, and avoid code paths that might (e.g. `std::vector::at`).
- **Priority inversion sources** — don't touch data the GUI holds a lock on.

## Allowed

- Reading/writing `std::atomic` with `std::memory_order_relaxed` (all our GUI↔audio
  parameters go through `EngineParameters`).
- Arithmetic, `std::sin`, `std::tanh`, `std::abs`, `std::copy` on pre-sized buffers.
- `juce::ScopedNoDenormals` (we set it at the top of the callback).
- `juce::FloatVectorOperations::*`.

## Patterns used in Noisefield

| Concern | Solution |
|---|---|
| GUI changes a parameter | `std::atomic` field in `EngineParameters`, relaxed load in the callback |
| Parameter change would click | `dsp::ParamSmoother` per-sample ramp on gains; `SineOscillator` ramps frequency internally |
| Scratch buffer | `std::vector<float>` sized once in `audioDeviceAboutToStart` (message thread), only indexed in the callback; the callback clamps `numSamples` to its size defensively |
| Noise re-seed | GUI writes `noiseSeed`; the callback compares against `appliedNoiseSeed_` and re-seeds in place (no allocation) |
| Meter readout | audio thread does a lock-free `compare_exchange` peak-hold into `std::atomic<float>`; GUI `exchange`s it back to 0 |
| Denormals | `juce::ScopedNoDenormals` for the whole callback |
| Summing clips | `dsp::SoftLimiter` on the master bus |

## Auditing

- Read the callback and every function it transitively calls; confirm none of the forbidden
  list appears.
- Build with `-DNOISEFIELD_WERROR=ON` so `-Wconversion` and friends stay clean.
- Manual listening test: 10+ minutes with parameter sweeps at a 256-frame buffer, watching
  the xrun counter in the status bar (target: 0).
- Optional tooling for deeper audits: `rtsan` (real-time sanitizer, Clang 20+), or
  `perf`/`osnoise` to catch scheduling stalls.

## Known M2 limitations

- `AudioEngine::initialise` / `shutdown` and device changes run on the message thread and do
  allocate — that is fine, they are not the audio thread.
- If the device delivers a block larger than the size reported to `audioDeviceAboutToStart`,
  the extra frames are zero-filled rather than rendered. Not observed with ALSA/JACK; revisit
  if it ever happens.
