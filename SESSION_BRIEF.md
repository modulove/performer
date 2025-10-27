# POW|FORMER v0.0.7 - Session Brief

## Current Status
**Build Complete**: Firmware compiled successfully (332 KB, 150.9 KB RAM used)
**Ready for Testing**: `UPDATE_16TRACKS_MINIMAL.DAT` on Desktop
**MD5**: ffc299ff5f05b5c463ad473868095a89

## Recent Changes (Just Completed)

### 1. Changed Gate Offset from Bidirectional to Delay-Only System
**Rationale**: User feedback indicated offsets always seemed positive, so simplified to unidirectional delay system
**Files Modified**:
- [NoteSequence.h:26,100-104](src/apps/sequencer/model/NoteSequence.h#L26) - Data type
- [NoteTrackEngine.cpp:314-326](src/apps/sequencer/engine/NoteTrackEngine.cpp#L314-L326) - Trigger calculation
- [NoteSequenceEditPage.cpp:719-732](src/apps/sequencer/ui/pages/NoteSequenceEditPage.cpp#L719-L732) - Display

**Changes**:
```cpp
// OLD: Bidirectional offset
using GateOffset = SignedValue<7>;  // -63 to +63 (early/late)

// NEW: Delay-only offset
using GateOffset = UnsignedValue<6>;  // 0 to 63 (0=on beat, 63=just before next)
```

**Engine Logic**:
```cpp
// Simplified calculation - all positive delays
int32_t gateOffset = (int32_t(divisor) * step.gateOffset()) / NoteSequence::GateOffset::Max;
uint32_t triggerTick = tick + uint32_t(gateOffset);
```

**Recording Logic**: Captures only positive delays (clamps negative to 0):
```cpp
int gateOffset = (timingOffset * NoteSequence::GateOffset::Max) / int32_t(divisor);
step.setGateOffset(std::max(0, gateOffset));
```

**Display**: Shows "0" to "63" instead of "-63" to "+63"

### 2. Updated Visual Offset Display (Previous Session)
**File**: [SequencePainter.cpp:14-57](src/apps/sequencer/ui/painters/SequencePainter.cpp#L14-L57)
- Removed grid markers for cleaner visualization
- Shows only center line and current value bar

## Features Implemented (POW|FORMER v0.0.7)

### 1. Micro-Timing System ✓
- **Gate Offset**: 0-63 delay system (0=on beat, 63=just before next beat)
- **Resolution**: 64th note grid (Max = 63)
- **Display**: Absolute value 0-63 with visual bar
- **Recording**: Capture Timing toggle per track (records positive delays only)
- **Location**: Track Page → Polyphony / Capture Timing settings

### 2. Polyphonic Voice System (Partial)
- **Setting**: 0-4 voices per track (Track Page → Polyphony)
- **Engine**: Voice allocation with round-robin stealing implemented
- **Status**: Infrastructure complete, needs hardware testing
- **Files**:
  - [NoteTrack.h:265-297](src/apps/sequencer/model/NoteTrack.h#L265-L297) - Properties
  - [NoteTrackEngine.h:108-115](src/apps/sequencer/engine/NoteTrackEngine.h#L108-L115) - Voice tracking
  - [NoteTrackEngine.cpp:438-474](src/apps/sequencer/engine/NoteTrackEngine.cpp#L438-L474) - Voice allocation

### 3. Firmware Branding ✓
- **Version**: 0.0.7
- **Name**: POW|FORMER SEQUENCER
- **Boot Screen**: "POW|FORMER" / "Modulove Artificial FW"
- **File**: [Config.h](src/apps/sequencer/Config.h), [StartupPage.cpp](src/apps/sequencer/ui/pages/StartupPage.cpp)

### 4. 16-Track Configuration ✓
- **Tracks**: Increased from 8 to 16
- **Routes**: Reduced from 16 to 4 (to save RAM)
- **Song Slots**: Reduced from 16 to 4 (to save RAM)
- **RAM Usage**: 150.9 KB / 192 KB (78%)

## Known Issues to Test

### Issue 1: Negative Offset Not Working (Just Fixed)
**User Report**: "adjusting the trigger/gate offset into negative range does not actually change the trigger output"
**Fix Applied**: int64_t signed arithmetic now allows negative trigger ticks
**Action**: Test on hardware - notes should trigger early with negative offset

### Issue 2: Polyphony Not Working (Expected)
**User Report**: "does not actually allow for more than [one] note to be recorded and played back at a time"
**Status**: Voice allocation engine implemented but needs testing
**Expected Behavior**: Setting Polyphony to 2-4 should allow multiple simultaneous notes
**Action**: Test on hardware - verify polyphonic MIDI playback works

## Quick Reference

### Build Commands
```bash
~/build-performer.sh
```
Output: `~/Desktop/UPDATE_16TRACKS_MINIMAL.DAT`

### Memory Analysis
```bash
arm-none-eabi-size ~/performer-build/build/stm32/release/src/apps/sequencer/sequencer.elf
```

### Key File Locations
- **Config**: [src/apps/sequencer/Config.h](src/apps/sequencer/Config.h)
- **NoteSequence**: [src/apps/sequencer/model/NoteSequence.h](src/apps/sequencer/model/NoteSequence.h)
- **NoteTrack**: [src/apps/sequencer/model/NoteTrack.h](src/apps/sequencer/model/NoteTrack.h)
- **Engine**: [src/apps/sequencer/engine/NoteTrackEngine.cpp](src/apps/sequencer/engine/NoteTrackEngine.cpp)
- **UI**: [src/apps/sequencer/ui/pages/NoteSequenceEditPage.cpp](src/apps/sequencer/ui/pages/NoteSequenceEditPage.cpp)

## Documentation Created
- [CHANGELOG_POWFORMER.md](CHANGELOG_POWFORMER.md) - Technical changelog
- [POLY_TRACK_IMPLEMENTATION_GUIDE.md](POLY_TRACK_IMPLEMENTATION_GUIDE.md) - Polyphony implementation guide
- [POWFORMER_USER_GUIDE.md](POWFORMER_USER_GUIDE.md) - User documentation
- [FILES_MODIFIED.md](FILES_MODIFIED.md) - File reference

## Next Steps (If Testing Reveals Issues)

### If Negative Offset Still Doesn't Work:
1. Check if `gateOffset` calculation is correct for divisor values
2. Verify `_gateQueue` processes early trigger ticks properly
3. Add debug output to confirm triggerTick values
4. Consider if timing needs to wrap around to previous step

### If Polyphony Doesn't Work:
1. Verify voice allocation is being called in polyphonic mode
2. Check MIDI gate output handles multiple voices
3. Confirm voice expiry logic releases voices correctly
4. May need separate MIDI note-on/off per voice

### Future Enhancements (Not Started):
- Full PolyTrack type with piano roll UI
- Polyphonic recording (currently only monophonic)
- Visual multi-note display system
- Per-voice parameter controls

## Technical Context

**Hardware**: STM32F405RGT6 (1MB Flash, 192KB RAM)
**Framework**: FreeRTOS + custom sequencer engine
**Data Format**: Bitfield-based structures (NoteSequence::Step uses 2× uint32_t)
**Timing**: Tick-based with divisor (96 PPQ internal clock)
**Version**: ProjectVersion 27 (for polyphony/capture timing serialization)

## Git Status
Many modified files, main changes in:
- Config.h, NoteSequence.h, NoteTrack.h/cpp
- NoteTrackEngine.h/cpp, MidiOutputEngine.h/cpp
- NoteSequenceEditPage.cpp, SequencePainter.h/cpp
- StartupPage.cpp, Track page models

Untracked new files:
- ModulatorPage.h/cpp (placeholder)
- FlashPatternStorage.h/cpp (reverted/unused)
- Modulator.h (placeholder)
- PolySequence.h (created but not integrated)
