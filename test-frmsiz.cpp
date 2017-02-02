#include <iostream>

int main (void)
{
    unsigned char pFrame[] = {0x0b, 0x77, 0x04, 0xff};

    int frmsizcode =  ((pFrame[2] & 0x07) << 8) | pFrame[3];
    int frameSize = (frmsizcode +1) * 2; // not including the syncword

    std::cout << "pFrame[]=" << std::hex << pFrame << std::endl;

    std::cout << "frmsizcode=0x" << std::hex << frmsizcode << ", frameSize=" <<
      std::dec << frameSize << std::endl;

    return 0;
}
