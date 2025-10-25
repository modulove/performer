#pragma once

#include "Config.h"
#include "Types.h"
#include "Serialize.h"
#include "ModelUtils.h"

#include "core/math/Math.h"
#include "core/utils/StringBuilder.h"

#include <cstdint>

class Modulator {
public:
    //----------------------------------------
    // Types
    //----------------------------------------

    enum class Shape : uint8_t {
        Sine,
        Triangle,
        SawUp,
        SawDown,
        Square,
        RandomSmooth,
        RandomStep,
        GateRandomStep,
        Last
    };

    static const char *shapeName(Shape shape) {
        switch (shape) {
        case Shape::Sine:               return "Sine";
        case Shape::Triangle:           return "Triangle";
        case Shape::SawUp:              return "Saw Up";
        case Shape::SawDown:            return "Saw Down";
        case Shape::Square:             return "Square";
        case Shape::RandomSmooth:       return "Smooth Random";
        case Shape::RandomStep:         return "Stepped Random";
        case Shape::GateRandomStep:     return "Stepped Random";
        case Shape::Last:               break;
        }
        return nullptr;
    }

    //----------------------------------------
    // Properties
    //----------------------------------------

    // shape

    Shape shape() const { return _shape; }
    void setShape(Shape shape) {
        _shape = ModelUtils::clampedEnum(shape);
    }

    void editShape(int value, bool shift) {
        setShape(ModelUtils::adjustedEnum(shape(), value));
    }

    void printShape(StringBuilder &str) const {
        str(shapeName(shape()));
    }

    // rate (in clock divisors: 1/96 to 16 bars)
    // Stored as divisor value (1-1536)

    int rate() const { return _rate; }
    void setRate(int rate) {
        _rate = clamp(rate, 1, 1536);
    }

    void editRate(int value, bool shift) {
        // Standard musical divisions (in PPQN units, where 1 bar = 384)
        // Including triplets and dotted notes
        static const int divisions[] = {
            1,    // 1/384
            2,    // 1/192
            3,    // 1/128
            4,    // 1/96
            6,    // 1/64
            8,    // 1/64T (triplet)
            12,   // 1/32
            16,   // 1/32T (triplet)
            18,   // 1/32. (dotted)
            24,   // 1/16
            32,   // 1/16T (triplet)
            36,   // 1/16. (dotted)
            48,   // 1/8
            64,   // 1/8T (triplet)
            72,   // 1/8. (dotted)
            96,   // 1/4
            128,  // 1/4T (triplet)
            144,  // 1/4. (dotted)
            192,  // 1/2
            288,  // 1/2. (dotted)
            384,  // 1 bar
            768,  // 2 bars
            1152, // 3 bars
            1536  // 4 bars
        };
        const int numDivisions = sizeof(divisions) / sizeof(divisions[0]);

        // Find current index
        int currentIndex = 0;
        for (int i = 0; i < numDivisions; ++i) {
            if (divisions[i] >= _rate) {
                currentIndex = i;
                break;
            }
        }

        // Move to next/prev division
        int newIndex = clamp(currentIndex + value, 0, numDivisions - 1);
        _rate = divisions[newIndex];
    }

    void printRate(StringBuilder &str) const {
        int r = rate();

        // Check for specific triplet and dotted divisions
        if (r == 8) str("1/64T");
        else if (r == 16) str("1/32T");
        else if (r == 18) str("1/32.");
        else if (r == 32) str("1/16T");
        else if (r == 36) str("1/16.");
        else if (r == 64) str("1/8T");
        else if (r == 72) str("1/8.");
        else if (r == 128) str("1/4T");
        else if (r == 144) str("1/4.");
        else if (r == 288) str("1/2.");
        // Standard divisions
        else if (r >= 384) {
            int bars = (r + 192) / 384;  // Round to nearest bar
            str("%d bar%s", bars, bars > 1 ? "s" : "");
        } else if (r >= 96) {
            int quarters = (r + 48) / 96;  // Round to nearest quarter
            str("%d/4", quarters);
        } else if (r >= 24) {
            int divisor = (384 + r/2) / r;  // Round to nearest division
            str("1/%d", divisor);
        } else {
            str("1/%d", 384 / r);
        }
    }

    // depth (0-127)

    int depth() const { return _depth; }
    void setDepth(int depth) {
        _depth = clamp(depth, 0, 127);
    }

    void editDepth(int value, bool shift) {
        // Better encoder scaling: increment by 4 (or 1 with shift)
        setDepth(depth() + value * (shift ? 1 : 4));
    }

    void printDepth(StringBuilder &str) const {
        str("%d", depth());
    }

    // offset (-64 to +63)

    int offset() const { return _offset; }
    void setOffset(int offset) {
        _offset = clamp(offset, -64, 63);
    }

    void editOffset(int value, bool shift) {
        // Better encoder scaling: increment by 4 (or 1 with shift)
        setOffset(offset() + value * (shift ? 1 : 4));
    }

    void printOffset(StringBuilder &str) const {
        str("%+d", offset());
    }

    // phase (0-360 degrees)

    int phase() const { return _phase; }
    void setPhase(int phase) {
        _phase = clamp(phase, 0, 360);
    }

    void editPhase(int value, bool shift) {
        setPhase(phase() + value * (shift ? 1 : 15));
    }

    void printPhase(StringBuilder &str) const {
        str("%d°", phase());
    }

    // smooth (0-2000ms, for smooth random shapes)

    int smooth() const { return _smooth; }
    void setSmooth(int smooth) {
        _smooth = clamp(smooth, 0, 2000);
    }

    void editSmooth(int value, bool shift) {
        // Better encoder scaling: increment by 20ms (or 1ms with shift)
        setSmooth(smooth() + value * (shift ? 1 : 20));
    }

    void printSmooth(StringBuilder &str) const {
        str("%dms", smooth());
    }

    // gateTrack (0-15, which track's gate triggers gate-random modes)

    int gateTrack() const { return _gateTrack; }
    void setGateTrack(int track) {
        _gateTrack = clamp(track, 0, 15);
    }

    void editGateTrack(int value, bool shift) {
        setGateTrack(gateTrack() + value);
    }

    void printGateTrack(StringBuilder &str) const {
        str("T%d", gateTrack() + 1);
    }

    //----------------------------------------
    // Methods
    //----------------------------------------

    void clear() {
        setShape(Shape::Sine);
        setRate(96);  // 1/4 note
        setDepth(127);
        setOffset(0);
        setPhase(0);
        setSmooth(100);  // 100ms default
        setGateTrack(0);
    }

    void write(VersionedSerializedWriter &writer) const {
        writer.write(_shape);
        writer.write(_rate);
        writer.write(_depth);
        writer.write(_offset);
        writer.write(_phase);
        writer.write(_smooth);
        writer.write(_gateTrack);
    }

    void read(VersionedSerializedReader &reader) {
        reader.read(_shape);
        reader.read(_rate);
        reader.read(_depth);
        reader.read(_offset);
        reader.read(_phase);
        reader.read(_smooth);
        reader.read(_gateTrack);
    }

private:
    Shape _shape = Shape::Sine;
    uint16_t _rate = 96;      // Clock divisor (1/4 note default)
    uint8_t _depth = 127;
    int8_t _offset = 0;
    uint16_t _phase = 0;
    uint16_t _smooth = 100;   // Smooth time in ms (0-2000)
    uint8_t _gateTrack = 0;   // Which track's gate triggers gate-random modes
};
