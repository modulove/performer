#pragma once

#include "BasePage.h"

#include "ui/model/OutputListModel.h"

#include "model/MidiOutput.h"

#include "engine/MidiLearn.h"

class MidiOutputPage : public BasePage {
public:
    MidiOutputPage(PageManager &manager, PageContext &context);

    void reset();

    virtual void enter() override;
    virtual void exit() override;

    virtual void draw(Canvas &canvas) override;
    virtual void updateLeds(Leds &leds) override;

    virtual void keyPress(KeyPressEvent &event) override;
    virtual void encoder(EncoderEvent &event) override;

private:
    enum class Function {
        Target = 0,
        Event = 1,
        Param1 = 2,
        Param2 = 3,
        Param3 = 4,
    };

    void showOutput(int outputIndex);
    void selectOutput(int outputIndex);
    void setSelectedFunction(Function function);

    uint8_t _selectedOutput = 0;
    Function _selectedFunction = Function::Target;
};
