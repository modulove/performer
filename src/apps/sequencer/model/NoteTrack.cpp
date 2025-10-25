#include "NoteTrack.h"
#include "ProjectVersion.h"

void NoteTrack::writeRouted(Routing::Target target, int intValue, float floatValue) {
    switch (target) {
    case Routing::Target::SlideTime:
        setSlideTime(intValue, true);
        break;
    case Routing::Target::Octave:
        setOctave(intValue, true);
        break;
    case Routing::Target::Transpose:
        setTranspose(intValue, true);
        break;
    case Routing::Target::Rotate:
        setRotate(intValue, true);
        break;
    case Routing::Target::GateProbabilityBias:
        setGateProbabilityBias(intValue, true);
        break;
    case Routing::Target::RetriggerProbabilityBias:
        setRetriggerProbabilityBias(intValue, true);
        break;
    case Routing::Target::LengthBias:
        setLengthBias(intValue, true);
        break;
    case Routing::Target::NoteProbabilityBias:
        setNoteProbabilityBias(intValue, true);
        break;
    default:
        break;
    }
}

void NoteTrack::clear() {
    setPlayMode(Types::PlayMode::Aligned);
    setFillMode(FillMode::Gates);
    setFillMuted(true);
    setCvUpdateMode(CvUpdateMode::Gate);
    setSlideTime(50);
    setOctave(0);
    setTranspose(0);
    setRotate(0);
    setGateProbabilityBias(0);
    setRetriggerProbabilityBias(0);
    setLengthBias(0);
    setNoteProbabilityBias(0);

    for (auto &sequence : _sequences) {
        sequence.clear();
    }
}

void NoteTrack::write(VersionedSerializedWriter &writer) const {
    writer.write(_playMode);
    writer.write(_fillMode);
    writer.write(_fillMuted);
    writer.write(_cvUpdateMode);
    writer.write(_slideTime.base);
    writer.write(_octave.base);
    writer.write(_transpose.base);
    writer.write(_rotate.base);
    writer.write(_gateProbabilityBias.base);
    writer.write(_retriggerProbabilityBias.base);
    writer.write(_lengthBias.base);
    writer.write(_noteProbabilityBias.base);
    writer.write(_polyphony);
    writer.write(_captureTiming);
    writer.write(_timingQuantize);
    writeArray(writer, _sequences);
}

void NoteTrack::read(VersionedSerializedReader &reader) {
    reader.backupHash();

    reader.read(_playMode);
    reader.read(_fillMode);
    reader.read(_fillMuted, ProjectVersion::Version26);
    reader.read(_cvUpdateMode, ProjectVersion::Version4);
    reader.read(_slideTime.base);
    reader.read(_octave.base);
    reader.read(_transpose.base);
    reader.read(_rotate.base);
    reader.read(_gateProbabilityBias.base);
    reader.read(_retriggerProbabilityBias.base);
    reader.read(_lengthBias.base);
    reader.read(_noteProbabilityBias.base);
    reader.read(_polyphony, ProjectVersion::Version27);  // Added in v27 for polyphonic MIDI
    reader.read(_captureTiming, ProjectVersion::Version27);  // Added in v27 for micro-timing capture
    reader.read(_timingQuantize, ProjectVersion::Version27);  // Added in v27 for timing quantization control

    // There is a bug in previous firmware versions where writing the properties
    // of a note track did not update the hash value.
    if (reader.dataVersion() < ProjectVersion::Version23) {
        reader.restoreHash();
    }

    readArray(reader, _sequences);
}
