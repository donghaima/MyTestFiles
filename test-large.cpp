#include <iostream>
#include <stdint.h>
#include <iostream>

uint8_t m_Header[20];

void SetPts(uint64_t pts)
{
    m_Header[11] = (m_Header[11] & ~0x01) | ((pts & 0x0000000100000000)>>32);
    m_Header[12] = (pts & 0x00000000FF000000) >> 24;
    m_Header[13] = (pts & 0x0000000000FF0000) >> 16;
    m_Header[14] = (pts & 0x000000000000FF00) >> 8;
    m_Header[15] = (pts & 0x00000000000000FF);
}


int main(int argc, char* argv[])
{
    uint64_t pts = 12345656L;

    SetPts(pts);


    return 0;
}
