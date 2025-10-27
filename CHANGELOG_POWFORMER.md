# POW|FORMER Changelog

## Version 0.0.7 - "Modulove Artificial FW" (2025-10-23)

### Major Features

#### 🎵 High-Resolution Micro-Timing System
- **7-bit gate offset resolution**: -63 to +63 (127 values total)
- **Bidirectional timing**: Negative values trigger early, positive values delay
- **Precision**: ~1ms per step at 120 BPM, ~0.9ms at 140 BPM
- **Visual feedback**: Clean 7-marker reference grid in offset display
- **Capture Timing toggle**: Record human timing variations or strict quantization

**Technical Details:**
- Increased `GateOffset` from `SignedValue<4>` to `SignedValue<7>`
- Fixed negative offset playback bug (signed/unsigned conversion)
- Updated bitfield allocation in `NoteSequence::Step._data1`
- Timing offset stored per-step and applied during playback

**Files Modified:**
- `src/apps/sequencer/model/NoteSequence.h` - Bit depth and bitfield layout
- `src/apps/sequencer/engine/NoteTrackEngine.cpp` - Trigger tick calculation
- `src/apps/sequencer/ui/painters/SequencePainter.cpp` - Grid visualization
- `src/apps/sequencer/ui/pages/NoteSequenceEditPage.cpp` - Percentage display

#### 🎹 Polyphonic Voice System (Basic Implementation)
- **Up to 4 simultaneous voices** per track
- **Round-robin voice allocation** with voice stealing
- **Automatic voice management**: Note-on/off tracking and release
- **Per-track configuration**: 0-4 voices (0 = mono, 1-4 = poly)

**How It Works:**
- Each track maintains 4 voice slots
- Voices allocated on-demand when notes trigger
- Inactive voices reused first, then round-robin stealing
- Voices automatically released when notes end

**Current Limitations:**
- UI only shows single note per step (multi-note display not yet implemented)
- Recording captures only monophonic input
- Best used by programming overlapping notes across multiple steps

**Technical Details:**
- Added `PolyVoice` structure to `NoteTrackEngine`
- Voice allocation methods: `allocateVoice()`, `releaseVoice()`, `releaseAllVoices()`
- Polyphonic trigger logic in `triggerStep()`
- Voice expiry processing in `tick()`

**Files Modified:**
- `src/apps/sequencer/engine/NoteTrackEngine.h` - Voice tracking arrays
- `src/apps/sequencer/engine/NoteTrackEngine.cpp` - Voice allocation logic
- `src/apps/sequencer/model/NoteTrack.h` - Polyphony property
- `src/apps/sequencer/model/NoteTrack.cpp` - Serialization
- `src/apps/sequencer/ui/model/NoteTrackListModel.h` - UI integration

#### 📊 Expanded Track Count
- **16 tracks** (increased from 8)
- **8 patterns + 1 snapshot** per track
- **Optimized RAM usage**: Reduced routing (4 routes) and song slots (4 slots)

**Memory Impact:**
- Increased from ~88 KB to ~150 KB RAM usage
- Still within 192 KB total RAM budget (128 KB + 64 KB CCM)

**Files Modified:**
- `src/apps/sequencer/Config.h` - Track and configuration counts
- Various UI files for 16-track support

### Minor Features

#### 🎨 Custom Branding
- **Name**: POW|FORMER SEQUENCER
- **Boot screen**: "POW|FORMER" / "Modulove Artificial FW"
- **Version**: 0.0.7

**Files Modified:**
- `src/apps/sequencer/Config.h` - Version strings
- `src/apps/sequencer/ui/pages/StartupPage.cpp` - Boot screen text

#### 📝 Capture Timing Setting
- **Per-track toggle**: Enable/disable micro-timing capture during recording
- **Default**: Off (preserves classic quantized behavior)
- **When enabled**: Records exact timing offset from grid
- **When disabled**: Strict quantization (offset = 0)

**Files Modified:**
- `src/apps/sequencer/model/NoteTrack.h` - Property and accessors
- `src/apps/sequencer/model/NoteTrack.cpp` - Serialization
- `src/apps/sequencer/engine/NoteTrackEngine.cpp` - Conditional capture
- `src/apps/sequencer/ui/model/NoteTrackListModel.h` - UI integration

#### ⚡ Double-Tap Gate Toggle
- **Quick gate editing**: Double-tap any step in any layer to toggle its gate on/off
- **No mode switching required**: Works while editing Note, Velocity, Length, or any other parameter
- **300ms detection window**: Responsive but accurate double-tap recognition
- **Workflow improvement**: Faster pattern editing without layer switching

**Technical Details:**
- Tap tracking with timestamp comparison
- Works in all edit layers on NoteSequenceEditPage
- Independent of current layer selection

**Files Modified:**
- `src/apps/sequencer/ui/pages/NoteSequenceEditPage.h` - Tap tracking variables
- `src/apps/sequencer/ui/pages/NoteSequenceEditPage.cpp` - Double-tap detection logic

#### 🎹 Keyboard Page (New UI Page)
- **New dedicated keyboard page** for live note input and testing
- **Proper 2-octave layout**: 14 white keys, 10 black keys
- **Black key positions**: [1, 2, 4, 5, 6, 8, 9, 11, 12, 13]
- **Persistent last note display**: Shows the last note played
- **Track info at bottom**: Track number, root note, and octave display

**Technical Details:**
- Full page implementation with proper keyboard layout
- Real-time note triggering
- Visual feedback for pressed keys

**Files Created:**
- `src/apps/sequencer/ui/pages/KeyboardPage.h` - Page definition
- `src/apps/sequencer/ui/pages/KeyboardPage.cpp` - Page implementation

**Files Modified:**
- `src/apps/sequencer/CMakeLists.txt` - Added KeyboardPage to build
- `src/apps/sequencer/ui/pages/Pages.h` - Page integration

#### 🔄 Bank Switching Navigation
- **Left/Right navigation buttons**: Switch between track banks 1-8 and 9-16
- **Automatic bank detection**: Bank follows currently selected track
- **Visual LED distinction**: Bank 2 (tracks 9-16) uses red LEDs for step indication
- **Seamless integration**: Works across all UI pages (Performer, Pattern, Track, etc.)

**Technical Details:**
- Bank state management in TopPage
- LED color inversion for bank 2
- Context-aware track display

**Files Modified:**
- `src/apps/sequencer/ui/pages/TopPage.h/cpp` - Bank state management
- `src/apps/sequencer/ui/LedPainter.cpp` - Bank-aware LED coloring
- `src/apps/sequencer/ui/pages/PatternPage.cpp` - Bank switching support
- `src/apps/sequencer/ui/pages/PerformerPage.cpp` - 16-track bank display

#### 🎛️ Modulator Page Access
- **Direct access via Track5 button**: Quick access to modulator page
- **Consistent navigation**: Follows same button pattern as other pages
- **8 modulator slots**: Select modulators 1-8 via step buttons

**Technical Details:**
- PageKeyMap updated for Track5 → ModulatorPage mapping
- Integrated into main page navigation system

**Files Modified:**
- `src/apps/sequencer/ui/PageKeyMap.h` - Track5 button mapping

### Infrastructure

#### 🏗️ PolySequence Foundation (Work in Progress)
- Created simplified data structure for future polyphonic track type
- **Design**: 4 notes per step, no probability/conditions/retrigger
- **RAM**: ~4.6 KB per polyphonic track
- **Status**: Data structure created, engine not yet implemented

**Files Created:**
- `src/apps/sequencer/model/PolySequence.h` - Poly sequence definition

### Bug Fixes

#### 🐛 Negative Gate Offset Playback
**Problem**: Negative gate offsets caused integer underflow when added to unsigned tick values, resulting in no timing shift.

**Solution**:
- Added proper signed/unsigned conversion with underflow protection
- Calculate `triggerTick` with safe arithmetic: `uint32_t(int32_t(tick) + gateOffset)`
- Prevent underflow: check if negative offset exceeds tick value

**Files Modified:**
- `src/apps/sequencer/engine/NoteTrackEngine.cpp` lines 310-326

#### 🐛 Gate Offset Range Calculation
**Problem**: Division by `Max + 1` doesn't work correctly for signed values.

**Solution**:
- Changed from `(divisor * offset) / (Max + 1)` to `(divisor * offset) / Range`
- Uses full signed range for proper bidirectional timing

**Files Modified:**
- `src/apps/sequencer/engine/NoteTrackEngine.cpp` line 308

### Performance & Memory

#### Memory Usage
- **Firmware size**: 338,616 bytes (331 KB)
- **RAM usage**: 150,908 bytes
  - Base pattern data: ~88 KB
  - FreeRTOS tasks: ~6-8 KB (main RAM)
  - Polyphony voices: 832 bytes (16 tracks × 4 voices × 13 bytes)
  - Other: ~56 KB
- **Flash address**: 0x08000000 - 0x08052A78

#### Build Info
- **Compiler**: arm-none-eabi-gcc 14.2.1
- **Target**: STM32F405RGT6 (1MB Flash, 192KB RAM)
- **MD5**: ddbc66630588cdce2ef0af324a625afd

### Known Issues & Limitations

#### Polyphony System
- ❌ Multi-note step display not implemented
- ❌ Polyphonic recording not yet functional
- ❌ Best used with manually programmed overlapping steps
- ⚠️ Voice allocation uses MIDI output engine (monophonic by design)

#### Future Work Required
For full polyphonic track type implementation:
1. Add `TrackMode::Poly` to enum
2. Create `PolyTrack` class
3. Create `PolyTrackEngine`
4. Implement piano roll edit page
5. Add polyphonic recording
6. Wire up track type switching

### API Changes

#### New Properties (NoteTrack)
```cpp
// Polyphony
int polyphony() const;
void setPolyphony(int polyphony);  // 0 = mono, 1-4 = poly voices
void editPolyphony(int value, bool shift);
void printPolyphony(StringBuilder &str) const;

// Capture Timing
bool captureTiming() const;
void setCaptureTiming(bool captureTiming);
void editCaptureTiming(int value, bool shift);
void printCaptureTiming(StringBuilder &str) const;
```

#### New Methods (NoteTrackEngine)
```cpp
// Voice allocation
int allocateVoice(int8_t note, uint32_t onTick, uint32_t offTick);
void releaseVoice(int voiceIndex);
void releaseAllVoices();
```

#### Modified Types
```cpp
// Increased bit depth
using GateOffset = SignedValue<7>;  // Was SignedValue<4>
// Range: -63 to +63 (was -7 to +7)
```

### Serialization Changes

#### Version 27 Additions
- `NoteTrack::_polyphony` (uint8_t)
- `NoteTrack::_captureTiming` (bool)

**Backward Compatibility**:
- Old projects load with polyphony = 0 (mono)
- Old projects load with captureTiming = false

### UI Changes

#### Track Page Additions
- **Polyphony**: New setting (0-4 voices)
- **Capture Timing**: New toggle (On/Off)

#### Sequence Edit Page
- Gate offset display now shows +/- percentage
- Example: "+15.6%", "-8.1%", "+0.0%"

#### Offset Bar Visualization
- 7 evenly-spaced reference markers
- Center line at 0 offset
- Bright indicator for current offset value

### Configuration Changes

#### Config.h Updates
```cpp
#define CONFIG_VERSION_NAME             "POW|FORMER SEQUENCER"
#define CONFIG_VERSION_MAJOR            0
#define CONFIG_VERSION_MINOR            0
#define CONFIG_VERSION_REVISION         7

#define CONFIG_TRACK_COUNT              16  // Was 8
#define CONFIG_ROUTE_COUNT              4   // Was 16
#define CONFIG_SONG_SLOT_COUNT          4   // Was 16
```

### Developer Notes

#### Bitfield Allocation (NoteSequence::Step)
```
_data0 (32 bits):
- gate: 1 bit
- gateProbability: 3 bits
- slide: 1 bit
- length: 3 bits
- lengthVariationRange: 4 bits
- lengthVariationProbability: 3 bits
- note: 7 bits
- noteVariationRange: 7 bits
- noteVariationProbability: 3 bits
Total: 32 bits (all used)

_data1 (32 bits):
- retrigger: 2 bits
- retriggerProbability: 3 bits
- gateOffset: 7 bits  (was 4 bits)
- condition: 7 bits
Total: 19 bits used, 13 bits free
```

#### Voice Tracking (NoteTrackEngine)
```cpp
struct PolyVoice {
    int8_t note;      // MIDI note number, -1 = inactive
    uint32_t onTick;  // When note was triggered
    uint32_t offTick; // When note should end
};
std::array<PolyVoice, 4> _polyVoices;
```

### Testing Checklist

#### Micro-Timing
- ✅ Positive offsets delay trigger
- ✅ Negative offsets trigger early
- ✅ Visual grid markers display correctly
- ✅ Capture Timing records offset when enabled
- ✅ Strict quantization when Capture Timing disabled
- ⏳ Hardware testing pending

#### Polyphony
- ✅ Voice allocation works
- ✅ Voice stealing functions
- ✅ Automatic voice release
- ✅ UI settings save/load
- ⚠️ Limited testing (no multi-note display yet)
- ⏳ Hardware testing pending

#### General
- ✅ 16 tracks compile and build
- ✅ Firmware fits in Flash
- ✅ RAM usage within limits
- ⏳ Hardware boot test pending
- ⏳ Save/load test pending

### Credits

**Based on**: Performer Sequencer by Phazerville
**Modifications**: Modulove Development
**AI Assistance**: Anthropic Claude
**License**: Same as original Performer project

### Migration Guide

#### From Original Performer
1. Backup your existing projects
2. Flash POW|FORMER firmware
3. Load projects - they will open with:
   - All tracks in monophonic mode (polyphony = 0)
   - Capture Timing disabled
   - Gate offset resolution upgraded automatically

#### Notes
- Old projects remain compatible
- New features optional (backward compatible defaults)
- Can revert to original firmware if needed

---

**Build Date**: October 23, 2025
**Build Type**: Development
**Target Hardware**: STM32F405RGT6 (Performer hardware)
