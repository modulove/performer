# POW|FORMER v0.6 - Major Feature Restoration & Enhancement

## Summary
This commit restores critical features that were lost from the POW|FORMER firmware and adds new UX improvements. The changes bring back the full 16-track dual-bank system, modulator/LFO engine, enhanced MIDI routing capabilities, and adds a new double-tap gate toggle feature for improved workflow.

## Features Restored/Added

### 1. 🎛️ **16-Track Dual Bank System**
- **Full 16-track support** with 2 banks of 8 tracks each
- **Bank switching** via Left/Right navigation buttons
- **Visual feedback**: Bank 1 (T1-8) uses green LEDs, Bank 2 (T9-16) uses red LEDs
- **Smart UI adaptation**: All pages automatically show the active bank
- **Files modified**:
  - `src/apps/sequencer/Config.h`: Set CONFIG_TRACK_COUNT to 16
  - `src/apps/sequencer/ui/LedPainter.cpp`: Bank-aware LED coloring
  - `src/apps/sequencer/ui/pages/TopPage.cpp/h`: Bank state management
  - `src/apps/sequencer/ui/pages/OverviewPage.cpp`: Bank-aware track display
  - `src/apps/sequencer/ui/pages/PatternPage.cpp`: Bank switching and display
  - `src/apps/sequencer/ui/pages/PerformerPage.cpp`: 16-track support

### 2. 🔊 **Modulator/LFO System**
- **8 independent modulators** with comprehensive parameters:
  - Waveform shapes (Sine, Triangle, Square, Ramp Up/Down, Random, Noise)
  - Rate, Depth, Offset, Phase, Source, Target
  - Event triggering and smoothing options
- **New files added**:
  - `src/apps/sequencer/model/Modulator.h`: Modulator model definition
  - `src/apps/sequencer/engine/ModulatorEngine.h`: Real-time modulator engine
  - `src/apps/sequencer/ui/pages/ModulatorPage.cpp/h`: UI for modulator control
- **Integration files**:
  - `src/apps/sequencer/CMakeLists.txt`: Added ModulatorPage to build
  - `src/apps/sequencer/ui/pages/Pages.h`: Integrated ModulatorPage
  - `src/apps/sequencer/engine/Engine.cpp/h`: ModulatorEngine integration
  - `src/apps/sequencer/ui/PageKeyMap.h`: Added Modulator page mapping

### 3. 🎹 **Enhanced MIDI Output Routing**
- **Expanded routing sources**:
  - 16 tracks (previously 8)
  - 8 modulators as MIDI CC sources
  - 4 CV inputs as MIDI CC sources
- **Files modified**:
  - `src/apps/sequencer/model/MidiOutput.h`: Extended ControlSource enum
  - `src/apps/sequencer/engine/MidiOutputEngine.cpp/h`: Added sendModulator() and sendCvIn() functions
  - `src/apps/sequencer/ui/pages/MidiOutputPage.cpp/h`: UI updates for new sources

### 4. 🎯 **Double-Tap Gate Toggle (NEW)**
- **UX Enhancement**: Double-tap any step in ANY edit layer to toggle its gate
- **No mode switching required**: Works while editing Note, Velocity, or any parameter
- **300ms double-tap window** for responsive but accurate detection
- **Files modified**:
  - `src/apps/sequencer/ui/pages/NoteSequenceEditPage.h`: Added tap tracking variables
  - `src/apps/sequencer/ui/pages/NoteSequenceEditPage.cpp`: Implemented double-tap logic

### 5. 🔧 **Curve Tracks Disabled**
- **Memory optimization**: CONFIG_ENABLE_CURVE_TRACKS set to 0
- **Proper guards added** to prevent compilation errors
- **Files fixed**:
  - `src/apps/sequencer/model/ClipBoard.cpp`: Added CONFIG_ENABLE_CURVE_TRACKS guards

## Technical Details

### Memory Impact
- Binary size: ~331KB (reduced from 359KB)
- Text segment: 332,532 bytes
- Data segment: 6,196 bytes
- BSS segment: 150,912 bytes

### Build Configuration
- Platform: STM32F4
- Compiler: ARM GCC 14.2.1
- Track count: 16
- Pattern count: 16
- MIDI outputs: 8
- Curve tracks: Disabled

## Files Changed

### Modified Files (38)
- CMakeLists.txt
- scripts/makeupdate
- src/apps/sequencer/CMakeLists.txt
- src/apps/sequencer/Config.h
- src/apps/sequencer/engine/Engine.cpp
- src/apps/sequencer/engine/Engine.h
- src/apps/sequencer/engine/MidiOutputEngine.cpp
- src/apps/sequencer/engine/MidiOutputEngine.h
- src/apps/sequencer/engine/NoteTrackEngine.h
- src/apps/sequencer/model/ClipBoard.cpp
- src/apps/sequencer/model/MidiOutput.h
- src/apps/sequencer/model/NoteSequence.h
- src/apps/sequencer/model/NoteTrack.cpp
- src/apps/sequencer/model/NoteTrack.h
- src/apps/sequencer/model/Project.h
- src/apps/sequencer/model/Routing.cpp
- src/apps/sequencer/model/Routing.h
- src/apps/sequencer/model/Track.cpp
- src/apps/sequencer/model/Track.h
- src/apps/sequencer/python/project.cpp
- src/apps/sequencer/ui/LedPainter.cpp
- src/apps/sequencer/ui/PageKeyMap.h
- src/apps/sequencer/ui/controllers/launchpad/LaunchpadController.cpp
- src/apps/sequencer/ui/model/NoteTrackListModel.h
- src/apps/sequencer/ui/pages/MidiOutputPage.cpp
- src/apps/sequencer/ui/pages/MidiOutputPage.h
- src/apps/sequencer/ui/pages/NoteSequenceEditPage.cpp
- src/apps/sequencer/ui/pages/NoteSequenceEditPage.h
- src/apps/sequencer/ui/pages/OverviewPage.cpp
- src/apps/sequencer/ui/pages/Pages.h
- src/apps/sequencer/ui/pages/PatternPage.cpp
- src/apps/sequencer/ui/pages/PerformerPage.cpp
- src/apps/sequencer/ui/pages/PerformerPage.h
- src/apps/sequencer/ui/pages/StartupPage.cpp
- src/apps/sequencer/ui/pages/TopPage.cpp
- src/apps/sequencer/ui/pages/TopPage.h
- src/apps/sequencer/ui/pages/TrackPage.cpp

### Added Files (4)
- src/apps/sequencer/model/Modulator.h
- src/apps/sequencer/engine/ModulatorEngine.h
- src/apps/sequencer/ui/pages/ModulatorPage.cpp
- src/apps/sequencer/ui/pages/ModulatorPage.h

## Testing Notes
- Firmware compiled successfully with all features
- Binary size optimized compared to previous versions
- All 16 tracks accessible via bank switching
- Modulator page accessible via Track5 button
- MIDI routing supports all new sources
- Double-tap gate toggle working in all edit layers

## Known Issues
- None identified in current build

## Version
POW|FORMER v0.6

## Contributors
- Restoration from Google Drive version history
- Feature implementation and bug fixes
- Claude Assistant for code recovery and implementation

---
*Firmware binary: UPDATE_16TRACKS_DOUBLETAP.DAT (331KB)*
*MD5: e4e41030b153caa3467a1e018992973a*