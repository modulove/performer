#include "MidiOutputPage.h"

#include "ui/painters/WindowPainter.h"
#include "ui/LedPainter.h"

#include "core/utils/StringBuilder.h"

MidiOutputPage::MidiOutputPage(PageManager &manager, PageContext &context) :
    BasePage(manager, context)
{
}

void MidiOutputPage::reset() {
    _selectedOutput = 0;
    _selectedFunction = Function::Target;
}

void MidiOutputPage::enter() {
}

void MidiOutputPage::exit() {
}

void MidiOutputPage::draw(Canvas &canvas) {
    WindowPainter::clear(canvas);
    WindowPainter::drawHeader(canvas, _model, _engine, FixedStringBuilder<32>("OUT %d  MIDI OUTPUT", _selectedOutput + 1));

    auto &output = _project.midiOutput().output(_selectedOutput);

    // Build function names array dynamically based on event type
    const char *functionNames[5];
    functionNames[0] = "TARGET";
    functionNames[1] = "EVENT";

    if (output.isNoteEvent()) {
        functionNames[2] = "GATE";
        functionNames[3] = "NOTE";
        functionNames[4] = "VEL";
    } else if (output.isControlChangeEvent()) {
        functionNames[2] = "CC#";
        functionNames[3] = "SOURCE";
        functionNames[4] = "";
    } else {
        functionNames[2] = "";
        functionNames[3] = "";
        functionNames[4] = "";
    }

    // Draw footer tabs
    WindowPainter::drawFooter(canvas, functionNames, pageKeyState(), int(_selectedFunction));

    // Draw parameter value centered with larger, bold text
    canvas.setBlendMode(BlendMode::Set);
    canvas.setColor(Color::Bright);
    canvas.setFont(Font::Small);

    FixedStringBuilder<32> valueStr;

    switch (_selectedFunction) {
    case Function::Target:
        output.target().print(valueStr);
        break;
    case Function::Event:
        output.printEvent(valueStr);
        break;
    case Function::Param1:
        if (output.isNoteEvent()) {
            output.printGateSource(valueStr);
        } else if (output.isControlChangeEvent()) {
            output.printControlNumber(valueStr);
        }
        break;
    case Function::Param2:
        if (output.isNoteEvent()) {
            output.printNoteSource(valueStr);
        } else if (output.isControlChangeEvent()) {
            output.printControlSource(valueStr);
        }
        break;
    case Function::Param3:
        if (output.isNoteEvent()) {
            output.printVelocitySource(valueStr);
        }
        break;
    }

    // Center the text horizontally and vertically in the main area
    // Display area is 256x64, header is ~12px, footer is ~10px
    canvas.drawTextCentered(0, 16, 256, 32, valueStr);
}

void MidiOutputPage::updateLeds(Leds &leds) {
    // Step LEDs (0-15) show selected MIDI output
    for (int i = 0; i < 16; ++i) {
        bool isSelected = (i == _selectedOutput);
        leds.set(MatrixMap::fromStep(i), false, isSelected);  // Selected = green
    }
}

void MidiOutputPage::keyPress(KeyPressEvent &event) {
    const auto &key = event.key();

    // Step buttons (0-15) select MIDI output
    if (key.isStep() && key.step() < 16) {
        selectOutput(key.step());
        event.consume();
    }

    // F-buttons select parameter (like ModulatorPage)
    if (key.isFunction()) {
        setSelectedFunction(Function(key.function()));
        event.consume();
    }
}

void MidiOutputPage::encoder(EncoderEvent &event) {
    auto &output = _project.midiOutput().output(_selectedOutput);

    // Encoder edits selected parameter
    switch (_selectedFunction) {
    case Function::Target:
        output.target().edit(event.value(), event.pressed());
        break;
    case Function::Event:
        output.editEvent(event.value(), event.pressed());
        // When event type changes, default to first parameter
        _selectedFunction = Function::Target;
        break;
    case Function::Param1:
        if (output.isNoteEvent()) {
            output.editGateSource(event.value(), event.pressed());
        } else if (output.isControlChangeEvent()) {
            output.editControlNumber(event.value(), event.pressed());
        }
        break;
    case Function::Param2:
        if (output.isNoteEvent()) {
            output.editNoteSource(event.value(), event.pressed());
        } else if (output.isControlChangeEvent()) {
            output.editControlSource(event.value(), event.pressed());
        }
        break;
    case Function::Param3:
        if (output.isNoteEvent()) {
            output.editVelocitySource(event.value(), event.pressed());
        }
        break;
    }
}

void MidiOutputPage::showOutput(int outputIndex) {
    _selectedOutput = outputIndex;
    _selectedFunction = Function::Target;
}

void MidiOutputPage::selectOutput(int outputIndex) {
    outputIndex = clamp(outputIndex, 0, CONFIG_MIDI_OUTPUT_COUNT - 1);
    if (outputIndex != _selectedOutput) {
        showOutput(outputIndex);
    }
}

void MidiOutputPage::setSelectedFunction(Function function) {
    auto &output = _project.midiOutput().output(_selectedOutput);

    // Don't allow selecting unavailable functions
    if (function == Function::Param1 && !output.isNoteEvent() && !output.isControlChangeEvent()) {
        return;
    }
    if (function == Function::Param2 && !output.isNoteEvent() && !output.isControlChangeEvent()) {
        return;
    }
    if (function == Function::Param3 && !output.isNoteEvent()) {
        return;
    }

    _selectedFunction = function;
}
