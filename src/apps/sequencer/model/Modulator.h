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
        Random,
        ADSR,
        Last
    };

    static const char *shapeName(Shape shape) {
        switch (shape) {
        case Shape::Sine:               return "Sine";
        case Shape::Triangle:           return "Triangle";
        case Shape::SawUp:              return "Saw Up";
        case Shape::SawDown:            return "Saw Down";
        case Shape::Square:             return "Square";
        case Shape::Random:             return "Random";
        case Shape::ADSR:               return "ADSR";
        case Shape::Last:               break;
        }
        return nullptr;
    }

    // Mode for LFO shapes (Sine, Triangle, Saw, Square)
    enum class Mode : uint8_t {
        Free,         // Free-running LFO
        Sync,         // Hard sync - reset phase on gate
        Retrigger,    // Soft retrigger - restart from phase offset
        Last
    };

    static const char *modeName(Mode mode) {
        switch (mode) {
        case Mode::Free:         return "Free";
        case Mode::Sync:         return "Sync";
        case Mode::Retrigger:    return "Retrig";
        case Mode::Last:         break;
        }
        return nullptr;
    }

    // Mode for Random shape
    enum class RandomMode : uint8_t {
        Clocked,    // Generate new random value based on Rate parameter
        Triggered,  // Generate new random value on gate trigger
        Last
    };

    static const char *randomModeName(RandomMode mode) {
        switch (mode) {
        case RandomMode::Clocked:      return "Clocked";
        case RandomMode::Triggered:    return "Triggered";
        case RandomMode::Last:         break;
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

    // rate (in clock divisors: 1/64 to 16 bars)
    // Stored as divisor value (6-6144)

    int rate() const { return _rate; }
    void setRate(int rate) {
        _rate = clamp(rate, 6, 6144);
    }

    void editRate(int value, bool shift) {
        // Standard musical divisions (in PPQN units, where 1 bar = 384)
        // Including triplets and dotted notes
        static const int divisions[] = {
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
            1536, // 4 bars
            2304, // 6 bars
            3072, // 8 bars
            4608, // 12 bars
            6144  // 16 bars
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
        // Pressed (shift=true): Fine control (1x)
        // Not pressed (shift=false): Coarse control (4x)
        int multiplier = shift ? 1 : 4;
        setDepth(depth() + value * multiplier);
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
        // Pressed (shift=true): Fine control (1x)
        // Not pressed (shift=false): Coarse control (4x)
        int multiplier = shift ? 1 : 4;
        setOffset(offset() + value * multiplier);
    }

    void printOffset(StringBuilder &str) const {
        str("%+d", offset());
    }

    // phase (0-360 degrees)

    int phase() const { return _phase; }
    void setPhase(int phase) {
        // Wrap around: allow values to wrap from 360 to 0 and vice versa
        while (phase < 0) phase += 360;
        while (phase >= 360) phase -= 360;
        _phase = phase;
    }

    void editPhase(int value, bool shift) {
        // Encoder acceleration: faster when turning quickly
        int absValue = (value < 0) ? -value : value;
        int multiplier = shift ? 1 : 15;
        // Acceleration: 1x for slow, 2x for medium, 4x for fast turns
        if (absValue >= 4) {
            multiplier *= 4;
        } else if (absValue >= 2) {
            multiplier *= 2;
        }
        // Invert direction so turning right shifts waveform right (wraps around)
        setPhase(phase() - value * multiplier);
    }

    void printPhase(StringBuilder &str) const {
        str("%d°", phase());
    }

    // smooth (0-5000ms, for smooth random shapes - slew time)

    int smooth() const { return _smooth; }
    void setSmooth(int smooth) {
        _smooth = clamp(smooth, 0, 5000);
    }

    void editSmooth(int value, bool shift) {
        // Pressed (shift=true): Fine control (1ms steps)
        // Not pressed (shift=false): Coarse control (50ms steps)
        setSmooth(smooth() + value * (shift ? 1 : 50));
    }

    void printSmooth(StringBuilder &str) const {
        str("%dms", smooth());
    }

    // gateTrack (0-15, which track's gate triggers triggered random mode)

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

    // randomMode (Clocked or Triggered)

    RandomMode randomMode() const { return _randomMode; }
    void setRandomMode(RandomMode mode) {
        _randomMode = ModelUtils::clampedEnum(mode);
    }

    void editRandomMode(int value, bool shift) {
        setRandomMode(ModelUtils::adjustedEnum(randomMode(), value));
    }

    void printRandomMode(StringBuilder &str) const {
        str(randomModeName(randomMode()));
    }

    // mode (Free/Sync/Retrigger for LFO shapes)

    Mode mode() const { return _mode; }
    void setMode(Mode mode) {
        _mode = ModelUtils::clampedEnum(mode);
    }

    void editMode(int value, bool shift) {
        setMode(ModelUtils::adjustedEnum(mode(), value));
    }

    void printMode(StringBuilder &str) const {
        str(modeName(mode()));
    }

    // attack (0-2000ms, for ADSR)

    int attack() const { return _attack; }
    void setAttack(int attack) {
        _attack = clamp(attack, 0, 6000);
    }

    void editAttack(int value, bool shift) {
        setAttack(attack() + value * (shift ? 1 : 20));
    }

    void printAttack(StringBuilder &str) const {
        str("%dms", attack());
    }

    // decay (0-2000ms, for ADSR)

    int decay() const { return _decay; }
    void setDecay(int decay) {
        _decay = clamp(decay, 0, 6000);
    }

    void editDecay(int value, bool shift) {
        setDecay(decay() + value * (shift ? 1 : 20));
    }

    void printDecay(StringBuilder &str) const {
        str("%dms", decay());
    }

    // sustain (0-127, for ADSR)

    int sustain() const { return _sustain; }
    void setSustain(int sustain) {
        _sustain = clamp(sustain, 0, 127);
    }

    void editSustain(int value, bool shift) {
        // Encoder acceleration: faster when turning quickly
        int absValue = (value < 0) ? -value : value;
        int multiplier = shift ? 4 : 1;
        // Acceleration: 1x for slow, 2x for medium, 4x for fast turns
        if (absValue >= 4) {
            multiplier *= 4;
        } else if (absValue >= 2) {
            multiplier *= 2;
        }
        setSustain(sustain() + value * multiplier);
    }

    void printSustain(StringBuilder &str) const {
        str("%d", sustain());
    }

    // release (0-2000ms, for ADSR)

    int release() const { return _release; }
    void setRelease(int release) {
        _release = clamp(release, 0, 6000);
    }

    void editRelease(int value, bool shift) {
        setRelease(release() + value * (shift ? 1 : 20));
    }

    void printRelease(StringBuilder &str) const {
        str("%dms", release());
    }

    // amplitude (0-127, for ADSR)

    int amplitude() const { return _amplitude; }
    void setAmplitude(int amplitude) {
        _amplitude = clamp(amplitude, 0, 127);
    }

    void editAmplitude(int value, bool shift) {
        // Pressed (shift=true): Fine control (1x)
        // Not pressed (shift=false): Coarse control (4x)
        int multiplier = shift ? 1 : 4;
        setAmplitude(amplitude() + value * multiplier);
    }

    void printAmplitude(StringBuilder &str) const {
        str("%d", amplitude());
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
        setRandomMode(RandomMode::Clocked);
        setMode(Mode::Free);
        setAttack(100);   // 100ms default
        setDecay(100);    // 100ms default
        setSustain(100);  // ~78% level default
        setRelease(200);  // 200ms default
        setAmplitude(127);  // Full amplitude default
    }

    void write(VersionedSerializedWriter &writer) const {
        writer.write(_shape);
        writer.write(_rate);
        writer.write(_depth);
        writer.write(_offset);
        writer.write(_phase);
        writer.write(_smooth);
        writer.write(_gateTrack);
        writer.write(_randomMode);
        writer.write(_mode);
        writer.write(_attack);
        writer.write(_decay);
        writer.write(_sustain);
        writer.write(_release);
        writer.write(_amplitude);
    }

    void read(VersionedSerializedReader &reader) {
        reader.read(_shape);
        reader.read(_rate);
        reader.read(_depth);
        reader.read(_offset);
        reader.read(_phase);
        reader.read(_smooth);
        reader.read(_gateTrack);
        reader.read(_randomMode);
        reader.read(_mode);
        reader.read(_attack);
        reader.read(_decay);
        reader.read(_sustain);
        reader.read(_release);
        reader.read(_amplitude, 127);  // Default to full amplitude for old projects
    }

private:
    Shape _shape = Shape::Sine;
    uint16_t _rate = 96;          // Clock divisor (1/4 note default)
    uint8_t _depth = 127;
    int8_t _offset = 0;
    uint16_t _phase = 0;
    uint16_t _smooth = 100;       // Smooth time in ms (0-2000)
    uint8_t _gateTrack = 0;       // Which track's gate triggers triggered modes
    RandomMode _randomMode = RandomMode::Clocked;
    Mode _mode = Mode::Free;      // LFO mode (Free/Sync/Retrigger)
    uint16_t _attack = 100;       // ADSR attack time (0-2000ms)
    uint16_t _decay = 100;        // ADSR decay time (0-2000ms)
    uint8_t _sustain = 100;       // ADSR sustain level (0-127)
    uint16_t _release = 200;      // ADSR release time (0-2000ms)
    uint8_t _amplitude = 127;     // ADSR amplitude (0-127)
};
