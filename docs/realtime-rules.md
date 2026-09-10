# Real-time audio rules

The audio callback runs on a high-priority thread with a hard deadline (one buffer period —
e.g. 5.3 ms at 256 frames / 48 kHz). Missing the deadline produces an xrun (an audible click).

The whole real-time path is `engine::SignalGraph::process()`, called from the standalone
`engine::AudioEngine` callback and from the plugin's `processBlock`. Everything it transitively
calls must obey these rules. `SignalGraph` is deliberately framework-free — no JUCE — so it
only ever touches `std` and our own `dsp::` code.

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

- Reading/writing `std::atomic` with `std::memory_order_relaxed` (all controls flow through
  `engine::EngineParameters`).
- Arithmetic, `std::sin`, `std::tanh`, `std::abs`, `std::copy` / `std::fill` on pre-sized
  buffers.
- Denormal protection: the callers wrap `SignalGraph::process` in `juce::ScopedNoDenormals`
  (`SignalGraph` itself stays JUCE-free).

## Patterns used in Noisefield

| Concern | Solution |
|---|---|
| Controls change a parameter | `std::atomic` field in `EngineParameters` (per-layer in `LayerParameters`), relaxed load in `process()`. The plugin mirrors its APVTS into it once per block. |
| Parameter change would click | `dsp::ParamSmoother` per-sample ramp on per-layer and master gains; `SineOscillator` ramps frequency internally |
| Add / remove a layer | Writer toggles `LayerParameters::active`; the per-slot gain smoother crossfades it in/out. An inactive slot whose gain has reached 0 and is not smoothing is skipped entirely — no state advanced, no CPU. |
| Reuse / retype a slot | Writer sets the slot's fields then bumps `LayerParameters::epoch`. `process()` sees the epoch change, hard-resets that voice (frequency/seed/colour immediate, phase 0) and forces its gain smoother to 0 so it fades in and no tail of the previous layer leaks through. |
| Scratch buffer | `std::vector<float>` sized once in `SignalGraph::prepare()` (non-audio thread), only indexed in `process()`; `process()` clamps `numSamples` to its size defensively |
| Noise re-seed | writer sets a layer's `LayerParameters::seed`; `process()` compares it against that voice's cached `appliedSeed` and re-seeds the layer in place (no allocation) |
| Meter readout | audio thread does a lock-free `compare_exchange` peak-hold into `std::atomic<float>`; GUI `exchange`s it back to 0 |
| Oscilloscope | audio thread writes a lock-free SPSC ring (`dsp::ScopeBuffer`); GUI copies the most recent window on a timer |
| Summing clips | `dsp::SoftLimiter` on the master bus |

## Auditing

- Read `SignalGraph::process` and every function it transitively calls; confirm none of the
  forbidden list appears.
- Build with `-DNOISEFIELD_WERROR=ON` so `-Wconversion` and friends stay clean.
- Manual listening test: 10+ minutes with parameter sweeps at a 256-frame buffer, watching
  the xrun counter in the status bar (target: 0).
- Optional tooling for deeper audits: `rtsan` (real-time sanitizer, Clang 20+), or
  `perf`/`osnoise` to catch scheduling stalls.

## Known limitations

- `AudioEngine::initialise` / `shutdown`, `SignalGraph::prepare`, and device changes run on a
  non-audio thread and do allocate — that is fine, they are not the audio thread.
- If a block larger than the size passed to `prepare()` arrives, the extra frames are
  zero-filled rather than rendered. Not observed with ALSA/JACK; revisit if it ever happens.
