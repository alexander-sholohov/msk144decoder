//
// Author: Alexander Sholohov <ra9yer@yahoo.com>
//
// License: MIT
//

#include "wavfile.h"

#include <string.h>
#include <fstream>

//---------------------------------------------------------------------------------------------
WavFile::WavHeader::WavHeader(std::vector<short> const& stream) {
    uint32_t streamSizeInBytes = stream.size() * sizeof(short);

    strncpy(chunkID, "RIFF", 4);
    chunkSize = 0x24 + streamSizeInBytes;
    strncpy(format, "WAVE", 4);
    strncpy(subchunk1ID, "fmt ", 4);

    strncpy(dataId, "data", 4);
    dataSize = streamSizeInBytes;
};

//---------------------------------------------------------------------------------------------
void WavFile::writeToFile(std::string const& filename, std::vector<short> const& stream)
{
    std::ofstream myfile;
    myfile.open(filename, std::ios::out | std::ios::binary);

    WavHeader hdr(stream);
    myfile.write((char*)&hdr, sizeof(hdr));
    myfile.write((char*)&stream[0], stream.size() * sizeof(short));
    myfile.close();
}
