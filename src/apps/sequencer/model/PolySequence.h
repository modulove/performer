#pragma once

#include "Config.h"
#include "Bitfield.h"
#include "Scale.h"
#include "Serialize.h"

#include <array>
#include <cstdint>

class PolySequence {
public:
    //----------------------------------------
    // Types
    //----------------------------------------

    // Simplified types for polyphonic track
    using Length = UnsignedValue<3>;  // 0-7 length values

    //----------------------------------------
    // Step
    //----------------------------------------

    class Step {
    public:
        //----------------------------------------
        // Properties
        //----------------------------------------

        // gate
        bool gate() const { return _data.gate ? true : false; }
        void setGate(bool gate) { _data.gate = gate; }
        void toggleGate() { setGate(!gate()); }

        // length (simplified, no variation)
        int length() const { return _data.length; }
        void setLength(int length) {
            _data.length = Length::clamp(length);
        }

        // notes (up to 4 simultaneous notes)
        int8_t note(int index) const {
            return (index >= 0 && index < 4) ? _notes[index] : -1;
        }
        void setNote(int index, int8_t note) {
            if (index >= 0 && index < 4) {
                _notes[index] = note;
            }
        }
        bool hasNote(int index) const {
            return note(index) >= 0;
        }
        int activeNoteCount() const {
            int count = 0;
            for (int i = 0; i < 4; ++i) {
                if (_notes[i] >= 0) ++count;
            }
            return count;
        }
        void clearNotes() {
            for (int i = 0; i < 4; ++i) {
                _notes[i] = -1;
            }
        }

        void clear() {
            _data.raw = 0;
            clearNotes();
        }

        void write(VersionedSerializedWriter &writer) const {
            writer.write(_data.raw);
            for (int i = 0; i < 4; ++i) {
                writer.write(_notes[i]);
            }
        }

        void read(VersionedSerializedReader &reader) {
            reader.read(_data.raw);
            for (int i = 0; i < 4; ++i) {
                reader.read(_notes[i]);
            }
        }

    private:
        union {
            uint32_t raw;
            BitField<uint32_t, 0, 1> gate;
            BitField<uint32_t, 1, Length::Bits> length;
            // 28 bits left for future use
        } _data;

        int8_t _notes[4] = { -1, -1, -1, -1 };  // MIDI note numbers, -1 = inactive
    };

    using StepArray = std::array<Step, CONFIG_STEP_COUNT>;

    //----------------------------------------
    // Properties
    //----------------------------------------

    // name
    const char *name() const { return _name; }
    void setName(const char *name) {
        StringUtils::copy(_name, name, sizeof(_name));
    }

    // scale
    int scale() const { return _scale; }
    void setScale(int scale) {
        _scale = clamp(scale, -1, Scale::Count - 1);
    }

    // rootNote
    int rootNote() const { return _rootNote; }
    void setRootNote(int rootNote) {
        _rootNote = clamp(rootNote, -1, 11);
    }

    // divisor
    int divisor() const { return _divisor; }
    void setDivisor(int divisor) {
        _divisor = clamp(divisor, 1, 192);
    }

    // resetMeasure
    int resetMeasure() const { return _resetMeasure; }
    void setResetMeasure(int resetMeasure) {
        _resetMeasure = clamp(resetMeasure, 0, 128);
    }

    // runMode
    Types::RunMode runMode() const { return _runMode; }
    void setRunMode(Types::RunMode runMode) {
        _runMode = ModelUtils::clampedEnum(runMode);
    }

    // firstStep
    int firstStep() const { return _firstStep; }
    void setFirstStep(int firstStep) {
        _firstStep = clamp(firstStep, 0, lastStep());
    }

    // lastStep
    int lastStep() const { return _lastStep; }
    void setLastStep(int lastStep) {
        _lastStep = clamp(lastStep, firstStep(), CONFIG_STEP_COUNT - 1);
    }

    // steps
    const StepArray &steps() const { return _steps; }
          StepArray &steps()       { return _steps; }

    const Step &step(int index) const { return _steps[index]; }
          Step &step(int index)       { return _steps[index]; }

    //----------------------------------------
    // Methods
    //----------------------------------------

    void clear() {
        setName("INIT");
        setScale(-1);
        setRootNote(-1);
        setDivisor(12);
        setResetMeasure(0);
        setRunMode(Types::RunMode::Free);
        setFirstStep(0);
        setLastStep(CONFIG_STEP_COUNT - 1);
        clearSteps();
    }

    void clearSteps() {
        for (auto &step : _steps) {
            step.clear();
        }
    }

    void write(VersionedSerializedWriter &writer) const {
        writer.write(_name, NameLength + 1);
        writer.write(_scale);
        writer.write(_rootNote);
        writer.write(_divisor);
        writer.write(_resetMeasure);
        writer.write(_runMode);
        writer.write(_firstStep);
        writer.write(_lastStep);
        writeArray(writer, _steps);
    }

    void read(VersionedSerializedReader &reader) {
        reader.read(_name, NameLength + 1);
        reader.read(_scale);
        reader.read(_rootNote);
        reader.read(_divisor);
        reader.read(_resetMeasure);
        reader.read(_runMode);
        reader.read(_firstStep);
        reader.read(_lastStep);
        readArray(reader, _steps);
    }

private:
    static constexpr size_t NameLength = 8;

    char _name[NameLength + 1];
    int8_t _scale = -1;
    int8_t _rootNote = -1;
    uint16_t _divisor = 12;
    uint8_t _resetMeasure = 0;
    Types::RunMode _runMode = Types::RunMode::Free;
    uint8_t _firstStep = 0;
    uint8_t _lastStep = CONFIG_STEP_COUNT - 1;

    StepArray _steps;
};
