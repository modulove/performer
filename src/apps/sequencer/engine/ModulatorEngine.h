#pragma once

#include "Config.h"
#include "model/Modulator.h"

#include <cstdint>
#include <cmath>

class ModulatorEngine {
public:
    ModulatorEngine() {}

    void reset() {
        for (int i = 0; i < CONFIG_MODULATOR_COUNT; ++i) {
            _phaseAccumulator[i] = 0;
            _lastRandomValue[i] = 0;
            _currentValue[i] = 0;
            _lastGate[i] = false;
            _targetValue[i] = 0;
        }
    }

    void tick(uint32_t tick, const Modulator &modulator, int index, bool gate) {
        if (index < 0 || index >= CONFIG_MODULATOR_COUNT) {
            return;
        }

        // Handle gate-triggered random modes
        if (modulator.shape() == Modulator::Shape::GateRandomStep) {
            // Detect rising edge (gate went from false to true)
            if (gate && !_lastGate[index]) {
                _lastRandomValue[index] = (rand() % 255) - 127;
            }
            _lastGate[index] = gate;

            int value = _lastRandomValue[index];
            value = (value * modulator.depth()) / 127;
            value += modulator.offset();
            _currentValue[index] = clamp(value + 64, 0, 127);
            return;
        }

        // GateRandomSmooth removed - smooth behavior now handled by Smooth parameter

        // Standard LFO modes
        // Calculate phase increment based on rate
        // tick is in PPQN units (CONFIG_PPQN = 192)
        uint32_t rate = modulator.rate();  // Divisor value

        // Increment phase based on rate divisor
        // phase is 0-65535 representing 0-360 degrees
        uint32_t phaseIncrement = (65536 * CONFIG_PPQN) / (rate * CONFIG_PPQN / 4);
        _phaseAccumulator[index] += phaseIncrement;

        // Add phase offset
        uint32_t phase = _phaseAccumulator[index] + ((uint32_t)modulator.phase() * 65536 / 360);

        // Generate waveform value (-127 to +127)
        int value = generateWaveform(modulator.shape(), phase & 0xFFFF, index, modulator);

        // Apply depth and offset
        value = (value * modulator.depth()) / 127;
        value += modulator.offset();

        // Clamp to 0-127 for MIDI CC output
        _currentValue[index] = clamp(value + 64, 0, 127);
    }

    int currentValue(int index) const {
        if (index < 0 || index >= CONFIG_MODULATOR_COUNT) {
            return 0;
        }
        return _currentValue[index];
    }

private:
    int generateWaveform(Modulator::Shape shape, uint16_t phase, int modulatorIndex, const Modulator &modulator) {
        // phase is 0-65535 representing 0-360 degrees
        // return value is -127 to +127

        switch (shape) {
        case Modulator::Shape::Sine: {
            // Sine wave using approximation
            int angle = (phase * 360) / 65536;
            if (angle <= 90) {
                return (angle * 127) / 90;
            } else if (angle <= 180) {
                return ((180 - angle) * 127) / 90;
            } else if (angle <= 270) {
                return -((angle - 180) * 127) / 90;
            } else {
                return -((360 - angle) * 127) / 90;
            }
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
        case Modulator::Shape::RandomSmooth: {
            // Generate new random target value every cycle
            if (phase < _lastPhase[modulatorIndex]) {
                _targetValue[modulatorIndex] = (rand() % 255) - 127;
            }
            _lastPhase[modulatorIndex] = phase;

            // Smooth interpolation towards target using phase as smoothness control
            // Phase 0 = fastest (1/2), Phase 360 = slowest (1/128)
            int smoothness = 2 + (modulator.phase() * 126 / 360);  // Range: 2 to 128

            int current = _lastRandomValue[modulatorIndex];
            int target = _targetValue[modulatorIndex];
            int diff = target - current;
            current += diff / smoothness;
            _lastRandomValue[modulatorIndex] = current;

            return current;
        }
        case Modulator::Shape::RandomStep: {
            // Step random - new value every 1/16th of cycle
            uint8_t step = phase / 4096;
            if (step != _lastRandomStep[modulatorIndex]) {
                _lastRandomValue[modulatorIndex] = (rand() % 255) - 127;
                _lastRandomStep[modulatorIndex] = step;
            }
            return _lastRandomValue[modulatorIndex];
        }
        case Modulator::Shape::GateRandomStep:
            // Gate-triggered step random - handled in tick()
            return _lastRandomValue[modulatorIndex];
        default:
            return 0;
        }
    }

    int clamp(int value, int min, int max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }

    uint32_t _phaseAccumulator[CONFIG_MODULATOR_COUNT] = {0};
    int _currentValue[CONFIG_MODULATOR_COUNT] = {0};
    int _lastRandomValue[CONFIG_MODULATOR_COUNT] = {0};
    uint16_t _lastPhase[CONFIG_MODULATOR_COUNT] = {0};
    uint8_t _lastRandomStep[CONFIG_MODULATOR_COUNT] = {0};
    bool _lastGate[CONFIG_MODULATOR_COUNT] = {false};
    int _targetValue[CONFIG_MODULATOR_COUNT] = {0};
};
