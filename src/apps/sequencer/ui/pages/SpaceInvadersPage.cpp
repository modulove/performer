#include "SpaceInvadersPage.h"

#include "os/os.h"

SpaceInvadersPage::SpaceInvadersPage(PageManager &manager, PageContext &context) :
    BasePage(manager, context)
{}

void SpaceInvadersPage::enter() {
    _lastTicks = os::ticks();
    _game.init();

    _engine.setGateOutputOverride(true);
    _engine.setGateOutput(0);
    _engine.suspend();
}

void SpaceInvadersPage::exit() {
    _engine.setGateOutputOverride(false);
    _engine.resume();
}

void SpaceInvadersPage::draw(Canvas &canvas) {
    uint32_t currentTicks = os::ticks();
    float dt = float(currentTicks - _lastTicks) / os::time::ms(1000);
    _lastTicks = currentTicks;

    const auto &cvInput = _engine.cvInput();
    spaceinvaders::Inputs cvInputs;
    cvInputs.left  |= cvInput.channel(0) > 1.f;
    cvInputs.right |= cvInput.channel(1) > 1.f;
    cvInputs.shoot |= cvInput.channel(2) > 1.f;

    spaceinvaders::Inputs inputs;
    inputs.keys = _keyInputs.keys | cvInputs.keys;
    spaceinvaders::Outputs outputs;

    _game.update(dt, inputs, outputs);
    _game.draw(canvas);

    _engine.setGateOutput(outputs.gates);
}

void SpaceInvadersPage::updateLeds(Leds &leds) {

}

void SpaceInvadersPage::keyDown(KeyEvent &event) {
    const auto &key = event.key();

    _keyInputs.left |= key.is(Key::F0);
    _keyInputs.right |= key.is(Key::F2);
    _keyInputs.shoot |= key.is(Key::F4);

    event.consume();
}

void SpaceInvadersPage::keyUp(KeyEvent &event) {
    const auto &key = event.key();

    _keyInputs.left &= !key.is(Key::F0);
    _keyInputs.right &= !key.is(Key::F2);
    _keyInputs.shoot &= !key.is(Key::F4);

    event.consume();
}

void SpaceInvadersPage::keyPress(KeyPressEvent &event) {
    const auto &key = event.key();

    if (key.pageModifier() && key.is(Key::Step15)) {
        close();
    }
}

void SpaceInvadersPage::encoder(EncoderEvent &event) {
    event.consume();
}
