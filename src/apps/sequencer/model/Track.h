#pragma once

#include "Config.h"
#include "Types.h"
#include "Serialize.h"
#include "ModelUtils.h"
#include "NoteTrack.h"
#if CONFIG_ENABLE_CURVE_TRACKS
#include "CurveTrack.h"
#endif
#if CONFIG_ENABLE_MIDICV_TRACKS
#include "MidiCvTrack.h"
#endif

#include "core/Debug.h"
#include "core/math/Math.h"
#include "core/utils/StringUtils.h"
#include "core/utils/Container.h"

#include <cstdint>
#include <cstring>

#if CONFIG_ENABLE_SANITIZE
# define SANITIZE_TRACK_MODE(_actual_, _expected_) ASSERT(_actual_ == _expected_, "invalid track mode");
#else // CONFIG_ENABLE_SANITIZE
# define SANITIZE_TRACK_MODE(_actual_, _expected_) {}
#endif // CONFIG_ENABLE_SANITIZE

class Project;

class Track {
public:
    //----------------------------------------
    // Types
    //----------------------------------------

    enum class TrackMode : uint8_t {
        Note,
#if CONFIG_ENABLE_CURVE_TRACKS
        Curve,
#endif
#if CONFIG_ENABLE_MIDICV_TRACKS
        MidiCv,
#endif
        Last,
        Default = Note
    };

    static const char *trackModeName(TrackMode trackMode) {
        switch (trackMode) {
        case TrackMode::Note:   return "Note";
#if CONFIG_ENABLE_CURVE_TRACKS
        case TrackMode::Curve:  return "Curve";
#endif
#if CONFIG_ENABLE_MIDICV_TRACKS
        case TrackMode::MidiCv: return "MIDI/CV";
#endif
        case TrackMode::Last:   break;
        }
        return nullptr;
    }

    static uint8_t trackModeSerialize(TrackMode trackMode) {
        switch (trackMode) {
        case TrackMode::Note:   return 0;
#if CONFIG_ENABLE_CURVE_TRACKS
        case TrackMode::Curve:  return 1;
#endif
#if CONFIG_ENABLE_MIDICV_TRACKS
        case TrackMode::MidiCv: return 2;
#endif
        case TrackMode::Last:   break;
        }
        return 0;
    }


    //----------------------------------------
    // Properties
    //----------------------------------------

    // trackIndex

    int trackIndex() const { return _trackIndex; }

    // trackMode

    TrackMode trackMode() const { return _trackMode; }

    void printTrackMode(StringBuilder &str) const {
        str(trackModeName(trackMode()));
    }

    // linkTrack

    int linkTrack() const { return _linkTrack; }
    void setLinkTrack(int linkTrack) {
        _linkTrack = clamp(linkTrack, -1, _trackIndex - 1);
    }

    void editLinkTrack(int value, bool shift) {
        setLinkTrack(linkTrack() + value);
    }

    void printLinkTrack(StringBuilder &str) const {
        if (linkTrack() == -1) {
            str("None");
        } else {
            str("Track%d", linkTrack() + 1);
        }
    }

    // noteTrack

    const NoteTrack &noteTrack() const { SANITIZE_TRACK_MODE(_trackMode, TrackMode::Note); return *_track.note; }
          NoteTrack &noteTrack()       { SANITIZE_TRACK_MODE(_trackMode, TrackMode::Note); return *_track.note; }

    // curveTrack
#if CONFIG_ENABLE_CURVE_TRACKS
    const CurveTrack &curveTrack() const { SANITIZE_TRACK_MODE(_trackMode, TrackMode::Curve); return *_track.curve; }
          CurveTrack &curveTrack()       { SANITIZE_TRACK_MODE(_trackMode, TrackMode::Curve); return *_track.curve; }
#endif

    // midiCvTrack
#if CONFIG_ENABLE_MIDICV_TRACKS
    const MidiCvTrack &midiCvTrack() const { SANITIZE_TRACK_MODE(_trackMode, TrackMode::MidiCv); return *_track.midiCv; }
          MidiCvTrack &midiCvTrack()       { SANITIZE_TRACK_MODE(_trackMode, TrackMode::MidiCv); return *_track.midiCv; }
#endif

    //----------------------------------------
    // Methods
    //----------------------------------------

    Track() {
        initContainer();
    }

    void clear();
    void clearPattern(int patternIndex);
    void copyPattern(int src, int dst);
    bool duplicatePattern(int patternIndex);

    void gateOutputName(int index, StringBuilder &str) const;
    void cvOutputName(int index, StringBuilder &str) const;

    void write(VersionedSerializedWriter &writer) const;
    void read(VersionedSerializedReader &reader);

    Track &operator=(const Track &other) {
        ASSERT(_trackMode == other._trackMode, "invalid track mode");
        _linkTrack = other._linkTrack;
        _container = other._container;
        setContainerTrackIndex(_trackIndex);
        return *this;
    }

private:
    void setTrackIndex(int trackIndex);
    void setContainerTrackIndex(int trackIndex);

    // Note: always call through Project::setTrackMode
    void setTrackMode(TrackMode trackMode) {
        trackMode = ModelUtils::clampedEnum(trackMode);
        if (trackMode != _trackMode) {
            _trackMode = trackMode;
            initContainer();
        }
    }

    void initContainer();

    uint8_t _trackIndex = -1;
    TrackMode _trackMode;
    int8_t _linkTrack;

    Container<NoteTrack
#if CONFIG_ENABLE_CURVE_TRACKS
              , CurveTrack
#endif
#if CONFIG_ENABLE_MIDICV_TRACKS
              , MidiCvTrack
#endif
              > _container;
    union {
        NoteTrack *note;
#if CONFIG_ENABLE_CURVE_TRACKS
        CurveTrack *curve;
#endif
#if CONFIG_ENABLE_MIDICV_TRACKS
        MidiCvTrack *midiCv;
#endif
    } _track;

    friend class Project;
    friend class ClipBoard;
};

#undef SANITIZE_TRACK_MODE
