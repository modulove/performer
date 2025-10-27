# POW|FORMER - Polyphonic Track Implementation Guide

## Overview

This guide details how to complete the polyphonic track type implementation for POW|FORMER. The foundation (`PolySequence.h`) has been created, but significant work remains to create a fully functional polyphonic track with piano roll editing.

## Current Status

### ✅ Completed
- `PolySequence.h` - Data structure for 4-note polyphonic steps
- Basic voice allocation system in `NoteTrackEngine`
- Polyphony setting in track properties

### ⏳ Remaining Work
- Add Poly track mode to Track system
- Create PolyTrack class
- Create PolyTrackEngine
- Implement piano roll edit page
- Add polyphonic recording
- Wire up track switching logic

## Architecture

### Data Model Hierarchy
```
PolySequence (DONE)
    └── PolySequence::Step
        ├── gate: bool
        ├── length: 0-7
        └── notes[4]: int8_t (-1 = inactive)

PolyTrack (TODO)
    ├── Track properties (volume, octave, etc.)
    └── PolySequenceArray[9] (8 patterns + 1 snapshot)

PolyTrackEngine (TODO)
    ├── Voice allocation
    ├── MIDI output
    └── Recording
```

## Step 1: Add Poly Track Mode

### File: `src/apps/sequencer/model/Track.h`

**Location**: Line 37-47

**Change**:
```cpp
enum class TrackMode : uint8_t {
    Note,
#if CONFIG_ENABLE_CURVE_TRACKS
    Curve,
#endif
#if CONFIG_ENABLE_MIDICV_TRACKS
    MidiCv,
#endif
    Poly,  // ADD THIS LINE
    Last,
    Default = Note
};
```

**Location**: Line 49-60

**Change**:
```cpp
static const char *trackModeName(TrackMode trackMode) {
    switch (trackMode) {
    case TrackMode::Note:   return "Note";
#if CONFIG_ENABLE_CURVE_TRACKS
    case TrackMode::Curve:  return "Curve";
#endif
#if CONFIG_ENABLE_MIDICV_TRACKS
    case TrackMode::MidiCv: return "MIDI/CV";
#endif
    case TrackMode::Poly:   return "Poly";  // ADD THIS LINE
    case TrackMode::Last:   break;
    }
    return nullptr;
}
```

## Step 2: Create PolyTrack Class

### File: `src/apps/sequencer/model/PolyTrack.h` (NEW FILE)

```cpp
#pragma once

#include "Config.h"
#include "PolySequence.h"
#include "Types.h"
#include "Routing.h"
#include "Serialize.h"

#include <array>
#include <cstdint>

class PolyTrack {
public:
    //----------------------------------------
    // Types
    //----------------------------------------

    typedef std::array<PolySequence, CONFIG_PATTERN_COUNT + CONFIG_SNAPSHOT_COUNT> PolySequenceArray;

    //----------------------------------------
    // Properties
    //----------------------------------------

    // trackMode (always Poly for this class)
    Track::TrackMode trackMode() const { return Track::TrackMode::Poly; }

    // name
    const char *name() const { return _name; }
    void setName(const char *name) {
        StringUtils::copy(_name, name, sizeof(_name));
    }

    // midiInput
    Types::MidiInputMode midiInputMode() const { return _midiInputMode; }
    void setMidiInputMode(Types::MidiInputMode mode) {
        _midiInputMode = ModelUtils::clampedEnum(mode);
    }

    // octave
    int octave() const { return _octave; }
    void setOctave(int octave) {
        _octave = clamp(octave, -10, 10);
    }

    // transpose
    int transpose() const { return _transpose; }
    void setTranspose(int transpose) {
        _transpose = clamp(transpose, -100, 100);
    }

    // sequences
    const PolySequenceArray &sequences() const { return _sequences; }
          PolySequenceArray &sequences()       { return _sequences; }

    const PolySequence &sequence(int index) const { return _sequences[index]; }
          PolySequence &sequence(int index)       { return _sequences[index]; }

    //----------------------------------------
    // Methods
    //----------------------------------------

    void clear() {
        setName("POLY");
        _midiInputMode = Types::MidiInputMode::Off;
        _octave = 0;
        _transpose = 0;
        for (auto &sequence : _sequences) {
            sequence.clear();
        }
    }

    void write(VersionedSerializedWriter &writer) const {
        writer.write(_name, NameLength + 1);
        writer.write(_midiInputMode);
        writer.write(_octave);
        writer.write(_transpose);
        writeArray(writer, _sequences);
    }

    void read(VersionedSerializedReader &reader) {
        reader.read(_name, NameLength + 1);
        reader.read(_midiInputMode);
        reader.read(_octave);
        reader.read(_transpose);
        readArray(reader, _sequences);
    }

private:
    static constexpr size_t NameLength = 8;

    char _name[NameLength + 1];
    Types::MidiInputMode _midiInputMode = Types::MidiInputMode::Off;
    int8_t _octave = 0;
    int8_t _transpose = 0;

    PolySequenceArray _sequences;
};
```

### File: `src/apps/sequencer/model/PolyTrack.cpp` (NEW FILE)

```cpp
#include "PolyTrack.h"

// Implementation is mostly in header (inline methods)
// Add any complex methods here if needed
```

## Step 3: Update Track Class to Include PolyTrack

### File: `src/apps/sequencer/model/Track.h`

**Add include** (after line 13):
```cpp
#include "PolyTrack.h"
```

**Add to union** (around line 200):
```cpp
union {
    NoteTrack _noteTrack;
#if CONFIG_ENABLE_CURVE_TRACKS
    CurveTrack _curveTrack;
#endif
#if CONFIG_ENABLE_MIDICV_TRACKS
    MidiCvTrack _midiCvTrack;
#endif
    PolyTrack _polyTrack;  // ADD THIS LINE
};
```

**Add accessor methods** (around line 150):
```cpp
const PolyTrack &polyTrack() const {
    SANITIZE_TRACK_MODE(_trackMode, TrackMode::Poly);
    return _polyTrack;
}
PolyTrack &polyTrack() {
    SANITIZE_TRACK_MODE(_trackMode, TrackMode::Poly);
    return _polyTrack;
}
```

### File: `src/apps/sequencer/model/Track.cpp`

**Update clear() method**:
```cpp
void Track::clear() {
    _trackMode = TrackMode::Note;
    _linkTrack = -1;
    _muted = false;
    _midiOutputs.clear();
    switch (_trackMode) {
    case TrackMode::Note:
        _noteTrack.clear();
        break;
#if CONFIG_ENABLE_CURVE_TRACKS
    case TrackMode::Curve:
        _curveTrack.clear();
        break;
#endif
#if CONFIG_ENABLE_MIDICV_TRACKS
    case TrackMode::MidiCv:
        _midiCvTrack.clear();
        break;
#endif
    case TrackMode::Poly:  // ADD THIS
        _polyTrack.clear();
        break;
    case TrackMode::Last:
        break;
    }
}
```

**Update setTrackMode() method**:
```cpp
void Track::setTrackMode(TrackMode trackMode) {
    if (trackMode != _trackMode) {
        _trackMode = trackMode;
        switch (_trackMode) {
        case TrackMode::Note:
            new (&_noteTrack) NoteTrack();
            _noteTrack.clear();
            break;
#if CONFIG_ENABLE_CURVE_TRACKS
        case TrackMode::Curve:
            new (&_curveTrack) CurveTrack();
            _curveTrack.clear();
            break;
#endif
#if CONFIG_ENABLE_MIDICV_TRACKS
        case TrackMode::MidiCv:
            new (&_midiCvTrack) MidiCvTrack();
            _midiCvTrack.clear();
            break;
#endif
        case TrackMode::Poly:  // ADD THIS
            new (&_polyTrack) PolyTrack();
            _polyTrack.clear();
            break;
        case TrackMode::Last:
            break;
        }
    }
}
```

**Update write() method**:
```cpp
void Track::write(VersionedSerializedWriter &writer) const {
    writer.write(_trackMode);
    writer.write(_linkTrack);
    writer.write(_muted);
    _midiOutputs.write(writer);

    switch (_trackMode) {
    case TrackMode::Note:
        _noteTrack.write(writer);
        break;
#if CONFIG_ENABLE_CURVE_TRACKS
    case TrackMode::Curve:
        _curveTrack.write(writer);
        break;
#endif
#if CONFIG_ENABLE_MIDICV_TRACKS
    case TrackMode::MidiCv:
        _midiCvTrack.write(writer);
        break;
#endif
    case TrackMode::Poly:  // ADD THIS
        _polyTrack.write(writer);
        break;
    case TrackMode::Last:
        break;
    }
}
```

**Update read() method** (similar to write):
```cpp
void Track::read(VersionedSerializedReader &reader) {
    reader.read(_trackMode);
    setTrackMode(_trackMode);
    reader.read(_linkTrack);
    reader.read(_muted);
    _midiOutputs.read(reader);

    switch (_trackMode) {
    case TrackMode::Note:
        _noteTrack.read(reader);
        break;
#if CONFIG_ENABLE_CURVE_TRACKS
    case TrackMode::Curve:
        _curveTrack.read(reader);
        break;
#endif
#if CONFIG_ENABLE_MIDICV_TRACKS
    case TrackMode::MidiCv:
        _midiCvTrack.read(reader);
        break;
#endif
    case TrackMode::Poly:  // ADD THIS
        _polyTrack.read(reader);
        break;
    case TrackMode::Last:
        break;
    }
}
```

## Step 4: Create PolyTrackEngine

### File: `src/apps/sequencer/engine/PolyTrackEngine.h` (NEW FILE)

```cpp
#pragma once

#include "Config.h"
#include "TrackEngine.h"
#include "SequenceState.h"
#include "SortedQueue.h"
#include "Groove.h"

#include "model/PolyTrack.h"

class PolyTrackEngine : public TrackEngine {
public:
    PolyTrackEngine(Engine &engine, Model &model, Track &track, const TrackEngine *linkedTrackEngine) :
        TrackEngine(engine, model, track, linkedTrackEngine),
        _polyTrack(track.polyTrack())
    {
        reset();
    }

    virtual Track::TrackMode trackMode() const override { return Track::TrackMode::Poly; }

    virtual void reset() override;
    virtual void restart() override;
    virtual TickResult tick(uint32_t tick) override;
    virtual void update(float dt) override;

    virtual void changePattern() override;

    virtual void monitorMidi(uint32_t track, const MidiMessage &message) override;

    virtual void clearMidiMonitoring() override;

    const PolySequence &sequence() const { return *_sequence; }

private:
    struct Voice {
        int8_t note = -1;
        uint32_t onTick = 0;
        uint32_t offTick = 0;
    };

    void triggerStep(uint32_t tick, uint32_t divisor);
    void updateVoices(uint32_t tick);

    PolyTrack &_polyTrack;
    PolySequence *_sequence = nullptr;

    SequenceState _sequenceState;

    std::array<Voice, 4> _voices;
    bool _activity = false;
};
```

### File: `src/apps/sequencer/engine/PolyTrackEngine.cpp` (NEW FILE)

```cpp
#include "PolyTrackEngine.h"
#include "Engine.h"
#include "MidiOutputEngine.h"

void PolyTrackEngine::reset() {
    _sequenceState.reset();
    _sequence = &_polyTrack.sequence(pattern());
    for (auto &voice : _voices) {
        voice.note = -1;
    }
    _activity = false;
}

void PolyTrackEngine::restart() {
    _sequenceState.reset();
}

void PolyTrackEngine::changePattern() {
    _sequence = &_polyTrack.sequence(pattern());
}

TickResult PolyTrackEngine::tick(uint32_t tick) {
    TickResult result = TickResult::None;

    const auto &sequence = *_sequence;

    bool running = _engine.state().running();
    if (!running) {
        return result;
    }

    _sequenceState.clockDivisor(_engine.clock().divisor() * sequence.divisor());
    const auto &state = _sequenceState.tick(tick, sequence.runMode(), sequence.firstStep(), sequence.lastStep());

    if (state.step >= 0) {
        uint32_t divisor = _sequenceState.divisor();
        triggerStep(tick, divisor);
    }

    updateVoices(tick);

    return result;
}

void PolyTrackEngine::triggerStep(uint32_t tick, uint32_t divisor) {
    const auto &sequence = *_sequence;
    const auto &step = sequence.step(_sequenceState.step());

    if (!step.gate()) {
        return;
    }

    // Calculate step length
    uint32_t stepLength = (divisor * step.length()) / PolySequence::Length::Max;

    // Allocate voices for each active note
    for (int i = 0; i < 4; ++i) {
        int8_t note = step.note(i);
        if (note < 0) continue;  // No note in this slot

        // Apply octave and transpose
        note += _polyTrack.octave() * 12;
        note += _polyTrack.transpose();

        // Find available voice
        int voiceIndex = -1;
        for (int v = 0; v < 4; ++v) {
            if (_voices[v].note < 0) {
                voiceIndex = v;
                break;
            }
        }

        if (voiceIndex >= 0) {
            _voices[voiceIndex].note = note;
            _voices[voiceIndex].onTick = tick;
            _voices[voiceIndex].offTick = tick + stepLength;

            // Send MIDI note-on
            float cv = (note - 60) * (1.f / 12.f);
            auto &midiOutputEngine = _engine.midiOutputEngine();
            midiOutputEngine.sendGate(_track.trackIndex(), true);
            midiOutputEngine.sendCv(_track.trackIndex(), cv);

            _activity = true;
        }
    }
}

void PolyTrackEngine::updateVoices(uint32_t tick) {
    auto &midiOutputEngine = _engine.midiOutputEngine();

    for (auto &voice : _voices) {
        if (voice.note >= 0 && tick >= voice.offTick) {
            // Send note-off
            midiOutputEngine.sendGate(_track.trackIndex(), false);
            voice.note = -1;
        }
    }
}

void PolyTrackEngine::update(float dt) {
    // Minimal update needed
}

void PolyTrackEngine::monitorMidi(uint32_t track, const MidiMessage &message) {
    // TODO: Implement polyphonic recording
}

void PolyTrackEngine::clearMidiMonitoring() {
    // TODO: Clear recording state
}
```

## Step 5: Create Piano Roll Edit Page

### File: `src/apps/sequencer/ui/pages/PolySequenceEditPage.h` (NEW FILE)

```cpp
#pragma once

#include "BasePage.h"
#include "ui/model/PolySequenceListModel.h"

class PolySequenceEditPage : public BasePage {
public:
    PolySequenceEditPage(PageManager &manager, PageContext &context);

    virtual void enter() override;
    virtual void exit() override;

    virtual void draw(Canvas &canvas) override;
    virtual void updateLeds(Leds &leds) override;

    virtual void keyDown(KeyEvent &event) override;
    virtual void keyUp(KeyEvent &event) override;
    virtual void keyPress(KeyPressEvent &event) override;
    virtual void encoder(EncoderEvent &event) override;

private:
    void drawPianoRoll(Canvas &canvas);
    void drawNotes(Canvas &canvas);

    int _selectedStep = 0;
    int _selectedNote = 60;  // Middle C
    int _octaveOffset = 0;   // For scrolling note range

    PolySequenceListModel _listModel;
};
```

### Implementation Notes for Piano Roll Page

**Display Layout**:
```
┌────────────────────────────────┐
│ C5  │█│ │█│ │ │█│ │█│ │█│ │ │  │
│ B4  │ │█│ │ │█│ │█│ │ │█│ │█│  │
│ A#4 │ │ │ │█│ │ │ │█│ │ │ │ │  │
│ A4  │█│ │ │ │█│ │█│ │█│ │ │█│  │
│ ...  (show 2 octaves, 24 notes) │
└────────────────────────────────┘
  Step 1-16 displayed horizontally
```

**Interaction**:
- **Encoder**: Move cursor (step/note)
- **Step keys 1-16**: Select step
- **Shift + Step**: Toggle note on/off at cursor
- **F1-F4**: Switch between 4 note layers
- **Turn + encoder**: Scroll octaves

## Step 6: Wire Up PolyTrackEngine in Engine

### File: `src/apps/sequencer/engine/Engine.cpp`

**Include header**:
```cpp
#include "PolyTrackEngine.h"
```

**In createTrackEngine() method**:
```cpp
TrackEngine *Engine::createTrackEngine(int trackIndex) {
    auto &track = _model.project().track(trackIndex);
    const TrackEngine *linkedTrackEngine = nullptr;
    if (track.linkTrack() >= 0) {
        linkedTrackEngine = _trackEngines[track.linkTrack()].get();
    }

    switch (track.trackMode()) {
    case Track::TrackMode::Note:
        return new NoteTrackEngine(*this, _model, track, linkedTrackEngine);
#if CONFIG_ENABLE_CURVE_TRACKS
    case Track::TrackMode::Curve:
        return new CurveTrackEngine(*this, _model, track, linkedTrackEngine);
#endif
#if CONFIG_ENABLE_MIDICV_TRACKS
    case Track::TrackMode::MidiCv:
        return new MidiCvTrackEngine(*this, _model, track, linkedTrackEngine);
#endif
    case Track::TrackMode::Poly:  // ADD THIS
        return new PolyTrackEngine(*this, _model, track, linkedTrackEngine);
    case Track::TrackMode::Last:
        break;
    }
    return nullptr;
}
```

## Step 7: Update CMakeLists.txt

### File: `src/apps/sequencer/CMakeLists.txt`

**Add to source files** (around line 25-35):
```cmake
# Model
model/PolySequence.h
model/PolyTrack.h
model/PolyTrack.cpp

# Engine
engine/PolyTrackEngine.h
engine/PolyTrackEngine.cpp

# UI Pages
ui/pages/PolySequenceEditPage.h
ui/pages/PolySequenceEditPage.cpp
```

## Step 8: Add Polyphonic Recording

### In PolyTrackEngine::monitorMidi()

```cpp
void PolyTrackEngine::monitorMidi(uint32_t track, const MidiMessage &message) {
    if (!_engine.state().recording()) {
        return;
    }

    // Track up to 4 simultaneous note-ons
    if (message.isNoteOn()) {
        int8_t note = message.note();
        // Find next available slot in recording buffer
        // Add to current step's note array
        // Handle chord detection (notes within ~50ms = same step)
    }

    if (message.isNoteOff()) {
        // Calculate gate length based on note-on to note-off time
        // Update step's length parameter
    }
}
```

## Memory Budget

### Per Polyphonic Track
- **PolySequence**: 64 steps × 8 bytes = 512 bytes
- **9 sequences**: 512 × 9 = 4,608 bytes
- **PolyTrack overhead**: ~50 bytes
- **PolyTrackEngine overhead**: ~100 bytes
- **Total per track**: ~4.8 KB

### For 2 Poly Tracks
- **Total RAM**: ~9.6 KB
- **Current usage**: 150.9 KB
- **After 2 poly tracks**: ~160.5 KB
- **Still within budget**: 192 KB total available

## Testing Plan

### Phase 1: Basic Functionality
1. Switch track 16 to Poly mode
2. Program a simple chord (C-E-G)
3. Verify playback
4. Verify MIDI output
5. Test voice allocation

### Phase 2: Piano Roll
1. Open poly sequence edit page
2. Verify note display
3. Test note input
4. Test cursor movement
5. Test octave scrolling

### Phase 3: Recording
1. Enable recording
2. Play chord on MIDI keyboard
3. Verify notes captured
4. Verify chord timing
5. Test polyphonic playback

### Phase 4: Edge Cases
1. Test >4 simultaneous notes (voice stealing)
2. Test overlapping notes
3. Test rapid chord changes
4. Test save/load
5. Test track mode switching

## Known Limitations

### Current Design
- Maximum 4 notes per step
- No note variation/probability
- No slide between chords
- No conditions
- No retrigger

### Future Enhancements
- Increase to 6 or 8 voices
- Add velocity per note
- Add chord templates
- Add arpeggiator mode
- Add chord transpose

## Rollback Plan

If implementation encounters issues:

1. **Remove from build**:
   - Comment out PolyTrack includes in Track.h
   - Comment out Poly case statements
   - Remove from CMakeLists.txt

2. **Disable track mode**:
   - Keep code but disable in TrackMode enum
   - Users won't see Poly option

3. **Revert completely**:
   - Delete PolySequence.h
   - Delete PolyTrack files
   - Delete PolyTrackEngine files
   - Delete PolySequenceEditPage files

## Next Steps

1. Implement Track integration (Step 3)
2. Create PolyTrackEngine (Step 4)
3. Test basic playback
4. Create piano roll page (Step 5)
5. Test UI interaction
6. Add recording support
7. Full integration testing
8. Hardware testing

## Questions & Decisions

### Q: Should polyphonic tracks share MIDI channel?
**A**: Yes, all voices on same MIDI channel is simpler and standard.

### Q: Limit to specific tracks?
**A**: Initially limit to tracks 15-16 to manage complexity and RAM.

### Q: What about CV output?
**A**: Polyphonic tracks are MIDI-only. CV outputs remain monophonic.

### Q: Backward compatibility?
**A**: Projects without poly tracks load normally. Track mode defaults to Note.

---

**Document Version**: 1.0
**Last Updated**: October 23, 2025
**Status**: Implementation Ready
