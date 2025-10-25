#pragma once

#include "Config.h"
#include "NoteSequence.h"
#include "FlashPatternStorage.h"

#include <array>
#include <cstdint>

/**
 * PatternCacheManager - Manages RAM/Flash pattern storage
 *
 * Strategy:
 * - Keeps 2-3 patterns per track in RAM (current + neighbors)
 * - Stores unused patterns in Flash
 * - LRU eviction policy when RAM is needed
 * - Pre-loads next pattern for seamless switching
 *
 * Memory Savings:
 * - Without cache: 16 tracks × 9 patterns = 144 patterns in RAM (~75 KB)
 * - With cache: 16 tracks × 3 patterns = 48 patterns in RAM (~25 KB)
 * - Saves: ~50 KB RAM
 */
class PatternCacheManager {
public:
    // Maximum patterns to keep in RAM per track (current + next + prev)
    static constexpr int PATTERNS_PER_TRACK_IN_RAM = 3;

    // Total patterns that can be cached in RAM across all tracks
    static constexpr int MAX_CACHED_PATTERNS = CONFIG_TRACK_COUNT * PATTERNS_PER_TRACK_IN_RAM;

    struct CacheEntry {
        int8_t trackIndex;      // Which track (0-15, -1 = unused)
        int8_t patternIndex;    // Which pattern (0-8)
        uint32_t lastAccess;    // Timestamp for LRU
        bool dirty;             // Needs to be written to Flash
        NoteSequence sequences[CONFIG_TRACK_COUNT];  // Actual pattern data

        CacheEntry() : trackIndex(-1), patternIndex(-1), lastAccess(0), dirty(false) {}
    };

    /**
     * Initialize the cache manager
     * Called once at boot
     */
    static void initialize();

    /**
     * Get a pattern (loads from Flash if not cached)
     * @param trackIndex Track number (0-15)
     * @param patternIndex Pattern number (0-8)
     * @return Pointer to sequences or nullptr on error
     */
    static NoteSequence* getPattern(int trackIndex, int patternIndex);

    /**
     * Mark a pattern as modified (needs Flash write)
     * @param trackIndex Track number
     * @param patternIndex Pattern number
     */
    static void markDirty(int trackIndex, int patternIndex);

    /**
     * Pre-load a pattern into cache (async)
     * Used to load next pattern before switching
     * @param trackIndex Track number
     * @param patternIndex Pattern number
     */
    static void preload(int trackIndex, int patternIndex);

    /**
     * Flush dirty patterns to Flash
     * @param blocking If true, wait for write to complete
     */
    static void flush(bool blocking = false);

    /**
     * Evict pattern from cache (writes to Flash if dirty)
     * @param trackIndex Track number
     * @param patternIndex Pattern number
     */
    static void evict(int trackIndex, int patternIndex);

    /**
     * Get cache statistics
     */
    struct Stats {
        int cachedPatterns;     // Patterns currently in RAM
        int hits;               // Cache hits
        int misses;             // Cache misses (Flash loads)
        int evictions;          // Patterns evicted
        int dirtyCount;         // Patterns waiting for Flash write
    };
    static Stats getStats();

private:
    // Find cache entry for pattern
    static CacheEntry* findEntry(int trackIndex, int patternIndex);

    // Find or create cache entry (evicting LRU if needed)
    static CacheEntry* allocateEntry(int trackIndex, int patternIndex);

    // Find least recently used entry
    static CacheEntry* findLRU();

    // Load pattern from Flash into cache entry
    static bool loadFromFlash(CacheEntry* entry, int trackIndex, int patternIndex);

    // Save cache entry to Flash
    static bool saveToFlash(const CacheEntry* entry);

    // Update access timestamp
    static void touch(CacheEntry* entry);

    // Get current timestamp
    static uint32_t getTimestamp();

    // Cache storage
    static std::array<CacheEntry, MAX_CACHED_PATTERNS> _cache;

    // Statistics
    static Stats _stats;

    // Current timestamp counter
    static uint32_t _timestamp;
};
