#pragma once

#include "BasePage.h"

class KeyboardPage : public BasePage {
public:
    KeyboardPage(PageManager &manager, PageContext &context);

    virtual void enter() override;
    virtual void exit() override;

    virtual void draw(Canvas &canvas) override;
    virtual void updateLeds(Leds &leds) override;

    virtual bool isModal() const override { return true; }

    virtual void keyDown(KeyEvent &event) override;
    virtual void keyUp(KeyEvent &event) override;
    virtual void keyPress(KeyPressEvent &event) override;
    virtual void encoder(EncoderEvent &event) override;

private:
    struct KeyState {
        bool pressed = false;
        int note = -1;
    };

    int noteForKey(int step) const;
    void playNote(int step);
    void releaseNote(int step);
    void drawKey(Canvas &canvas, int keyIndex, bool isBlack, bool pressed, bool active);

    int _octave = 4;  // Middle octave (C4 = MIDI note 60)
    int _selectedTrackIndex = 0;
    KeyState _keyStates[16];
    uint32_t _lastPerformerPressTicks = 0;

    // Track switching with double-tap
    int _trackBank = 0;  // 0 = tracks 0-7, 1 = tracks 8-15
    uint32_t _lastTrackPressTime[8] = {0};
    int _lastTrackPressed = -1;

    // Pagination: 24 keys (2 octaves) shown in 3 frames of 8 keys
    int _frameOffset = 0;  // 0, 1, or 2 (which frame of 8 keys is shown)
    int _rootOffset = 0;   // 0-11 (encoder shift within the 2 octaves)

    // Last note played (for persistent display)
    int _lastNotePlayed = -1;  // MIDI note number, -1 if none
};
