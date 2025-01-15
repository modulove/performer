#pragma once

#include "Config.h"

#include "RhythmString.h"

namespace Rhythm {

    using Pattern = RhythmString<CONFIG_STEP_COUNT>;

    Pattern euclidean(int beats, int steps);

} // namespace Rhythm
