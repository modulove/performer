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
        Mode = 1,
        Rate = 2,
        Depth = 3,
        Offset = 4,
        Phase = 5,
    };

    // Routing overlay function enum (reordered: F1=MODE, F2=GATE, F3=TARGET, F4=EVENT, F5=CC NUM)
    enum class RoutingFunction {
        Mode = 0,      // Free/Sync/Retrigger
        Gate = 1,      // Gate track 1-16
        Target = 2,    // MIDI 1-16 / CV 1-8
        Event = 3,     // Note vs CC (MIDI only)
        CCNumber = 4,  // CC# 0-127 (MIDI CC only)
    };

    void setSelectedModulator(int index);
    void setSelectedFunction(Function function);
    void setSelectedRoutingFunction(RoutingFunction function);
    void updateLedPreview();
    int generateWaveformPreview(Modulator::Shape shape, uint16_t phase);
    void updateWaveformCache();

    // Context menu for quick mapping
    void contextShow();
    void contextAction(int index);
    void quickMapToOutput(int outputIndex);
    void showRoutingPopup();
    void loadRoutingFromMidiOutput();
    void applyRoutingToMidiOutput();

    int _selectedModulator = 0;
    Function _selectedFunction = Function::Shape;

    // Pagination state
    int _currentPage = 0;  // Current function button page (0 or 1)
    int _totalPages = 1;   // Total number of pages (depends on shape)

    // Routing overlay state (Shift+Page toggle)
    bool _showRoutingOverlay = false;
    RoutingFunction _selectedRoutingFunction = RoutingFunction::Mode;

    enum class RoutingTargetType {
        Midi,  // MIDI outputs 1-16
        CV,    // CV outputs 1-8
    };

    RoutingTargetType _routingTargetType = RoutingTargetType::Midi;
    int _routingTargetIndex = 0;  // Index within the target type (0-15 for MIDI, 0-7 for CV)
    bool _routingEventIsCC = true;  // false = Note, true = CC (only for MIDI)
    int _routingCCNum = 0;  // CC number being configured (0-127, only for MIDI CC)

    // Routing popup state (old popup system, kept for context menu compatibility)
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

    // Waveform cache for performance optimization (sized for left half window)
    static constexpr int WAVEFORM_CACHE_SIZE = 112;  // waveformW - 4 (116 - 4)
    int8_t _waveformCache[WAVEFORM_CACHE_SIZE];
    bool _waveformCacheValid = false;

    // Cache validation - track last parameters used
    Modulator::Shape _lastShape = Modulator::Shape::Last;
    int _lastDepth = -1;
    int _lastOffset = -999;
    int _lastPhase = -1;
};
