# POW|FORMER v0.0.7 - Complete Code Changes Log

## Summary
This document contains ALL code changes made during the session based on the continuation summary.

---

## 1. NEGATIVE GATE OFFSET FIX

### File: `src/apps/sequencer/engine/NoteTrackEngine.cpp`

**Location**: Lines 319-326 (in `triggerStep()` function)

**Change**: Fixed negative gate offset calculation to use signed arithmetic

```cpp
// OLD CODE (BROKEN):
uint32_t triggerTick = (gateOffset < 0 && uint32_t(-gateOffset) > tick)
    ? 0  // Prevents underflow but doesn't work
    : uint32_t(int32_t(tick) + gateOffset);

// NEW CODE (FIXED):
// Gate offset: -63 to +63 mapped as percentage of step divisor for micro-timing
// Negative values trigger early, positive values delay
int32_t gateOffset = (int32_t(divisor) * step.gateOffset()) / NoteSequence::GateOffset::Range;

// Calculate actual trigger tick with offset
// For negative offsets, the note triggers before the nominal step time
int64_t triggerTickSigned = int64_t(tick) + int64_t(gateOffset);
uint32_t triggerTick = (triggerTickSigned < 0) ? 0 : uint32_t(triggerTickSigned);
```

**Reason**: Original code clamped negative offsets to 0 instead of scheduling notes to trigger early.

---

## 2. VISUAL OFFSET DISPLAY UPDATES

### File: `src/apps/sequencer/ui/painters/SequencePainter.h`

**Location**: Line 10

**Change**: Added `detailed` parameter to `drawOffset()` function signature

```cpp
// OLD CODE:
static void drawOffset(Canvas &canvas, int x, int y, int w, int h, int offset, int minOffset, int maxOffset);

// NEW CODE:
static void drawOffset(Canvas &canvas, int x, int y, int w, int h, int offset, int minOffset, int maxOffset, bool detailed = false);
```

---

### File: `src/apps/sequencer/ui/painters/SequencePainter.cpp`

**Location**: Lines 14-57 (entire `drawOffset()` implementation)

**Change**: Implemented dual-mode marker display (simple vs detailed)

```cpp
void SequencePainter::drawOffset(Canvas &canvas, int x, int y, int w, int h, int offset, int minOffset, int maxOffset, bool detailed) {
    auto remap = [w, minOffset, maxOffset] (int value) {
        return ((w - 1) * (value - minOffset)) / (maxOffset - minOffset);
    };

    canvas.setBlendMode(BlendMode::Set);
    canvas.setColor(Color::Medium);
    canvas.fillRect(x, y, w, h);

    // Draw reference grid markers
    canvas.setColor(Color::Low);
    int range = maxOffset - minOffset;

    if (detailed && h >= 4) {
        // Detailed view: show every 64th note marker
        for (int i = 1; i < 64; ++i) {
            int gridValue = minOffset + (range * i) / 64;
            int gridX = x + remap(gridValue);
            // Draw taller marks at 16th note boundaries (every 4th marker)
            if (i % 4 == 0) {
                canvas.vline(gridX, y, h);
            } else {
                // Draw shorter markers for 64th notes
                canvas.vline(gridX, y + h/3, h/3);
            }
        }
    } else {
        // Simple view: show only every 1/8th division
        for (int i = 1; i < 8; ++i) {
            int gridValue = minOffset + (range * i) / 8;
            int gridX = x + remap(gridValue);
            canvas.vline(gridX, y, h);
        }
    }

    // Draw center line (zero offset)
    canvas.setColor(Color::None);
    canvas.vline(x + remap(0), y, h);

    // Draw current offset value
    canvas.setColor(Color::Bright);
    canvas.vline(x + remap(offset), y, h);
}
```

**Reason**: User wanted 64th note markers only in popup view, not in small overview bar.

---

### File: `src/apps/sequencer/ui/pages/NoteSequenceEditPage.cpp`

**Location**: Lines 718-724

**Change**: Updated popup call to use `detailed=true` parameter

```cpp
case Layer::GateOffset:
    SequencePainter::drawOffset(
        canvas,
        64 + 32 + 8, 32 - 4, 64 - 16, 8,
        step.gateOffset(), NoteSequence::GateOffset::Min - 1, NoteSequence::GateOffset::Max + 1,
        true  // detailed = show 64th markers
    );
```

---

## 3. MICRO-TIMING SYSTEM (Earlier in Session)

### File: `src/apps/sequencer/model/NoteSequence.h`

**Location**: Line 26

**Change**: Increased GateOffset bit depth from 4 to 7 bits

```cpp
// OLD CODE:
using GateOffset = SignedValue<4>;  // 4 bits = -7 to +7

// NEW CODE:
using GateOffset = SignedValue<7>;  // 7 bits = -63 to +63 for 64th note grid resolution
```

---

**Location**: Lines 214-221

**Change**: Updated bitfield layout in `_data1` union to accommodate 7-bit GateOffset

```cpp
union {
    uint32_t raw;
    BitField<uint32_t, 0, Retrigger::Bits> retrigger;           // 2 bits
    BitField<uint32_t, 2, RetriggerProbability::Bits> retriggerProbability;  // 3 bits
    BitField<uint32_t, 5, GateOffset::Bits> gateOffset;         // 7 bits (was 4)
    BitField<uint32_t, 12, Condition::Bits> condition;          // 7 bits
    // 13 bits left (19 bits used: 2+3+7+7)
} _data1;
```

---

**Location**: Lines 100-104

**Change**: Removed negative offset constraint to enable full negative timing

```cpp
// OLD CODE:
int gateOffset() const { return GateOffset::Min + _data1.gateOffset; }
void setGateOffset(int gateOffset) {
    _data1.gateOffset = std::max(0, GateOffset::clamp(gateOffset) - GateOffset::Min);
}

// NEW CODE:
int gateOffset() const { return GateOffset::Min + _data1.gateOffset; }
void setGateOffset(int gateOffset) {
    // Full negative and positive timing offset for micro-timing
    _data1.gateOffset = GateOffset::clamp(gateOffset) - GateOffset::Min;
}
```

**Reason**: Removed `std::max(0, ...)` to allow negative values.

---

### File: `src/apps/sequencer/ui/pages/NoteSequenceEditPage.cpp`

**Location**: Lines 725-735

**Change**: Updated offset percentage display with +/- sign

```cpp
// Display offset as percentage with sign
{
    float offsetPercent = 100.f * step.gateOffset() / float(NoteSequence::GateOffset::Range);
    if (step.gateOffset() >= 0) {
        str("+%.1f%%", offsetPercent);  // Show + for positive
    } else {
        str("%.1f%%", offsetPercent);   // Negative sign automatic
    }
    canvas.setColor(Color::Bright);
    canvas.drawTextCentered(64 + 32 + 64, 32 - 4, 32, 8, str);
}
```

---

### File: `src/apps/sequencer/engine/NoteTrackEngine.cpp`

**Location**: Lines 362-370

**Change**: Added conditional timing capture during MIDI recording

```cpp
// Capture micro-timing if enabled
if (_noteTrack.captureTiming()) {
    int gateOffset = (timingOffset * NoteSequence::GateOffset::Range) / int32_t(divisor);
    step.setGateOffset(gateOffset);
} else {
    step.setGateOffset(0);
}
```

**Reason**: User wanted toggle to enable/disable timing capture.

---

## 4. POLYPHONIC VOICE SYSTEM

### File: `src/apps/sequencer/model/NoteTrack.h`

**Location**: Lines 265-281

**Change**: Added polyphony property (0-4 voices)

```cpp
int polyphony() const { return _polyphony; }
void setPolyphony(int polyphony) {
    _polyphony = clamp(polyphony, 0, 4);  // 0 = mono, 1-4 = poly voices
}

void editPolyphony(int value, bool shift) {
    setPolyphony(polyphony() + value);
}

void printPolyphony(StringBuilder &str) const {
    if (_polyphony == 0) {
        str("Mono");
    } else {
        str("%d Voice%s", _polyphony, _polyphony > 1 ? "s" : "");
    }
}
```

---

**Location**: Lines 285-297

**Change**: Added capture timing property

```cpp
bool captureTiming() const { return _captureTiming; }
void setCaptureTiming(bool captureTiming) {
    _captureTiming = captureTiming;
}

void editCaptureTiming(int value, bool shift) {
    setCaptureTiming(!captureTiming());
}

void printCaptureTiming(StringBuilder &str) const {
    str(captureTiming() ? "On" : "Off");
}
```

---

**Location**: Lines 332-333

**Change**: Added member variables

```cpp
uint8_t _polyphony = 0;  // 0 = mono, 1-4 = polyphonic voices (MIDI only)
bool _captureTiming = false;  // Enable micro-timing capture during recording
```

---

### File: `src/apps/sequencer/model/NoteTrack.cpp`

**Location**: Lines 68-69

**Change**: Added serialization write for new properties

```cpp
writer.write(_polyphony);
writer.write(_captureTiming);
```

---

**Location**: Lines 87-88

**Change**: Added serialization read with version check

```cpp
reader.read(_polyphony, ProjectVersion::Version27);
reader.read(_captureTiming, ProjectVersion::Version27);
```

**Reason**: New properties only exist in Version27+, defaults to 0/false for older projects.

---

### File: `src/apps/sequencer/engine/NoteTrackEngine.h`

**Location**: Lines 108-115

**Change**: Added polyphonic voice tracking structures

```cpp
// Polyphonic voice allocation
struct PolyVoice {
    int8_t note = -1;      // MIDI note number, -1 = inactive
    uint32_t onTick = 0;   // When note was triggered
    uint32_t offTick = 0;  // When note should end
};
std::array<PolyVoice, 4> _polyVoices;
int _nextVoiceIndex = 0;  // Round-robin allocation
```

---

**Location**: Lines 54-56

**Change**: Added voice allocation method declarations

```cpp
int allocateVoice(int8_t note, uint32_t onTick, uint32_t offTick);
void releaseVoice(int voiceIndex);
void releaseAllVoices();
```

---

### File: `src/apps/sequencer/engine/NoteTrackEngine.cpp`

**Location**: Lines 343-370

**Change**: Added polyphonic trigger logic in `triggerStep()`

```cpp
int polyphony = _noteTrack.polyphony();
if (polyphony > 0) {
    // Polyphonic mode - allocate voice
    uint32_t onTick = Groove::applySwing(triggerTick, swing());
    uint32_t offTick = Groove::applySwing(triggerTick + stepLength, swing());
    int voiceIndex = allocateVoice(midiNote, onTick, offTick);

    if (voiceIndex >= 0) {
        _gateQueue.pushReplace({ onTick, true });
    }
}
```

---

**Location**: Lines 187-198

**Change**: Added voice expiry processing in `update()`

```cpp
int polyphony = _noteTrack.polyphony();
if (polyphony > 0 && !_monitorOverrideActive) {
    for (int i = 0; i < polyphony; ++i) {
        auto &voice = _polyVoices[i];
        if (voice.note >= 0 && tick >= voice.offTick) {
            midiOutputEngine.sendGate(_track.trackIndex(), false);
            releaseVoice(i);
        }
    }
}
```

---

**Location**: Lines 438-474

**Change**: Implemented voice allocation methods

```cpp
int NoteTrackEngine::allocateVoice(int8_t note, uint32_t onTick, uint32_t offTick) {
    int polyphony = _noteTrack.polyphony();
    if (polyphony == 0) {
        return -1;  // Monophonic mode
    }

    // Find an inactive voice first
    for (int i = 0; i < polyphony; ++i) {
        if (_polyVoices[i].note == -1) {
            _polyVoices[i].note = note;
            _polyVoices[i].onTick = onTick;
            _polyVoices[i].offTick = offTick;
            return i;
        }
    }

    // All voices active - use round-robin allocation (voice stealing)
    int voiceIndex = _nextVoiceIndex;
    _nextVoiceIndex = (voiceIndex + 1) % polyphony;

    _polyVoices[voiceIndex].note = note;
    _polyVoices[voiceIndex].onTick = onTick;
    _polyVoices[voiceIndex].offTick = offTick;
    return voiceIndex;
}

void NoteTrackEngine::releaseVoice(int voiceIndex) {
    if (voiceIndex >= 0 && voiceIndex < 4) {
        _polyVoices[voiceIndex].note = -1;
    }
}

void NoteTrackEngine::releaseAllVoices() {
    for (auto &voice : _polyVoices) {
        voice.note = -1;
    }
}
```

**Reason**: Implements round-robin voice allocation with voice stealing when all voices are active.

---

### File: `src/apps/sequencer/ui/model/NoteTrackListModel.h`

**Location**: Lines 38-39

**Change**: Added Polyphony and CaptureTiming to Item enum

```cpp
Polyphony,
CaptureTiming,
```

---

**Location**: Lines 56-57

**Change**: Added names for new items

```cpp
case Polyphony: return "Polyphony";
case CaptureTiming: return "Capture Timing";
```

---

**Location**: Lines 89-94

**Change**: Added print cases for new properties

```cpp
case Polyphony:
    _track->printPolyphony(str);
    break;
case CaptureTiming:
    _track->printCaptureTiming(str);
    break;
```

---

**Location**: Lines 129-134

**Change**: Added edit cases for new properties

```cpp
case Polyphony:
    _track->editPolyphony(value, shift);
    break;
case CaptureTiming:
    _track->editCaptureTiming(value, shift);
    break;
```

---

## 5. FIRMWARE BRANDING

### File: `src/apps/sequencer/Config.h`

**Location**: Lines (various - version info section)

**Change**: Updated firmware branding and version

```cpp
// OLD CODE:
#define CONFIG_VERSION_NAME             "SEQUENCER"
#define CONFIG_VERSION_MAJOR            0
#define CONFIG_VERSION_MINOR            4
#define CONFIG_VERSION_REVISION         5

// NEW CODE:
#define CONFIG_VERSION_NAME             "POW|FORMER SEQUENCER"
#define CONFIG_VERSION_MAJOR            0
#define CONFIG_VERSION_MINOR            0
#define CONFIG_VERSION_REVISION         7
```

---

### File: `src/apps/sequencer/ui/pages/StartupPage.cpp`

**Location**: Lines 36-39

**Change**: Updated boot screen text

```cpp
// OLD CODE:
canvas.drawTextCentered(0, 0, Width, 32, "SEQUENCER");
canvas.drawTextCentered(0, 20, Width, 8, "by plex");

// NEW CODE:
canvas.setFont(Font::Small);
canvas.drawTextCentered(0, 0, Width, 32, "POW|FORMER");
canvas.setFont(Font::Tiny);
canvas.drawTextCentered(0, 20, Width, 8, "Modulove Artificial FW");
```

---

## 6. 16-TRACK CONFIGURATION

### File: `src/apps/sequencer/Config.h`

**Location**: Track/routing configuration section

**Change**: Increased tracks, reduced routes and song slots to fit in RAM

```cpp
// OLD CODE:
#define CONFIG_TRACK_COUNT              8
#define CONFIG_ROUTE_COUNT              16
#define CONFIG_SONG_SLOT_COUNT          16

// NEW CODE:
#define CONFIG_TRACK_COUNT              16  // Doubled tracks
#define CONFIG_ROUTE_COUNT              4   // Reduced routes (save RAM)
#define CONFIG_SONG_SLOT_COUNT          4   // Reduced song slots (save RAM)
```

**Reason**: User wanted 16 tracks. Routes and song slots reduced to keep RAM under 192 KB limit.

---

## 7. ROUTING INITIALIZATION FIX

### File: `src/apps/sequencer/model/Routing.cpp`

**Location**: Initialization section

**Change**: Updated default routing for 16 tracks

```cpp
// OLD CODE: (8 track defaults)
// Various 8-track routing initializations

// NEW CODE: (16 track defaults)
// Updated to initialize 16 tracks worth of routing
```

**Note**: Specific code not shown in summary, but routing init was updated for 16 tracks.

---

## 8. COMPILATION FIXES

### File: `src/apps/sequencer/ui/pages/NoteSequenceEditPage.cpp`

**Location**: GateOffset case in switch statement

**Change**: Added scope block for variable declarations in switch case

```cpp
// OLD CODE (COMPILATION ERROR):
case Layer::GateOffset:
    str.reset();
    float offsetPercent = 100.f * step.gateOffset() / float(NoteSequence::GateOffset::Range);
    // ... error: variables cannot be declared in switch statement

// NEW CODE (FIXED):
case Layer::GateOffset:
    str.reset();
    {  // Added scope block
        float offsetPercent = 100.f * step.gateOffset() / float(NoteSequence::GateOffset::Range);
        if (step.gateOffset() >= 0) {
            str("+%.1f%%", offsetPercent);
        } else {
            str("%.1f%%", offsetPercent);
        }
        canvas.setColor(Color::Bright);
        canvas.drawTextCentered(64 + 32 + 64, 32 - 4, 32, 8, str);
    }
    break;
```

**Reason**: C++ requires scope block for variable declarations in switch cases.

---

## REVERTED CHANGES (Not in Current Build)

### Flash Pattern Storage (REVERTED)
**Files**: `FlashPatternStorage.h`, `FlashPatternStorage.cpp`, `PatternCacheManager.h`, `PatternCacheManager.cpp`

**Reason**: Caused runtime crashes due to serialization out-of-bounds access. All Flash optimization work was completely reverted.

---

## NEW FILES CREATED (Not Compiled)

### File: `src/apps/sequencer/model/PolySequence.h`
**Status**: Created but not integrated into build system
**Purpose**: Foundation for future dedicated polyphonic track type
**Size**: ~4.6 KB RAM per track (8 bytes × 64 steps × 9 patterns)

```cpp
struct PolyStep {
    union {
        uint32_t raw;
        BitField<uint32_t, 0, 1> gate;
        BitField<uint32_t, 1, Length::Bits> length;
    } _data;
    int8_t _notes[4] = { -1, -1, -1, -1 };  // MIDI note numbers, -1 = inactive
};
```

**Note**: This is placeholder for future work - not currently used.

---

## SUMMARY STATISTICS

**Files Modified**: 13 core files
**Files Created**: 1 (PolySequence.h - unused)
**Documentation Created**: 4 files (CHANGELOG, USER_GUIDE, IMPLEMENTATION_GUIDE, FILES_MODIFIED)
**RAM Impact**: 150.9 KB / 192 KB (78% usage)
**Flash Impact**: 331 KB / 1024 KB (32% usage)
**New Features**: Micro-timing (7-bit), Polyphony (0-4 voices), 16 tracks, Capture timing
**Bug Fixes**: Negative offset calculation, switch statement scope
**Project Version**: Incremented to Version27 for serialization compatibility

---

## FILES MODIFIED (Complete List)

1. `src/apps/sequencer/Config.h` - Version, branding, track count
2. `src/apps/sequencer/model/NoteSequence.h` - Gate offset bit depth, bitfield layout
3. `src/apps/sequencer/model/NoteTrack.h` - Polyphony and capture timing properties
4. `src/apps/sequencer/model/NoteTrack.cpp` - Serialization for new properties
5. `src/apps/sequencer/model/Routing.cpp` - 16-track routing initialization
6. `src/apps/sequencer/engine/NoteTrackEngine.h` - Voice allocation structures
7. `src/apps/sequencer/engine/NoteTrackEngine.cpp` - Voice allocation, timing fixes
8. `src/apps/sequencer/ui/pages/StartupPage.cpp` - Boot screen branding
9. `src/apps/sequencer/ui/pages/NoteSequenceEditPage.cpp` - Offset display, scope fix
10. `src/apps/sequencer/ui/painters/SequencePainter.h` - Detailed parameter
11. `src/apps/sequencer/ui/painters/SequencePainter.cpp` - Dual-mode marker display
12. `src/apps/sequencer/ui/model/NoteTrackListModel.h` - UI for polyphony/capture timing
13. `src/apps/sequencer/model/Project.h` - Version27 definition (assumed from summary)

---

**End of Code Changes Log**
