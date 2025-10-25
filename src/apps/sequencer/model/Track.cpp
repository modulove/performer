#include "Track.h"
#include "Project.h"

void Track::clear() {
    _trackMode = TrackMode::Default;
    _linkTrack = -1;

    initContainer();
}

void Track::clearPattern(int patternIndex) {
    switch (_trackMode) {
    case TrackMode::Note:
        _track.note->sequence(patternIndex).clear();
        break;
#if CONFIG_ENABLE_CURVE_TRACKS
    case TrackMode::Curve:
        _track.curve->sequence(patternIndex).clear();
        break;
#endif
#if CONFIG_ENABLE_MIDICV_TRACKS
    case TrackMode::MidiCv:
        break;
#endif
    case TrackMode::Last:
        break;
    }
}

void Track::copyPattern(int src, int dst) {
    switch (_trackMode) {
    case TrackMode::Note:
        _track.note->sequence(dst) = _track.note->sequence(src);
        break;
#if CONFIG_ENABLE_CURVE_TRACKS
    case TrackMode::Curve:
        _track.curve->sequence(dst) = _track.curve->sequence(src);
        break;
#endif
#if CONFIG_ENABLE_MIDICV_TRACKS
    case TrackMode::MidiCv:
        break;
#endif
    case TrackMode::Last:
        break;
    }
}

bool Track::duplicatePattern(int patternIndex) {
    if (patternIndex < CONFIG_PATTERN_COUNT - 1) {
        copyPattern(patternIndex, patternIndex + 1);
        return true;
    }
    return false;
}

void Track::gateOutputName(int index, StringBuilder &str) const {
    switch (_trackMode) {
    case TrackMode::Note:
#if CONFIG_ENABLE_CURVE_TRACKS
    case TrackMode::Curve:
#endif
        str("Gate");
        break;
#if CONFIG_ENABLE_MIDICV_TRACKS
    case TrackMode::MidiCv:
        _track.midiCv->gateOutputName(index, str);
        break;
#endif
    case TrackMode::Last:
        break;
    }
}

void Track::cvOutputName(int index, StringBuilder &str) const {
    switch (_trackMode) {
    case TrackMode::Note:
#if CONFIG_ENABLE_CURVE_TRACKS
    case TrackMode::Curve:
#endif
        str("CV");
        break;
#if CONFIG_ENABLE_MIDICV_TRACKS
    case TrackMode::MidiCv:
        _track.midiCv->cvOutputName(index, str);
        break;
#endif
    case TrackMode::Last:
        break;
    }
}

void Track::write(VersionedSerializedWriter &writer) const {
    writer.writeEnum(_trackMode, trackModeSerialize);
    writer.write(_linkTrack);

    switch (_trackMode) {
    case TrackMode::Note:
        _track.note->write(writer);
        break;
#if CONFIG_ENABLE_CURVE_TRACKS
    case TrackMode::Curve:
        _track.curve->write(writer);
        break;
#endif
#if CONFIG_ENABLE_MIDICV_TRACKS
    case TrackMode::MidiCv:
        _track.midiCv->write(writer);
        break;
#endif
    case TrackMode::Last:
        break;
    }
}

void Track::read(VersionedSerializedReader &reader) {
    reader.readEnum(_trackMode, trackModeSerialize);
    reader.read(_linkTrack);

    initContainer();

    switch (_trackMode) {
    case TrackMode::Note:
        _track.note->read(reader);
        break;
#if CONFIG_ENABLE_CURVE_TRACKS
    case TrackMode::Curve:
        _track.curve->read(reader);
        break;
#endif
#if CONFIG_ENABLE_MIDICV_TRACKS
    case TrackMode::MidiCv:
        _track.midiCv->read(reader);
        break;
#endif
    case TrackMode::Last:
        break;
    }
}

void Track::initContainer() {
    _track.note = nullptr;
#if CONFIG_ENABLE_CURVE_TRACKS
    _track.curve = nullptr;
#endif
#if CONFIG_ENABLE_MIDICV_TRACKS
    _track.midiCv = nullptr;
#endif

    switch (_trackMode) {
    case TrackMode::Note:
        _track.note = _container.create<NoteTrack>();
        break;
#if CONFIG_ENABLE_CURVE_TRACKS
    case TrackMode::Curve:
        _track.curve = _container.create<CurveTrack>();
        break;
#endif
#if CONFIG_ENABLE_MIDICV_TRACKS
    case TrackMode::MidiCv:
        _track.midiCv = _container.create<MidiCvTrack>();
        break;
#endif
    case TrackMode::Last:
        break;
    }

    setContainerTrackIndex(_trackIndex);
}

void Track::setTrackIndex(int trackIndex) {
    _trackIndex = trackIndex;
    setContainerTrackIndex(_trackIndex);
}

void Track::setContainerTrackIndex(int trackIndex) {
    switch (_trackMode) {
    case TrackMode::Note:
        _track.note->setTrackIndex(trackIndex);
        break;
#if CONFIG_ENABLE_CURVE_TRACKS
    case TrackMode::Curve:
        _track.curve->setTrackIndex(trackIndex);
        break;
#endif
#if CONFIG_ENABLE_MIDICV_TRACKS
    case TrackMode::MidiCv:
        _track.midiCv->setTrackIndex(trackIndex);
        break;
#endif
    case TrackMode::Last:
        break;
    }
}
