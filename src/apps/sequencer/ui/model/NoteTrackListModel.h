#pragma once

#include "Config.h"

#include "RoutableListModel.h"

#include "model/NoteTrack.h"

class NoteTrackListModel : public RoutableListModel {
public:
    void setTrack(NoteTrack &track) {
        _track = &track;
    }

    virtual int rows() const override {
        return Last;
    }

    virtual int columns() const override {
        return 2;
    }

    virtual void cell(int row, int column, StringBuilder &str) const override {
        if (column == 0) {
            formatName(Item(row), str);
        } else if (column == 1) {
            formatValue(Item(row), str);
        }
    }

    virtual void edit(int row, int column, int value, bool shift) override {
        if (column == 1) {
            editValue(Item(row), value, shift);
        }
    }

    virtual Routing::Target routingTarget(int row) const override {
        switch (Item(row)) {
        case SlideTime:
            return Routing::Target::SlideTime;
        case Octave:
            return Routing::Target::Octave;
        case Transpose:
            return Routing::Target::Transpose;
        case Rotate:
            return Routing::Target::Rotate;
        case GateProbabilityBias:
            return Routing::Target::GateProbabilityBias;
        case RetriggerProbabilityBias:
            return Routing::Target::RetriggerProbabilityBias;
        case LengthBias:
            return Routing::Target::LengthBias;
        case NoteProbabilityBias:
            return Routing::Target::NoteProbabilityBias;
        default:
            return Routing::Target::None;
        }
    }

private:
    enum Item {
        PlayMode,
        FillMode,
        FillMuted,
        CvUpdateMode,
        SlideTime,
        Octave,
        Transpose,
        Rotate,
        GateProbabilityBias,
        RetriggerProbabilityBias,
        LengthBias,
        NoteProbabilityBias,
        Polyphony,
        CaptureTiming,
        TimingQuantize,
        Last
    };

    static const char *itemName(Item item) {
        switch (item) {
        case PlayMode:  return "Play Mode";
        case FillMode:  return "Fill Mode";
        case FillMuted: return "Fill Muted";
        case CvUpdateMode:  return "CV Update Mode";
        case SlideTime: return "Slide Time";
        case Octave:    return "Octave";
        case Transpose: return "Transpose";
        case Rotate:    return "Rotate";
        case GateProbabilityBias: return "Gate P. Bias";
        case RetriggerProbabilityBias: return "Retrig P. Bias";
        case LengthBias: return "Length Bias";
        case NoteProbabilityBias: return "Note P. Bias";
        case Polyphony: return "Polyphony";
        case CaptureTiming: return "Capture Timing";
        case TimingQuantize: return "Timing Quantize";
        case Last:      break;
        }
        return nullptr;
    }

    void formatName(Item item, StringBuilder &str) const {
        str(itemName(item));
    }

    void formatValue(Item item, StringBuilder &str) const {
        switch (item) {
        case PlayMode:
            _track->printPlayMode(str);
            break;
        case FillMode:
            _track->printFillMode(str);
            break;
        case FillMuted:
            _track->printFillMuted(str);
            break;
        case CvUpdateMode:
            _track->printCvUpdateMode(str);
            break;
        case SlideTime:
            _track->printSlideTime(str);
            break;
        case Octave:
            _track->printOctave(str);
            break;
        case Transpose:
            _track->printTranspose(str);
            break;
        case Rotate:
            _track->printRotate(str);
            break;
        case GateProbabilityBias:
            _track->printGateProbabilityBias(str);
            break;
        case RetriggerProbabilityBias:
            _track->printRetriggerProbabilityBias(str);
            break;
        case LengthBias:
            _track->printLengthBias(str);
            break;
        case NoteProbabilityBias:
            _track->printNoteProbabilityBias(str);
            break;
        case Polyphony:
            _track->printPolyphony(str);
            break;
        case CaptureTiming:
            _track->printCaptureTiming(str);
            break;
        case TimingQuantize:
            _track->printTimingQuantize(str);
            break;
        case Last:
            break;
        }
    }

    void editValue(Item item, int value, bool shift) {
        switch (item) {
        case PlayMode:
            _track->editPlayMode(value, shift);
            break;
        case FillMode:
            _track->editFillMode(value, shift);
            break;
        case FillMuted:
            _track->editFillMuted(value, shift);
            break;
        case CvUpdateMode:
            _track->editCvUpdateMode(value, shift);
            break;
        case SlideTime:
            _track->editSlideTime(value, shift);
            break;
        case Octave:
            _track->editOctave(value, shift);
            break;
        case Transpose:
            _track->editTranspose(value, shift);
            break;
        case Rotate:
            _track->editRotate(value, shift);
            break;
        case GateProbabilityBias:
            _track->editGateProbabilityBias(value, shift);
            break;
        case RetriggerProbabilityBias:
            _track->editRetriggerProbabilityBias(value, shift);
            break;
        case LengthBias:
            _track->editLengthBias(value, shift);
            break;
        case NoteProbabilityBias:
            _track->editNoteProbabilityBias(value, shift);
            break;
        case Polyphony:
            _track->editPolyphony(value, shift);
            break;
        case CaptureTiming:
            _track->editCaptureTiming(value, shift);
            break;
        case TimingQuantize:
            _track->editTimingQuantize(value, shift);
            break;
        case Last:
            break;
        }
    }

    NoteTrack *_track;
};
