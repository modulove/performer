#pragma once

#include "BasePage.h"

class ModulatorPage : public BasePage {
public:
    ModulatorPage(PageManager &manager, PageContext &context);

    virtual void enter() override;
    virtual void exit() override;

    virtual void draw(Canvas &canvas) override;
    virtual void updateLeds(Leds &leds) override;

    virtual void keyPress(KeyPressEvent &event) override;
    virtual void encoder(EncoderEvent &event) override;

private:
    enum class Function {
        Shape = 0,
        Rate = 1,
        Depth = 2,
        Offset = 3,
        Phase = 4,
    };

    void setSelectedModulator(int index);
    void setSelectedFunction(Function function);
    void updateLedPreview();
    int clamp(int value, int min, int max);
    int generateWaveformPreview(Modulator::Shape shape, uint16_t phase);
    void updateWaveformCache();

    // Context menu for quick mapping
    void contextShow();
    void contextAction(int index);
    void quickMapToOutput(int outputIndex);
    void applyCustomPreset();
    void applyModulatorCC0Preset();
    void showRoutingPopup();

    int _selectedModulator = 0;
    Function _selectedFunction = Function::Shape;

    // Routing popup state
    enum class RoutingField {
        Output = 0,    // MIDI Output 1-8
        Mode = 1,      // Note or CC
        CCNumber = 2,  // CC# (only for CC mode)
    };

    bool _showRoutingPopup = false;
    RoutingField _routingField = RoutingField::Output;
    int _routingOutputIndex = 0;
    bool _routingToCC = true;  // false = Note, true = CC
    int _routingCCNumber = 1;  // CC# 0-127

    // Waveform cache for performance optimization
    static constexpr int WAVEFORM_CACHE_SIZE = 252;
    int8_t _waveformCache[WAVEFORM_CACHE_SIZE];
    bool _waveformCacheValid = false;

    // Cache validation - track last parameters used
    Modulator::Shape _lastShape = Modulator::Shape::Last;
    int _lastDepth = -1;
    int _lastOffset = -999;
    int _lastPhase = -1;
};
