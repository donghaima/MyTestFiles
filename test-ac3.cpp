#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <byteswap.h>

typedef struct
{
  // bitstream params
  int fscod;
  int frmsizecod;
  int bsid;
  int bsmod;
  int acmod;
  int lfeon;

  // computed from above
  int bit_rate;
  int sample_rate;
  int num_channels;
  int format;
} tInletAC3Params;


int main (void)
{
  

  // AC3(format=7) stream params found, fscod = 0x0, frmsizecod = 0x1c, acmod = 0x7, lfeon = 0x1, srate = 48000, xnum_chans = 6,  brate = 384000, j=46,  i=4

  tInletAC3Params ac3Params, *pAC3Params;
  ac3Params.fscod = 0;
  ac3Params.bsid = 16;
  ac3Params.bsmod = 0;
  ac3Params.acmod = 7;
  ac3Params.lfeon = 1;
  ac3Params.sample_rate = 48000;
  ac3Params.num_channels = 6;
  ac3Params.bit_rate = 384000;
  ac3Params.frmsizecod = 0x1c;

  //ac3Params.bit_rate = 401600;

  pAC3Params = &ac3Params;

  uint8_t ac3SpecificBox[3];
  int ac3SpecificBoxLength, *pAc3SpecificBoxLength;
  pAc3SpecificBoxLength = &ac3SpecificBoxLength;


  // Construct the 24-bit AC3SpecificBox
  uint8_t ac3Box[3];
  int size = 0;

  // Bits in ac3SpecificBox[]: 
  //      fscod                :2,
  //      bsid                  :5,
  //      bsmod             :3,   (set to 0 as this is optional for an independent substream)
  //      acmod             :3,
  //      lfeon                :1,
  //      bit_rate_code  :5,
  //      reserved          :5
  //
  uint8_t reserved = 0;
  uint32_t u32 = (ac3Params.fscod << 22) |  // 24 - 2
    (ac3Params.bsid << 17) |                // 22 - 5
    (ac3Params.bsmod << 14) |    // 17 - 3
    (ac3Params.acmod << 11) |    // 14 - 3
    (ac3Params.lfeon << 10) |    // 11 - 0
    ((ac3Params.frmsizecod  >> 1) << 5) |  // 10 - 5
    (reserved & 0x1F);
  ac3Box[0] = 0;    // unused
  ac3Box[1] = (u32 & 0xFF0000) >> 16;
  ac3Box[2] = (u32 & 0xFF00) >> 8;
  ac3Box[3] = u32 & 0xFF;
  size += 3;
  
  // Return the AC3SpecificBox is requested
  if (ac3SpecificBox && pAc3SpecificBoxLength)
  {
    memcpy(ac3SpecificBox, &ac3Box[1], size);
    *pAc3SpecificBoxLength = size;
  }
  
  printf("AC3SpecificBox bytes: length=%d, %02x:%02x:%02x\n",
         *pAc3SpecificBoxLength,
         ac3SpecificBox[0], ac3SpecificBox[1], ac3SpecificBox[2]);


  // construct the 32-bit dwChannelMask, which is part of the CodecPrivateData
  //   - little endian byte ordering
  //   - only depends on acmod and lfeon for independent substreams
  uint32_t dwChannelMask = 0;

  // 5.1: 3/2 mode with LFE on
  uint8_t acmod = 0x7;
  uint8_t lfeon = 1;

  // 2.0: 2/0 mode with no LFE
  //uint8_t acmod = 0x2;
  //uint8_t lfeon = 0;

  dwChannelMask |= (lfeon << 3);
  switch (acmod) {
  case 1:  // '001' - 1/0 coding mode
    dwChannelMask |= 0x4;
    break;
  case 2:  // '010' - 2/0 coding mode
    dwChannelMask |= 0x3;
    break;
  case 3:  // '011' - 3/0 coding mode
    dwChannelMask |= 0x7;
    break;
  case 4:  // '100' - 2/1 coding mode
    dwChannelMask |= 0x106;
    break;
  case 5:  // '101' - 3/1 coding mode
    dwChannelMask |= 0x107;
    break;
  case 6:  // '110' - 2/2 coding mode
    dwChannelMask |= 0x36;
    break;
  case 7:  // '111' - 3/2 coding mode
    dwChannelMask |= 0x37;
    break;
  default:
    dwChannelMask = 0;
  }
  
  uint8_t chanMask_le[4];
  int pos=0;
  chanMask_le[pos++] = dwChannelMask & 0x000000ff;
  chanMask_le[pos++] = (dwChannelMask & 0x0000ff00) >> 8;
  chanMask_le[pos++] = (dwChannelMask & 0x00ff0000) >> 16;
  chanMask_le[pos++] = (dwChannelMask & 0xff000000) >> 24;

  printf("dwChannelMask: lfeon=%d acmod=%d: dwChannelMask=0x%04x (little-endian: 0x%04x\n",
         ac3Params.lfeon, ac3Params.acmod, dwChannelMask, __bswap_32(dwChannelMask));
  printf("dwChannelMask: pos=%d, []=0x%02x:%02x:%02x:%02x\n",
         pos, chanMask_le[0], chanMask_le[1], chanMask_le[2], chanMask_le[3]);

  return 0;
}
