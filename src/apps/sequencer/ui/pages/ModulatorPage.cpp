#include "ModulatorPage.h"

#include "ui/painters/WindowPainter.h"
#include "ui/pages/ContextMenu.h"

#include "core/utils/StringBuilder.h"

enum class ModulatorContextAction {
    Route,
    PresetT9T16,
    PresetModCC0,
    Last
};

static const ContextMenuModel::Item modulatorContextMenuItems[] = {
    { "Route" },
    { "PR. T9-16" },
    { "PR. M1-8" },
};

ModulatorPage::ModulatorPage(PageManager &manager, PageContext &context) :
    BasePage(manager, context)
{
}

void ModulatorPage::enter() {
    _selectedModulator = _project.selectedModulatorIndex();
}

void ModulatorPage::exit() {
}

void ModulatorPage::draw(Canvas &canvas) {
    auto &modulator = _project.modulator(_selectedModulator);

    // Determine which parameters to show based on shape
    bool isGateTriggered = (modulator.shape() == Modulator::Shape::GateRandomStep);
    bool isRandomSmooth = (modulator.shape() == Modulator::Shape::RandomSmooth);

    const char *functionNames[] = {
        "SHAPE",
        isGateTriggered ? "GATE" : "RATE",
        "DEPTH",
        "OFSET",
        isGateTriggered ? "" : (isRandomSmooth ? "SMOOTH" : "PHASE")
    };

    WindowPainter::clear(canvas);

    // Draw header with MOD number on the left
    FixedStringBuilder<32> title("MOD %d - MODULATOR", _selectedModulator + 1);
    WindowPainter::drawHeader(canvas, _model, _engine, title);
    WindowPainter::drawFooter(canvas, functionNames, pageKeyState(), int(_selectedFunction));

    canvas.setBlendMode(BlendMode::Set);
    canvas.setColor(Color::Bright);
    canvas.setFont(Font::Small);

    // Build parameter value string based on selected function
    FixedStringBuilder<32> valueStr;
    switch (_selectedFunction) {
    case Function::Shape:
        valueStr("%s", Modulator::shapeName(modulator.shape()));
        break;
    case Function::Rate:
        if (isGateTriggered) {
            modulator.printGateTrack(valueStr);
        } else {
            modulator.printRate(valueStr);
        }
        break;
    case Function::Depth:
        valueStr("%d", modulator.depth());
        break;
    case Function::Offset:
        valueStr("%+d", modulator.offset());
        break;
    case Function::Phase:
        if (!isGateTriggered) {
            if (isRandomSmooth) {
                modulator.printSmooth(valueStr);
            } else {
                modulator.printPhase(valueStr);
            }
        }
        break;
    }

    // Center the parameter value text with larger, bold font
    canvas.drawTextCentered(0, 14, 256, 12, valueStr);

    // Draw oscilloscope-style waveform preview
    const int scopeY = 34;      // Y position of scope centerline
    const int scopeHeight = 20; // Height of scope area (±10 pixels from center)
    const int scopeWidth = 252; // Width of scope display
    const int scopeX = 2;       // X position

    // Draw centerline
    canvas.setColor(Color::Low);
    canvas.hline(scopeX, scopeY, scopeWidth);

    // Draw waveform
    canvas.setColor(Color::Bright);

    // For gate-triggered modes, show the current value as a horizontal line
    if (isGateTriggered) {
        int currentValue = _engine.modulatorEngine().currentValue(_selectedModulator);
        // Map 0-127 to scope height
        int y = scopeY - ((currentValue - 64) * scopeHeight / 2) / 64;
        canvas.hline(scopeX, y, scopeWidth);
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
        for (int x = 0; x < scopeWidth - 1; ++x) {
            int y1 = scopeY - (_waveformCache[x] * scopeHeight / 2) / 127;
            int y2 = scopeY - (_waveformCache[x + 1] * scopeHeight / 2) / 127;
            canvas.line(scopeX + x, y1, scopeX + x + 1, y2);
        }
    }

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
}

void ModulatorPage::keyPress(KeyPressEvent &event) {
    const auto &key = event.key();

    // Handle routing popup first
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

    // Context menu for quick mapping (Shift+Page)
    if (key.isContextMenu()) {
        contextShow();
        event.consume();
        return;
    }

    // Quick mapping shortcuts with Shift+Function keys
    if (key.shiftModifier() && key.isFunction()) {
        int func = key.function();
        if (func < 4) {
            // Shift+F0-F3: Quick map to MIDI Out 1-4
            quickMapToOutput(func);
        } else if (func == 4) {
            // Shift+F4: Apply custom preset
            applyCustomPreset();
        }
        event.consume();
        return;
    }

    // Track buttons (1-8) select modulator
    if (key.isTrackSelect()) {
        setSelectedModulator(key.trackSelect());
        event.consume();
    }

    // Function buttons select active parameter (like NoteSequenceEditPage)
    if (key.isFunction()) {
        auto &modulator = _project.modulator(_selectedModulator);
        bool isGateTriggered = (modulator.shape() == Modulator::Shape::GateRandomStep);

        Function function = Function(key.function());

        // Don't allow Phase selection for gate-triggered modes
        if (function == Function::Phase && isGateTriggered) {
            return;
        }

        setSelectedFunction(function);
        event.consume();
    }
}

void ModulatorPage::encoder(EncoderEvent &event) {
    // Handle routing popup encoder
    if (_showRoutingPopup) {
        if (event.pressed()) {
            // Encoder press confirms selection
            quickMapToOutput(_routingOutputIndex);
            _showRoutingPopup = false;
        } else {
            // Encoder edits the currently selected field
            switch (_routingField) {
            case RoutingField::Output:
                _routingOutputIndex = clamp(_routingOutputIndex + event.value(), 0, CONFIG_MIDI_OUTPUT_COUNT - 1);
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
                _routingCCNumber = clamp(_routingCCNumber + event.value(), 0, 127);
                break;
            }
        }
        return;
    }

    auto &modulator = _project.modulator(_selectedModulator);

    bool isGateTriggered = (modulator.shape() == Modulator::Shape::GateRandomStep);
    bool isRandomSmooth = (modulator.shape() == Modulator::Shape::RandomSmooth);

    // Edit the currently selected parameter (one-handed operation!)
    switch (_selectedFunction) {
    case Function::Shape:
        modulator.editShape(event.value(), event.pressed());
        break;
    case Function::Rate:
        if (isGateTriggered) {
            modulator.editGateTrack(event.value(), event.pressed());
        } else {
            modulator.editRate(event.value(), event.pressed());
        }
        break;
    case Function::Depth:
        modulator.editDepth(event.value(), event.pressed());
        break;
    case Function::Offset:
        modulator.editOffset(event.value(), event.pressed());
        break;
    case Function::Phase:
        if (!isGateTriggered) {
            if (isRandomSmooth) {
                modulator.editSmooth(event.value(), event.pressed());
            } else {
                modulator.editPhase(event.value(), event.pressed());
            }
        }
        break;
    }
}

void ModulatorPage::setSelectedModulator(int index) {
    _selectedModulator = clamp(index, 0, CONFIG_MODULATOR_COUNT - 1);
    _project.setSelectedModulatorIndex(_selectedModulator);
    // Invalidate cache when changing modulators
    _waveformCacheValid = false;
}

void ModulatorPage::setSelectedFunction(Function function) {
    _selectedFunction = function;
}

int ModulatorPage::clamp(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

int ModulatorPage::generateWaveformPreview(Modulator::Shape shape, uint16_t phase) {
    // phase is 0-65535 representing 0-360 degrees
    // return value is -127 to +127

    switch (shape) {
    case Modulator::Shape::Sine: {
        // Improved sine wave using parabolic approximation
        // Convert phase to -32768 to +32767 range
        int32_t x = (int32_t)phase - 32768;

        // Normalize to -1.0 to +1.0 range (using fixed point: x / 32768)
        // Use parabolic approximation: sin(x) ≈ 4x(1-|x|) for x in [-1, 1]
        int32_t abs_x = (x < 0) ? -x : x;

        // Calculate 4 * x * (32768 - abs_x) / 32768
        // This gives us a parabolic approximation
        int32_t result = (4 * x * (32768 - abs_x)) / 32768;

        // Scale to -127 to +127 range
        return (int)(result * 127 / 32768);
    }
    case Modulator::Shape::Triangle:
        if (phase < 16384) {
            return (phase * 127) / 16384;
        } else if (phase < 49152) {
            return 127 - ((phase - 16384) * 254) / 32768;
        } else {
            return -127 + ((phase - 49152) * 254) / 16384;
        }
    case Modulator::Shape::SawUp:
        return ((int)phase * 254 / 65536) - 127;
    case Modulator::Shape::SawDown:
        return 127 - ((int)phase * 254 / 65536);
    case Modulator::Shape::Square:
        return (phase < 32768) ? 127 : -127;
    case Modulator::Shape::RandomSmooth:
    case Modulator::Shape::RandomStep:
        // For random modes, just show a flat line in preview
        return 0;
    case Modulator::Shape::GateRandomStep:
        // Gate-triggered modes handled separately
        return 0;
    default:
        return 0;
    }
}

void ModulatorPage::updateWaveformCache() {
    auto &modulator = _project.modulator(_selectedModulator);

    // Cache current parameters
    _lastShape = modulator.shape();
    _lastDepth = modulator.depth();
    _lastOffset = modulator.offset();
    _lastPhase = modulator.phase();

    // Pre-calculate waveform for all x positions
    for (int x = 0; x < WAVEFORM_CACHE_SIZE; ++x) {
        // Calculate phase for this x position (0-65535)
        uint16_t phase = (x * 65536) / WAVEFORM_CACHE_SIZE;

        // Add phase offset
        phase += (modulator.phase() * 65536) / 360;

        // Generate waveform value (-127 to +127)
        int value = generateWaveformPreview(modulator.shape(), phase);

        // Apply depth and offset
        value = (value * modulator.depth()) / 127;
        value += modulator.offset();
        value = clamp(value, -127, 127);

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
    case ModulatorContextAction::PresetT9T16:
        // Apply custom preset
        applyCustomPreset();
        break;
    case ModulatorContextAction::PresetModCC0:
        // Apply modulator CC0 preset
        applyModulatorCC0Preset();
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

void ModulatorPage::applyCustomPreset() {
    // Custom preset: Map tracks 9-16 to MIDI outputs 1-8 with velocity 127
    for (int i = 0; i < 8; ++i) {
        auto &output = _project.midiOutput().output(i);

        // Set event to Note
        output.setEvent(MidiOutput::Output::Event::Note);

        // Set sources to tracks 9-16 (indices 8-15)
        int trackIndex = i + 8;
        output.setGateSource(MidiOutput::Output::GateSource(
            int(MidiOutput::Output::GateSource::FirstTrack) + trackIndex));
        output.setNoteSource(MidiOutput::Output::NoteSource(
            int(MidiOutput::Output::NoteSource::FirstTrack) + trackIndex));

        // Set velocity to fixed 127
        output.setVelocitySource(MidiOutput::Output::VelocitySource(
            int(MidiOutput::Output::VelocitySource::FirstVelocity) + 127)); // 127 velocity

        // Set target to MIDI port, channel = i + 1
        output.target().setPort(Types::MidiPort::Midi);
        output.target().setChannel(i);
    }

    showMessage("Preset Applied: T9-16 → MIDI 1-8 @127", 2500);
}

void ModulatorPage::applyModulatorCC0Preset() {
    // Map Mod 1-8 to MIDI Output 1-8, all with CC 0
    for (int i = 0; i < 8; ++i) {
        auto &output = _project.midiOutput().output(i);

        // Set event to ControlChange
        output.setEvent(MidiOutput::Output::Event::ControlChange);

        // Set control source to Modulator i (0-7)
        output.setControlSource(MidiOutput::Output::ControlSource(
            int(MidiOutput::Output::ControlSource::FirstModulator) + i));

        // Set CC number to 0
        output.setControlNumber(0);

        // Set target to MIDI port, channel = i + 1
        output.target().setPort(Types::MidiPort::Midi);
        output.target().setChannel(i);
    }

    showMessage("Preset Applied: Mod 1-8 → CC 0", 2500);
}

void ModulatorPage::showRoutingPopup() {
    _showRoutingPopup = true;
    _routingField = RoutingField::Output;
    _routingOutputIndex = 0; // Start at output 1
    _routingToCC = true; // Default to CC mode (most common use case)
    _routingCCNumber = _selectedModulator + 1; // Default CC# based on modulator (1-8)
}
