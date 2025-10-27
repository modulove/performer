#pragma once

#include "BasePage.h"

#include "spaceinvaders/SpaceInvaders.h"

class SpaceInvadersPage : public BasePage {
public:
    SpaceInvadersPage(PageManager &manager, PageContext &context);

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
    spaceinvaders::Game _game;
    spaceinvaders::Inputs _keyInputs;

    uint32_t _lastTicks;
};
