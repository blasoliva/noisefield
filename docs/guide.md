# Noisefield — User guide

Noisefield generates a pure tone and coloured noise, mixes them, and sends the result to your
audio output. Use it to build a sound that helps in the moment.

## Getting started

- Press **Play**.
- Pick a **Preset**, or raise the **Tone** and/or **Noise** level from the bottom.
- Set the **Frequency**, the **Noise colour**, and the master **Level** to taste.
- **Mute** silences the output instantly without stopping playback.

Start with the master **Level** low and bring it up slowly. All settings are remembered for
next time.

## Main window

### Preset

A starting point that sets every control at once. Pick one, then adjust to taste — the
preset is not "locked", it just moves the controls.

- **Menu** — factory presets at the top, your own below:
  - **Tinnitus mask** / **Tinnitus mask + tone** — pink noise, with a 4 kHz tone in the
    second one that you can drag to your own tinnitus pitch.
  - **Focus** — steady pink noise, no tone.
  - **Sleep** — brown noise with a soft 60 Hz hum, quieter master.
  - **Deep rain** — brown noise.
  - **White wash** — plain white noise.
  - **Test tone 1 kHz** — a 1 kHz tone, no noise.
  - **Silence** — everything down.
- **Save** — stores the current settings as a named preset under
  `~/.config/Noisefield/presets/` (a `.nfp` JSON file). Re-using a name overwrites it.
- **Delete** — removes the selected user preset (factory presets can't be deleted).

### Transport

- **Play / Stop** — starts and stops audio processing.
- **Mute** — instantly silences the master output; playback keeps running.
- **Level meter** — master output level on a dBFS scale (faint ticks at −48, −36, −24, −12,
  −6). The filled bar is RMS (perceived loudness); the moving mark is the recent peak and
  turns red at 0 dBFS. The small block on the right is a clip latch: it lights red if the
  signal hits 0 dBFS and clears itself after a few quiet seconds. The status line shows the
  current peak in dBFS.
- **Scope** — expands or collapses the oscilloscope, which appears just below the buttons.
  The window grows and shrinks to fit; the state is remembered between runs.
- **Guide** — opens this guide.
- **Settings** — opens the audio-device settings and the soft limiter.

### Oscilloscope

The master output waveform over time, aligned to a rising zero-crossing so a steady tone
appears to stand still. Use it to see the tone shape, how much noise is mixed in, and whether
the signal is being clipped by the limiter. Toggle it with the **Scope** button.

### Tone

- **Enabled** — includes the tone in the mix.
- **Frequency** — pitch of the tone, 20 Hz to 20 kHz. Drag the knob, or double-click the
  number to type an exact value in Hz. The knob is logarithmic, so low frequencies get more
  travel. Changes are ramped over about 30 ms so they never click.
- **Level** — loudness of the tone, in decibels. At the bottom it is fully silent.

### Noise

- **Enabled** — includes the noise in the mix.
- **Colour** — the spectral tilt of the noise:
  - **White** — flat, equal energy per hertz; bright and hissy.
  - **Pink** — −3 dB/octave, equal energy per octave; the "balanced" broadband noise.
  - **Brown** — −6 dB/octave; deep, like heavy rain or a waterfall.
  - **Blue** — +3 dB/octave; brighter than white.
  - **Violet** — +6 dB/octave; very bright, mostly high hiss.
  - **Grey** — shaped so it sounds roughly equally loud across the spectrum (approximate).
  All colours are level-matched, so switching colour keeps a similar loudness.
- **Level** — loudness of the noise, in decibels.
- **Re-seed** — jumps the noise to a new random starting point. The character is the same;
  use it if you want a different exact stream.

### Master

- **Level** — overall output loudness, applied after the tone and noise are summed.

The status line at the bottom shows the current sample rate, the recent peak level in dBFS,
and the number of audio dropouts (xruns) since the device opened. The xrun count should stay
at 0; if it climbs, raise the buffer size in Settings.

## Settings window

- **Soft limiter** — safety net on the master bus. Below about -1 dBFS it does nothing; above
  that it smoothly compresses peaks so the output never clips. Leave it on unless you
  specifically want the raw signal.
- **Output** — which sound card or device Noisefield plays through.
- **Sample rate** — samples per second, for example 48000. Higher is not audibly better
  here; match what the rest of your system uses.
- **Audio buffer size** — latency against stability. Smaller is more responsive but more
  likely to produce xruns; larger is rock solid with a little lag. 256 or 512 samples is a
  good start.
- **Active output channels** — which physical outputs receive the signal.
- **Show advanced settings** — extra device options provided by the driver.

Changes here are saved and restored automatically.

## Tips

- For masking, noise with a touch of tone often works better than either alone.
- If a tone frequency feels right, type it in so you can return to it exactly.
- Keep the limiter on and leave headroom on the master, so bringing a level up never jumps
  to a harsh peak.
- **Save** a mix you like so you can come back to it; delete the ones that don't work.

This app is a personal tool, not a medical device.
