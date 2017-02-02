#include <iostream>
#include <stdint.h>

struct Header
{
    static const uint32_t Version = 0;
    static const uint32_t MagicNumber = 0xdeadbeaf;
    uint32_t flags;
    uint8_t TS[188];
};


int main (void)
{
    uint8_t ec3_substream = 0xc4;
    uint32_t m_dwCurFrameSize = 2;
    std::cout << "frame size=" << m_dwCurFrameSize << " , frameSize-5=" << std::hex << m_dwCurFrameSize-5 << std::endl;
    std::cout << ",DDP_PROGRAM0=0x" <<int(ec3_substream) << std::endl; 

    int nextBitLoc = 25;
    int byteIndex = 2 + nextBitLoc / 8;
    int bitPos = nextBitLoc % 8;
    std::cout << "nextBitLoc=" << std::dec << nextBitLoc <<
      ": byteIndex=" << byteIndex << ", bitPos=" << bitPos << std::endl;

    std::cout << "struct Header: " << Header::Version << ", " << std::hex << Header::MagicNumber << std::endl;
    

    return 0;
}
