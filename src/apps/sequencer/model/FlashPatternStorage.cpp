#include "FlashPatternStorage.h"
#include "Serialize.h"
#include "ProjectVersion.h"

#include <libopencm3/stm32/flash.h>

// CRC32 implementation for checksum validation
static uint32_t crc32_table[256];
static bool crc32_table_initialized = false;

static void init_crc32_table() {
    if (crc32_table_initialized) return;

    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (uint32_t j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
        }
        crc32_table[i] = crc;
    }
    crc32_table_initialized = true;
}

uint32_t FlashPatternStorage::calculateChecksum(const void* data, size_t length) {
    init_crc32_table();

    uint32_t crc = 0xFFFFFFFF;
    const uint8_t* bytes = static_cast<const uint8_t*>(data);

    for (size_t i = 0; i < length; i++) {
        crc = (crc >> 8) ^ crc32_table[(crc ^ bytes[i]) & 0xFF];
    }

    return ~crc;
}

bool FlashPatternStorage::isAddressSafe(uint32_t address) {
    // CRITICAL SAFETY CHECK
    // Verify address is:
    // 1. At or after our base address (0x08060000)
    // 2. Before end of Flash (0x08100000)
    // 3. NOT in firmware area (0x08000000 - 0x08060000)

    if (address < FLASH_BASE_ADDRESS) {
        // DANGER: Would write to firmware area!
        return false;
    }

    if (address >= 0x08100000) {
        // Beyond Flash memory
        return false;
    }

    return true;
}

uint32_t FlashPatternStorage::getPatternAddress(int patternIndex) {
    if (patternIndex < 0 || patternIndex >= MAX_FLASH_PATTERNS) {
        return 0;
    }

    return FLASH_BASE_ADDRESS + (patternIndex * PATTERN_SIZE_BYTES);
}

uint8_t FlashPatternStorage::getSectorForPattern(int patternIndex) {
    uint32_t address = getPatternAddress(patternIndex);
    uint32_t offset = address - 0x08000000;

    // STM32F405 Flash sector layout:
    // Sectors 0-3: 16 KB each (0x00000 - 0x0FFFF)
    // Sector 4:    64 KB      (0x10000 - 0x1FFFF)
    // Sectors 5-11: 128 KB each (0x20000 - 0xFFFFF)

    if (offset < 0x20000) {
        // We should never be here (our base is 0x60000)
        return 0xFF;  // Invalid
    } else {
        // 128 KB sectors starting at 0x20000
        return 5 + ((offset - 0x20000) / 0x20000);
    }
}

void FlashPatternStorage::initialize() {
    // Initialize CRC table
    init_crc32_table();

    // Verify our base address is safe
    if (!isAddressSafe(FLASH_BASE_ADDRESS)) {
        // CRITICAL ERROR: Base address unsafe!
        // This should never happen with 0x08060000
        while(1);  // Halt - something is very wrong
    }
}

bool FlashPatternStorage::patternExists(int patternIndex) {
    if (patternIndex < 0 || patternIndex >= MAX_FLASH_PATTERNS) {
        return false;
    }

    PatternHeader header;
    if (!readHeader(patternIndex, header)) {
        return false;
    }

    // Verify magic number
    if (header.magic != 0xDEADBEEF) {
        return false;
    }

    // Verify pattern index matches
    if (header.patternIndex != patternIndex) {
        return false;
    }

    // Verify version
    if (header.version != PATTERN_VERSION) {
        return false;
    }

    return true;
}

bool FlashPatternStorage::readHeader(int patternIndex, PatternHeader& header) {
    uint32_t address = getPatternAddress(patternIndex);

    // Safety check
    if (!isAddressSafe(address)) {
        return false;
    }

    // Read header directly from Flash
    const PatternHeader* flashHeader = reinterpret_cast<const PatternHeader*>(address);
    std::memcpy(&header, flashHeader, sizeof(PatternHeader));

    return true;
}

bool FlashPatternStorage::writeHeader(int patternIndex, const PatternHeader& header) {
    uint32_t address = getPatternAddress(patternIndex);

    // Safety check
    if (!isAddressSafe(address)) {
        return false;
    }

    // Write header word by word
    flash_unlock();

    const uint32_t* data = reinterpret_cast<const uint32_t*>(&header);
    for (size_t i = 0; i < sizeof(PatternHeader) / 4; i++) {
        flash_program_word(address + i * 4, data[i]);
    }

    flash_wait_for_last_operation();
    flash_lock();

    return true;
}

void FlashPatternStorage::erasePattern(int patternIndex) {
    if (patternIndex < 0 || patternIndex >= MAX_FLASH_PATTERNS) {
        return;
    }

    uint32_t address = getPatternAddress(patternIndex);

    // Safety check
    if (!isAddressSafe(address)) {
        return;
    }

    uint8_t sector = getSectorForPattern(patternIndex);
    if (sector == 0xFF) {
        return;  // Invalid sector
    }

    // Erase the sector
    flash_unlock();
    flash_erase_sector(sector, FLASH_CR_PROGRAM_X32);
    flash_wait_for_last_operation();
    flash_lock();
}

bool FlashPatternStorage::loadPattern(int patternIndex, NoteSequence sequences[CONFIG_TRACK_COUNT]) {
    if (patternIndex < 0 || patternIndex >= MAX_FLASH_PATTERNS) {
        return false;
    }

    uint32_t address = getPatternAddress(patternIndex);

    // Safety check
    if (!isAddressSafe(address)) {
        return false;
    }

    // Read and verify header
    PatternHeader header;
    if (!readHeader(patternIndex, header)) {
        return false;
    }

    if (header.magic != 0xDEADBEEF) {
        return false;
    }

    // Read data from Flash
    uint32_t dataAddress = address + sizeof(PatternHeader);
    if (!isAddressSafe(dataAddress) || !isAddressSafe(dataAddress + header.dataSize - 1)) {
        return false;
    }

    const uint8_t* flashData = reinterpret_cast<const uint8_t*>(dataAddress);

    // Copy to temp buffer for verification
    static uint8_t tempBuffer[PATTERN_SIZE_BYTES];
    std::memcpy(tempBuffer, flashData, header.dataSize);

    // Verify checksum
    uint32_t calculatedChecksum = calculateChecksum(tempBuffer, header.dataSize);
    if (calculatedChecksum != header.checksum) {
        return false;  // Data corruption
    }

    // Deserialize using callback
    size_t readOffset = 0;
    uint8_t* bufferPtr = tempBuffer;
    VersionedSerializedReader reader(
        [bufferPtr, &readOffset] (void *data, size_t len) {
            std::memcpy(data, bufferPtr + readOffset, len);
            readOffset += len;
        },
        ProjectVersion::Latest
    );

    for (int track = 0; track < CONFIG_TRACK_COUNT; track++) {
        sequences[track].read(reader);
    }

    return true;
}

bool FlashPatternStorage::savePattern(int patternIndex, const NoteSequence sequences[CONFIG_TRACK_COUNT]) {
    if (patternIndex < 0 || patternIndex >= MAX_FLASH_PATTERNS) {
        return false;
    }

    uint32_t address = getPatternAddress(patternIndex);

    // Safety check
    if (!isAddressSafe(address)) {
        return false;
    }

    // Serialize to temp buffer
    static uint8_t tempBuffer[PATTERN_SIZE_BYTES];
    uint8_t* bufferPtr = tempBuffer;
    size_t writeOffset = 0;

    VersionedSerializedWriter writer(
        [bufferPtr, &writeOffset] (const void *data, size_t len) {
            std::memcpy(bufferPtr + writeOffset, data, len);
            writeOffset += len;
        },
        ProjectVersion::Latest
    );

    for (int track = 0; track < CONFIG_TRACK_COUNT; track++) {
        sequences[track].write(writer);
    }

    size_t dataSize = writeOffset;

    // Calculate checksum
    uint32_t checksum = calculateChecksum(tempBuffer, dataSize);

    // Create header
    PatternHeader header;
    header.magic = 0xDEADBEEF;
    header.version = PATTERN_VERSION;
    header.patternIndex = patternIndex;
    header.reserved1 = 0;
    header.checksum = checksum;
    header.dataSize = dataSize;
    header.reserved2[0] = 0;
    header.reserved2[1] = 0;

    // Erase sector first
    erasePattern(patternIndex);

    // Write header
    if (!writeHeader(patternIndex, header)) {
        return false;
    }

    // Write data
    uint32_t dataAddress = address + sizeof(PatternHeader);
    if (!isAddressSafe(dataAddress) || !isAddressSafe(dataAddress + dataSize - 1)) {
        return false;
    }

    flash_unlock();

    // Write data word by word
    uint32_t* wordData = reinterpret_cast<uint32_t*>(tempBuffer);
    size_t wordCount = (dataSize + 3) / 4;  // Round up to word boundary

    for (size_t i = 0; i < wordCount; i++) {
        flash_program_word(dataAddress + i * 4, wordData[i]);
    }

    flash_wait_for_last_operation();
    flash_lock();

    return true;
}
