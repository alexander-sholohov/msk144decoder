#pragma once
//
// Author: Alexander Sholohov <ra9yer@yahoo.com>
//
// License: MIT
//

#include <stdint.h>
#include <vector>
#include <string>

/*
* Simple class to write wav file.
* Fixed format:
* Mono, 16bits, 12000 samples per second
*/
class WavFile
{
    struct WavHeader
    {
        WavHeader(std::vector<short> const& stream);

        char chunkID[4]; // RIFF
        uint32_t chunkSize;
        char format[4]; // WAVE
        char subchunk1ID[4]; // 'fmt '
        uint32_t subchunk1Size = 0x10;
        uint16_t audioFormat = 0x01;
        uint16_t numChannels = 0x01;
        uint32_t sampleRate = 12000;
        uint32_t byteRate = 12000 * 2;
        uint16_t blockAlign = 0x2;
        uint16_t bitsPerSample = 16;
        // one chunk
        char dataId[4];
        uint32_t dataSize;

    } __attribute__((packed));

public:
    static void writeToFile(std::string const& filename, std::vector<short> const& stream);
};
