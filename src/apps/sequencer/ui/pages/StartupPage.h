#pragma once

#include "BasePage.h"

class StartupPage : public BasePage {
public:
    StartupPage(PageManager &manager, PageContext &context);

    virtual void draw(Canvas &canvas) override;
    virtual void updateLeds(Leds &leds) override;

    virtual bool isModal() const override { return true; }

private:
    enum class State {
        Initial,
        Loading,
        Ready,
    };

    float time() const;
    float relTime() const { return time() / LoadTime; }
    void drawSpaceInvader(Canvas &canvas, int x, int y, int type, int scale = 1);
    void drawHeart(Canvas &canvas, int x, int y, Color color);
    void drawPewFormerLogo(Canvas &canvas, int x, int y, Color color);
    void drawModuloveLogo(Canvas &canvas, float fade);
    void drawKeyboardIcon(Canvas &canvas, int x, int y, float fade);

    static constexpr int LoadTime = 2;

    uint32_t _startTicks;
    State _state = State::Initial;
};
