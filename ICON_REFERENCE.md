# POW|FORMER Procedural Icon Library Reference

## Overview

A complete library of **34 procedural icons** for the POW|FORMER sequencer hardware. These icons are drawn algorithmically instead of stored as bitmaps, providing:

- **Scalability**: Any size from 4x4 to 32x32 pixels
- **Memory efficiency**: ~2.7KB total code vs ~408 bytes bitmap storage
- **Flexibility**: Can be animated, colored, and modified easily
- **Consistency**: Unified visual style across the UI

## File Location

```
src/apps/sequencer/ui/ProceduralIcons.h
```

## Quick Start

```cpp
#include "ui/ProceduralIcons.h"

// Draw a play icon
Icons::play(canvas, x, y, size);

// size parameter is optional (defaults shown for each category)
```

## Complete Icon Reference

### Playback Controls (8x8 default)

| Function | Description | Usage |
|----------|-------------|-------|
| `play()` | Right-pointing triangle | `Icons::play(canvas, x, y, 8)` |
| `pause()` | Two vertical bars | `Icons::pause(canvas, x, y, 8)` |
| `stop()` | Solid square | `Icons::stop(canvas, x, y, 8)` |
| `record()` | Filled circle | `Icons::record(canvas, x, y, 8)` |
| `rewind()` | Two left triangles | `Icons::rewind(canvas, x, y, 8)` |
| `fastForward()` | Two right triangles | `Icons::fastForward(canvas, x, y, 8)` |

### Audio Controls (8x8 default)

| Function | Description | Usage |
|----------|-------------|-------|
| `mute()` | Speaker with X | `Icons::mute(canvas, x, y, 8)` |
| `volumeUp()` | Speaker with waves | `Icons::volumeUp(canvas, x, y, 8)` |
| `volumeDown()` | Speaker with wave | `Icons::volumeDown(canvas, x, y, 8)` |

### Sequencer Functions (12x12 default)

| Function | Description | Usage |
|----------|-------------|-------|
| `pattern()` | 3x4 grid of blocks | `Icons::pattern(canvas, x, y, 12)` |
| `sequence()` | Step sequence bars | `Icons::sequence(canvas, x, y, 12)` |
| `keyboard()` | Piano keys | `Icons::keyboard(canvas, x, y, 12)` |
| `clock()` | Clock face | `Icons::clock(canvas, x, y, 12)` |
| `sync()` | Circular arrows | `Icons::sync(canvas, x, y, 12)` |
| `quantize()` | Grid with notes | `Icons::quantize(canvas, x, y, 12)` |

### Modulation & Effects (12x12 default)

| Function | Description | Usage |
|----------|-------------|-------|
| `lfo()` | Sine wave | `Icons::lfo(canvas, x, y, 12)` |
| `envelope()` | ADSR curve | `Icons::envelope(canvas, x, y, 12)` |
| `filter()` | Filter slope | `Icons::filter(canvas, x, y, 12)` |
| `modulation()` | Wavy lines | `Icons::modulation(canvas, x, y, 12)` |

### File & System (8x8 default)

| Function | Description | Usage |
|----------|-------------|-------|
| `save()` | Floppy disk | `Icons::save(canvas, x, y, 8)` |
| `load()` | Folder | `Icons::load(canvas, x, y, 8)` |
| `settings()` | Gear | `Icons::settings(canvas, x, y, 8)` |
| `info()` | Circle with 'i' | `Icons::info(canvas, x, y, 8)` |
| `warning()` | Triangle with '!' | `Icons::warning(canvas, x, y, 8)` |

### Navigation (8x8 default)

| Function | Description | Usage |
|----------|-------------|-------|
| `arrowUp()` | Up arrow | `Icons::arrowUp(canvas, x, y, 8)` |
| `arrowDown()` | Down arrow | `Icons::arrowDown(canvas, x, y, 8)` |
| `arrowLeft()` | Left arrow | `Icons::arrowLeft(canvas, x, y, 8)` |
| `arrowRight()` | Right arrow | `Icons::arrowRight(canvas, x, y, 8)` |
| `checkmark()` | Check symbol | `Icons::checkmark(canvas, x, y, 8)` |
| `cross()` | X symbol | `Icons::cross(canvas, x, y, 8)` |

### MIDI & CV (12x12 default)

| Function | Description | Usage |
|----------|-------------|-------|
| `midiIn()` | Box with arrow in | `Icons::midiIn(canvas, x, y, 12)` |
| `midiOut()` | Box with arrow out | `Icons::midiOut(canvas, x, y, 12)` |
| `cvGate()` | Gate pulse waveform | `Icons::cvGate(canvas, x, y, 12)` |
| `trigger()` | Lightning bolt | `Icons::trigger(canvas, x, y, 12)` |

## Usage Examples

### Basic Icon

```cpp
// Draw a play icon at (10, 10) with default size (8x8)
Icons::play(canvas, 10, 10);

// Draw larger version (16x16)
Icons::play(canvas, 10, 10, 16);
```

### Status Indicator

```cpp
void drawPlaybackStatus(Canvas &canvas, bool playing) {
    if (playing) {
        Icons::play(canvas, 240, 2, 8);
    } else {
        Icons::pause(canvas, 240, 2, 8);
    }
}
```

### Menu Item

```cpp
void drawMenuItem(Canvas &canvas, int y, bool selected) {
    if (selected) {
        canvas.setColor(Color::Bright);
        Icons::checkmark(canvas, 5, y, 6);
    }
    canvas.drawText(15, y, "Pattern 1");
}
```

### Animated Icon

```cpp
void drawRecording(Canvas &canvas, uint32_t time) {
    // Pulsing record icon
    int size = 8 + sin(time * 0.005f) * 2;

    // Flash on/off
    if ((time / 500) % 2 == 0) {
        Icons::record(canvas, 10, 10, size);
    }
}
```

### Modulator Display

```cpp
void drawModulator(Canvas &canvas, int index, float rate) {
    int x = 10 + (index % 4) * 60;
    int y = 10 + (index / 4) * 25;

    // Icon
    Icons::lfo(canvas, x, y, 12);

    // Label
    canvas.setFont(Font::Tiny);
    canvas.drawText(x + 15, y, "M");
    canvas.drawText(x + 21, y, std::to_string(index + 1).c_str());
}
```

### MIDI Activity

```cpp
void drawMidiStatus(Canvas &canvas, bool in, bool out) {
    if (in) {
        Icons::midiIn(canvas, 220, 2, 10);
    }
    if (out) {
        Icons::midiOut(canvas, 235, 2, 10);
    }
}
```

## Design Guidelines

### Icon Sizing

- **Small icons (6-8px)**: Use for inline indicators, menu items
- **Medium icons (12px)**: Default for most UI elements
- **Large icons (16-24px)**: Use for primary actions, main controls

### Spacing

- Leave at least 2-4 pixels between icons
- Align icons to grid positions for consistency

### Color

All icons support the Canvas color system:
```cpp
canvas.setColor(Color::Bright);  // Full brightness
canvas.setColor(Color::Medium);  // Medium brightness
canvas.setColor(Color::Low);     // Low brightness
canvas.setColor(Color::None);    // Black/off
```

### Animation

Icons can be animated by:
1. **Size**: Pulse by varying the size parameter
2. **Position**: Move by changing x, y coordinates
3. **Visibility**: Flash by conditional drawing
4. **Rotation**: Not directly supported (draw custom variants)

## Memory Comparison

| Storage Method | 34 Icons | Bytes per Icon | Total |
|----------------|----------|----------------|-------|
| Bitmaps (8x8) | 272 bytes | 8 bytes | 272 bytes |
| Bitmaps (12x12) | 612 bytes | 18 bytes | 612 bytes |
| Procedural Code | ~2700 bytes | ~80 bytes | ~2700 bytes |

**Note**: While procedural icons use more total code space, they:
- Scale to any size (no need for multiple bitmap sizes)
- Can be animated without storing animation frames
- Take up less space when you need variety in sizing

## Tips & Tricks

### Combining Icons

```cpp
// Draw record + mute = "recording muted"
Icons::record(canvas, x, y, 8);
Icons::mute(canvas, x + 10, y, 8);
```

### Custom Variations

```cpp
// Outlined version (draw twice with offset)
canvas.setColor(Color::None);
Icons::play(canvas, x+1, y+1, 8);
canvas.setColor(Color::Bright);
Icons::play(canvas, x, y, 8);
```

### Blinking Effect

```cpp
// Blink at 2Hz
if ((os::ticks() / 250) % 2 == 0) {
    Icons::warning(canvas, x, y, 8);
}
```

## Integration Checklist

- [ ] Include `ProceduralIcons.h` in your page
- [ ] Set canvas color before drawing icons
- [ ] Choose appropriate icon size for context
- [ ] Test on actual hardware display
- [ ] Consider animation for status indicators
- [ ] Use consistent spacing between icons

## Future Enhancements

Potential additions to the library:

- [ ] More waveform shapes (saw, triangle, square)
- [ ] Musical note symbols
- [ ] Tempo/metronome icon
- [ ] Loop/repeat icon
- [ ] Shuffle/random icon
- [ ] Link/chain icon
- [ ] Lock/unlock icon
- [ ] Duplicate/clone icon

## Credits

Created with svg2performer.py and hand-optimized for the POW|FORMER hardware.
