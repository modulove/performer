# Feature Tracking & Roadmap

This document consolidates the open feature work identified across the existing notes and provides a lightweight workflow for tracking new development.

## Priority Scale

Priorities follow the numeric convention already used in `doc/Priorities.md` (higher number = higher priority):

- **P5 – Critical**: Release blockers or prerequisites for promised functionality.
- **P4 – High**: Important user-facing improvements that should ship in the next release.
- **P3 – Medium**: Enhancements with clear value but no imminent deadline.
- **P2 – Low**: Nice-to-have improvements or experiments.
- **P1 – Icebox**: Parked ideas; revisit once higher priorities clear.

## Active Feature Tasks

| Feature | Status | Next Step | Priority | Source Notes |
|---------|--------|-----------|----------|--------------|
| Polyphonic step editor (piano roll) | Open | Design multi-note editing UI and update `SequencePainter`/page flow. | P2 | PEWFORMER_USER_GUIDE.md:63,270 |
| Polyphonic live recording | Open | Extend recording pipeline to capture simultaneous note-ons per step. | P2 | PEWFORMER_USER_GUIDE.md:87-89,270 |
| Micro-timing reliability at extreme offsets | Open | Benchmark offsets >±30 at high tempos; add guard rails or documentation. | P4 | PEWFORMER_USER_GUIDE.md:277 |
| Swing + capture timing interplay | Open | Reproduce swing/offset conflicts and add regression tests or UI guidance. | P3 | PEWFORMER_USER_GUIDE.md:278 |
| Hardware regression pass for v0.0.7 features | Open | Run full hardware QA checklist and log results. | P4 | PEWFORMER_USER_GUIDE.md:282 |
| Audio-rate modulator mode | Open | Prototype a high-rate path so selected modulators can run fast enough for FM audio modulation while keeping CPU usage acceptable. | P4 | Modulator audio-rate request |
| Gate-held playback mode for modulators | Open | Add a mode where the modulator plays only while its gate input is high and pauses/resets when released. | P3 | Modulator gate-mode request |
| Routing mixer matrix page | Planning | Draft specification for a mixer-style matrix showing all routable connections with brightness-coded squares and per-node mix control via buttons + encoder. | P4 | Modulator branch planning |
| Pattern ↔ MIDI file import/export | Planning | Draft spec using `FileManager` async tasks to write/read Standard MIDI Files per pattern (with background progress), ignore unsupported poly notes above mono limit, and map encoder/buttons for slot selection while playback continues. | P4 | MIDI pattern interchange request |
| CV routing voltage scaling | Done | Display now switches to millivolts for CV routes with adjustable ±1…±5 V scaling; UI and CV output verified and shipped in v0.0.75. | P4 | Modulator CV display request |
| Bipolar ADSR envelope mode | Done | ADSR envelopes can now output bipolar voltage (-5V to +5V) when routed to CV; toggle on Modulator Page 2; shipped in v0.0.75. | P4 | ADSR bipolar mode request |
| Startup reliability improvements | Done | Added 300ms SD card init delay and error handling to prevent boot crashes; shipped in v0.0.75. | P4 | Stability improvement |
| Undo/redo via double-tap Left/Right | Planning | Implement a 16-entry history buffer with double-tap Left/Right for undo/redo; define scope (global vs page), persistence, and feedback. | P4 | Workflow polish request |
| Keyboard function page | Planning | Scope new page showing full keyboard with routing-friendly interaction; design layout, navigation, and integration with mixer matrix work. | P3 | Keyboard feature branch kickoff |

## Known Bugs

| Bug | Status | Severity | Next Step | Source Notes |
|-----|--------|----------|-----------|--------------|
| LFOs stop in Free mode and rate displays beat divisions instead of ms/sec | Done | High | Fixed in v0.0.75: Free mode now prevented during boot/suspension; changed default to Sync mode to avoid this issue. | Modulator Free mode expectation (branch request) |
| LFO retrigger mode does not restart phase on gate events | Done | High | Fixed in v0.0.75: Retrigger mode now properly resets phase to 0 on gate rising edge; Sync mode runs continuously without reset. | Modulator retrig expectation (branch request) |
| Modulator waveform Y-axis indicator is too thin | Done | Medium | Thickened centerline and verified in simulator. | Modulator scope visibility feedback |
| LFO waveform dead-zone near extrema | Ready for HW Test | High | Triangle/saw generator now spans full range; verify with MIDI monitor and close if smooth. | Modulator FM workflow feedback |
| ADSR envelope timing inaccurate and release inactive | In Progress | High | Engine now honours per-stage ms and UI playhead uses clock timing; verify against hardware then mark resolved. | ADSR modulation feedback |
| Keyboard focus window skips black keys | New | Medium | Rework keyboard page focus navigation so chromatic scrolling includes sharps/flats. | Keyboard feature branch |
| ^Keyboard note ordering incorrect | New | High | Fix chromatic output order so white/black keys emit C, C#, D, D#, … (current sequence repeats sharps). | Keyboard feature branch |
| Red LEDs not lighting all steps | New | Medium | Investigate column driver mapping so steps 1/5/8 respond when scrolling keyboard. | Keyboard feature branch |
| Keyboard info overlay placement | New | Low | Move footer text into main window with header/footer padding; relocate `T#`, root, octave, and last note to requested corners. | Keyboard feature branch |

^ indicates the bug may be caused by the item immediately above it.

## Backlog & Ideas

| Feature Idea | Status | Notes | Priority | Source |
|--------------|--------|-------|----------|--------|
| Increase pattern/song slot capacity beyond 8/4 | Icebox | Requires memory budget review after recent expansions. | P2 | README.md:43 |
| Additional branding polish | Icebox | Optional follow-up now that core branding strings are updated. | P1 | CHANGELOG.md:34 |

## New Feature Workflow

1. **Capture the need** – Record the user problem or goal and link back to the originating note (guide, changelog, issue).
2. **Add to the tracker** – Create or update an entry in *Active Feature Tasks* (or *Backlog* if deferred) with priority, owner, and next step.
3. **Plan implementation** – Outline affected modules/files and checkpoints (design, prototype, tests).
4. **Validate** – Ensure automated tests and, where relevant, hardware validation are completed before moving the item to done.
5. **Document** – Update `CHANGELOG.md` and user-facing docs when the feature ships, then remove or demote the tracker entry.

Use this checklist whenever a new feature request appears to keep development priorities visible alongside the technical notes that inspired them.
