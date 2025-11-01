#pragma once

#include "Config.h"
#include "model/Modulator.h"
#include "WaveformGenerator.h"
#include "core/utils/Random.h"
#include "core/utils/MathUtils.h"

#include <cstdint>
#include <cmath>
#include <algorithm>

class ModulatorEngine {
public:
    // ADSR envelope states (public for UI visualization)
    enum class ADSRState : uint8_t {
        Idle,       // Envelope inactive (gate off, release complete)
        Attack,     // Rising from 0 to peak
        Decay,      // Falling from peak to sustain level
        Sustain,    // Holding at sustain level
        Release     // Falling from current level to 0
    };

    ModulatorEngine() {}

    void reset() {
        for (int i = 0; i < CONFIG_MODULATOR_COUNT; ++i) {
            _phaseAccumulator[i] = 0;
            _lastRandomValue[i] = 0;
            _currentValue[i] = 0;
            _lastGate[i] = false;
            _targetValue[i] = 0;
            // Seed each modulator's RNG with a unique seed based on index
            // This ensures different random sequences for each modulator
            _rng[i].seed(12345 + i * 6789);
            // ADSR state
            _adsrState[i] = ADSRState::Idle;
            _adsrLevel[i] = 0;
            _adsrTimer[i] = 0;
            _adsrReleaseStartLevel[i] = 0;
        }
        _tickDurationSec = DefaultTickDurationSec;
    }

    void tick(uint32_t tick, const Modulator &modulator, int index, bool gate, float tickDurationSec) {
        if (index < 0 || index >= CONFIG_MODULATOR_COUNT) {
            return;
        }

        _tickDurationSec = (tickDurationSec > 0.f) ? tickDurationSec : DefaultTickDurationSec;
        (void)tick;

        // Handle Random shape in Triggered mode
        if (modulator.shape() == Modulator::Shape::Random &&
            modulator.randomMode() == Modulator::RandomMode::Triggered) {
            // Detect rising edge (gate went from false to true)
            if (gate && !_lastGate[index]) {
                _targetValue[index] = _rng[index].nextRange(255) - 127;
            }
            _lastGate[index] = gate;

            // Apply smooth interpolation
            int current = _lastRandomValue[index];
            int target = _targetValue[index];
            int smoothMs = modulator.smooth();

            // Calculate slew rate using tempo-aware calculation (unified with clocked mode)
            int slewRate = msToTicks(smoothMs);

            int diff = target - current;
            current += diff / slewRate;
            _lastRandomValue[index] = current;

            int value = current;
            value = (value * modulator.depth()) / 127;
            value += modulator.offset();
            _currentValue[index] = utils::clamp(value + 64, 0, 127);
            return;
        }

        // Handle ADSR shape
        if (modulator.shape() == Modulator::Shape::ADSR) {
            // Detect gate changes
            bool gateRising = gate && !_lastGate[index];
            bool gateFalling = !gate && _lastGate[index];
            _lastGate[index] = gate;

            // ADSR state machine
            if (gateRising) {
                // Start attack phase
                _adsrState[index] = ADSRState::Attack;
                _adsrTimer[index] = 0;
                _adsrReleaseStartLevel[index] = _adsrLevel[index];
            } else if (gateFalling && _adsrState[index] != ADSRState::Release) {
                // Start release phase
                _adsrState[index] = ADSRState::Release;
                _adsrTimer[index] = 0;
                _adsrReleaseStartLevel[index] = _adsrLevel[index];
            }

            // Calculate envelope level based on current state
            int level = 0;
            switch (_adsrState[index]) {
            case ADSRState::Idle:
                level = 0;
                break;
            case ADSRState::Attack: {
                int attackMs = modulator.attack();
                if (attackMs == 0) {
                    level = 127;
                    _adsrState[index] = ADSRState::Decay;
                    _adsrTimer[index] = 0;
                } else {
                    int attackTicks = msToTicks(attackMs);
                    _adsrTimer[index]++;
                    level = utils::clamp(static_cast<int>((127 * _adsrTimer[index]) / attackTicks), 0, 127);
                    if (_adsrTimer[index] >= static_cast<uint32_t>(attackTicks)) {
                        level = 127;
                        _adsrState[index] = ADSRState::Decay;
                        _adsrTimer[index] = 0;
                    }
                }
                break;
            }
            case ADSRState::Decay: {
                int decayMs = modulator.decay();
                int sustainLevel = modulator.sustain();
                if (decayMs == 0 || sustainLevel >= 127) {
                    level = sustainLevel;
                    _adsrState[index] = ADSRState::Sustain;
                } else {
                    int decayTicks = msToTicks(decayMs);
                    _adsrTimer[index]++;
                    int decayAmount = 127 - sustainLevel;
                    int decayProgress = utils::clamp(static_cast<int>((decayAmount * _adsrTimer[index]) / decayTicks), 0, decayAmount);
                    level = 127 - decayProgress;
                    if (_adsrTimer[index] >= static_cast<uint32_t>(decayTicks)) {
                        level = sustainLevel;
                        _adsrState[index] = ADSRState::Sustain;
                    }
                }
                break;
            }
            case ADSRState::Sustain:
                level = modulator.sustain();
                break;
            case ADSRState::Release: {
                int releaseMs = modulator.release();
                int startLevel = _adsrReleaseStartLevel[index];
                if (releaseMs == 0) {
                    level = 0;
                    _adsrState[index] = ADSRState::Idle;
                } else {
                    int releaseTicks = msToTicks(releaseMs);
                    _adsrTimer[index]++;
                    int releaseProgress = utils::clamp(static_cast<int>((startLevel * _adsrTimer[index]) / releaseTicks), 0, startLevel);
                    level = startLevel - releaseProgress;
                    if (_adsrTimer[index] >= static_cast<uint32_t>(releaseTicks)) {
                        level = 0;
                        _adsrState[index] = ADSRState::Idle;
                        _adsrReleaseStartLevel[index] = 0;
                    }
                }
                break;
            }
            }

            if (_adsrState[index] != ADSRState::Release) {
                _adsrReleaseStartLevel[index] = level;
            }
            _adsrLevel[index] = level;

            // Apply amplitude scaling (Amplitude is the main level control for ADSR)
            // ADSR does not use depth or offset parameters
            int value = (level * modulator.amplitude()) / 127;
            _currentValue[index] = utils::clamp(value, 0, 127);
            return;
        }

        // Standard LFO modes (including Random in Clocked mode)
        // Calculate phase increment based on rate
        // tick is called at CONFIG_PPQN (192) per quarter note
        // rate is the period in ticks (e.g., 384 ticks = 1 bar at PPQN=96)
        // But CONFIG_PPQN is 192, so we need to double the rate value
        uint32_t rate = modulator.rate() * 2;  // Convert from PPQN=96 to PPQN=192

        // Increment phase based on rate (in ticks)
        // phase is 0-65535 representing one complete cycle
        // phaseIncrement per tick = 65536 / rate
        uint32_t phaseIncrement = 65536 / rate;
        _phaseAccumulator[index] += phaseIncrement;

        // Add phase offset
        uint32_t phase = _phaseAccumulator[index] + ((uint32_t)modulator.phase() * 65536 / 360);

        // Generate waveform value (-127 to +127)
        int value = generateWaveform(modulator.shape(), phase & 0xFFFF, index, modulator);

        // Apply depth and offset
        value = (value * modulator.depth()) / 127;
        value += modulator.offset();

        // Clamp to 0-127 for MIDI CC output
        _currentValue[index] = utils::clamp(value + 64, 0, 127);
    }

    int currentValue(int index) const {
        if (index < 0 || index >= CONFIG_MODULATOR_COUNT) {
            return 0;
        }
        return _currentValue[index];
    }

    // Get current phase (0-65535) for playhead visualization
    uint16_t currentPhase(int index) const {
        if (index < 0 || index >= CONFIG_MODULATOR_COUNT) {
            return 0;
        }
        return _phaseAccumulator[index] & 0xFFFF;
    }

    // Get ADSR state for visualization
    ADSRState adsrState(int index) const {
        if (index < 0 || index >= CONFIG_MODULATOR_COUNT) {
            return ADSRState::Idle;
        }
        return _adsrState[index];
    }

    // Get ADSR timer (ticks in current state) for playhead calculation
    uint32_t adsrTimer(int index) const {
        if (index < 0 || index >= CONFIG_MODULATOR_COUNT) {
            return 0;
        }
        return _adsrTimer[index];
    }

private:
    int generateWaveform(Modulator::Shape shape, uint16_t phase, int modulatorIndex, const Modulator &modulator) {
        // phase is 0-65535 representing 0-360 degrees
        // return value is -127 to +127

        // Handle Random shape separately since it requires state
        if (shape == Modulator::Shape::Random) {
            // Clocked Random mode - generates new random value at each cycle
            // Generate new random target value every cycle (phase wraps from high to low)
            if (phase < _lastPhase[modulatorIndex]) {
                _targetValue[modulatorIndex] = _rng[modulatorIndex].nextRange(255) - 127;
            }
            _lastPhase[modulatorIndex] = phase;

            // Apply smooth interpolation based on Smooth parameter (0-2000ms)
            int current = _lastRandomValue[modulatorIndex];
            int target = _targetValue[modulatorIndex];
            int smoothMs = modulator.smooth();

            // Calculate slew rate using tempo-aware calculation
            int slewRate = msToTicks(smoothMs);

            int diff = target - current;
            current += diff / slewRate;
            _lastRandomValue[modulatorIndex] = current;

            return current;
        }

        // Use WaveformGenerator for all other shapes
        return WaveformGenerator::generate(shape, phase);
    }

    /**
     * Calculate slew rate for smooth parameter interpolation
     * Uses tempo-aware calculation for musical timing
     */
    int msToTicks(int ms) const {
        if (ms <= 0) {
            return 1;
        }
        float duration = (_tickDurationSec > 0.f) ? _tickDurationSec : DefaultTickDurationSec;
        float ticks = ms / (duration * 1000.f);
        return std::max(1, static_cast<int>(std::ceil(ticks)));
    }

    static constexpr float DefaultTickDurationSec = 60.f / (120.f * CONFIG_PPQN);

    uint32_t _phaseAccumulator[CONFIG_MODULATOR_COUNT] = {0};
    int _currentValue[CONFIG_MODULATOR_COUNT] = {0};
    int _lastRandomValue[CONFIG_MODULATOR_COUNT] = {0};
    uint16_t _lastPhase[CONFIG_MODULATOR_COUNT] = {0};
    bool _lastGate[CONFIG_MODULATOR_COUNT] = {false};
    int _targetValue[CONFIG_MODULATOR_COUNT] = {0};
    Random _rng[CONFIG_MODULATOR_COUNT];  // Per-modulator random number generators

    // ADSR envelope state
    ADSRState _adsrState[CONFIG_MODULATOR_COUNT] = {ADSRState::Idle};
    int _adsrLevel[CONFIG_MODULATOR_COUNT] = {0};
    uint32_t _adsrTimer[CONFIG_MODULATOR_COUNT] = {0};
    int _adsrReleaseStartLevel[CONFIG_MODULATOR_COUNT] = {0};
    float _tickDurationSec = DefaultTickDurationSec;
};
