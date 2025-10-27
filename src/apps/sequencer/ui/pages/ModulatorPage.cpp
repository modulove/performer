#include "ModulatorPage.h"

#include "Config.h"
#include "ui/painters/WindowPainter.h"
#include "ui/pages/ContextMenu.h"
#include "engine/WaveformGenerator.h"
#include "core/utils/MathUtils.h"
#include "core/utils/StringBuilder.h"

enum class ModulatorContextAction {
    Route,
    Last
};

static const ContextMenuModel::Item modulatorContextMenuItems[] = {
    { "Route" },
};

ModulatorPage::ModulatorPage(PageManager &manager, PageContext &context) :
    BasePage(manager, context)
{
}

void ModulatorPage::enter() {
    _selectedModulator = _project.selectedModulatorIndex();
    // Initialize routing overlay values based on current modulator index
    _routingCCNum = _selectedModulator;  // Default: Mod 1 → CC 0, Mod 2 → CC 1, etc.
    _routingTargetType = RoutingTargetType::Midi;  // Default to MIDI
    _routingTargetIndex = _selectedModulator + 8;  // Default: Mod 1 → MIDI 9, Mod 2 → MIDI 10, etc.
    _routingEventIsCC = true;  // Default to CC mode
}

void ModulatorPage::exit() {
}

void ModulatorPage::draw(Canvas &canvas) {
    auto &modulator = _project.modulator(_selectedModulator);

    // Determine which parameters to show based on shape and overlay mode
    bool isRandom = (modulator.shape() == Modulator::Shape::Random);
    bool isADSR = (modulator.shape() == Modulator::Shape::ADSR);
    bool isTriggered = isRandom && (modulator.randomMode() == Modulator::RandomMode::Triggered);

    // Update total pages based on shape
    if (isADSR) {
        _totalPages = 2;  // ADSR needs 2 pages (7 parameters total)
    } else {
        _totalPages = 1;  // LFO and Random fit on 1 page
    }

    // Clamp current page
    if (_currentPage >= _totalPages) {
        _currentPage = 0;
    }

    const char *functionNames[6];

    if (_showRoutingOverlay) {
        // Routing overlay mode (F1=MODE, F2=GATE, F3=TARGET, F4=EVENT, F5=CC NUM)
        functionNames[0] = "MODE";
        functionNames[1] = "GATE";
        functionNames[2] = "TARGET";
        functionNames[3] = "EVENT";
        functionNames[4] = "CC NUM";
        functionNames[5] = nullptr;
    } else {
        // Normal parameter mode - support pagination for ADSR
        if (isADSR) {
            if (_currentPage == 0) {
                // ADSR Page 1: SHAPE, ATTACK, DECAY, SUSTAIN, RELEASE
                functionNames[0] = "SHAPE";
                functionNames[1] = "ATTACK";
                functionNames[2] = "DECAY";
                functionNames[3] = "SUSTAIN";
                functionNames[4] = "RELEAS";
                functionNames[5] = nullptr;
            } else {
                // ADSR Page 2: AMPLITUDE, DEPTH, OFFSET
                functionNames[0] = "AMPLIT";
                functionNames[1] = "DEPTH";
                functionNames[2] = "OFSET";
                functionNames[3] = nullptr;
                functionNames[4] = nullptr;
                functionNames[5] = nullptr;
            }
        } else {
            // LFO and Random modes (single page)
            functionNames[0] = "SHAPE";
            functionNames[1] = isRandom ? "MODE" : "RATE";
            functionNames[2] = isTriggered ? "GATE" : "DEPTH";
            functionNames[3] = "OFSET";
            functionNames[4] = isRandom ? "SLEW" : "PHASE";
            functionNames[5] = nullptr;  // F6 button unused
        }
    }

    WindowPainter::clear(canvas);

    // Draw header - show different title for routing overlay
    FixedStringBuilder<32> title("MOD %d - %s", _selectedModulator + 1, _showRoutingOverlay ? "ROUTING" : "MODULATOR");
    WindowPainter::drawHeader(canvas, _model, _engine, title);

    int highlightedFunction = _showRoutingOverlay ? int(_selectedRoutingFunction) : int(_selectedFunction);

    // For routing overlay, mark EVENT and CC NUM as unavailable when routing to CV
    if (_showRoutingOverlay) {
        bool available[6];
        for (int i = 0; i < 6; ++i) {
            available[i] = true;  // Default: all available
        }

        // Dim EVENT (F4) and CC NUM (F5) when routing to CV
        if (_routingTargetType == RoutingTargetType::CV) {
            available[int(RoutingFunction::Event)] = false;
            available[int(RoutingFunction::CCNumber)] = false;
        }

        WindowPainter::drawFooter(canvas, functionNames, pageKeyState(), highlightedFunction, available);
    } else {
        WindowPainter::drawFooter(canvas, functionNames, pageKeyState(), highlightedFunction);
    }

    // Draw pagination indicators if multiple pages exist
    if (!_showRoutingOverlay) {
        WindowPainter::drawPagination(canvas, _currentPage, _totalPages);
    }

    canvas.setBlendMode(BlendMode::Set);
    canvas.setColor(Color::Bright);
    canvas.setFont(Font::Small);

    // Define split screen layout
    // Screen: 256x64, Header: 0-15, Footer: 54-63, Available: 16-53 (38px height)
    const int waveformX = 4;
    const int waveformY = 15;     // Centered vertically (15 + 34 = 49, leaving 5px to footer)
    const int waveformW = 116;    // Narrower (was 124)
    const int waveformH = 34;     // Height to fit centered with equal spacing

    const int paramX = 128;
    const int paramY = 16;
    const int paramW = 128;

    // Build parameter value string based on mode (routing overlay or normal)
    FixedStringBuilder<32> valueStr;
    FixedStringBuilder<16> paramNameStr;

    if (_showRoutingOverlay) {
        // Routing overlay parameter values
        switch (_selectedRoutingFunction) {
        case RoutingFunction::Mode:
            paramNameStr("MODE");
            modulator.printMode(valueStr);
            break;
        case RoutingFunction::Gate:
            paramNameStr("GATE");
            modulator.printGateTrack(valueStr);
            break;
        case RoutingFunction::Target:
            paramNameStr("TARGET");
            if (_routingTargetType == RoutingTargetType::Midi) {
                valueStr("MIDI %d", _routingTargetIndex + 1);
            } else {
                valueStr("CV %d", _routingTargetIndex + 1);
            }
            break;
        case RoutingFunction::Event:
            paramNameStr("EVENT");
            if (_routingTargetType == RoutingTargetType::CV) {
                valueStr("N/A");  // CV doesn't use event type
            } else {
                valueStr("%s", _routingEventIsCC ? "CC" : "Note");
            }
            break;
        case RoutingFunction::CCNumber:
            paramNameStr("CC NUM");
            if (_routingTargetType == RoutingTargetType::CV || !_routingEventIsCC) {
                valueStr("N/A");  // Only relevant for MIDI CC mode
            } else {
                valueStr("CC %d", _routingCCNum);
            }
            break;
        }
    } else {
        // Normal modulator parameter values - handle pagination for ADSR
        if (isADSR) {
            if (_currentPage == 0) {
                // ADSR Page 1
                switch (_selectedFunction) {
                case Function::Shape:
                    paramNameStr("SHAPE");
                    valueStr("%s", Modulator::shapeName(modulator.shape()));
                    break;
                case Function::Mode:  // ATTACK on page 1
                    paramNameStr("ATTACK");
                    modulator.printAttack(valueStr);
                    break;
                case Function::Rate:  // DECAY on page 1
                    paramNameStr("DECAY");
                    modulator.printDecay(valueStr);
                    break;
                case Function::Depth:  // SUSTAIN on page 1
                    paramNameStr("SUSTAIN");
                    modulator.printSustain(valueStr);
                    break;
                case Function::Offset:  // RELEASE on page 1
                    paramNameStr("RELEASE");
                    modulator.printRelease(valueStr);
                    break;
                default:
                    break;
                }
            } else {
                // ADSR Page 2 - Only AMPLITUDE is used for ADSR
                switch (_selectedFunction) {
                case Function::Shape:  // AMPLITUDE on page 2
                    paramNameStr("AMPLITUDE");
                    modulator.printAmplitude(valueStr);
                    break;
                default:
                    // Depth and Offset are not used for ADSR
                    break;
                }
            }
        } else {
            // LFO and Random modes (single page)
            switch (_selectedFunction) {
            case Function::Shape:
                paramNameStr("SHAPE");
                valueStr("%s", Modulator::shapeName(modulator.shape()));
                break;
            case Function::Mode:
                if (isRandom) {
                    paramNameStr("MODE");
                    modulator.printRandomMode(valueStr);
                } else {
                    // LFO: RATE
                    paramNameStr("RATE");
                    modulator.printRate(valueStr);
                }
                break;
            case Function::Rate:
                if (isTriggered) {
                    paramNameStr("GATE");
                    modulator.printGateTrack(valueStr);
                } else {
                    // LFO: DEPTH
                    paramNameStr("DEPTH");
                    valueStr("%d", modulator.depth());
                }
                break;
            case Function::Depth:
                paramNameStr("OFFSET");
                valueStr("%+d", modulator.offset());
                break;
            case Function::Offset:
                if (isRandom) {
                    paramNameStr("SLEW");
                    modulator.printSmooth(valueStr);
                } else {
                    paramNameStr("PHASE");
                    modulator.printPhase(valueStr);
                }
                break;
            case Function::Phase:
                // Phase is not used for Random shape (SLEW replaces it)
                // This function button is inactive for Random
                break;
            }
        }
    }

    // Draw RIGHT HALF - Parameter display
    canvas.setFont(Font::Tiny);
    canvas.setColor(Color::Medium);
    canvas.drawTextCentered(paramX, paramY + 1, paramW, 8, paramNameStr);

    canvas.setFont(Font::Small);
    canvas.setColor(Color::Bright);
    canvas.drawTextCentered(paramX, paramY + 13, paramW, 12, valueStr);

    // Draw LEFT HALF - Waveform window
    // Draw border box for waveform
    canvas.setColor(Color::Bright);
    canvas.drawRect(waveformX, waveformY, waveformW, waveformH);

    // Waveform rendering area (inside the box)
    const int scopeX = waveformX + 2;
    const int scopeY = waveformY + (waveformH / 2);  // Centerline
    const int scopeWidth = waveformW - 4;
    const int scopeHeight = waveformH - 4;

    // Draw centerline
    canvas.setColor(Color::Low);
    canvas.hline(scopeX, scopeY, scopeWidth);

    // Draw waveform
    canvas.setColor(Color::Bright);

    // For Random shape, show the current value as a horizontal line
    if (isRandom) {
        int currentValue = _engine.modulatorEngine().currentValue(_selectedModulator);
        // Map 0-127 to scope height
        int y = scopeY - ((currentValue - 64) * scopeHeight / 2) / 127;
        canvas.hline(scopeX, y, scopeWidth);
    } else if (isADSR) {
        // For ADSR, draw envelope preview with amplitude control
        int attackMs = modulator.attack();
        int decayMs = modulator.decay();
        int sustainLevel = modulator.sustain();
        int releaseMs = modulator.release();
        int amplitude = modulator.amplitude();

        // Calculate time segments (simplified visualization)
        int totalTime = attackMs + decayMs + 500 + releaseMs;  // +500ms for sustain display (wider for better shape visualization)
        if (totalTime == 0) totalTime = 1;  // Avoid division by zero
        int attackWidth = (attackMs * scopeWidth) / totalTime;
        int decayWidth = (decayMs * scopeWidth) / totalTime;
        int sustainWidth = (500 * scopeWidth) / totalTime;
        int releaseWidth = (releaseMs * scopeWidth) / totalTime;

        // Draw ADSR envelope shape - use full height with amplitude control
        int x = scopeX;
        int bottomY = scopeY + (scopeHeight / 2) - 1;  // Bottom of scope

        // Attack - rise from bottom to peak (scaled by amplitude)
        int attackEndX = x + attackWidth;
        int peakY = scopeY - (scopeHeight / 2) + 1;  // Top of scope
        peakY = bottomY - ((bottomY - peakY) * amplitude) / 127;  // Scale by amplitude
        canvas.line(x, bottomY, attackEndX, peakY);
        x = attackEndX;

        // Decay - fall to sustain level (scaled by amplitude)
        int decayEndX = x + decayWidth;
        int sustainY = bottomY - ((bottomY - (scopeY - (scopeHeight / 2) + 1)) * sustainLevel * amplitude) / (127 * 127);
        canvas.line(x, peakY, decayEndX, sustainY);
        x = decayEndX;

        // Sustain - hold level
        int sustainEndX = x + sustainWidth;
        if (sustainWidth > 0) {
            canvas.hline(x, sustainY, sustainWidth);
        }
        x = sustainEndX;

        // Release - fall back to bottom
        int releaseEndX = x + releaseWidth;
        canvas.line(x, sustainY, releaseEndX, bottomY);

        // Draw ADSR playhead based on current state
        auto adsrState = _engine.modulatorEngine().adsrState(_selectedModulator);
        uint32_t adsrTimer = _engine.modulatorEngine().adsrTimer(_selectedModulator);
        int playheadX = scopeX;

        // Calculate playhead position based on tick progress through each phase
        // Use the same tick calculation formula as the engine to ensure accurate sync
        switch (adsrState) {
        case decltype(adsrState)::Idle:
            playheadX = scopeX;  // At start
            break;
        case decltype(adsrState)::Attack: {
            if (attackMs > 0) {
                // Use same formula as engine: attackTicks = (attackMs * CONFIG_PPQN) / 2500
                int attackTicks = (attackMs * CONFIG_PPQN) / 2500;
                if (attackTicks == 0) attackTicks = 1;
                // Progress as percentage based on ticks
                int progress = utils::clamp((int)((adsrTimer * attackWidth) / attackTicks), 0, attackWidth);
                playheadX = scopeX + progress;
            } else {
                playheadX = scopeX + attackWidth;  // Instant attack
            }
            break;
        }
        case decltype(adsrState)::Decay: {
            if (decayMs > 0 && sustainLevel < 127) {
                int decayTicks = (decayMs * CONFIG_PPQN) / 2500;
                if (decayTicks == 0) decayTicks = 1;
                int progress = utils::clamp((int)((adsrTimer * decayWidth) / decayTicks), 0, decayWidth);
                playheadX = scopeX + attackWidth + progress;
            } else {
                playheadX = scopeX + attackWidth + decayWidth;  // Instant decay
            }
            break;
        }
        case decltype(adsrState)::Sustain:
            // Hold at end of decay phase (start of sustain section)
            playheadX = scopeX + attackWidth + decayWidth;
            break;
        case decltype(adsrState)::Release: {
            if (releaseMs > 0) {
                int releaseTicks = (releaseMs * CONFIG_PPQN) / 2500;
                if (releaseTicks == 0) releaseTicks = 1;
                int progress = utils::clamp((int)((adsrTimer * releaseWidth) / releaseTicks), 0, releaseWidth);
                playheadX = scopeX + attackWidth + decayWidth + sustainWidth + progress;
            } else {
                playheadX = scopeX + attackWidth + decayWidth + sustainWidth + releaseWidth;  // Instant release
            }
            break;
        }
        }

        // Draw playhead line
        canvas.setColor(Color::Medium);
        canvas.vline(playheadX, waveformY + 1, waveformH - 2);
    } else {
        // For LFO modes, use cached waveform for performance (only recalculate when params change)
        if (!_waveformCacheValid ||
            modulator.shape() != _lastShape ||
            modulator.depth() != _lastDepth ||
            modulator.offset() != _lastOffset ||
            modulator.phase() != _lastPhase) {
            updateWaveformCache();
        }

        // Draw waveform from cache - much faster than recalculating every frame!
        // Use line-only rendering to reduce screen noise
        for (int x = 0; x < WAVEFORM_CACHE_SIZE - 1; ++x) {
            int y1 = scopeY - (_waveformCache[x] * scopeHeight / 2) / 127;
            int y2 = scopeY - (_waveformCache[x + 1] * scopeHeight / 2) / 127;
            canvas.line(scopeX + x, y1, scopeX + x + 1, y2);
        }

        // Draw playhead position (current phase)
        uint16_t currentPhase = _engine.modulatorEngine().currentPhase(_selectedModulator);
        // Map phase (0-65535) to x position (0 to scopeWidth-1)
        int playheadX = (currentPhase * scopeWidth) / 65536;
        canvas.setColor(Color::Medium);
        canvas.vline(scopeX + playheadX, waveformY + 1, waveformH - 2);
    }

    // Draw real-time modulator output level bar (slim vertical bar on right side of waveform)
    int currentValue = _engine.modulatorEngine().currentValue(_selectedModulator);
    const int barX = waveformX + waveformW + 4;  // Right side of waveform, offset slightly
    const int barWidth = 4;
    const int barHeight = waveformH;
    const int barY = waveformY;

    // Draw bar background
    canvas.setColor(Color::Medium);
    canvas.drawRect(barX, barY, barWidth, barHeight);

    // Draw level indicator (white line that moves vertically based on output 0-127)
    canvas.setColor(Color::Bright);
    int levelY = barY + barHeight - 2 - ((currentValue * (barHeight - 4)) / 127);
    canvas.hline(barX + 1, levelY, barWidth - 2);
    canvas.hline(barX + 1, levelY + 1, barWidth - 2);  // 2px thick for visibility

    // Draw routing popup if active
    if (_showRoutingPopup) {
        // Draw black background
        canvas.setBlendMode(BlendMode::Set);
        canvas.setColor(Color::None);
        canvas.fillRect(32, 16, 192, 44);

        // Draw border
        canvas.setColor(Color::Bright);
        canvas.drawRect(32, 16, 192, 44);

        // Draw title
        canvas.setFont(Font::Small);
        canvas.drawTextCentered(32, 20, 192, 8, "Route Modulator");

        // Draw three fields: Output, Mode, CC#
        canvas.setFont(Font::Tiny);

        // Field 1: MIDI Output
        canvas.setColor(_routingField == RoutingField::Output ? Color::Bright : Color::Medium);
        FixedStringBuilder<16> outputStr("Out %d", _routingOutputIndex + 1);
        canvas.drawTextCentered(40, 32, 48, 8, outputStr);

        // Field 2: Mode (Note/CC)
        canvas.setColor(_routingField == RoutingField::Mode ? Color::Bright : Color::Medium);
        const char *modeStr = _routingToCC ? "CC" : "Note";
        canvas.drawTextCentered(96, 32, 48, 8, modeStr);

        // Field 3: CC# (only visible in CC mode)
        if (_routingToCC) {
            canvas.setColor(_routingField == RoutingField::CCNumber ? Color::Bright : Color::Medium);
            FixedStringBuilder<16> ccStr("CC %d", _routingCCNumber);
            canvas.drawTextCentered(152, 32, 48, 8, ccStr);
        }

        // Show instructions
        canvas.setFont(Font::Tiny);
        canvas.setColor(Color::Medium);
        canvas.drawTextCentered(32, 48, 192, 8, "F=Field  Turn=Val  Press=OK");
    }
}

void ModulatorPage::updateLeds(Leds &leds) {
    // Track LEDs (1-8) show selected modulator
    for (int i = 0; i < 8; ++i) {
        bool isSelected = (i == _selectedModulator);
        leds.set(MatrixMap::fromTrack(i), false, isSelected);  // Selected = green
    }

    // Page indicator LEDs above Left/Right buttons (only if multiple pages and not in routing overlay)
    if (_totalPages > 1 && !_showRoutingOverlay) {
        // Global4 (above Left/prev button) = shows green when prev page available
        bool hasPrevPage = (_currentPage > 0);
        leds.set(Key::Global4, false, hasPrevPage);  // Green when can go to previous page

        // Global5 (above Right/next button) = shows green when next page available
        bool hasNextPage = (_currentPage < _totalPages - 1);
        leds.set(Key::Global5, false, hasNextPage);  // Green when can go to next page
    }
}

void ModulatorPage::keyPress(KeyPressEvent &event) {
    const auto &key = event.key();

    // Handle routing popup first (old popup system)
    if (_showRoutingPopup) {
        // Function buttons select field
        if (key.isFunction()) {
            int func = key.function();
            if (func == 0) {
                _routingField = RoutingField::Output;
            } else if (func == 1) {
                _routingField = RoutingField::Mode;
            } else if (func == 2 && _routingToCC) {
                _routingField = RoutingField::CCNumber;
            }
            event.consume();
            return;
        }

        // Any other key dismisses the popup
        _showRoutingPopup = false;
        event.consume();
        return;
    }

    // Shift+Page toggles routing overlay
    if (key.isPage() && key.shiftModifier()) {
        _showRoutingOverlay = !_showRoutingOverlay;
        // When entering routing overlay, select first function
        if (_showRoutingOverlay) {
            _selectedRoutingFunction = RoutingFunction::Mode;
            // Load current routing settings from MIDI output
            loadRoutingFromMidiOutput();
        } else {
            // Exiting overlay - apply the routing settings
            applyRoutingToMidiOutput();
            _selectedFunction = Function::Shape;
        }
        event.consume();
        return;
    }

    // Context menu for quick mapping (hold Shift without Page)
    if (key.isContextMenu() && !key.pageModifier()) {
        contextShow();
        event.consume();
        return;
    }

    // Pagination controls (Left/Right buttons without shift)
    if (_totalPages > 1 && !_showRoutingOverlay) {
        if (key.is(Key::Left)) {
            // Previous page
            if (_currentPage > 0) {
                _currentPage--;
                _selectedFunction = Function::Shape;  // Reset to first function on page change
            }
            event.consume();
            return;
        } else if (key.is(Key::Right)) {
            // Next page
            if (_currentPage < _totalPages - 1) {
                _currentPage++;
                _selectedFunction = Function::Shape;  // Reset to first function on page change
            }
            event.consume();
            return;
        }
    }

    // Track buttons (1-8) select modulator
    if (key.isTrackSelect()) {
        setSelectedModulator(key.trackSelect());
        event.consume();
        return;
    }

    // Function buttons select active parameter
    if (key.isFunction()) {
        if (_showRoutingOverlay) {
            // Routing overlay mode
            setSelectedRoutingFunction(RoutingFunction(key.function()));
            event.consume();
        } else {
            // Normal parameter mode
            setSelectedFunction(Function(key.function()));
            event.consume();
        }
    }
}

void ModulatorPage::encoder(EncoderEvent &event) {
    // Handle routing popup encoder (old popup system)
    if (_showRoutingPopup) {
        if (event.pressed()) {
            // Encoder press confirms selection
            quickMapToOutput(_routingOutputIndex);
            _showRoutingPopup = false;
        } else {
            // Encoder edits the currently selected field
            switch (_routingField) {
            case RoutingField::Output:
                _routingOutputIndex = utils::clamp(_routingOutputIndex + event.value(), 0, CONFIG_MIDI_OUTPUT_COUNT - 1);
                break;
            case RoutingField::Mode:
                if (event.value() != 0) {
                    _routingToCC = !_routingToCC;
                    // Auto-switch to CC# field when switching to CC mode
                    if (_routingToCC) {
                        _routingField = RoutingField::CCNumber;
                    }
                }
                break;
            case RoutingField::CCNumber:
                _routingCCNumber = utils::clamp(_routingCCNumber + event.value(), 0, 127);
                break;
            }
        }
        return;
    }

    auto &modulator = _project.modulator(_selectedModulator);

    if (_showRoutingOverlay) {
        // Routing overlay parameter editing
        switch (_selectedRoutingFunction) {
        case RoutingFunction::Mode:
            modulator.editMode(event.value(), event.pressed());
            break;
        case RoutingFunction::Gate:
            modulator.editGateTrack(event.value(), event.pressed());
            break;
        case RoutingFunction::Target: {
            // Cycle through MIDI 1-16, then CV 1-8
            int totalTargets = 16 + 8;  // MIDI 1-16 + CV 1-8
            int currentGlobal = (_routingTargetType == RoutingTargetType::Midi)
                ? _routingTargetIndex
                : (16 + _routingTargetIndex);

            currentGlobal = (currentGlobal + event.value() + totalTargets) % totalTargets;

            if (currentGlobal < 16) {
                _routingTargetType = RoutingTargetType::Midi;
                _routingTargetIndex = currentGlobal;
            } else {
                _routingTargetType = RoutingTargetType::CV;
                _routingTargetIndex = currentGlobal - 16;
            }
            break;
        }
        case RoutingFunction::Event:
            // Only allow editing if routing to MIDI (CV doesn't support event types)
            if (_routingTargetType == RoutingTargetType::Midi && event.value() != 0) {
                _routingEventIsCC = !_routingEventIsCC;
            }
            break;
        case RoutingFunction::CCNumber:
            // Only allow editing if routing to MIDI in CC mode
            if (_routingTargetType == RoutingTargetType::Midi && _routingEventIsCC) {
                _routingCCNum = utils::clamp(_routingCCNum + event.value(), 0, 127);
            }
            break;
        }
    } else {
        // Normal modulator parameter editing - handle pagination for ADSR
        bool isRandom = (modulator.shape() == Modulator::Shape::Random);
        bool isADSR = (modulator.shape() == Modulator::Shape::ADSR);
        bool isTriggered = isRandom && (modulator.randomMode() == Modulator::RandomMode::Triggered);

        // Edit the currently selected parameter (one-handed operation!)
        if (isADSR) {
            if (_currentPage == 0) {
                // ADSR Page 1
                switch (_selectedFunction) {
                case Function::Shape:
                    modulator.editShape(event.value(), event.pressed());
                    break;
                case Function::Mode:  // ATTACK
                    modulator.editAttack(event.value(), event.pressed());
                    break;
                case Function::Rate:  // DECAY
                    modulator.editDecay(event.value(), event.pressed());
                    break;
                case Function::Depth:  // SUSTAIN
                    modulator.editSustain(event.value(), event.pressed());
                    break;
                case Function::Offset:  // RELEASE
                    modulator.editRelease(event.value(), event.pressed());
                    break;
                default:
                    break;
                }
            } else {
                // ADSR Page 2 - Only AMPLITUDE is used
                switch (_selectedFunction) {
                case Function::Shape:  // AMPLITUDE
                    modulator.editAmplitude(event.value(), event.pressed());
                    break;
                default:
                    // Depth and Offset not used for ADSR
                    break;
                }
            }
        } else {
            // LFO and Random modes (single page)
            switch (_selectedFunction) {
            case Function::Shape:
                modulator.editShape(event.value(), event.pressed());
                break;
            case Function::Mode:
                if (isRandom) {
                    modulator.editRandomMode(event.value(), event.pressed());
                } else {
                    // LFO: RATE
                    modulator.editRate(event.value(), event.pressed());
                }
                break;
            case Function::Rate:
                if (isTriggered) {
                    modulator.editGateTrack(event.value(), event.pressed());
                } else {
                    // LFO: DEPTH
                    modulator.editDepth(event.value(), event.pressed());
                }
                break;
            case Function::Depth:
                // LFO: OFFSET
                modulator.editOffset(event.value(), event.pressed());
                break;
            case Function::Offset:
                if (isRandom) {
                    // Random: SLEW
                    modulator.editSmooth(event.value(), event.pressed());
                } else {
                    // LFO: PHASE
                    modulator.editPhase(event.value(), event.pressed());
                }
                break;
            case Function::Phase:
                // Phase is not used for Random shape (SLEW replaces it on Function::Offset)
                // This function button does nothing for Random
                break;
            }
        }
    }
}

void ModulatorPage::setSelectedModulator(int index) {
    _selectedModulator = utils::clamp(index, 0, CONFIG_MODULATOR_COUNT - 1);
    _project.setSelectedModulatorIndex(_selectedModulator);
    // Invalidate cache when changing modulators
    _waveformCacheValid = false;
    // Reload routing settings if in routing overlay mode
    if (_showRoutingOverlay) {
        loadRoutingFromMidiOutput();
    }
}

void ModulatorPage::setSelectedFunction(Function function) {
    _selectedFunction = function;
}

void ModulatorPage::setSelectedRoutingFunction(RoutingFunction function) {
    _selectedRoutingFunction = function;
}

int ModulatorPage::generateWaveformPreview(Modulator::Shape shape, uint16_t phase) {
    // Use WaveformGenerator for consistent waveforms between engine and UI
    return WaveformGenerator::generate(shape, phase);
}

void ModulatorPage::updateWaveformCache() {
    auto &modulator = _project.modulator(_selectedModulator);

    // Cache current parameters
    _lastShape = modulator.shape();
    _lastDepth = modulator.depth();
    _lastOffset = modulator.offset();
    _lastPhase = modulator.phase();

    // Pre-calculate waveform for all x positions
    // Display exactly one cycle of the waveform
    for (int x = 0; x < WAVEFORM_CACHE_SIZE; ++x) {
        // Calculate phase for this x position (0-65535) for exactly one cycle
        // Use 32-bit to avoid overflow when adding phase offset
        uint32_t phase = ((uint32_t)x * 65536) / WAVEFORM_CACHE_SIZE;

        // Add phase offset (wraps around at 65536)
        phase = (phase + ((uint32_t)modulator.phase() * 65536) / 360) & 0xFFFF;

        // Generate waveform value (-127 to +127)
        int value = generateWaveformPreview(modulator.shape(), (uint16_t)phase);

        // Apply depth and offset
        value = (value * modulator.depth()) / 127;
        value += modulator.offset();
        value = utils::clamp(value, -127, 127);

        // Store in cache as int8_t
        _waveformCache[x] = value;
    }

    _waveformCacheValid = true;
}

void ModulatorPage::contextShow() {
    showContextMenu(ContextMenu(
        modulatorContextMenuItems,
        int(ModulatorContextAction::Last),
        [&] (int index) { contextAction(index); },
        [&] (int index) { return true; } // All items always enabled
    ));
}

void ModulatorPage::contextAction(int index) {
    switch (ModulatorContextAction(index)) {
    case ModulatorContextAction::Route:
        // Show routing popup
        showRoutingPopup();
        break;
    case ModulatorContextAction::Last:
        break;
    }
}

void ModulatorPage::quickMapToOutput(int outputIndex) {
    if (outputIndex >= CONFIG_MIDI_OUTPUT_COUNT) return;

    auto &output = _project.midiOutput().output(outputIndex);

    if (_routingToCC) {
        // CC Mode: Set up modulator as CC source
        output.setEvent(MidiOutput::Output::Event::ControlChange);

        // Set control source to current modulator
        int modSource = int(MidiOutput::Output::ControlSource::FirstModulator) + _selectedModulator;
        output.setControlSource(MidiOutput::Output::ControlSource(modSource));

        // Use the CC number from popup
        output.setControlNumber(_routingCCNumber);

        // Set target to MIDI port, channel = output number + 1
        output.target().setPort(Types::MidiPort::Midi);
        output.target().setChannel(outputIndex);

        showMessage(FixedStringBuilder<32>("Mod %d → Out %d CC%d",
            _selectedModulator + 1, outputIndex + 1, _routingCCNumber), 2000);
    } else {
        // Note Mode: Route modulator to MIDI channel as note/gate (not implemented yet)
        // This could be used for envelope triggers, etc.
        showMessage("Note mode not implemented", 2000);
    }
}

void ModulatorPage::loadRoutingFromMidiOutput() {
    // Search for where the current modulator is routed
    // Current modulator is _selectedModulator (0-7 = Mod 1-8)
    int targetModSource = int(MidiOutput::Output::ControlSource::FirstModulator) + _selectedModulator;

    // First, search MIDI outputs for this modulator
    bool foundInMidi = false;
    for (int i = 0; i < CONFIG_MIDI_OUTPUT_COUNT; ++i) {
        auto &output = _project.midiOutput().output(i);

        // Check if this output uses our modulator as control source
        if (output.event() == MidiOutput::Output::Event::ControlChange &&
            int(output.controlSource()) == targetModSource) {
            // Found it! Load settings from this MIDI output
            _routingTargetType = RoutingTargetType::Midi;
            _routingTargetIndex = i;
            _routingCCNum = output.controlNumber();
            _routingEventIsCC = true;
            foundInMidi = true;
            break;
        }
    }

    // If not found in MIDI outputs, search CV outputs
    if (!foundInMidi) {
        bool foundInCV = false;
        for (int i = 0; i < CONFIG_CV_OUTPUT_CHANNELS; ++i) {
            int cvMod = _project.cvOutputModulator(i);
            // cvOutputModulator stores 0 = none, 1-8 = Mod 1-8
            if (cvMod == (_selectedModulator + 1)) {
                // Found it! This CV output uses our modulator
                _routingTargetType = RoutingTargetType::CV;
                _routingTargetIndex = i;
                _routingCCNum = 0;  // Not used for CV
                _routingEventIsCC = true;  // Not used for CV
                foundInCV = true;
                break;
            }
        }

        // If not found anywhere, set defaults based on modulator index
        if (!foundInCV) {
            _routingTargetType = RoutingTargetType::Midi;
            _routingTargetIndex = _selectedModulator + 8;  // Default: Mod 1→MIDI 9, etc.
            _routingCCNum = _selectedModulator;  // Default: Mod 1→CC 0, etc.
            _routingEventIsCC = true;
        }
    }
}

void ModulatorPage::applyRoutingToMidiOutput() {
    if (_routingTargetType == RoutingTargetType::Midi) {
        // Apply routing overlay settings to the selected MIDI output
        auto &output = _project.midiOutput().output(_routingTargetIndex);

        if (_routingEventIsCC) {
            // CC Mode: Set up modulator as CC source
            output.setEvent(MidiOutput::Output::Event::ControlChange);

            // Set control source to current modulator
            int modSource = int(MidiOutput::Output::ControlSource::FirstModulator) + _selectedModulator;
            output.setControlSource(MidiOutput::Output::ControlSource(modSource));

            // Use the CC number from overlay
            output.setControlNumber(_routingCCNum);

            // Set target to MIDI port
            // For outputs 9-16 (index 8-15), map to MIDI channels 1-8 (index 0-7)
            output.target().setPort(Types::MidiPort::Midi);
            int midiChannel = _routingTargetIndex >= 8 ? _routingTargetIndex - 8 : _routingTargetIndex;
            output.target().setChannel(midiChannel);

            showMessage(FixedStringBuilder<32>("Mod %d → Out %d → MIDI %d CC%d",
                _selectedModulator + 1, _routingTargetIndex + 1, midiChannel + 1, _routingCCNum), 2000);
        } else {
            // Note Mode: Not fully supported yet for modulators
            showMessage("Note mode not supported", 2000);
        }
    } else {
        // CV output routing - assign modulator to CV output
        // The modulator value will be added to the track's CV output
        _project.setCvOutputModulator(_routingTargetIndex, _selectedModulator + 1);  // 1-8 = Mod 1-8

        showMessage(FixedStringBuilder<32>("Mod %d → CV %d + Track",
            _selectedModulator + 1, _routingTargetIndex + 1), 2000);
    }
}

void ModulatorPage::showRoutingPopup() {
    _showRoutingPopup = true;
    _routingField = RoutingField::Output;
    _routingOutputIndex = 0; // Start at output 1
    _routingToCC = true; // Default to CC mode (most common use case)
    _routingCCNumber = _selectedModulator + 1; // Default CC# based on modulator (1-8)
}
