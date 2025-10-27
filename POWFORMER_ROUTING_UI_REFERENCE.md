# POW|FORMER Routing & UI Reference

## 1. MIDI Output Routing Matrix

This table shows all possible routing sources for MIDI outputs (16 outputs total):

| Source Type | Source Range | Can Route To | Notes |
|-------------|--------------|--------------|-------|
| Tracks | T1 - T16 | MIDI Out 1-16 (Note/CC) | Gate, Note, Velocity sources for Note events; Control sources for CC events |
| Modulators | Mod 1 - Mod 8 | MIDI Out 1-16 (Note/CC) | LFO/ADSR outputs → MIDI CC or Note with fixed velocity |
| CV Inputs | CV In 1-4 | MIDI Out 1-16 (CC only) | External CV → MIDI CC conversion |
| Fixed Values | Note 0-127, Vel 0-127 | MIDI Out (Note events) | Static note/velocity values |

### MIDI Output Event Types

- **Note Events**: Requires Gate source, Note source, Velocity source
- **Control Change (CC)**: Requires CC# (0-127), Control source

### Routing Configuration

- Access via: **MIDI OUTPUT** page
- Step buttons (0-15) select output 1-16
- F1-F5 buttons configure routing parameters (see Function Button table below)

---

## 2. Global Routing System

POW|FORMER has a global routing system (up to 4 routes) for internal parameter control:

| Routing Sources | Routing Targets | Usage |
|-----------------|-----------------|-------|
| CV In 1-4 | Play, Record, Tempo, Swing, Mute, Fill, Pattern | External CV control of global parameters |
| CV Out 1-8 | Track-specific parameters | Monitor/feedback routing |
| MIDI | Sequence/track parameters | MIDI CC → internal parameter mapping |

**Access**: **ROUTING** page (Shift+Page → ROUTING)

---

## 3. Function Button Mapping (F1-F5)

This table shows function button assignments for each major page:

| Page | F1 | F2 | F3 | F4 | F5 |
|------|----|----|----|----|-----|
| **PERFORMER** | LATCH | SYNC | UNMUTE | FILL | PATTERN |
| + Shift | - | - | - | - | CANCEL |
|  |  |  |  |  |  |
| **MODULATOR** | SHAPE | MODE* | RATE** | DEPTH/OFFSET*** | PHASE/SMOOTH/GATE**** |
| + Shift+Page | MODE | GATE | CC NUM | TARGET | EVENT |
|  |  |  |  |  |  |
| **NOTE SEQ EDIT** | GATE | RETRIG | LENGTH | NOTE | COND |
| + Shift+Page | INIT | COPY | PASTE | DUPL | GEN |
|  |  |  |  |  |  |
| **NOTE SEQUENCE** | - | - | - | - | - |
| + Shift+Page | INIT | COPY | PASTE | DUPL | ROUTE |
|  |  |  |  |  |  |
| **TRACK** | - | - | - | - | - |
| + Shift+Page | INIT | COPY | PASTE | ROUTE | - |
|  |  |  |  |  |  |
| **MIDI OUTPUT** | TARGET | EVENT | GATE/CC#** | NOTE/SOURCE** | VEL/-** |
| + Shift | - | - | - | - | - |
|  |  |  |  |  |  |
| **PATTERN** | LATCH | SYNC | SNAP/REVERT*** | COMMIT*** | CANCEL*** |
| + Shift+Page | INIT | COPY | PASTE | DUP | SAVE PR. |
|  |  |  |  |  |  |
| **SONG** | CHAIN | ADD | REMOVE | DUPL | PLAY/STOP |
| + Shift | - | INSERT^ | - | - | - |
|  |  |  |  |  |  |
| **ROUTING** | PREV | NEXT | INIT | LEARN^^ | COMMIT^^ |
| + Shift | - | - | - | - | - |
|  |  |  |  |  |  |
| **LAYOUT** | MODE | LINK | GATE | CV | COMMIT^^^ |
| + Shift | - | - | - | - | - |
|  |  |  |  |  |  |
| **SYSTEM** | CAL | - | UTILS | UPDATE | SETTINGS |
| + Shift+Page | INIT | SAVE | BACKUP | RESTORE | - |
|  |  |  |  |  |  |
| **MONITOR** | CV IN | CV OUT | MIDI | STATS | Version |
| + Shift | - | - | - | - | - |
|  |  |  |  |  |  |
| **USER SCALE** | USER1 | USER2 | USER3 | USER4 | - |
| + Shift | - | - | - | - | - |

### Legend

- `*` **MODULATOR F2 (MODE)**: Changes based on shape
  - **LFO shapes** (Sine, Triangle, Saw, Square): MODE (Free/Sync/Retrigger)
  - **Random shape**: MODE (Clocked/Triggered)
  - **ADSR shape**: ATTACK (0-2000ms)
- `**` **MODULATOR F3 (RATE)**: Changes based on shape and mode
  - **LFO shapes**: RATE (clock divisor with triplets and dotted notes)
  - **Random Clocked**: RATE (clock divisor)
  - **Random Triggered**: GATE (track 1-16 triggers)
  - **ADSR shape**: DECAY (0-2000ms)
- `***` **MODULATOR F4 (DEPTH/OFFSET)**: Combined parameter
  - **LFO shapes**: DEPTH (0-127) and OFFSET (-64 to +63) both map to F4
  - **Random shape**: DEPTH (0-127)
  - **ADSR shape**: SUSTAIN (0-127)
- `****` **MODULATOR F5 (PHASE/SMOOTH/GATE)**: Changes based on shape
  - **LFO shapes**: PHASE (0-360°)
  - **Random shape**: SMOOTH (0-2000ms interpolation time)
  - **ADSR shape**: GATE (track 1-16 triggers) or RELEASE
- **Shift+Page on MODULATOR**: Toggles routing overlay mode
  - Shows: F1=MODE (Free/Sync/Retrig), F2=GATE (track), F3=CC NUM, F4=TARGET (MIDI Out), F5=EVENT (Note/CC)
  - Waveform preview remains on left, parameter display on right
- `**` **MIDI OUTPUT**: F3-F5 change based on event type
  - **Note events**: F3=GATE, F4=NOTE, F5=VEL
  - **CC events**: F3=CC#, F4=SOURCE, F5=(empty)
- `***` **PATTERN**: F3-F5 change when snapshot active or cancel available
- `^` **SONG**: F2 = ADD normally, INSERT when Shift held
- `^^` **ROUTING**: F4-F5 only show when learn/commit available
- `^^^` **LAYOUT**: F5 only shows when commit available

---

## 4. Modulator Quick Routing Presets

Accessible via **Shift+Page** context menu on **MODULATOR** page:

| Preset | Routing Configuration |
|--------|----------------------|
| Mod 1-8 → CC0 | Mod 1 → MIDI 1 CC 0<br>Mod 2 → MIDI 2 CC 0<br>...<br>Mod 8 → MIDI 8 CC 0 |
| Custom | Tracks 9-16 → MIDI 1-8 @ velocity 127<br>(Alternative mapping for tracks beyond first 8) |

---

## 5. Page Navigation & Overlays

### Main Pages (accessible via Shift+Page)

- PERFORMER
- TRACK
- NOTE SEQUENCE
- NOTE SEQ EDIT
- MODULATOR (NEW in POW|FORMER)
- PATTERN
- SONG
- ROUTING
- LAYOUT
- MIDI OUTPUT
- SYSTEM
- MONITOR
- USER SCALE

### Track Selection

- Track buttons 1-8 select tracks in current bank
- Selected track determines bank (tracks 1-8 vs 9-16)
- Bank 2 (tracks 9-16) shows **inverted LEDs** for step sequencing

### Pattern Selection

- Step buttons 1-16 select pattern 1-16 for selected track(s)
- In Pattern mode (F5 on PERFORMER page):
  - Green LED = active pattern
  - Red LED = muted pattern
  - Yellow LED = selected pattern

---

## 6. Special UI Behaviors

| Feature | Behavior | Visual Feedback |
|---------|----------|-----------------|
| Bank Switching | Automatic based on selected track (T1-T8 vs T9-T16) | LED inversion for bank 2 (red for active steps) |
| Latch/Sync | Pattern/mute changes wait for sync point | Sync progress bar at top of screen |
| Snapshot Mode | Save/restore pattern state across all tracks | "S" indicator, REVERT/COMMIT buttons |
| Microtiming | Gate offset recording with quantize strength (0-100%) | Captured during live recording, visual offset bars |
| ADSR Envelope Preview | Real-time waveform display on MODULATOR page | Attack/Decay/Sustain/Release shape visualization |

---

## 7. Track & Pattern Limitations

| Parameter | Limit | Notes |
|-----------|-------|-------|
| Total Tracks | 16 | Dual bank system (1-8, 9-16) |
| Patterns per Track | 8 | CONFIG_PATTERN_COUNT = 8 |
| Song Slots | 4 | CONFIG_SONG_SLOT_COUNT = 4 |
| Modulators/LFOs | 8 | Independent LFO/ADSR engines |
| Global Routes | 4 | CONFIG_ROUTE_COUNT = 4 |
| CV Inputs | 4 | Can route to MIDI CC or internal params |
| CV Outputs | 8 | Voltage outputs for external gear |
| MIDI Outputs | 16 | Virtual MIDI outputs (routing sources) |

---

## 8. Modulator Shapes & Parameters

| Shape | F1 (SHAPE) | F2 | F3 | F4 | F5 | Description |
|-------|------------|----|----|----|----|-------------|
| Sine | SHAPE | - | RATE | DEPTH | PHASE | Smooth parabolic sine wave |
| Triangle | SHAPE | - | RATE | DEPTH | PHASE | Linear triangle wave |
| Saw Up | SHAPE | - | RATE | DEPTH | PHASE | Rising sawtooth |
| Saw Down | SHAPE | - | RATE | DEPTH | PHASE | Falling sawtooth |
| Square | SHAPE | - | RATE | DEPTH | PHASE | 50% duty cycle square |
| Smooth Random | MODE | MODE | RATE | DEPTH | SMOOTH | Interpolated random values |
| Stepped Random | MODE | MODE | RATE | DEPTH | SMOOTH | Sample & hold random |
| ADSR | SHAPE | ATTACK | DECAY | SUSTAIN | RELEASE | Gate-triggered envelope |

### Rate Divisions

Available for all LFO shapes (via F3=RATE):

**Standard**: 1/64, 1/32, 1/16, 1/8, 1/4, 1/2, 1, 2, 4, 8, 16, 32, 64

**Triplets**: 1/64T, 1/32T, 1/16T, 1/8T, 1/4T

**Dotted**: 1/32., 1/16., 1/8., 1/4., 1/2.

### ADSR Parameters

- **Attack** (F2): 0-2000ms - Time to reach peak level
- **Decay** (F3): 0-2000ms - Time to fall to sustain level
- **Sustain** (F4): 0-127 - Held level while gate is high
- **Release** (F5): 0-2000ms - Time to return to zero after gate off
- **Gate** (F5 alternate): Select gate source track (1-16)

---

## 9. Microtiming System

### Gate Offset

- **Range**: 0-15 ticks (stored in 4 bits)
- **Resolution**: At 192 PPQN and 120 BPM: ~2.6ms per tick
- **Max Offset**: 15 ticks = ~39ms late timing
- **Recording**: Automatically captured during live recording
- **Quantize Strength**: 0-100% blends between natural feel and grid precision
  - 0% = full microtiming (human feel)
  - 100% = perfect grid quantization
  - 50% = half the timing offset

### Editing

- Access via: **NOTE SEQ EDIT** page → GateOffset layer
- Visual: Offset bars show timing deviation from grid
- Adjustment: Encoder to fine-tune per-step offsets

---

## 10. Quick Reference: Common Tasks

| Task | Navigation | Action |
|------|-----------|--------|
| Route Mod 1 to MIDI CC 1 | MIDI OUTPUT page → Step 0 → F1=TARGET (MIDI 1) → F2=EVENT (CC) → F3=CC# (1) → F4=SOURCE (Mod 1) | Configure MIDI output routing |
| Create ADSR envelope | MODULATOR page → Step 0-7 → F1=SHAPE (ADSR) → F2/F3/F4/F5 adjust A/D/S/R | Envelope for dynamics |
| Switch to Bank 2 (tracks 9-16) | Press Track button 1-8 while track 9-16 selected | Bank follows selected track |
| Record with microtiming | PERFORMER page → Hold track button → Play notes | Gate offsets captured automatically |
| Snapshot patterns | PATTERN page → F3=SNAP → Make changes → F3=REVERT or F4=COMMIT | Try changes non-destructively |

---

## Notes

- **Bank system**: Tracks 1-8 (bank 1) and 9-16 (bank 2) share same 8 physical track buttons
- **LED colors**: Green=active, Red=muted/bank2, Yellow=selected, inverted in bank 2
- **Context menus**: Hold Shift+Page for quick actions (INIT, COPY, PASTE, etc.)
- **Routing flexibility**: CV inputs can route to MIDI CC, modulators to notes or CC
- **Firmware size**: ~336KB text, total ~493KB (fits STM32 flash)
