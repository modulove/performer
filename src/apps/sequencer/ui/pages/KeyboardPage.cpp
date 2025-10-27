#include "KeyboardPage.h"

#include "ui/LedPainter.h"
#include "ui/painters/WindowPainter.h"

#include "core/utils/StringBuilder.h"
#include "core/midi/MidiMessage.h"
#include "model/Types.h"

#include "os/os.h"

KeyboardPage::KeyboardPage(PageManager &manager, PageContext &context) :
    BasePage(manager, context)
{
    for (int i = 0; i < 16; ++i) {
        _keyStates[i].pressed = false;
        _keyStates[i].note = -1;
    }
}

void KeyboardPage::enter() {
    _selectedTrackIndex = _project.selectedTrackIndex();
    _octave = 4;
    _frameOffset = 0;
    _rootOffset = 0;
    _lastNotePlayed = -1;  // Reset last note on page enter

    for (int i = 0; i < 16; ++i) {
        _keyStates[i].pressed = false;
        _keyStates[i].note = -1;
    }
}

void KeyboardPage::exit() {
    for (int i = 0; i < 16; ++i) {
        if (_keyStates[i].pressed) {
            releaseNote(i);
        }
    }
}

// Map hardware buttons to notes
// S9-S16 (step 8-15) = white keys (8 visible keys)
// S2,S3,S5,S6,S7 (steps 1,2,4,5,6) = black keys (5 black keys in piano pattern 2-3)
int KeyboardPage::noteForKey(int step) const {
    int baseNote = _octave * 12 + _rootOffset;
    
    // White keys mapping (S9-S16 map to 8 semitones starting at rootOffset)
    if (step >= 8 && step <= 15) {
        int keyIndex = step - 8;  // 0-7
        return baseNote + keyIndex;
    }
    
    // Black keys - standard piano pattern within active window
    // Pattern: C# D# _ F# G# A# (positions 1,3,_,6,8,10 in chromatic scale)
    // Map to buttons: S2(pos 1), S3(pos 3), S5(pos 6), S6(pos 8), S7(pos 10)
    if (step == 1) return baseNote + 1;   // C#
    if (step == 2) return baseNote + 3;   // D#
    if (step == 4) return baseNote + 6;   // F#
    if (step == 5) return baseNote + 8;   // G#
    if (step == 6) return baseNote + 10;  // A#
    
    return -1;
}

void KeyboardPage::playNote(int step) {
    int note = noteForKey(step);
    if (note < 0 || note > 127) return;

    float cv = (note - 60) / 12.f;
    _engine.midiOutputEngine().sendGate(_selectedTrackIndex, true);
    _engine.midiOutputEngine().sendCv(_selectedTrackIndex, cv);

    _keyStates[step].pressed = true;
    _keyStates[step].note = note;
    _lastNotePlayed = note;  // Store last note for persistent display
}

void KeyboardPage::releaseNote(int step) {
    if (!_keyStates[step].pressed || _keyStates[step].note < 0) return;

    _engine.midiOutputEngine().sendGate(_selectedTrackIndex, false);

    _keyStates[step].pressed = false;
    _keyStates[step].note = -1;
}

void KeyboardPage::draw(Canvas &canvas) {
    WindowPainter::clear(canvas);

    FixedStringBuilder<32> str;
    str("KEYBOARD");
    WindowPainter::drawHeader(canvas, _model, _engine, str);

    // Function buttons: F1, F2, F3=EXIT, F4=OCT-, F5=OCT+, F6
    const char *functionNames[] = { nullptr, nullptr, "EXIT", "OCT-", "OCT+", nullptr };
    WindowPainter::drawFooter(canvas, functionNames, pageKeyState());

    // Draw 14 white keys (2 octaves = 7 white keys per octave)
    const int whiteKeyWidth = 12;  // 12px per white key
    const int whiteKeyHeight = 34;
    const int blackKeyWidth = 9;   // 9px per black key
    const int blackKeyHeight = 20;
    const int totalWidth = 14 * whiteKeyWidth;  // 168px total
    const int startX = (256 - totalWidth) / 2;  // Center: 44px margin each side
    const int startY = 15;
    const int keyboardMidY = startY + (whiteKeyHeight / 2);  // Vertical center of keyboard

    const char *noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

    canvas.setFont(Font::Small);
    canvas.setColor(Color::Bright);
    canvas.setBlendMode(BlendMode::Set);

    int leftMargin = startX;
    int keyboardEndX = startX + totalWidth;
    int rightMargin = 256 - keyboardEndX;

    // Move track/root/octave to BOTTOM of window (below footer)
    const int bottomY = 56;  // Just above footer line at y=57

    // Track number at bottom left
    canvas.setFont(Font::Tiny);
    FixedStringBuilder<8> trackStr;
    trackStr("T%d", _selectedTrackIndex + 1);
    int trackTextWidth = canvas.textWidth(trackStr);
    int trackX = (leftMargin - trackTextWidth) / 2;
    canvas.drawText(trackX, bottomY, trackStr);

    // Root note/octave at bottom (BOLD) - centered in left margin
    canvas.setFont(Font::Small);  // Use Small (bold) instead of Tiny
    FixedStringBuilder<8> rootStr;
    rootStr("%s%d", noteNames[_rootOffset], _octave);
    int rootTextWidth = canvas.textWidth(rootStr);
    int rootX = (leftMargin - rootTextWidth) / 2;
    canvas.drawText(rootX, bottomY + 8, rootStr);  // 8px below track number

    // Last note played - persistent display at bottom right
    if (_lastNotePlayed >= 0) {
        canvas.setFont(Font::Small);
        int noteNum = _lastNotePlayed % 12;
        int noteOct = (_lastNotePlayed / 12) - 1;
        FixedStringBuilder<8> lastNoteStr;
        lastNoteStr("%s%d", noteNames[noteNum], noteOct);
        int lastNoteWidth = canvas.textWidth(lastNoteStr);
        int lastNoteX = keyboardEndX + (rightMargin - lastNoteWidth) / 2;
        canvas.drawText(lastNoteX, bottomY + 8, lastNoteStr);  // Align with root note
    }

    // Draw left pagination arrow - closer to keys, centered vertically with keyboard
    if (_rootOffset > 0) {
        canvas.setColor(Color::Bright);
        int arrowX = startX - 14;  // Closer to keys (was in center of margin)
        int arrowY = keyboardMidY;  // Vertically centered with keyboard
        canvas.fillRect(arrowX, arrowY, 2, 2);
        canvas.fillRect(arrowX + 2, arrowY - 1, 2, 4);
        canvas.fillRect(arrowX + 4, arrowY - 2, 2, 6);
    }

    // Draw right pagination arrow - closer to keys, centered vertically with keyboard
    if (_rootOffset < 11) {
        canvas.setColor(Color::Bright);
        int arrowX = keyboardEndX + 6;  // Closer to keys (was in center of margin)
        int arrowY = keyboardMidY;  // Vertically centered with keyboard
        canvas.fillRect(arrowX + 4, arrowY - 2, 2, 6);
        canvas.fillRect(arrowX + 6, arrowY - 1, 2, 4);
        canvas.fillRect(arrowX + 8, arrowY, 2, 2);
    }

    // Draw all 14 white keys (2 octaves)
    canvas.setBlendMode(BlendMode::Set);
    for (int i = 0; i < 14; ++i) {
        int x = startX + i * whiteKeyWidth;
        int y = startY;
        
        // Determine if this key is in the active 8-key window
        bool active = (i >= _rootOffset && i < _rootOffset + 8);
        
        // Check if this key is currently being pressed
        int step = (i - _rootOffset) + 8;  // Map to S9-S16
        bool pressed = (active && step >= 8 && step <= 15 && _keyStates[step].pressed);
        
        // Draw key outline
        canvas.setColor(active ? Color::Bright : Color::Low);
        canvas.drawRect(x, y, whiteKeyWidth - 1, whiteKeyHeight);
        
        // Fill key
        if (pressed) {
            canvas.setColor(Color::Medium);
        } else if (active) {
            canvas.setColor(Color::Low);
        } else {
            canvas.setColor(Color::None);
        }
        canvas.fillRect(x + 1, y + 1, whiteKeyWidth - 3, whiteKeyHeight - 2);
    }

    // Draw ALL black keys in standard piano pattern
    // 14 white keys (2 full octaves) require 10 black keys
    // Pattern per octave: C-C#-D-D#-E-F-F#-G-G#-A-A#-B (5 black keys per octave)
    // White keys: C(0) D(1) E(2) F(3) G(4) A(5) B(6) | C(7) D(8) E(9) F(10) G(11) A(12) B(13)
    // Black keys appear BETWEEN white keys at positions:
    // Octave 1: C#(1), D#(2), F#(4), G#(5), A#(6)
    // Octave 2: C#(8), D#(9), F#(11), G#(12), A#(13)
    // No black keys after E(2) or B(6,13)
    int allBlackKeyPositions[] = {1, 2, 4, 5, 6, 8, 9, 11, 12, 13};  // 10 black keys total
    int numBlackKeys = 10;

    for (int i = 0; i < numBlackKeys; ++i) {
        int screenPos = allBlackKeyPositions[i];

        // Position black key between white keys on screen
        int x = startX + screenPos * whiteKeyWidth - (blackKeyWidth / 2);
        int y = startY;

        // Calculate which chromatic position this represents relative to rootOffset
        int chromPos = screenPos;

        // Check if in active 8-key window
        bool active = (chromPos >= _rootOffset && chromPos < _rootOffset + 8);

        // Check if pressed (only if in active window and matches hardware button)
        bool pressed = false;
        if (active) {
            // Map screen position to hardware step
            int relativePos = chromPos - _rootOffset;
            int blackKeySteps[] = {1, 2, 4, 5, 6};  // S2,S3,S5,S6,S7
            int blackKeyChromaticPos[] = {1, 3, 6, 8, 10};  // Within active window

            for (int j = 0; j < 5; ++j) {
                if (relativePos == blackKeyChromaticPos[j]) {
                    pressed = _keyStates[blackKeySteps[j]].pressed;
                    break;
                }
            }
        }

        // Draw black key
        if (pressed) {
            canvas.setColor(Color::Bright);
            canvas.drawRect(x, y, blackKeyWidth, blackKeyHeight);
            canvas.setColor(Color::Medium);
            canvas.fillRect(x + 1, y + 1, blackKeyWidth - 2, blackKeyHeight - 2);
        } else {
            canvas.setColor(active ? Color::Medium : Color::Low);
            canvas.drawRect(x, y, blackKeyWidth, blackKeyHeight);
            canvas.setColor(Color::None);
            canvas.fillRect(x + 1, y + 1, blackKeyWidth - 2, blackKeyHeight - 2);
        }
    }

    // Show pressed note and octave on right (BOLD) - centered vertically with keyboard
    canvas.setFont(Font::Small);
    canvas.setColor(Color::Bright);
    canvas.setBlendMode(BlendMode::Set);

    for (int i = 0; i < 16; ++i) {
        if (_keyStates[i].pressed && _keyStates[i].note >= 0) {
            int octave = _keyStates[i].note / 12;
            int noteIndex = _keyStates[i].note % 12;

            // Note name - centered vertically with keyboard
            FixedStringBuilder<8> noteStr;
            noteStr("%s", noteNames[noteIndex]);
            int noteTextWidth = canvas.textWidth(noteStr);
            int noteX = keyboardEndX + (rightMargin - noteTextWidth) / 2;
            int noteY = keyboardMidY - 10;  // Match track number vertical position
            canvas.drawText(noteX, noteY, noteStr);

            // Octave number
            canvas.setFont(Font::Small);  // Use Small (bold) instead of Tiny
            FixedStringBuilder<8> octaveStr;
            octaveStr("O%d", octave);
            int octaveTextWidth = canvas.textWidth(octaveStr);
            int octaveX = keyboardEndX + (rightMargin - octaveTextWidth) / 2;
            int octaveY = noteY + 12;  // Below note name
            canvas.drawText(octaveX, octaveY, octaveStr);

            break;
        }
    }
}

void KeyboardPage::updateLeds(Leds &leds) {
    // Clear all LEDs first
    for (int i = 0; i < 16; ++i) {
        leds.set(i, false, false);
    }

    // White keys (S9-S16): Solid GREEN LEDs always on, inverted addressing
    for (int i = 8; i < 16; ++i) {
        int ledIndex = 15 - i;  // S9=LED6, S10=LED5, ..., S16=LED0
        // Always green (not red), brighter when pressed
        leds.set(ledIndex, false, true);  // Green
    }

    // Black keys (S2,S3,S5,S6,S7): Red LEDs that scroll with encoder
    // Piano pattern within active 8-key window: positions 1,3,6,8,10
    int blackKeySteps[] = {1, 2, 4, 5, 6};  // S2,S3,S5,S6,S7
    int blackKeyChromaticPos[] = {1, 3, 6, 8, 10};  // Relative to rootOffset

    for (int i = 0; i < 5; ++i) {
        int step = blackKeySteps[i];
        int chromPos = blackKeyChromaticPos[i];
        int ledIndex = 15 - step;  // Inverted: S2=LED13, S3=LED12, S5=LED10, S6=LED9, S7=LED8

        // Check if this black key position is within the active window
        bool inActiveRange = (chromPos >= _rootOffset && chromPos < _rootOffset + 8);

        // Red LED on when in active range
        leds.set(ledIndex, inActiveRange, false);  // Red when active
    }

    // Pagination LEDs: Use Global4 (left) and Global5 (right) - step page indicators
    leds.set(Key::Global4, false, _rootOffset > 0);   // Left: green when can scroll left
    leds.set(Key::Global5, false, _rootOffset < 12);  // Right: green when can scroll right (11 is max)
}

void KeyboardPage::keyDown(KeyEvent &event) {
    const auto &key = event.key();

    if (key.isStep()) {
        int step = key.step();
        if (noteForKey(step) >= 0) {
            playNote(step);
            event.consume();
        }
    }
}

void KeyboardPage::keyUp(KeyEvent &event) {
    const auto &key = event.key();

    if (key.isStep()) {
        int step = key.step();
        if (noteForKey(step) >= 0) {
            releaseNote(step);
            event.consume();
        }
    }
}

void KeyboardPage::keyPress(KeyPressEvent &event) {
    const auto &key = event.key();

    // Track selection with double-tap
    if (key.isTrackSelect() && !key.pageModifier()) {
        int trackButton = key.trackSelect();
        int targetTrack = (_trackBank * 8) + trackButton;

        for (int i = 0; i < 16; ++i) {
            if (_keyStates[i].pressed) {
                releaseNote(i);
            }
        }

        uint32_t now = os::ticks();
        uint32_t doubleTapWindow = os::time::ms(400);

        if (_lastTrackPressed == trackButton &&
            (now - _lastTrackPressTime[trackButton]) < doubleTapWindow) {
            targetTrack = ((1 - _trackBank) * 8) + trackButton;
            _trackBank = 1 - _trackBank;
            _lastTrackPressed = -1;
        } else {
            _lastTrackPressed = trackButton;
            _lastTrackPressTime[trackButton] = now;
        }

        _selectedTrackIndex = targetTrack;
        _project.setSelectedTrackIndex(targetTrack);
        event.consume();
        return;
    }

    // F3 (center) = Exit
    if (key.is(Key::F2)) {
        close();
        event.consume();
        return;
    }

    // F4 = Octave down (jump by 12 semitones)
    if (key.is(Key::F3)) {
        if (_octave > 0) {
            for (int i = 0; i < 16; ++i) {
                if (_keyStates[i].pressed) {
                    releaseNote(i);
                }
            }
            --_octave;
        }
        event.consume();
        return;
    }

    // F5 = Octave up (jump by 12 semitones)
    if (key.is(Key::F4)) {
        if (_octave < 9) {
            for (int i = 0; i < 16; ++i) {
                if (_keyStates[i].pressed) {
                    releaseNote(i);
                }
            }
            ++_octave;
        }
        event.consume();
        return;
    }

    // Left/Right shift semitone (same as encoder)
    if (key.is(Key::Left)) {
        if (_rootOffset > 0) {
            for (int i = 0; i < 16; ++i) {
                if (_keyStates[i].pressed) {
                    releaseNote(i);
                }
            }
            --_rootOffset;
        }
        event.consume();
        return;
    }

    if (key.is(Key::Right)) {
        if (_rootOffset < 11) {
            for (int i = 0; i < 16; ++i) {
                if (_keyStates[i].pressed) {
                    releaseNote(i);
                }
            }
            ++_rootOffset;
        }
        event.consume();
        return;
    }
}

void KeyboardPage::encoder(EncoderEvent &event) {
    // Encoder shifts root note by 1 semitone (0-11 range)
    int oldOffset = _rootOffset;
    _rootOffset = clamp(_rootOffset + event.value(), 0, 11);

    if (_rootOffset != oldOffset) {
        for (int i = 0; i < 16; ++i) {
            if (_keyStates[i].pressed) {
                releaseNote(i);
            }
        }
    }
}
