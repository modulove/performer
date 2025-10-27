# POW|FORMER v0.0.7 - User Guide

## What is POW|FORMER?

POW|FORMER is a modified version of the Performer sequencer firmware with enhanced features for advanced music production, developed by Modulove with AI assistance.

**Key Enhancements:**
- High-resolution micro-timing for humanized grooves
- Polyphonic voice allocation for chord sequencing
- 16 tracks (doubled from original 8)
- Advanced timing capture from live performance

## New Features

### 🎵 Micro-Timing System

#### What It Does
Add subtle (or dramatic) timing variations to your sequences - trigger notes early or late relative to the grid for humanized, swung, or intentionally off-grid grooves.

#### How to Use

**1. Edit Gate Offset:**
- In sequence edit mode, select **GATE OFFSET** layer
- Adjust per-step timing: **-63 to +63**
- Display shows **+/-** percentage of step length
- Examples: "+15.6%" (late), "-8.1%" (early), "+0.0%" (on-grid)

**2. Visual Reference:**
- Offset bar shows 7 reference markers
- Center line = perfect grid timing
- Bright indicator = current offset value

**3. Timing Resolution:**
At 120 BPM with 1/16th notes:
- Range: ±48.4% of step length
- Precision: ~1ms per offset step
- Perfect for subtle groove adjustments

**Example Uses:**
- **Swing groove**: Alternate +10 / -10 offsets on hi-hats
- **Humanize drums**: Random small offsets (-5 to +5)
- **Laid-back feel**: All offsets slightly positive (+5 to +15)
- **Rushing feel**: All offsets slightly negative (-15 to -5)

### 🎹 Polyphonic Voices

#### What It Does
Play up to 4 simultaneous notes per track for chord sequencing and polyphonic melodies.

#### How to Use

**1. Enable Polyphony:**
- Press **TRACK** button
- Scroll to **"Polyphony"** setting
- Set to **1-4 voices**:
  - `0` = Mono (default)
  - `1 Voice` = Monophonic
  - `2 Voices` = Dyads/intervals
  - `3 Voices` = Triads
  - `4 Voices` = Full chords (C-E-G-B)

**2. Programming Chords (Current Method):**
Since piano roll is not yet implemented, program chords by overlapping notes:

**Method A - Multiple Steps:**
```
Step 1: Note C (length 50%)
Step 2: Note E (length 50%, gate offset -50%)
Step 3: Note G (length 50%, gate offset -50%)
```
This creates a C major triad.

**Method B - Overlapping Gates:**
```
Step 1: Note C (length 100%)
Step 5: Note E (length 75%)
Step 9: Note G (length 50%)
```
When these steps overlap in time, voices play simultaneously.

**3. Voice Allocation:**
- System allocates voices automatically
- Up to 4 voices active simultaneously
- When all voices busy, oldest voice is replaced (voice stealing)
- Voices release automatically when notes end

**Current Limitations:**
- No piano roll editor yet (manual chord programming required)
- Recording captures only monophonic input
- Best results on MIDI output tracks

### 📝 Capture Timing

#### What It Does
Record the exact timing of your live MIDI performance, preserving human feel and groove variations.

#### How to Use

**1. Enable Capture Timing:**
- Press **TRACK** button
- Scroll to **"Capture Timing"**
- Set to **On**

**2. Record with Timing:**
- Enable **RECORD** mode
- Play MIDI keyboard with natural timing
- System captures how early/late each note was played
- Notes are quantized to steps but timing offset is preserved

**3. Disable for Strict Quantization:**
- Set **Capture Timing** to **Off**
- All recorded notes snap perfectly to grid
- Classic sequencer behavior

**Example Workflow:**
1. Set track to Capture Timing: On
2. Record your drum groove with natural timing variations
3. System captures exact timing offsets
4. Playback preserves your human feel
5. Fine-tune individual offsets in sequence edit mode

## Track Settings Reference

### New Settings in Track Page

#### Polyphony
- **Location**: Track page, after Note P. Bias
- **Range**: 0-4 voices
- **Default**: 0 (Mono)
- **Display**: "Mono", "1 Voice", "2 Voices", "3 Voices", "4 Voices"
- **Saved**: Per-track, saved with project

#### Capture Timing
- **Location**: Track page, after Polyphony
- **Options**: On / Off
- **Default**: Off
- **Effect**: Enables/disables timing offset recording
- **Saved**: Per-track, saved with project

## Workflow Examples

### Example 1: Humanized Hi-Hats

**Goal**: Add natural timing variations to mechanical hi-hat pattern

**Steps:**
1. Program standard 1/16th hi-hat pattern (every step)
2. Go to GATE OFFSET layer
3. Add random small offsets: -3, +2, -1, +3, -2, +1, etc.
4. Result: Subtle human-like timing variations

**Advanced:**
1. Enable Capture Timing
2. Record yourself playing hi-hats
3. System captures your natural timing
4. Edit captured offsets to taste

### Example 2: Swung Bassline

**Goal**: Create swung 1/8th note bassline

**Steps:**
1. Program bassline on 1/8th notes (steps 1, 3, 5, 7, etc.)
2. Go to GATE OFFSET layer
3. On beats 2 and 4: Set offset to +15%
4. On beats 1 and 3: Set offset to 0% or -5%
5. Result: Classic swing feel

### Example 3: Polyphonic Chord Progression

**Goal**: Create C-Am-F-G chord progression

**Steps:**
1. Set track Polyphony to 4 Voices
2. Program first chord (C major):
   - Step 1: C (length 75%)
   - Step 2: E (length 75%, offset -50%)
   - Step 3: G (length 75%, offset -50%)
3. Repeat for other chords on steps 17, 33, 49
4. Result: Polyphonic chord sequence

**Alternative (using overlapping steps):**
1. Step 1: C (length 200%)
2. Step 2: E (length 100%)
3. Step 3: G (length 50%)
4. All overlap = simultaneous playback

### Example 4: Live Chord Recording

**Goal**: Record keyboard chords with natural timing

**Setup:**
1. Track 16 set to Polyphony: 4 Voices
2. Capture Timing: On
3. MIDI Input: On

**Process:**
1. Enable RECORD
2. Play chord progression on keyboard
3. System captures timing offsets
4. Each chord's notes trigger simultaneously
5. Timing feel preserved

**Current Limitation**: Monophonic recording only - chords must be programmed manually.

## Tips & Tricks

### Micro-Timing Tips

**1. Subtlety is Key**
- Start with offsets ±3 to ±5
- Too much sounds sloppy, too little is imperceptible
- Listen and adjust to taste

**2. Consistent Patterns**
- Same offset pattern on repeating elements (hi-hats, etc.)
- Creates consistent groove character

**3. Genre-Specific Feels**
- **Hip-hop**: Drums slightly behind (+5 to +10)
- **Jazz**: Slight randomness (±3 to ±8)
- **Electronic**: Tight on kicks, looser on hats
- **Funk**: Strong swing on hi-hats and snares

**4. Use with Swing**
- Combine gate offset with global swing setting
- Offset adds individual character on top of swing

### Polyphony Tips

**1. Voice Count Planning**
- 2 voices: Bass + melody, power chords
- 3 voices: Triads, basic chords
- 4 voices: Full chords with extensions

**2. Note Lengths**
- Longer lengths = voices overlap more
- Shorter lengths = more staccato chords
- Vary for dynamics

**3. MIDI Channel Management**
- All voices on same MIDI channel
- Configure external synth for polyphony
- Some synths need poly mode enabled

**4. Mix with Mono Tracks**
- Use mono tracks for bass, leads
- Use poly tracks for chords, pads
- Clearer arrangement

### Capture Timing Tips

**1. When to Use**
- **On**: Expressive playing, live feel, grooves
- **Off**: Precise programming, clicks, technical parts

**2. Practice First**
- Get comfortable with your part
- Consistent playing = better capture
- Can always fine-tune after recording

**3. Combining with Manual Editing**
- Record with Capture Timing on
- Then manually adjust outlier offsets
- Best of both worlds

## Limitations & Known Issues

### Polyphony
- ❌ Piano roll editor not yet available
- ❌ Recording captures only single notes
- ❌ Must manually program chords using overlapping steps
- ✅ Playback works perfectly
- ✅ Voice allocation reliable

### Micro-Timing
- ⚠️ Very large offsets (>±30) may cause timing issues at fast tempos
- ⚠️ Interaction with global swing may produce unexpected results
- ✅ Generally works well for typical musical uses

### General
- Hardware tested: ⏳ Pending
- All features compile and build successfully
- Backward compatible with original Performer projects

## Troubleshooting

### Polyphony Not Working
**Symptom**: Only one note plays
**Solutions**:
1. Check Polyphony setting is >0
2. Verify notes actually overlap in time
3. Check MIDI output configuration
4. Ensure external synth supports polyphony

### Timing Offset Not Audible
**Symptom**: Can't hear timing changes
**Solutions**:
1. Check MIDI/audio sync settings
2. Try larger offsets (±10 to ±20)
3. Slower tempos make timing more noticeable
4. Check if offset is actually set (not 0)

### Capture Timing Not Recording
**Symptom**: All notes perfectly quantized
**Solutions**:
1. Verify Capture Timing is On
2. Check recording mode is active
3. Verify MIDI input is configured
4. Play with more exaggerated timing variations

### Strange Playback Timing
**Symptom**: Notes trigger at wrong times
**Solutions**:
1. Check if global swing is enabled
2. Verify gate offset values are reasonable
3. Check divisor settings
4. Reset problematic step offsets to 0

## Compatibility

### Project Compatibility
- **Opening old projects**: ✅ Fully compatible
  - Polyphony defaults to 0 (mono)
  - Capture Timing defaults to Off
  - No data loss

- **Saving projects**: ✅ New format
  - Old firmware cannot read POW|FORMER projects
  - Keep backups before upgrading

### Hardware Compatibility
- **Target**: STM32F405RGT6 (Performer hardware)
- **Flash**: 338,616 bytes (fits in 1MB)
- **RAM**: 150,908 bytes (fits in 192KB)
- **Compatible**: ✅ Standard Performer hardware

## Installation

### Requirements
- Performer sequencer hardware
- SD card
- Backup of existing firmware (recommended)

### Steps
1. **Backup**:
   - Save all your projects
   - Backup current firmware if desired

2. **Install**:
   - Copy `UPDATE_16TRACKS_MINIMAL.DAT` to SD card
   - Insert SD card
   - Power on while holding button
   - Follow on-screen update process

3. **Verify**:
   - Boot screen shows "POW|FORMER"
   - Subtitle: "Modulove Artificial FW"
   - Version 0.0.7

### Reverting
- Flash original Performer firmware
- Your projects may need format conversion
- Settings will reset to defaults

## Credits

**Original**: Performer Sequencer by Phazerville
**Modifications**: Modulove Development
**AI Development**: Anthropic Claude
**Version**: 0.0.7 "Modulove Artificial FW"
**Date**: October 23, 2025

## Support & Feedback

This is custom firmware. Use at your own risk.

**Issues?**
- Check this guide first
- Review changelog for known issues
- Test with fresh project

**Feature Requests?**
- Piano roll editor (in progress)
- Polyphonic recording (planned)
- Additional voices (possible)
- Other ideas welcome

## Quick Reference Card

### Key Combinations
Same as original Performer, plus:

**Track Page**:
- Navigate to Polyphony: 0-4 voices
- Navigate to Capture Timing: On/Off

**Sequence Edit**:
- Select GATE OFFSET layer for timing
- Values: -63 to +63
- Display: +/- percentage

### Settings Locations
```
TRACK page:
  ├── ... (existing settings)
  ├── Polyphony        [0-4]
  └── Capture Timing   [On/Off]

SEQUENCE EDIT:
  └── GATE OFFSET      [-63 to +63]
```

### Typical Values
- Micro-timing: ±3 to ±10 (subtle)
- Swing feel: +10 to +20 (late)
- Rush feel: -10 to -20 (early)
- Polyphony: 3-4 voices (chords)

---

**Document Version**: 1.0
**Firmware Version**: 0.0.7
**Last Updated**: October 23, 2025
