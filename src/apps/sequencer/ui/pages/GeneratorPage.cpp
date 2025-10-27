#include "GeneratorPage.h"

#include "ui/painters/WindowPainter.h"

#include "engine/generators/Generator.h"
#include "engine/generators/EuclideanGenerator.h"
#include "engine/generators/RandomGenerator.h"

enum class ContextAction {
    Init,
    Reserved1,
    Reserved2,
    Revert,
    Commit,
    Last
};

static const ContextMenuModel::Item contextMenuItems[] = {
    { "INIT" },
    { nullptr },
    { nullptr },
    { "REVERT" },
    { "COMMIT" },
};

GeneratorPage::GeneratorPage(PageManager &manager, PageContext &context) :
    BasePage(manager, context)
{}

void GeneratorPage::show(Generator *generator) {
    _generator = generator;

    BasePage::show();
}

void GeneratorPage::enter() {
    _valueRange.first = 0;
    _valueRange.second = 7;
}

void GeneratorPage::exit() {
}

void GeneratorPage::draw(Canvas &canvas) {
    const char *functionNames[5];
    for (int i = 0; i < 5; ++i) {
        functionNames[i] = i < _generator->paramCount() ? _generator->paramName(i) : nullptr;
    }

    WindowPainter::clear(canvas);
    WindowPainter::drawHeader(canvas, _model, _engine, "GENERATOR");
    WindowPainter::drawActiveFunction(canvas, _generator->name());
    WindowPainter::drawFooter(canvas, functionNames, pageKeyState());

    canvas.setFont(Font::Small);
    canvas.setBlendMode(BlendMode::Set);
    canvas.setColor(Color::Bright);

    auto drawValue = [&] (int index, const char *str) {
        int w = Width / 5;
        int x = (Width * index) / 5;
        int y = Height - 16;
        canvas.drawText(x + (w - canvas.textWidth(str)) / 2, y, str);
    };

    for (int i = 0; i < _generator->paramCount(); ++i) {
        FixedStringBuilder<8> str;
        _generator->printParam(i, str);
        drawValue(i, str);
    }

    switch (_generator->mode()) {
    case Generator::Mode::InitLayer:
        // no page
        break;
    case Generator::Mode::Euclidean:
        drawEuclideanGenerator(canvas, *static_cast<const EuclideanGenerator *>(_generator));
        break;
    case Generator::Mode::Random:
        drawRandomGenerator(canvas, *static_cast<const RandomGenerator *>(_generator));
        break;
    case Generator::Mode::Last:
        break;
    }
}

void GeneratorPage::updateLeds(Leds &leds) {
    // value range
    for (int i = 0; i < 8; ++i) {
        bool inRange = (i >= _valueRange.first && i <= _valueRange.second) || (i >= _valueRange.second && i <= _valueRange.first);
        bool inverted = _valueRange.first > _valueRange.second;
        leds.set(MatrixMap::toStep(i), inRange && inverted, inRange && !inverted);
    }
    for (int i = 0; i < 7; ++i) {
        leds.set(MatrixMap::toStep(8 + i), false, false);
    }
}

void GeneratorPage::keyDown(KeyEvent &event) {
    const auto &key = event.key();

    if (key.isGlobal()) {
        return;
    }

    if (key.isStep()) {
    }

    event.consume();
}

void GeneratorPage::keyUp(KeyEvent &event) {
    const auto &key = event.key();

    if (key.isGlobal()) {
        return;
    }

    if (key.isStep()) {
    }

    event.consume();
}

void GeneratorPage::keyPress(KeyPressEvent &event) {
    const auto &key = event.key();

    if (key.isContextMenu()) {
        contextShow();
        event.consume();
        return;
    }

    // Toggle function buttons (F0-F4)
    if (key.isFunction()) {
        int funcIndex = key.function();
        if (funcIndex < _generator->paramCount()) {
            // Toggle: if already active, deactivate; otherwise activate this function
            if (_activeFunction == funcIndex) {
                _activeFunction = -1;  // Deactivate
            } else {
                _activeFunction = funcIndex;  // Activate this function
            }
        }
        event.consume();
        return;
    }

    if (key.isGlobal()) {
        return;
    }

    if (key.isStep()) {
        int secondStep = key.step();
        if (secondStep < 8) {
            int count = 0;
            int firstStep = -1;
            for (int step = 0; step < 8; ++step) {
                if (key.state()[MatrixMap::fromStep(step)]) {
                    ++count;
                    if (step != secondStep) {
                        firstStep = step;
                    }
                }
            }
            if (count == 2) {
                _valueRange.first = firstStep;
                _valueRange.second = secondStep;
                DBG("range %d %d", firstStep, secondStep);
            }
        }

    }

    event.consume();
}

void GeneratorPage::encoder(EncoderEvent &event) {
    bool changed = false;

    // Use active function if set (toggle mode), otherwise check held buttons
    if (_activeFunction >= 0 && _activeFunction < _generator->paramCount()) {
        _generator->editParam(_activeFunction, event.value(), event.pressed());
        changed = true;
    } else {
        // Fallback to original behavior if no function is toggled
        for (int i = 0; i < _generator->paramCount(); ++i) {
            if (pageKeyState()[Key::F0 + i]) {
                _generator->editParam(i, event.value(), event.pressed());
                changed = true;
            }
        }
    }

    if (changed) {
        _generator->update();
    }
}

void GeneratorPage::drawEuclideanGenerator(Canvas &canvas, const EuclideanGenerator &generator) const {
    auto steps = generator.steps();
    const auto &pattern = generator.pattern();

    // Use two rows if more than 16 steps
    int rowCount = (steps > 16) ? 2 : 1;
    int stepsPerRow = (rowCount == 2) ? ((steps + 1) / 2) : steps;

    // Larger step boxes (12px high like step-edit mode)
    int stepHeight = 12;
    int stepWidth = Width / stepsPerRow;
    int rowGap = 3;  // Vertical spacing between rows

    // Center vertically in available space
    int totalHeight = (rowCount * stepHeight) + ((rowCount - 1) * rowGap);
    int startY = (Height - totalHeight) / 2;

    for (int i = 0; i < steps; ++i) {
        // Calculate which row and position within row
        int row = (rowCount == 2 && i >= stepsPerRow) ? 1 : 0;
        int col = (row == 0) ? i : (i - stepsPerRow);

        int x = (Width - stepsPerRow * stepWidth) / 2 + col * stepWidth;
        int y = startY + row * (stepHeight + rowGap);

        // Draw step box (larger, like step-edit mode)
        canvas.setColor(Color::Medium);
        canvas.drawRect(x + 1, y + 1, stepWidth - 2, stepHeight - 2);
        if (pattern[i]) {
            canvas.setColor(Color::Bright);
            canvas.fillRect(x + 2, y + 2, stepWidth - 4, stepHeight - 4);
        }
    }
}

void GeneratorPage::drawRandomGenerator(Canvas &canvas, const RandomGenerator &generator) const {
    const auto &pattern = generator.pattern();
    int steps = pattern.size();

    int stepWidth = Width / steps;
    int stepHeight = 16;
    int x = (Width - steps * stepWidth) / 2;
    int y = 16;

    for (int i = 0; i < steps; ++i) {
        int h = stepHeight - 2;
        int h2 = (h * pattern[i]) / 255;
        canvas.setColor(Color::Low);
        canvas.drawRect(x + 1, y + 1, stepWidth - 2, h);
        canvas.setColor(Color::Bright);
        canvas.hline(x + 1, y + 1 + h - h2, stepWidth - 2);
        // canvas.fillRect(x + 1, y + 1 + h - h2 , stepWidth - 2, h2);
        x += stepWidth;
    }
}

void GeneratorPage::contextShow() {
    showContextMenu(ContextMenu(
        contextMenuItems,
        int(ContextAction::Last),
        [&] (int index) { contextAction(index); },
        [&] (int index) { return contextActionEnabled(index); }
    ));
}

void GeneratorPage::contextAction(int index) {
    switch (ContextAction(index)) {
    case ContextAction::Init:
        init();
        break;
    case ContextAction::Reserved1:
    case ContextAction::Reserved2:
        break;
    case ContextAction::Revert:
        revert();
        break;
    case ContextAction::Commit:
        commit();
        break;
    case ContextAction::Last:
        break;
    }
}

bool GeneratorPage::contextActionEnabled(int index) const {
    return true;
}

void GeneratorPage::init() {
    _generator->init();
}

void GeneratorPage::revert() {
    _generator->revert();
    close();
}

void GeneratorPage::commit() {
    close();
}
