#include <cstdlib>
#include <iterator>
#include <vector>

#include <bitset>
#include <climits>

#include <boost/dynamic_bitset.hpp>
#include <iostream>
#include <iomanip>

template<int numBytes>
std::bitset<numBytes * CHAR_BIT> bytesToBitset(uint8_t *data)
{
    std::bitset<numBytes * CHAR_BIT> b = *data;

    for(int i = 1; i < numBytes; ++i)
    {
        b <<= CHAR_BIT;  // Move to next bit in array
        b |= data[i];    // Set the lowest CHAR_BIT bits
    }

    return b;
}


int main(void)
{
  // Make the block size one byte
  typedef boost::dynamic_bitset<unsigned char> Bitset;

  Bitset bitset(40); // 40 bits

  // Assign random bits
  for (int i=0; i<40; ++i)
  {
    bitset[i] = std::rand() % 2;
  }

  // Copy bytes to buffer
  std::vector<unsigned char> bytes;
  boost::to_block_range(bitset, std::back_inserter(bytes));

  std::cout << "bytes[] size=" << bytes.size() << std::endl;
  for (int i=0; i<bytes.size(); i++) {
    std::cout << std::hex << std::setfill('0') << std::setw(2) <<
      int(bytes[i]) << " ";
  }
  std::cout << std::endl;

  uint8_t data[5] = {0x00, 0xff, 0x08, 0xab, 0x11};
  std::bitset<40> bs = bytesToBitset<sizeof data>(data);
  std::cout << "bitset:" << bs << std::endl;
  
  return 0;
}
