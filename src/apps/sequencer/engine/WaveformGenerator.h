#pragma once

#include "model/Modulator.h"
#include <cstdint>

/**
 * WaveformGenerator - Shared waveform generation for ModulatorEngine and ModulatorPage
 *
 * Provides consistent waveform generation algorithms used by both the engine (audio)
 * and UI (visual preview). Using the same algorithms ensures what you see matches
 * what you hear.
 */
class WaveformGenerator {
public:
    /**
     * Generate a waveform sample value
     *
     * @param shape The waveform shape to generate
     * @param phase Phase position (0-65535 representing 0-360 degrees)
     * @return Waveform value in range -127 to +127
     */
    static int generate(Modulator::Shape shape, uint16_t phase) {
        switch (shape) {
        case Modulator::Shape::Sine:
            return generateSine(phase);
        case Modulator::Shape::Triangle:
            return generateTriangle(phase);
        case Modulator::Shape::SawUp:
            return generateSawUp(phase);
        case Modulator::Shape::SawDown:
            return generateSawDown(phase);
        case Modulator::Shape::Square:
            return generateSquare(phase);
        case Modulator::Shape::Random:
            // Random waveform is handled separately in ModulatorEngine
            // because it requires state (target values, interpolation)
            return 0;
        default:
            return 0;
        }
    }

private:
    /**
     * Generate sine wave using parabolic approximation
     * Higher quality than linear segment approximation
     */
    static int generateSine(uint16_t phase) {
        // Parabolic approximation for smooth sine wave
        // Convert phase to -32768 to +32767 range
        int32_t x = (int32_t)phase - 32768;

        // Normalize to -1.0 to +1.0 range (using fixed point: x / 32768)
        // Use parabolic approximation: sin(x) ≈ 4x(1-|x|) for x in [-1, 1]
        int32_t abs_x = (x < 0) ? -x : x;

        // Calculate 4 * x * (32768 - abs_x) / 32768
        // This gives us a parabolic approximation
        int32_t result = (4 * x * (32768 - abs_x)) / 32768;

        // Scale to -127 to +127 range and negate to fix inversion
        return -(int)(result * 127 / 32768);
    }

    /**
     * Generate triangle wave (one complete cycle: up then down)
     */
    static int generateTriangle(uint16_t phase) {
        // First half: rise from -127 to +127
        if (phase < 32768) {
            return -127 + ((phase * 254) / 32768);
        }
        // Second half: fall from +127 to -127
        else {
            return 127 - (((phase - 32768) * 254) / 32768);
        }
    }

    /**
     * Generate sawtooth wave (upward)
     */
    static int generateSawUp(uint16_t phase) {
        return ((int)phase * 254 / 65536) - 127;
    }

    /**
     * Generate sawtooth wave (downward)
     */
    static int generateSawDown(uint16_t phase) {
        return 127 - ((int)phase * 254 / 65536);
    }

    /**
     * Generate square wave
     */
    static int generateSquare(uint16_t phase) {
        return (phase < 32768) ? 127 : -127;
    }
};
