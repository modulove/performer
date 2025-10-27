# POW|FORMER Firmware Changelog v0.0.63

**Release Date**: October 27, 2025
**Build**: v0.0.63 (361028 bytes text)
**Base Fork**: PEW|FORMER by djphazer
**Original**: PER|FORMER by westlicht

---

## 💙 A Heartfelt Thank You

First and foremost, a **huge thank you** to **westlicht** (Simon Kallweit) for creating the original PER|FORMER sequencer. This incredible instrument has been such an integral part of my modular system for years now! The generosity of open-sourcing this beast of a sequencer has enabled not just my own creative work, but an entire community of musicians and developers to push the boundaries of what's possible.

I hope with POW|FORMER I can do justice to your original vision and breathe some fresh life into this amazing platform while pushing the boundaries of what's possible with open source development. Thank you for creating something so special and sharing it with the world!

---

## Executive Summary

This release brings major UI/UX improvements focused on startup experience and visual polish. Building on extensive 16-track, modulator, and microtiming features from v0.0.50-0.0.62, this version refines the user interface with professionally converted SVG logo, enhanced visual feedback, and improved layouts.

**Key Highlights**:
- SVG-based startup logo (converted from PEW_FORMER_LOGO.svg)
- Animated space invaders with bobbing motion
- Enlarged heart symbols for better visibility
- Fixed keyboard page (proper 2-octave: 14 white, 10 black keys)
- Improved pattern number positioning
- Dynamic two-column list layouts
- Python SVG-to-code converter tool

---

## UI/UX Enhancements

### Startup Screen (StartupPage.cpp)
- **SVG Logo**: Converted 363×75 SVG to optimized 87×18px (45 rectangles)
- **Space Invaders**: 6 animated invaders (3 per side, 16×16px) with odd/even bobbing
- **Heart Symbols**: Enlarged 5×5 → 9×7 pixels with y-4 offset for text alignment
- **Credits**: "WESTLICHT ❤️ MODULOVE ❤️ PHASERVILLE"

### Keyboard Page (KeyboardPage.cpp/h) - New
- **Proper 2-octave layout**: 14 white keys, 10 black keys
- **Black key positions**: [1,2,4,5,6,8,9,11,12,13]
- **Persistent last note display**
- **Info relocated to bottom**: Track/Root/Octave moved from top

### Pattern Page (PatternPage.cpp)
- **Fixed positioning**: Pattern numbers moved from y+13 to y+7 (subtract to move UP)

### List Pages (ListPage.cpp)
- **Dynamic column sizing**: Content-aware width calculation
- **No wasted space**: Column 0 gets exact width needed, column 1 fills remainder

---

## Previous Features (v0.0.50-0.0.62)

From commit 84ec511:

### 16-Track Support
- Dual bank system (tracks 1-8, 9-16)
- Extended MIDI routing
- Bank 2 LED distinction (red LEDs)
- Limitations: 8 patterns/track, 4 song slots

### 8 LFO Modulators
- Sine, Triangle, Saw, Square, Random shapes
- Waveform preview
- Routing popup for MIDI CC/Note mapping
- Triplet & dotted divisions

### Microtiming Recording
- timingQuantize parameter (0-100%)
- Captures gate timing offset (0-15)
- Blends natural feel with grid precision

### Performer Page Enhancements
- Pattern mode LEDs (Green=Active, Red=Muted, Yellow=Selected)
- Dimmed muted track numbers
- All-tracks 16-track view

---

## Developer Tools

### SVG to Logo Converter (svg_to_logo_converter.py)
- Input: SVG with rect elements + transforms
- Output: C++ fillRect() code + ASCII preview
- Features: Matrix math, preview modes (18/24/36px), rectangle merging
- Location: `~/Desktop/svg_to_logo_converter.py`

---

## Bug Fixes

### 1. Keyboard Layout (v0.0.56-58)
- Fixed: 16 white, 14 black → 14 white, 10 black keys

### 2. Pattern Positioning (v0.0.61-62)
- Fixed: Y-axis inversion (subtract to move UP, not down)

### 3. Logo Boot Failure (v0.0.63 initial)
- Fixed: 2x scaling caused boot failure
- Solution: Proper SVG conversion with Python tool

---

## Code Statistics

### Memory Usage
```
Version     Text      Data    BSS       Total
v0.0.50     360180    6712    153400    520292
v0.0.62     361412    6712    153400    521524
v0.0.63     361028    6712    153404    521144
```

### Files Changed (This Session)
```
38 files changed
+1865 insertions
-398 deletions
```

---

## Comparison with Original Codebases

### westlicht/performer (Original)
- 8-track sequencer
- Core features all preserved in POW|FORMER

### djphazer/performer (PEW|FORMER)
- Updated toolchain (gcc 14.2)
- jackpf improvements (noise, shapes, MIDI)
- CI workflow

### modulove/performer (POW|FORMER) - Unique
- ⭐ 16-track system
- ⭐ 8 LFO modulators
- ⭐ Microtiming recording
- ⭐ Custom startup screen
- ⭐ Keyboard page
- ⭐ Enhanced performer page

### Features Removed: None
All core functionality preserved. Only limitations: 8 patterns/track (was 16) due to memory for 16 tracks.

---

## Design Philosophy

**Original**: Studio-focused, precise sequencing, deep editing
**POW|FORMER**: Live performance-focused, visual feedback, quick access, real-time modulation

---

## 🙏 Credits & Acknowledgments

### Core Development
- **westlicht** (Simon Kallweit) - Original PER|FORMER firmware architecture and implementation
- **djphazer** (Phazerville) - PEW|FORMER fork, toolchain updates to gcc 14.2, libopencm3 updates, CI/CD workflow
- **jackpf** - Noise reduction algorithms, shape improvements, MIDI enhancements (integrated via Phazerville)
- **modulove** - POW|FORMER 16-track system, 8 LFO modulators, microtiming, UI enhancements (this fork)
- **Claude** (Anthropic) - AI-assisted implementation, debugging, and documentation

### Special Recognition
- **jackpf's contributions** (via Phazerville fork):
  - Advanced noise reduction for cleaner CV outputs
  - Improved waveform shapes and algorithms
  - Enhanced MIDI implementation and timing
  - These improvements form the foundation of POW|FORMER's audio quality

### Community
- **Phazerville community** - Ongoing testing and feedback
- **westlicht community** - Original user base and feature requests
- **YOU!** - Testing this experimental build and helping improve it

---

## 🤝 Contributing

This build is **experimental** and needs community testing! Ways to help:

### Testing Needed
- Try the new startup screen - does it boot reliably?
- Test the keyboard page - is the layout intuitive?
- Check pattern number positioning across different pages
- Validate 16-track functionality with new UI changes
- Test modulators with new UI improvements
- Long-running stability tests

### How to Contribute
1. **Test & Report**: Use the firmware and report any issues
2. **Documentation**: Help improve user guides and documentation
3. **Code Review**: Review changes and suggest improvements
4. **Feature Ideas**: Propose enhancements that fit the live performance focus

### Contact & Support
- **Repository**: https://github.com/modulove/performer
- **Issues**: Report bugs and feature requests via GitHub Issues
- **Discussions**: Share feedback and ideas

---

## 📄 License

GPL-3.0 (inherited from westlicht/performer)

All contributors retain copyright to their contributions while licensing under GPL-3.0.

