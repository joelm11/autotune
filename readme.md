# Autotune

A straightforward autotune implementation using a phase vocoder + resampler for pitch shifting.

The phase vocoder in [`src/vocoder/`](src/vocoder/) uses phase locking (Laroche & Dolson, 1999):
each analysis frame, spectral peaks are identified and the bins in each peak's influence zone
are phase-locked to the peak. Partials therefore keep their wave shape when the signal is
time-stretched or compressed, instead of degrading into the "phasy" smear of plain per-bin
phase accumulation.

## Places I've allowed AI to touch

- Phase vocoder implementation (`src/vocoder/`)
- Processor tests
- Some resampling API changes
