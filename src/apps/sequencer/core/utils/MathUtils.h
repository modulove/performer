#pragma once

namespace utils {
    /**
     * Clamps a value between a minimum and maximum value.
     *
     * @tparam T The type of the value (must support < and > operators)
     * @param value The value to clamp
     * @param min The minimum allowed value
     * @param max The maximum allowed value
     * @return The clamped value
     */
    template<typename T>
    inline T clamp(T value, T min, T max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }
}
