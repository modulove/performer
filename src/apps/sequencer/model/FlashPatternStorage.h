#pragma once

#include "Config.h"
#include "NoteSequence.h"

#include <cstdint>
#include <cstring>

/**
 * FlashPatternStorage - SAFE Implementation
 *
 * Stores patterns in Flash memory to save RAM.
 * Only the active pattern is kept in RAM, all others stored in Flash.
 *
 * CRITICAL SAFETY:
 * - Firmware size: ~331 KB (ends at 0x08050E68)
 * - Flash storage starts at: 0x08060000 (384 KB)
 * - This provides 64 KB safety margin
 * - Available space: 640 KB (for 32 patterns at 20 KB each)
 *
 * STM32F405 Flash Sectors Used:
 * - Sector 6: 0x08060000 - 0x0807FFFF (128 KB) - First 6 patterns
 * - Sector 7: 0x08080000 - 0x0809FFFF (128 KB) - Next 6 patterns
 * - Sector 8-11: Additional patterns
 */
class FlashPatternStorage {
public:
    // SAFE Flash memory configuration - WELL after firmware
    static constexpr uint32_t FLASH_BASE_ADDRESS = 0x08060000;  // 384 KB - SAFE!
    static constexpr uint32_t FIRMWARE_MAX_SIZE   = 0x00060000;  // 384 KB max firmware
    static constexpr uint32_t PATTERN_SIZE_BYTES  = 20 * 1024;   // 20 KB per pattern
    static constexpr int MAX_FLASH_PATTERNS       = 32;          // 640 KB / 20 KB
    static constexpr uint32_t PATTERN_VERSION     = 1;

    // Pattern header for validation
    struct PatternHeader {
        uint32_t magic;          // 0xDEADBEEF - pattern validity marker
        uint16_t version;        // Format version
        uint8_t  patternIndex;   // Pattern number (0-31)
        uint8_t  reserved1;
        uint32_t checksum;       // CRC32 of pattern data
        uint32_t dataSize;       // Actual data size
        uint32_t reserved2[2];   // Future use
    } __attribute__((packed));

    static_assert(sizeof(PatternHeader) == 24, "Pattern header must be 24 bytes");

    // Initialize Flash storage (call once at boot)
    static void initialize();

    // Check if pattern exists in Flash
    static bool patternExists(int patternIndex);

    // Load pattern from Flash to RAM
    static bool loadPattern(int patternIndex, NoteSequence sequences[CONFIG_TRACK_COUNT]);

    // Save pattern from RAM to Flash
    static bool savePattern(int patternIndex, const NoteSequence sequences[CONFIG_TRACK_COUNT]);

    // Erase pattern from Flash
    static void erasePattern(int patternIndex);

private:
    // Get Flash address for pattern
    static uint32_t getPatternAddress(int patternIndex);

    // Calculate CRC32 checksum
    static uint32_t calculateChecksum(const void* data, size_t length);

    // Read pattern header
    static bool readHeader(int patternIndex, PatternHeader& header);

    // Write pattern header
    static bool writeHeader(int patternIndex, const PatternHeader& header);

    // Get Flash sector number for pattern
    static uint8_t getSectorForPattern(int patternIndex);

    // Safety check - verify address is in valid range
    static bool isAddressSafe(uint32_t address);
};
