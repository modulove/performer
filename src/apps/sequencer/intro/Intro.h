#pragma once

#include "core/gfx/Canvas.h"

class Intro {
public:
    Intro();

    void init();

    void update(float dt, uint8_t gates = 0);
    void draw(Canvas &canvas);

private:
    float _time;
    uint8_t _pulse_state;
};
