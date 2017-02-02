#include <stdio.h>
#include <stdint.h>
#include <byteswap.h>

typedef struct
{
  // bitstream params
  int strmtyp;  // stream type 2 bits
  int substreamid; // 3 bits
  int frmsiz; // 11 bits (3 + 8)

  int fscod; // 2 bits
  int fscod2; // 2 bits
  int numblkscod; // 2 bits
  int acmod; // 3 bits
  int lfeon; // 1 bit

  int bsid;  // 5 bits

  // computed from above
  int audioBlocksPerSyncFrame;
  int bit_rate;
  int sample_rate;
  int num_channels;
  int format;
} tInletEAC3Params;



int main (void)
{

  // EAC3 stream params found, strmtyp=0, substreamid=0, frmsize = 0x2ff, fscod = 0, fscod2 = 0, bsid = 16, numblkscod=3(6 blocks/frame), acmod = 7 lfeon = 1, srate 48000, num_chans 6, brate 384000,
  
  tInletEAC3Params eac3Params, *pEAC3Params;
  eac3Params.strmtyp = 0;
  eac3Params.substreamid = 0;
  eac3Params.frmsiz = 767;
  eac3Params.fscod = 0;
  eac3Params.fscod2 = 0;
  eac3Params.bsid = 16;
  eac3Params.numblkscod = 3;
  eac3Params.audioBlocksPerSyncFrame = 6;
  eac3Params.acmod = 7;
  eac3Params.lfeon = 1;
  eac3Params.sample_rate = 48000;
  eac3Params.num_channels = 6;
  eac3Params.bit_rate = 384000;

  // 
  eac3Params.bit_rate = 401600;
  eac3Params.frmsiz = 768;

  pEAC3Params = &eac3Params;

  int size = 0;
  uint8_t ec3SpecificBox[6];
  int ec3SpecificBoxLength, *pEc3SpecificBoxLength;
  pEc3SpecificBoxLength = &ec3SpecificBoxLength;

  // ec3SpecificBox[0] and [1] are to store: 
  //      data_rate         :13 and
  //     num_ind_sub   :3
  //uint16_t *u16_1 = (uint16_t *)&ec3SpecificBox[0];
  uint8_t num_ind_sub = 1;    // only support: 1 independent substream and 0 dep substream
  uint16_t u16_1 = ((pEAC3Params->bit_rate/1000) << 3) | (num_ind_sub-1);
  ec3SpecificBox[0] = u16_1 >> 8;
  ec3SpecificBox[1] = u16_1 & 0xff;
  size += 2;
              
  // ec3SpecificBox[2] and [3] are to store:
  //      fscod      :2,
  //      bsid        :5,
  //      bsmod   :5,   (set to 0 as this is optional for an independent substream)
  //      acmod   :3, and
  //      lfeon      :1
  //uint16_t *u16_2 = (uint16_t *)&ec3SpecificBox[2];
  uint16_t u16_2 = (pEAC3Params->fscod << 14) | 
    (pEAC3Params->bsid << 9) | 
    (pEAC3Params->acmod << 1) | 
    (pEAC3Params->lfeon);
  ec3SpecificBox[2] = u16_2 >> 8;
  ec3SpecificBox[3] = u16_2 & 0xff;
  size += 2;
  
  // ec3SpecificBox[4] is to store:
  //      reserved           :3,
  //      num_dep_sub  :4, and
  //      reserved           :1  (for num_dep_sub = 0)
  ec3SpecificBox[4] = 0;
  size += 1;
  
  *pEc3SpecificBoxLength = size;

  printf("EC3SpecificBox bytes: length=%d, %02x:%02x:%02x:%02x:%02x\n",
          *pEc3SpecificBoxLength,
          ec3SpecificBox[0], ec3SpecificBox[1],
          ec3SpecificBox[2], ec3SpecificBox[3],
          ec3SpecificBox[4]);



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
         eac3Params.lfeon, eac3Params.acmod, dwChannelMask, __bswap_32(dwChannelMask));
  printf("dwChannelMask: pos=%d, []=0x%02x:%02x:%02x:%02x\n",
         pos, chanMask_le[0], chanMask_le[1], chanMask_le[2], chanMask_le[3]);

  return 0;
}
