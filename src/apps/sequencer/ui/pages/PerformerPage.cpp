#include "PerformerPage.h"

#include "Config.h"

#include "ui/LedPainter.h"
#include "ui/painters/WindowPainter.h"
#include "ui/painters/SequencePainter.h"
#include "core/utils/MathUtils.h"
#include "core/utils/StringBuilder.h"

enum class Function {
    Latch   = 0,
    Sync    = 1,
    Unmute  = 2,
    Fill    = 3,
    Pattern = 4,
    Cancel  = 5
};

PerformerPage::PerformerPage(PageManager &manager, PageContext &context) :
    BasePage(manager, context)
{}

void PerformerPage::enter() {
    _latching = false;
    _syncing = false;
}

void PerformerPage::exit() {
    _project.playState().commitLatchedRequests();
}

void PerformerPage::draw(Canvas &canvas) {
    const auto &playState = _project.playState();
    bool hasCancel = playState.hasSyncedRequests() || playState.hasLatchedRequests();
    const char *functionNames[] = { "LATCH", "SYNC", "UNMUTE", "FILL", "PATTERN", hasCancel ? "CANCEL" : nullptr };

    WindowPainter::clear(canvas);
    WindowPainter::drawHeader(canvas, _model, _engine, _allTracksView ? "PERFORM ALL" : "PERFORM");
    WindowPainter::drawFooter(canvas, functionNames, pageKeyState(), _patternMode ? int(Function::Pattern) : -1);

    constexpr int Border = 4;
    constexpr int BorderRequested = 6;

    bool hasRequested = false;

    canvas.setFont(Font::Tiny);
    canvas.setBlendMode(BlendMode::Set);

    // UX-23: All-tracks view shows all 16 tracks in compact layout
    int numTracks = _allTracksView ? 16 : 8;
    int trackOffset = _allTracksView ? 0 : ((_project.selectedTrackIndex() >= 8) ? 8 : 0);

    for (int i = 0; i < numTracks; ++i) {
        int trackIndex = trackOffset + i;
        const auto &trackEngine = _engine.trackEngine(trackIndex);
        const auto &trackState = playState.trackState(trackIndex);

        // UX-23: Compact layout for 16-track view (2 rows of 8) - minimal, no labels
        int x, y, w, h;
        if (_allTracksView) {
            int col = i % 8;
            int row = i / 8;
            x = col * 30 + 10;  // Simpler spacing
            y = 18 + (row * 14);  // Two rows, closer together
            w = 24;  // Slightly larger boxes
            h = 12;  // Compact height
        } else {
            x = i * 32;
            y = 16;
            w = 16;
            h = 16;
            x += 8;
        }

        if (_allTracksView) {
            // Simplified all-tracks view: just boxes for mute toggle
            // Reset blend mode at start of each track to avoid state pollution
            canvas.setBlendMode(BlendMode::Set);

            // draw outer rectangle (track activity)
            canvas.setColor(trackEngine.activity() ? Color::Bright : Color::Medium);
            canvas.drawRect(x, y, w, h);

            // draw mutes and mute requests
            bool isMuted = trackState.mute();
            if (trackState.hasMuteRequest() && trackState.mute() != trackState.requestedMute()) {
                hasRequested = true;
                canvas.setColor(Color::Bright);
                canvas.fillRect(x + BorderRequested, y + BorderRequested, w - 2 * BorderRequested, h - 2 * BorderRequested);
            } else if (isMuted) {
                // Fill entire box with dimmed background for better text visibility
                canvas.setColor(Color::MediumBright);
                canvas.fillRect(x + 1, y + 1, w - 2, h - 2);
            }

            // draw label inside the box
            // In pattern mode: always show pattern number (P1-P16), dark on white if muted
            // In normal mode: show track number (T1-T16) only when not muted
            if (_patternMode) {
                int patternNum = trackState.pattern() + 1;
                if (isMuted) {
                    // Dark text on white background: use Sub mode to darken
                    canvas.setBlendMode(BlendMode::Sub);
                    canvas.setColor(Color::Bright);
                } else {
                    // White text on dark background: use Add mode to lighten
                    canvas.setBlendMode(BlendMode::Add);
                    canvas.setColor(Color::Bright);
                }
                canvas.drawTextCentered(x, y + 2, w, 8, FixedStringBuilder<8>("P%d", patternNum));
            } else if (!isMuted) {
                // Show track number only when not muted
                canvas.setBlendMode(BlendMode::Add);
                canvas.setColor(Color::Bright);
                canvas.drawTextCentered(x, y + 2, w, 8, FixedStringBuilder<8>("T%d", trackIndex + 1));
            }
        } else {
            // Normal 8-track view with labels and progress
            // Always show track number above (T1-T16), highlight when fill is active
            canvas.setBlendMode(BlendMode::Add);
            canvas.setColor(trackState.fill() ? Color::Bright : Color::Medium);
            canvas.drawTextCentered(x, y - 2, w, 8, FixedStringBuilder<8>("T%d", trackIndex + 1));

            y += 8;

            // Reset blend mode before drawing rectangles
            canvas.setBlendMode(BlendMode::Set);

            // draw outer rectangle (track activity)
            canvas.setColor(trackEngine.activity() ? Color::Bright : Color::Medium);
            canvas.drawRect(x, y, w, h);

            // draw mutes and mute requests
            bool isMuted = trackState.mute();
            if (trackState.hasMuteRequest() && trackState.mute() != trackState.requestedMute()) {
                hasRequested = true;
                canvas.setColor(Color::Bright);
                canvas.fillRect(x + BorderRequested, y + BorderRequested, w - 2 * BorderRequested, h - 2 * BorderRequested);
            } else if (isMuted) {
                // Fill entire box with dimmed background for better text visibility
                canvas.setColor(Color::MediumBright);
                canvas.fillRect(x + 1, y + 1, w - 2, h - 2);
            }

            // In pattern mode: draw pattern number inside the square for quick feedback
            if (_patternMode) {
                int patternNum = trackState.pattern() + 1;
                if (isMuted) {
                    // Dark text on white background: use Sub mode to darken
                    canvas.setBlendMode(BlendMode::Sub);
                    canvas.setColor(Color::Bright);
                } else {
                    // White text on dark background: use Add mode to lighten
                    canvas.setBlendMode(BlendMode::Add);
                    canvas.setColor(Color::Bright);
                }
                // Center vertically within the box
                canvas.drawTextCentered(x, y + (h - 8) / 2, w, 8, FixedStringBuilder<8>("P%d", patternNum));
            }

            // draw sequence progress
            SequencePainter::drawSequenceProgress(canvas, x, y + h + 2, w, 2, trackEngine.sequenceProgress());

            // draw fill & fill amount amount
            // Use 'i' (0-7) for button detection, not trackIndex (0-15)
            bool pressed = pageKeyState()[MatrixMap::fromStep(i)];
            canvas.setColor(pressed ? Color::Medium : Color::Low);
            canvas.fillRect(x, y + h + 6, w, 4);
            canvas.setColor(pressed ? Color::Bright : Color::Medium);
            canvas.fillRect(x, y + h + 6, (trackState.fillAmount() * w) / 100, 4);
        }
    }

    if (playState.hasSyncedRequests() && hasRequested) {
        canvas.setColor(Color::Bright);
        canvas.hline(0, 10, _engine.syncFraction() * Width);
    }
}

void PerformerPage::updateLeds(Leds &leds) {
    const auto &playState = _project.playState();

    if (_allTracksView) {
        // UX-23: All-tracks view
        for (int i = 0; i < 8; ++i) {
            const auto &trackState1 = playState.trackState(i);
            const auto &trackState2 = playState.trackState(i + 8);

            if (_patternMode) {
                // Pattern mode: green=active, red=muted, yellow=selected
                bool selected1 = (_selectedTracks & (1 << i)) != 0;
                bool selected2 = (_selectedTracks & (1 << (i + 8))) != 0;
                bool mute1 = trackState1.mute();
                bool mute2 = trackState2.mute();

                // Bank 1: yellow if selected, else green if active, else red if muted
                bool red1 = selected1 || mute1;
                bool green1 = selected1 || !mute1;
                leds.set(MatrixMap::fromTrack(i), red1, green1);

                // Bank 2: yellow if selected, else green if active, else red if muted
                bool red2 = selected2 || mute2;
                bool green2 = selected2 || !mute2;
                leds.set(MatrixMap::fromStep(i), red2, green2);
            } else {
                // Normal mode: show mute status
                bool mute1 = trackState1.mute();
                bool mute2 = trackState2.mute();
                // Track LEDs (bank 1): green = active (not muted), red = muted
                leds.set(MatrixMap::fromTrack(i), mute1, !mute1);
                // Step LEDs (bank 2): green = active (not muted), red = muted (same as bank 1)
                leds.set(MatrixMap::fromStep(i), mute2, !mute2);
            }
        }

        // Turn off unused step LEDs
        for (int i = 8; i < 16; ++i) {
            leds.set(MatrixMap::fromStep(i), false, false);
        }
    } else {
        // Normal 8-track view
        int trackOffset = (_project.selectedTrackIndex() >= 8) ? 8 : 0;

        if (_patternMode) {
            // Pattern mode: green=active, red=muted, yellow=selected
            for (int i = 0; i < 8; ++i) {
                int trackIndex = trackOffset + i;
                const auto &trackState = playState.trackState(trackIndex);
                bool selected = (_selectedTracks & (1 << trackIndex)) != 0;
                bool mute = trackState.mute();

                // Yellow if selected, else green if active, else red if muted
                bool red = selected || mute;
                bool green = selected || !mute;
                leds.set(MatrixMap::fromTrack(i), red, green);
            }
        } else {
            // Normal mode: show track activity and fill
            LedPainter::drawTrackGatesAndSelectedTrack(leds, _engine, _project.playState(), _project.selectedTrackIndex());
        }

        uint8_t activeFills = 0;
        for (int i = 0; i < 8; ++i) {
            int trackIndex = trackOffset + i;
            const auto &trackState = playState.trackState(trackIndex);
            activeFills |= trackState.fill() ? (1<<i) : 0;
        }

        if (!_patternMode) {
            LedPainter::drawMutes(leds, 0, 0);
        }
        LedPainter::drawFills(leds, activeFills);
    }
}

void PerformerPage::keyDown(KeyEvent &event) {
    const auto &key = event.key();

    if (key.isFunction()) {
        switch (Function(key.function())) {
        case Function::Latch:
            _latching = true;
            break;
        case Function::Sync:
            _syncing = true;
            break;
        case Function::Fill:
            updateFills();
            break;
        case Function::Pattern:
            // Toggle pattern mode on/off
            _patternMode = !_patternMode;
            if (!_patternMode) {
                // Clear selection when exiting pattern mode
                _selectedTracks = 0;
            }
            break;
        default:
            break;
        }
        event.consume();
    }

    if (key.isStep()) {
        updateFills();
        event.consume();
    }
}

void PerformerPage::keyUp(KeyEvent &event) {
    const auto &key = event.key();

    bool closePage = false;

    if (key.isPerformer()) {
        closePage = true;
        event.consume();
    }

    if (key.isFunction()) {
        switch (Function(key.function())) {
        case Function::Latch:
            closePage = true;
            _latching = false;
            _project.playState().commitLatchedRequests();
            break;
        case Function::Sync:
            closePage = true;
            _syncing = false;
            break;
        case Function::Fill:
            updateFills();
            break;
        case Function::Pattern:
            // Pattern is toggle - no action needed on key up
            break;
        default:
            break;
        }
        event.consume();
    }

    if (key.isStep()) {
        closePage = true;
        updateFills();
        event.consume();
    }

    bool stepKeyPressed = false;
    for (int step = 0; step < 16; ++step) {
        stepKeyPressed |= pageKeyState()[MatrixMap::fromStep(step)];
    }

    bool canClose = _modal && !_latching && !_syncing && !globalKeyState()[Key::Performer] && !stepKeyPressed;
    if (canClose && closePage) {
        close();
    }
}

void PerformerPage::keyPress(KeyPressEvent &event) {
    const auto &key = event.key();
    auto &playState = _project.playState();

    if (key.pageModifier()) {
        return;
    }

    // Prev/Next: Bank 1 (T1-8) -> Bank 2 (T9-16) -> All Tracks View (UX-23)
    if (key.isRight()) {
        if (_allTracksView) {
            // Already at end, do nothing
        } else {
            int currentTrack = _project.selectedTrackIndex();
            if (currentTrack < 8) {
                // Bank 1 -> Bank 2
                _project.setSelectedTrackIndex(currentTrack + 8);
            } else {
                // Bank 2 -> All Tracks View
                _allTracksView = true;
            }
        }
        event.consume();
        return;
    }
    if (key.isLeft()) {
        if (_allTracksView) {
            // All Tracks -> Bank 2
            _allTracksView = false;
            int currentTrack = _project.selectedTrackIndex();
            if (currentTrack < 8) {
                _project.setSelectedTrackIndex(currentTrack + 8);
            }
        } else {
            int currentTrack = _project.selectedTrackIndex();
            if (currentTrack >= 8) {
                // Bank 2 -> Bank 1
                _project.setSelectedTrackIndex(currentTrack - 8);
            }
        }
        event.consume();
        return;
    }

    // use immediate by default
    // use latched when LATCH is pressed
    // use synced when SYNC is pressed
    PlayState::ExecuteType executeType = _latching ? PlayState::Latched : (_syncing ? PlayState::Synced : PlayState::Immediate);

    if (key.isFunction()) {
        switch (Function(key.function())) {
        case Function::Latch:
            break;
        case Function::Sync:
            break;
        case Function::Unmute:
            playState.unmuteAll(executeType);
            break;
        case Function::Fill:
            updateFills();
            break;
        case Function::Pattern:
            // Pattern mode handled in keyDown/keyUp
            break;
        case Function::Cancel:
            playState.cancelMuteRequests();
            break;
        }
        event.consume();
    }

    if (key.isTrackSelect()) {
        int trackIndex;

        if (_allTracksView) {
            // UX-23: In all-tracks view, tracks 1-8 directly map to tracks 1-8
            trackIndex = key.track();
        } else {
            // Normal view: use bank selection
            int trackOffset = (_project.selectedTrackIndex() >= 8) ? 8 : 0;
            trackIndex = trackOffset + key.track();
        }

        if (_patternMode) {
            // In pattern mode with shift: toggle track selection for multi-track pattern browsing
            if (key.shiftModifier()) {
                _selectedTracks ^= (1 << trackIndex);  // Toggle bit
            } else {
                // Without shift: single selection
                _selectedTracks = (1 << trackIndex);
            }
        } else {
            // Normal mode: mute/solo
            if (key.shiftModifier()) {
                playState.soloTrack(trackIndex, executeType);
            } else {
                playState.toggleMuteTrack(trackIndex, executeType);
            }
        }
        event.consume();
    }

    // UX-23: In all-tracks view, step buttons map to tracks 9-16
    if (_allTracksView && key.isStep() && key.step() < 8) {
        int trackIndex = 8 + key.step();  // Steps 0-7 = tracks 9-16

        if (_patternMode) {
            // In pattern mode with shift: toggle track selection for multi-track pattern browsing
            if (key.shiftModifier()) {
                _selectedTracks ^= (1 << trackIndex);  // Toggle bit
            } else {
                // Without shift: single selection
                _selectedTracks = (1 << trackIndex);
            }
        } else {
            // Normal mode: mute/solo
            if (key.shiftModifier()) {
                playState.soloTrack(trackIndex, executeType);
            } else {
                playState.toggleMuteTrack(trackIndex, executeType);
            }
        }
        event.consume();
    }
}

void PerformerPage::encoder(EncoderEvent &event) {
    auto &playState = _project.playState();

    // In pattern mode, encoder changes patterns for selected tracks
    if (_patternMode && _selectedTracks != 0) {
        // Apply pattern change to all selected tracks
        for (int track = 0; track < CONFIG_TRACK_COUNT; ++track) {
            if (_selectedTracks & (1 << track)) {
                int currentPattern = playState.trackState(track).pattern();
                int newPattern = utils::clamp(currentPattern + event.value(), 0, CONFIG_PATTERN_COUNT - 1);
                playState.selectTrackPattern(track, newPattern, PlayState::Immediate);
            }
        }
        event.consume();
    } else if (!_patternMode) {
        // Normal mode: encoder controls fill amount for held step buttons
        int trackOffset = (_project.selectedTrackIndex() >= 8) ? 8 : 0;

        for (int i = 0; i < 8; ++i) {
            int trackIndex = trackOffset + i;
            if (pageKeyState()[MatrixMap::fromStep(i)]) {
                playState.trackState(trackIndex).editFillAmount(event.value(), false);
            }
        }
    }
}

void PerformerPage::updateFills() {
    auto &playState = _project.playState();
    bool fillPressed = pageKeyState()[MatrixMap::fromFunction(int(Function::Fill))];
    bool holdPressed = pageKeyState()[Key::Shift];

    if (_allTracksView) {
        // In all-tracks view: track buttons 0-7 = fills for tracks 1-8, step buttons 8-15 = fills for tracks 9-16
        for (int i = 0; i < 8; ++i) {
            bool trackFill1 = pageKeyState()[MatrixMap::fromStep(8 + i)];  // Step buttons 8-15 for tracks 1-8
            playState.fillTrack(i, trackFill1 || fillPressed, holdPressed);

            // For tracks 9-16, we need a different approach since we don't have individual buttons
            // They only respond to global fill button
            playState.fillTrack(i + 8, fillPressed, holdPressed);
        }
    } else {
        // In bank view, step buttons 8-15 control fills for the current visible bank
        int trackOffset = (_project.selectedTrackIndex() >= 8) ? 8 : 0;

        // Handle fills for the 8 visible tracks in the current bank
        for (int i = 0; i < 8; ++i) {
            int trackIndex = trackOffset + i;
            bool trackFill = pageKeyState()[MatrixMap::fromStep(8 + i)];
            playState.fillTrack(trackIndex, trackFill || fillPressed, holdPressed);
        }

        // Non-visible tracks only respond to global fill button
        for (int trackIndex = 0; trackIndex < CONFIG_TRACK_COUNT; ++trackIndex) {
            if (trackIndex < trackOffset || trackIndex >= trackOffset + 8) {
                playState.fillTrack(trackIndex, fillPressed, holdPressed);
            }
        }
    }
}

