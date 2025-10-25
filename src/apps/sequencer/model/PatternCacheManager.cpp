#include "PatternCacheManager.h"
#include "os/os.h"

// Static member initialization
std::array<PatternCacheManager::CacheEntry, PatternCacheManager::MAX_CACHED_PATTERNS> PatternCacheManager::_cache;
PatternCacheManager::Stats PatternCacheManager::_stats = {0, 0, 0, 0, 0};
uint32_t PatternCacheManager::_timestamp = 0;

void PatternCacheManager::initialize() {
    // Initialize Flash storage
    FlashPatternStorage::initialize();

    // Clear cache
    for (auto& entry : _cache) {
        entry.trackIndex = -1;
        entry.patternIndex = -1;
        entry.lastAccess = 0;
        entry.dirty = false;
    }

    // Reset stats
    _stats = {0, 0, 0, 0, 0};
    _timestamp = 0;
}

uint32_t PatternCacheManager::getTimestamp() {
    return _timestamp++;
}

void PatternCacheManager::touch(CacheEntry* entry) {
    if (entry) {
        entry->lastAccess = getTimestamp();
    }
}

PatternCacheManager::CacheEntry* PatternCacheManager::findEntry(int trackIndex, int patternIndex) {
    for (auto& entry : _cache) {
        if (entry.trackIndex == trackIndex && entry.patternIndex == patternIndex) {
            return &entry;
        }
    }
    return nullptr;
}

PatternCacheManager::CacheEntry* PatternCacheManager::findLRU() {
    CacheEntry* lru = nullptr;
    uint32_t oldestTime = 0xFFFFFFFF;

    for (auto& entry : _cache) {
        // Find unused entry first
        if (entry.trackIndex == -1) {
            return &entry;
        }

        // Otherwise find oldest
        if (entry.lastAccess < oldestTime) {
            oldestTime = entry.lastAccess;
            lru = &entry;
        }
    }

    return lru;
}

bool PatternCacheManager::loadFromFlash(CacheEntry* entry, int trackIndex, int patternIndex) {
    if (!entry) return false;

    // Check if pattern exists in Flash
    int flashIndex = trackIndex * CONFIG_PATTERN_COUNT + patternIndex;
    if (!FlashPatternStorage::patternExists(flashIndex)) {
        // Pattern doesn't exist in Flash yet - initialize empty
        for (int i = 0; i < CONFIG_TRACK_COUNT; i++) {
            entry->sequences[i].clear();
        }
        entry->trackIndex = trackIndex;
        entry->patternIndex = patternIndex;
        entry->dirty = false;  // Empty pattern, no need to save
        touch(entry);
        return true;
    }

    // Load from Flash
    bool success = FlashPatternStorage::loadPattern(flashIndex, entry->sequences);
    if (success) {
        entry->trackIndex = trackIndex;
        entry->patternIndex = patternIndex;
        entry->dirty = false;
        touch(entry);
    }

    return success;
}

bool PatternCacheManager::saveToFlash(const CacheEntry* entry) {
    if (!entry || entry->trackIndex < 0) return false;

    // Don't save if not dirty
    if (!entry->dirty) return true;

    int flashIndex = entry->trackIndex * CONFIG_PATTERN_COUNT + entry->patternIndex;
    return FlashPatternStorage::savePattern(flashIndex, entry->sequences);
}

PatternCacheManager::CacheEntry* PatternCacheManager::allocateEntry(int trackIndex, int patternIndex) {
    // Check if already cached
    CacheEntry* existing = findEntry(trackIndex, patternIndex);
    if (existing) {
        touch(existing);
        return existing;
    }

    // Find LRU entry to evict
    CacheEntry* entry = findLRU();
    if (!entry) return nullptr;

    // Evict existing entry if needed
    if (entry->trackIndex >= 0) {
        if (entry->dirty) {
            saveToFlash(entry);
        }
        _stats.evictions++;
    }

    // Load new pattern
    bool success = loadFromFlash(entry, trackIndex, patternIndex);
    if (!success) {
        // Load failed, mark entry as unused
        entry->trackIndex = -1;
        return nullptr;
    }

    return entry;
}

NoteSequence* PatternCacheManager::getPattern(int trackIndex, int patternIndex) {
    // Validate inputs
    if (trackIndex < 0 || trackIndex >= CONFIG_TRACK_COUNT) return nullptr;
    if (patternIndex < 0 || patternIndex >= CONFIG_PATTERN_COUNT) return nullptr;

    // Check cache
    CacheEntry* entry = findEntry(trackIndex, patternIndex);
    if (entry) {
        // Cache hit
        _stats.hits++;
        touch(entry);
        return entry->sequences;
    }

    // Cache miss - allocate and load
    _stats.misses++;
    entry = allocateEntry(trackIndex, patternIndex);
    if (!entry) return nullptr;

    return entry->sequences;
}

void PatternCacheManager::markDirty(int trackIndex, int patternIndex) {
    CacheEntry* entry = findEntry(trackIndex, patternIndex);
    if (entry && !entry->dirty) {
        entry->dirty = true;
        _stats.dirtyCount++;
    }
}

void PatternCacheManager::preload(int trackIndex, int patternIndex) {
    // Validate inputs
    if (trackIndex < 0 || trackIndex >= CONFIG_TRACK_COUNT) return;
    if (patternIndex < 0 || patternIndex >= CONFIG_PATTERN_COUNT) return;

    // Check if already cached
    if (findEntry(trackIndex, patternIndex)) {
        return;  // Already loaded
    }

    // Load in background (non-blocking)
    allocateEntry(trackIndex, patternIndex);
}

void PatternCacheManager::flush(bool blocking) {
    for (auto& entry : _cache) {
        if (entry.trackIndex >= 0 && entry.dirty) {
            if (saveToFlash(&entry)) {
                entry.dirty = false;
                _stats.dirtyCount--;
            }

            if (blocking) {
                // Wait for Flash operation to complete
                os::time::ms(10);  // Small delay for Flash write
            }
        }
    }
}

void PatternCacheManager::evict(int trackIndex, int patternIndex) {
    CacheEntry* entry = findEntry(trackIndex, patternIndex);
    if (!entry) return;

    // Save if dirty
    if (entry->dirty) {
        saveToFlash(entry);
        _stats.dirtyCount--;
    }

    // Mark as unused
    entry->trackIndex = -1;
    entry->patternIndex = -1;
    _stats.evictions++;
}

PatternCacheManager::Stats PatternCacheManager::getStats() {
    // Update cached patterns count
    _stats.cachedPatterns = 0;
    for (const auto& entry : _cache) {
        if (entry.trackIndex >= 0) {
            _stats.cachedPatterns++;
        }
    }

    return _stats;
}
