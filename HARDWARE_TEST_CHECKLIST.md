# POW|FORMER v0.6 Hardware Testing Checklist

## Pre-Flash Checklist
- [ ] Backup current projects/patterns from SD card
- [ ] Verify firmware file: `UPDATE_16TRACKS_DOUBLETAP.DAT` (331KB)
- [ ] Place firmware on SD card root directory
- [ ] Perform firmware update procedure

## Post-Flash Basic Tests

### 1. Boot & Basic Operation
- [ ] Device boots successfully
- [ ] Display shows "POW|FORMER SEQUENCER" on startup
- [ ] No crashes or freezes during idle
- [ ] Encoder responds correctly
- [ ] All buttons register presses

### 2. 16-Track Dual Bank System

#### Bank 1 (Tracks 1-8)
- [ ] Track buttons 0-7 select tracks 1-8
- [ ] LED colors are GREEN for bank 1
- [ ] Pattern page shows tracks 1-8
- [ ] Overview page shows tracks 1-8

#### Bank 2 (Tracks 9-16)
- [ ] Press RIGHT button to switch to bank 2
- [ ] Track buttons 0-7 now select tracks 9-16
- [ ] LED colors change to RED for bank 2
- [ ] Pattern page shows tracks 9-16
- [ ] Overview page shows tracks 9-16

#### Bank Switching
- [ ] LEFT button switches from bank 2 to bank 1
- [ ] RIGHT button switches from bank 1 to bank 2
- [ ] Bank state persists across pages
- [ ] Selected track in each bank is remembered

### 3. Modulator System

#### Access & Navigation
- [ ] Press Track5 button from main menu to access Modulator page
- [ ] Page shows "MOD 1 - MODULATOR" at top
- [ ] Can navigate between 8 modulators (MOD 1-8)

#### Modulator Parameters
- [ ] SHAPE: Can select waveforms (Sine, Triangle, Square, Ramp, Random, Noise)
- [ ] RATE: Adjustable from 0.01Hz to 100Hz
- [ ] DEPTH: Adjustable 0-100%
- [ ] OFFSET: Adjustable -100% to +100%
- [ ] PHASE: Adjustable 0-360°
- [ ] SOURCE: Can select trigger sources
- [ ] TARGET: Can assign modulation destinations
- [ ] EVENT: Can set trigger events
- [ ] SMOOTH: Can adjust smoothing 0-100%

#### Modulator Operation
- [ ] Modulators run when play is pressed
- [ ] Visual feedback shows modulator activity
- [ ] Modulators affect assigned targets correctly

### 4. MIDI Output Routing

#### Access MIDI Output Page
- [ ] Press Track3 button from main menu
- [ ] Shows "OUT 1 - MIDI OUTPUT"
- [ ] Can navigate between 8 outputs

#### Source Selection for Control Change
- [ ] Set Event to "CC"
- [ ] Can select Track 1-16 as source
- [ ] Can select Mod 1-8 as source
- [ ] Can select CV In 1-4 as source
- [ ] Source names display correctly

#### MIDI Output Testing
- [ ] Connect MIDI monitor
- [ ] Track sources send MIDI correctly
- [ ] Modulator sources send CC values
- [ ] CV inputs (if connected) send CC values

### 5. Double-Tap Gate Toggle

#### Step Edit Mode
- [ ] Enter step edit for any track
- [ ] Select Note layer (or any non-Gate layer)

#### Single Tap Behavior
- [ ] Single tap on step shows parameter value
- [ ] Step selection works normally
- [ ] No gate changes on single tap (except in Gate layer)

#### Double-Tap Testing
- [ ] Tap a step twice quickly (within ~300ms)
- [ ] Gate toggles ON if it was OFF
- [ ] Gate toggles OFF if it was ON
- [ ] Works in Note layer
- [ ] Works in Velocity layer
- [ ] Works in Gate Probability layer
- [ ] Works in all other layers

#### Timing & Accuracy
- [ ] Double-tap window feels responsive
- [ ] No accidental triggers during normal use
- [ ] Triple-tap doesn't cause multiple toggles

### 6. Performance & Stability

#### Memory & Performance
- [ ] No noticeable lag compared to previous firmware
- [ ] Pattern switching remains smooth
- [ ] No audio glitches during heavy modulation
- [ ] UI remains responsive with all 16 tracks active

#### Extended Testing (15-30 minutes)
- [ ] Run complex pattern with all 16 tracks
- [ ] Use multiple modulators simultaneously
- [ ] Switch banks frequently during playback
- [ ] Save and load projects
- [ ] No crashes or freezes

### 7. Regression Testing

#### Existing Features Still Work
- [ ] Note sequences play correctly
- [ ] Pattern chaining works
- [ ] Song mode functions
- [ ] Clock sync (internal/external)
- [ ] CV/Gate outputs work
- [ ] MIDI input/output works
- [ ] File operations (save/load)
- [ ] Arpeggiator functions
- [ ] Routing page works

### 8. Edge Cases & Bugs

#### Potential Issues to Check
- [ ] Switching banks during playback doesn't cause issues
- [ ] Modulator values persist after power cycle
- [ ] Double-tap works correctly near pattern boundaries
- [ ] MIDI CC values scale correctly (0-127)
- [ ] No stuck notes when switching patterns
- [ ] Bank LEDs update correctly in all pages

## Test Results

### Pass/Fail Summary
- [ ] All critical features working
- [ ] No show-stopping bugs found
- [ ] Performance acceptable
- [ ] Ready for production use

### Issues Found
(List any problems discovered during testing)

1.
2.
3.

### Notes & Observations
(Any additional comments about the firmware)




---
**Test Date:** ___________
**Tester:** ___________
**Firmware Version:** POW|FORMER v0.6
**Binary MD5:** e4e41030b153caa3467a1e018992973a