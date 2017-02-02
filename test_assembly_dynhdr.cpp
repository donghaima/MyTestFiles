#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
   #define LINUX_KERNEL_26    1
#else
   #define LINUX_KERNEL_26    0
#endif

typedef uint8_t AlignedHeader;


#define MISSMASK 0x7FFF0000
#define ACKMASK 0x80000000
#define ACKUPDATE 0x40000000

#define IPOVERHEAD 40

#define QUOTE(x) #x
#define STRINGIZE(x) QUOTE(x)

#define CACHELINESIZE 64

// client_validate_header() should return 2a0000:
uint8_t received[] = {
  0x00, 0x1b, 0x21, 0x58, 0xca, 0xbd, 0x18, 0xef, 0x63, 0xe2, 0x02, 0x35, 0x08, 0x00, 0x45, 0x00,
  0x00, 0x28, 0x00, 0x00, 0x40, 0x00, 0x7f, 0x06, 0xb7, 0xcc, 0x14, 0x00, 0x0f, 0x02, 0x14, 0x00,
  0x0d, 0x02, 0x00, 0x50, 0xc0, 0x00, 0xf9, 0xe0, 0xfa, 0xf2, 0xe9, 0x13, 0x80, 0x03, 0x50, 0x10,
  0xff, 0xff, 0x4d, 0x95, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x9e, 0xe3, 0x50,
};

uint8_t predicted[] = {
  0x00, 0x1b, 0x21, 0x58, 0xca, 0xbd, 0x18, 0xef, 0x63, 0xe2, 0x02, 0x35, 0x08, 0x00, 0x45, 0x01,
  0x05, 0xce, 0x00, 0x00, 0x40, 0x00, 0x01, 0x06, 0xfa, 0xd0, 0x14, 0x00, 0x0f, 0x02, 0x14, 0x00,
  0x0d, 0x02, 0x00, 0x50, 0xc0, 0x00, 0xf9, 0xe0, 0xfa, 0xf3, 0xe9, 0x13, 0x80, 0x03, 0x50, 0x10,
  0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x0a, 0x30, 0x35, 0x39, 0x45, 0x0d, 0x0a, 0x00, 0x00,
};


// HTTP::Adapter::client_receive - client_validate_header for predicted: miss 3701d9, miss&MISSMASK 370000
// Dump receive (size 64):
AlignedHeader received1[] = {
  0x00, 0x1b, 0x21, 0x58, 0xca, 0xbd, 0x18, 0xef, 0x63, 0xe2, 0x02, 0x35, 0x08, 0x00, 0x45, 0x00,
  0x02, 0x01, 0x00, 0x00, 0x40, 0x00, 0x7f, 0x06, 0xb5, 0xf3, 0x14, 0x00, 0x0f, 0x02, 0x14, 0x00,
  0x0d, 0x02, 0x00, 0x50, 0xc0, 0x00, 0xf5, 0xe3, 0xe2, 0xcd, 0xf8, 0xc5, 0x60, 0xcc, 0x50, 0x10,
  0xff, 0xff, 0x5c, 0x42, 0x00, 0x00, 0x48, 0x54, 0x54, 0x50, 0x2f, 0x31, 0x2e, 0x31, 0x20, 0x32,
};
 // Dump predicted (size 64):
AlignedHeader predicted1[] = {
  0x00, 0x1b, 0x21, 0x58, 0xca, 0xbd, 0x18, 0xef, 0x63, 0xe2, 0x02, 0x35, 0x08, 0x00, 0x45, 0x01,
  0x05, 0xce, 0x00, 0x00, 0x40, 0x00, 0x01, 0x06, 0xfa, 0xd0, 0x14, 0x00, 0x0f, 0x02, 0x14, 0x00,
  0x0d, 0x02, 0x00, 0x50, 0xc0, 0x00, 0xf5, 0xe3, 0xe2, 0xcd, 0xf8, 0xc5, 0x60, 0xcc, 0x50, 0x10,
  0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x0a, 0x30, 0x35, 0x39, 0x45, 0x0d, 0x0a, 0x00, 0x00,
};


// HTTP::Adapter::client_receive - client_validate_header for predicted: miss 2a0000, miss&MISSMASK 2a0000
// Dump receive (size 64):
AlignedHeader received2[] = {
  0x00, 0x1b, 0x21, 0x58, 0xca, 0xbd, 0x18, 0xef, 0x63, 0xe2, 0x02, 0x35, 0x08, 0x00, 0x45, 0x00,
  0x00, 0x28, 0x00, 0x00, 0x40, 0x00, 0x7f, 0x06, 0xb7, 0xcc, 0x14, 0x00, 0x0f, 0x02, 0x14, 0x00,
  0x0d, 0x02, 0x00, 0x50, 0xc0, 0x00, 0xf5, 0xe3, 0xe4, 0xa5, 0xf8, 0xc5, 0x60, 0xcc, 0x50, 0x10,
  0xff, 0xff, 0x77, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa8, 0x92, 0x1f, 0xfe,
};
// Dump predicted (size 64):
AlignedHeader predicted2[] = {
  0x00, 0x1b, 0x21, 0x58, 0xca, 0xbd, 0x18, 0xef, 0x63, 0xe2, 0x02, 0x35, 0x08, 0x00, 0x45, 0x01,
  0x05, 0xce, 0x00, 0x00, 0x40, 0x00, 0x01, 0x06, 0xfa, 0xd0, 0x14, 0x00, 0x0f, 0x02, 0x14, 0x00,
  0x0d, 0x02, 0x00, 0x50, 0xc0, 0x00, 0xf5, 0xe3, 0xe4, 0xa6, 0xf8, 0xc5, 0x60, 0xcc, 0x50, 0x10,
  0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x0a, 0x30, 0x35, 0x39, 0x45, 0x0d, 0x0a, 0x00, 0x00,
};


// HTTP::Adapter::client_receive - client_validate_header for predicted: miss 800005a6, miss&MISSMASK 0
// Dump receive (size 64):
AlignedHeader received3[] = {
  0x00, 0x1b, 0x21, 0x58, 0xca, 0xbd, 0x18, 0xef, 0x63, 0xe2, 0x02, 0x35, 0x08, 0x00, 0x45, 0x00,
  0x05, 0xce, 0x00, 0x00, 0x40, 0x00, 0x7f, 0x06, 0xb1, 0x26, 0x14, 0x00, 0x10, 0x02, 0x14, 0x00,
  0x0d, 0x02, 0x00, 0x50, 0xc0, 0x01, 0xfa, 0x60, 0x7e, 0xc2, 0xfd, 0x0c, 0x21, 0xab, 0x50, 0x10,
  0xff, 0xff, 0x99, 0x79, 0x00, 0x00, 0x0d, 0x0a, 0x30, 0x35, 0x39, 0x45, 0x0d, 0x0a, 0xc3, 0x68,
};

 // Dump predicted (size 64):
AlignedHeader predicted3[] = {
  0x00, 0x1b, 0x21, 0x58, 0xca, 0xbd, 0x18, 0xef, 0x63, 0xe2, 0x02, 0x35, 0x08, 0x00, 0x45, 0x01,
  0x05, 0xce, 0x00, 0x00, 0x40, 0x00, 0x22, 0x06, 0xfa, 0xd0, 0x14, 0x00, 0x10, 0x02, 0x14, 0x00,
  0x0d, 0x02, 0x00, 0x50, 0xc0, 0x01, 0xfa, 0x60, 0x7e, 0xc2, 0xfd, 0x0c, 0x21, 0xab, 0x50, 0x10,
  0xff, 0xff, 0xf6, 0xd2, 0x35, 0x00, 0x0d, 0x0a, 0x30, 0x35, 0x39, 0x45, 0x0d, 0x0a, 0x00, 0x00,
};



// HTTP::Adapter::client_receive - client_validate_header for predicted: miss 5a6, miss&MISSMASK 0
// Dump receive (size 64):
AlignedHeader received4[] = {
  0x00, 0x1b, 0x21, 0x58, 0xca, 0xbd, 0x18, 0xef, 0x63, 0xe2, 0x02, 0x35, 0x08, 0x00, 0x45, 0x01,
  0x05, 0xce, 0x00, 0x00, 0x40, 0x00, 0x7f, 0x06, 0xb1, 0x26, 0x14, 0x00, 0x10, 0x02, 0x14, 0x00,
  0x0d, 0x02, 0x00, 0x50, 0xc0, 0x01, 0xfa, 0x60, 0x19, 0x16, 0xfd, 0x0c, 0x21, 0xab, 0x50, 0x10,
  0xff, 0xff, 0x57, 0x2b, 0x00, 0x00, 0x0d, 0x0a, 0x30, 0x35, 0x39, 0x45, 0x0d, 0x0a, 0x2f, 0xf2,
};

 // Dump predicted (size 64):
AlignedHeader predicted4[] = {
  0x00, 0x1b, 0x21, 0x58, 0xca, 0xbd, 0x18, 0xef, 0x63, 0xe2, 0x02, 0x35, 0x08, 0x00, 0x45, 0x00,
  0x05, 0xce, 0x00, 0x00, 0x40, 0x00, 0x22, 0x06, 0xfa, 0xd0, 0x14, 0x00, 0x10, 0x02, 0x14, 0x00,
  0x0d, 0x02, 0x00, 0x50, 0xc0, 0x01, 0xfa, 0x60, 0x19, 0x16, 0xfd, 0x0c, 0x21, 0xab, 0x50, 0x10,
  0xff, 0xff, 0xda, 0x6d, 0x35, 0x00, 0x0d, 0x0a, 0x30, 0x35, 0x39, 0x45, 0x0d, 0x0a, 0x00, 0x00,
};


// AVSM heade
// Dump receive (size 128):
AlignedHeader receivedAVSM[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x02, 0x00, 0x01, 0x0c, 0x00, 0x00, 0x05, 0x00,
  0x81, 0x00, 0x80, 0x81, 0x00, 0x00, 0x01, 0x00, 0x00, 0x04, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x45, 0x00, 0x00, 0x28, 0x00, 0x00, 0x40, 0x00, 0x7f, 0x06, 0xc3, 0xc8, 0x14, 0x00, 0x0f, 0x02,
  0x14, 0x04, 0x01, 0x02, 0x00, 0x50, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x71, 0xec, 0x6f,
  0x50, 0x14, 0xff, 0xff, 0xc9, 0x97, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// Dump predicted (size 128):
AlignedHeader predictedAVSM[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0c,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x45, 0x01, 0x00, 0x28, 0x00, 0x00, 0x40, 0x00, 0x01, 0x00, 0xfa, 0xd0, 0x14, 0x00, 0x0f, 0x02,
  0x14, 0x04, 0x01, 0x02, 0x00, 0x50, 0xc0, 0x00, 0xc6, 0xbe, 0x72, 0x01, 0x00, 0x00, 0x00, 0x00,
  0x50, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 };



// Dump received (size 128):
AlignedHeader receivedAVSM2[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x02, 0x00, 0x01, 0x0c, 0x00, 0x00, 0x05, 0x00,
  0x81, 0x00, 0x80, 0x40, 0x00, 0x00, 0x01, 0x00, 0x00, 0x04, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x45, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x40, 0x00, 0x7f, 0x06, 0xc3, 0xc4, 0x14, 0x00, 0x0f, 0x02,
  0x14, 0x04, 0x01, 0x02, 0x00, 0x50, 0xc0, 0x00, 0x6a, 0x36, 0x54, 0x61, 0x03, 0x26, 0x14, 0xcb,
  0x60, 0x12, 0xff, 0xff, 0xc9, 0x55, 0x00, 0x00, 0x02, 0x04, 0x05, 0x94, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// Dump predicted (size 128):
AlignedHeader predictedAVSM2[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0c,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x45, 0x00, 0x00, 0x28, 0x00, 0x00, 0x40, 0x00, 0x01, 0x06, 0xfa, 0xd0, 0x14, 0x00, 0x0f, 0x02,
  0x14, 0x04, 0x01, 0x02, 0x00, 0x50, 0xc0, 0x00, 0x6a, 0x36, 0x54, 0x61, 0x00, 0x00, 0x00, 0x00,
  0x60, 0x12, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


// 
// with ISM dynamic header
#if 0
AlignedHeader predictedDynHdr[] = {
  0x20, 0x20, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x59, 0x00, 0x00, 0x01, 0x00, 0x01, 0x0c,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x45, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x40, 0x00, 0x80, 0x06, 0xd3, 0xb3, 0x10, 0x05, 0x01, 0x02,
  0x0d, 0x08, 0x09, 0x0a, 0xc0, 0x00, 0x00, 0x50, 0xbd, 0x88, 0xc5, 0xad, 0x00, 0x00, 0x00, 0x00,
  0x60, 0x02, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x04, 0x05, 0x74, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

AlignedHeader receivedDynHdr[] = {
  0x04, 0x1c, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x59, 0x00, 0x00, 0x01, 0x00, 0x01, 0x0c,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x45, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x40, 0x00, 0x80, 0x06, 0xd3, 0xb3, 0x10, 0x05, 0x02, 0x02,
  0x0d, 0x08, 0x09, 0x0a, 0xc0, 0x00, 0x00, 0x50, 0xbd, 0x88, 0xc5, 0xad, 0x00, 0x00, 0x00, 0x00,
  0x60, 0x02, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x04, 0x05, 0x74, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
#endif  // if 0

AlignedHeader receivedDynHdr[] = {
  0x04, 0x1c, 0x00, 0x04, 0x00, 0x14, 0x00, 0x00, 0x02, 0x00, 0x01, 0x0c, 0x00, 0x00, 0x00, 0x00,
  0x81, 0x00, 0x80, 0x81, 0x00, 0x00, 0x1b, 0x00, 0x00, 0x02, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x45, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x40, 0x00, 0x7e, 0x06, 0xd5, 0xb3, 0x0d, 0x08, 0x09, 0x0a,
  0x10, 0x05, 0x01, 0x02, 0x00, 0x50, 0xc0, 0x00, 0x59, 0x10, 0x21, 0xf8, 0x9e, 0x2c, 0x83, 0xc8,
  0x60, 0x12, 0xff, 0xff, 0x13, 0xf0, 0x00, 0x00, 0x02, 0x04, 0x05, 0x74, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

AlignedHeader predictedDynHdr[] = {
  0x20, 0x20, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x59, 0x00, 0x00, 0x01, 0x00, 0x01, 0x0c,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x45, 0x01, 0x02, 0x40, 0x00, 0x00, 0x40, 0x00, 0x01, 0x06, 0xfa, 0xd0, 0x0d, 0x08, 0x09, 0x0a,
  0x10, 0x05, 0x01, 0x02, 0x00, 0x50, 0xc0, 0x00, 0x59, 0x10, 0x21, 0xf9, 0x9e, 0x2c, 0x83, 0xc8,
  0x50, 0x10, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x0a, 0x30, 0x32, 0x31, 0x30, 0x0d, 0x0a,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/*
   0     1     1     1     1     1     1     1     1     0     1     1     0     0     0     0
  0x45, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x40, 0x00, 0x7e, 0x06, 0xd5, 0xb3, 0x0d, 0x08, 0x09, 0x0a,
  0x45, 0x01, 0x02, 0x40, 0x00, 0x00, 0x40, 0x00, 0x01, 0x06, 0xfa, 0xd0, 0x0d, 0x08, 0x09, 0x0a,

   0     0     0     0     0     0     0     0     0     0     0     0     1     1     1     1
  0x10, 0x05, 0x01, 0x02, 0x00, 0x50, 0xc0, 0x00, 0x59, 0x10, 0x21, 0xf8, 0x9e, 0x2c, 0x83, 0xc8,
  0x10, 0x05, 0x01, 0x02, 0x00, 0x50, 0xc0, 0x00, 0x59, 0x10, 0x21, 0xf9, 0x9e, 0x2c, 0x83, 0xc8,

   0     0     1     1     1     1     1     1     0     0     0     0     0     0     0     0  
  0x60, 0x12, 0xff, 0xff, 0x13, 0xf0, 0x00, 0x00, 0x02, 0x04, 0x05, 0x74, 0x00, 0x00, 0x00, 0x00,
  0x50, 0x10, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x0a, 0x30, 0x32, 0x31, 0x30, 0x0d, 0x0a,

   1     1     1     1     1     1     1     1     1     1     1     1     1     1     1     1
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
*/

#if 0 // orig
// Dump predicted (size 128):
AlignedHeader predictedAVSM2[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0c,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x45, 0x00, 0x00, 0x28, 0x00, 0x00, 0x40, 0x00, 0x01, 0x06, 0xfa, 0xd0, 0x14, 0x00, 0x0f, 0x02,
  0x14, 0x04, 0x01, 0x02, 0x00, 0x50, 0xc0, 0x00, 0x92, 0xdf, 0x26, 0x03, 0x00, 0x00, 0x00, 0x00,
  0x50, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
#endif

AlignedHeader *receive = received;

void dump (AlignedHeader *data, size_t size, char *name)
{
  int i;
  printf("Dump %s (size %d):\n", name, size);
  printf("-----------------\n");
  for (i = 0; i < size; i++) {
    if (!(i%16))
      printf("%s%04X: ", i?"\n":"", i);
    printf("%02x ", data[i]);
  }
  printf("\n");
}



// AVSM bit masks for the 32-95th bytes in the TCP headers. Skip the first 32 RBEH header and 
// the last 32 bytes, which are unused bytes to pad to the cache line boundry
#define AVSMCLIENTDC1 0xF0000DFE
#define AVSMCLIENTDC2 0xFFFF00FC


#define CLIENTDC1 0x037F8000
#define CLIENTDC2 0xC03F3C00
#define SERVERDC1 0x037F8000
#define SERVERDC2 0xFFFF3C00



inline unsigned int client_validate_header(AlignedHeader* predicted)
{
   register unsigned int result;
   register AlignedHeader* src = receive;

   dump(src, 64, "Receive header");
   dump(predicted, 64, "AVSM Predicted header");

   // rsi[] <-- src
   // rdi[] <-- predicted

   __asm__ __volatile__ (   
        ".intel_syntax noprefix;"
        "movdqa     xmm0, [rsi];"
        "movdqa     xmm1, [rsi+16];"
        "movdqa     xmm2, [rsi+32];"
        "movdqa     xmm3, [rsi+48];"
        "pcmpeqb    xmm0, [rdi];"
        "pcmpeqb    xmm1, [rdi+16];"
        "pcmpeqb    xmm2, [rdi+32];"
        "pcmpeqb    xmm3, [rdi+48];"
        "pmovmskb   eax, xmm0;"
        "pmovmskb   ebx, xmm1;"
        "pmovmskb   ecx, xmm2;"
        "pmovmskb   edx, xmm3;"
        "shl        ebx, 16;"
        "shl        edx, 16;"
        "or         eax, "STRINGIZE(CLIENTDC1)";"
        "or         ecx, "STRINGIZE(CLIENTDC2)";"
        "or         eax, ebx;"
        "or         ecx, edx;"
        "mov        ebx, eax;"
        "and        eax, ecx;"
        "add        eax, 1;"
        "jnz        mismatch2%=;"
        "mov        ax, word ptr[rsi+16];"
        "mov        ecx, dword ptr[rsi+38];"
        "mov        edx, dword ptr[rsi+42];"
        "mov        bx, word ptr[rsi+48];"
        "rol        ax, 8;"
        "bswap      ecx;"
        "mov        esi, dword ptr[rdi+42];"
        "bswap      edx;"
        "bswap      esi;"
        "cmp        esi, edx;"
        "jg         getlength%=;"
        "bswap      edx;"
        "mov        word ptr[rdi+48], bx;"
        "mov        dword ptr[rdi+42], edx;"
"getlength%=:;"
        "sub        eax, "STRINGIZE(IPOVERHEAD)";"
        "jz         done2%=;"
        "add        ecx, eax ;"
        "mov        dl, byte ptr[rdi+22];"
        "bswap      ecx;"
        "sub        byte ptr[rdi+15], 1;"
        "mov        dword ptr[rdi+38], ecx;"
        "jnz        done2%=;"
        "mov        byte ptr[rdi+15], dl;"
        "or         eax, "STRINGIZE(ACKMASK)";"
        "jmp        done2%=;"
"mismatch2%=:;"
        "mov        dx, word ptr[rsi+16];"
        "not        ebx;"
        "rol        dx, 8;"
        "test       ebx, ebx;"
        "jz         second2%=;"
        "bsf        eax, ebx;"
        "add        eax, 1;"
        "jmp        misdone2%=;"
"second2%=:;"
        "not        ecx;"
        "bsf        eax, ecx;"
        "add        eax, 33;"
"misdone2%=:;"
        "sub        dx, "STRINGIZE(IPOVERHEAD)";"
        "shl        eax, 16;"
        "mov        ax, dx;"
"done2%=:;"
        ".att_syntax prefix"
        : "=a"(result) , "=S" (src), "=D" (predicted)
        : "1" (src), "2" (predicted)
        : "memory",
          "ebx", "ecx", "edx",
          "bx", "dx",
          "dl",
#if !LINUX_KERNEL_26
          "xmm0", "xmm1", "xmm2", "xmm3",
#endif
          "flags"
    );

    return result;
}

// A cache line (64 bytes) aligned AVSM TCP header is 128 byte long:
//   0-31  RBEH
//  32-95  IP/TCP
//  96-127 paddings
//  
// Validate the AVSM TCP header excluding the first 32-byte RBEH and the 
// cache line padding bytes (byte 96 onward).
inline unsigned int client_validate_avsm_header(AlignedHeader* predicted)
{
   register unsigned int result;
   register AlignedHeader* src = receive;
   unsigned char hdrLen = 32;

   dump(src, 128, "Receive header");
   dump(predicted, 128, "Predicted header");

   // rsi[] <-- src
   // rdi[] <-- predicted

   __asm__ __volatile__ (   
        ".intel_syntax noprefix;"
        "movdqa     xmm0, [rsi+32];"  // 32- 47
        "movdqa     xmm1, [rsi+48];"  // 48- 63
        "movdqa     xmm2, [rsi+64];"  // 64- 79
        "movdqa     xmm3, [rsi+80];"  // 80- 95
        "pcmpeqb    xmm0, [rdi+32];"
        "pcmpeqb    xmm1, [rdi+48];"
        "pcmpeqb    xmm2, [rdi+64];"
        "pcmpeqb    xmm3, [rdi+80];"
        "pmovmskb   eax, xmm0;"
        "pmovmskb   ebx, xmm1;"
        "pmovmskb   ecx, xmm2;"
        "pmovmskb   edx, xmm3;"
        "shl        ebx, 16;"
        "shl        edx, 16;"
        "or         eax, "STRINGIZE(AVSMCLIENTDC1)";"
        "or         ecx, "STRINGIZE(AVSMCLIENTDC2)";"
        "or         eax, ebx;"
        "or         ecx, edx;"
        "mov        ebx, eax;"
        "and        eax, ecx;"
        "add        eax, 1;"
        "jnz        mismatch2%=;"
        "mov        ax, word ptr[rsi+34];"  // IP length
        "mov        ecx, dword ptr[rsi+56];" // TCP sequence
        "mov        edx, dword ptr[rsi+60];" // TCP ack 
        "mov        bx, word ptr[rsi+66];"   // TCP window
        "rol        ax, 8;"
        "bswap      ecx;"
        "mov        esi, dword ptr[rdi+60];"  // TCP ack
        "bswap      edx;"
        "bswap      esi;"
        "cmp        esi, edx;"
        "jg         getlength%=;"
        "bswap      edx;"
        "mov        word ptr[rdi+66], bx;"  // TCP window size
        "mov        dword ptr[rdi+60], edx;"  // TCP ack
"getlength%=:;"
        "sub        eax, "STRINGIZE(IPOVERHEAD)";"
        "jz         done2%=;"
        "add        ecx, eax ;"
        "mov        dl, byte ptr[rdi+40];" // IP ttl
        "bswap      ecx;"
        "sub        byte ptr[rdi+33], 1;"  // IP tos
        "mov        dword ptr[rdi+56], ecx;"  // TCP sequence
        "jnz        done2%=;"
        "mov        byte ptr[rdi+33], dl;"  // IP tos
        "or         eax, "STRINGIZE(ACKMASK)";"
        "jmp        done2%=;"
"mismatch2%=:;"
        "mov        dx, word ptr[rsi+34];" // IP length
        "not        ebx;"
        "rol        dx, 8;"
        "test       ebx, ebx;"
        "jz         second2%=;"
        "bsf        eax, ebx;"
        "add        eax, 1;"
        "jmp        misdone2%=;"
"second2%=:;"
        "not        ecx;"
        "bsf        eax, ecx;"
        "add        eax, 33;"
"misdone2%=:;"
        "sub        dx, "STRINGIZE(IPOVERHEAD)";"
        "shl        eax, 16;"
        "mov        ax, dx;"
"done2%=:;"
        ".att_syntax prefix"
        : "=a"(result) , "=S" (src), "=D" (predicted)
        : "1" (src), "2" (predicted)
        : "memory",
          "ebx", "ecx", "edx",
          "bx", "dx",
          "dl",
#if !LINUX_KERNEL_26
          "xmm0", "xmm1", "xmm2", "xmm3",xxx
#endif
          "flags"
    );

    return result;
}


// A cache line (64 bytes) aligned AVSM TCP header is 128 byte long:
//   0 - (N-1)  variable length RBEH
//   IP/TCP
//   Padding
//  
// Validate the AVSM TCP header excluding the first 32-byte RBEH and the 
// cache line padding bytes (byte 96 onward).
inline unsigned int client_validate_dyn_header(AlignedHeader* predicted)
{
   register unsigned int result;
   register AlignedHeader* src;

   // Skip the dynamic headers, start directly from IP/TCP headers
   unsigned char *ptr = (unsigned char*)receive;
   src = ptr + (ptr[0] + ptr[1] + ptr[2]);

   ptr = (unsigned char*)predicted;
   predicted = ptr + (ptr[0] + ptr[1] + ptr[2]);

   //dump(src, 64, "Receive header");
   //dump(predicted, 64, "Predicted header");

   // rsi[] <-- src
   // rdi[] <-- predicted

   __asm__ __volatile__ (   
        ".intel_syntax noprefix;"
        "movdqa     xmm0, [rsi];"
        "movdqa     xmm1, [rsi+16];"
        "movdqa     xmm2, [rsi+32];"
        "movdqa     xmm3, [rsi+48];"
        "pcmpeqb    xmm0, [rdi];"
        "pcmpeqb    xmm1, [rdi+16];"
        "pcmpeqb    xmm2, [rdi+32];"
        "pcmpeqb    xmm3, [rdi+48];"
        "pmovmskb   eax, xmm0;"
        "pmovmskb   ebx, xmm1;"
        "pmovmskb   ecx, xmm2;"
        "pmovmskb   edx, xmm3;"
        "shl        ebx, 16;"
        "shl        edx, 16;"
        "or         eax, "STRINGIZE(AVSMCLIENTDC1)";"
        "or         ecx, "STRINGIZE(AVSMCLIENTDC2)";"
        "or         eax, ebx;"
        "or         ecx, edx;"
        "mov        ebx, eax;"
        "and        eax, ecx;"
        "add        eax, 1;"
        "jnz        mismatch2%=;"
        "mov        ax, word ptr[rsi+2];"  // IP length
        "mov        ecx, dword ptr[rsi+24];" // TCP sequence
        "mov        edx, dword ptr[rsi+28];" // TCP ack 
        "mov        bx, word ptr[rsi+34];"   // TCP window
        "rol        ax, 8;"
        "bswap      ecx;"
        "mov        esi, dword ptr[rdi+28];"  // TCP ack
        "bswap      edx;"
        "bswap      esi;"
        "cmp        esi, edx;"
        "jg         getlength%=;"
        "bswap      edx;"
        "mov        word ptr[rdi+34], bx;"  // TCP window size
        "mov        dword ptr[rdi+28], edx;"  // TCP ack
"getlength%=:;"
        "sub        eax, "STRINGIZE(IPOVERHEAD)";"
        "jz         done2%=;"
        "add        ecx, eax ;"
        "mov        dl, byte ptr[rdi+8];" // IP ttl
        "bswap      ecx;"
        "sub        byte ptr[rdi+1], 1;"  // IP tos
        "mov        dword ptr[rdi+24], ecx;"  // TCP sequence
        "jnz        done2%=;"
        "mov        byte ptr[rdi+1], dl;"  // IP tos
        "or         eax, "STRINGIZE(ACKMASK)";"
        "jmp        done2%=;"
"mismatch2%=:;"
        "mov        dx, word ptr[rsi+2];" // IP length
        "not        ebx;"
        "rol        dx, 8;"
        "test       ebx, ebx;"
        "jz         second2%=;"
        "bsf        eax, ebx;"
        "add        eax, 1;"
        "jmp        misdone2%=;"
"second2%=:;"
        "not        ecx;"
        "bsf        eax, ecx;"
        "add        eax, 33;"
"misdone2%=:;"
        "sub        dx, "STRINGIZE(IPOVERHEAD)";"
        "shl        eax, 16;"
        "mov        ax, dx;"
"done2%=:;"
        ".att_syntax prefix"
        : "=a"(result) , "=S" (src), "=D" (predicted)
        : "1" (src), "2" (predicted)
        : "memory",
          "ebx", "ecx", "edx",
          "bx", "dx",
          "dl",
#if !LINUX_KERNEL_26
          "xmm0", "xmm1", "xmm2", "xmm3",
#endif
          "flags"
    );

    return result;
}

// A cache line (64 bytes) aligned AVSM TCP header is 128 byte long:
//   0-31  RBEH
//  32-95  IP/TCP
//  96-127 paddings
//  
// Validate the AVSM TCP header excluding the first 32-byte RBEH and the 
// cache line padding bytes (byte 96 onward).
inline unsigned int server_validate_avsm_header(AlignedHeader* predicted)
{
   register unsigned int result;
   register AlignedHeader* src = receive;

   dump(src, 128, "Receive header");
   dump(predicted, 128, "Predicted header");

   // rsi[] <-- src
   // rdi[] <-- predicted

    __asm__ __volatile__ (   
        ".intel_syntax noprefix;"
        "movdqa     xmm0, [rsi+32];"  // bytes 32-80
        "movdqa     xmm1, [rsi+48];"
        "movdqa     xmm2, [rsi+64];"
        "pcmpeqb    xmm0, [rdi+32];"
        "pcmpeqb    xmm1, [rdi+48];"
        "pcmpeqb    xmm2, [rdi+64];"
        "pmovmskb   eax, xmm0;"
        "pmovmskb   ebx, xmm1;"
        "pmovmskb   ecx, xmm2;"
        "shl        ebx, 16;"
        "or         eax, "STRINGIZE(AVSMSERVERDC1)";"
        "or         ecx, "STRINGIZE(AVSMSERVERDC2)";"
        "or         eax, ebx;"
        "mov        ebx, eax;"
        "and        eax, ecx;"
        "add        eax, 1;"
        "jnz        mismatch%=;"
        "mov        ax, word ptr[rsi+34];"   // IP length
        "mov        ecx, dword ptr[rsi+56];" // TCP sequence
        "mov        bx, word ptr[rsi+66];"   // TCP window size
        "mov        edx, dword ptr[rsi+60];" // TCP ack
        "rol        ax, 8;"
        "bswap      ecx;"
        "sub        eax, "STRINGIZE(IPOVERHEAD)";"
        "add        ecx, eax ;"
        "test       eax, eax;"
        "jnz        miss2%=;"
        "cmp        dword ptr[rdi+60], edx;" // TCP sync
        "jnz        miss3%=;"
        "sub        byte ptr[rdi+40], 1;"    // TCP ttl
        "jnz        miss2%=;"
        "mov        eax, "STRINGIZE(ACKMASK)";"
        "mov        byte ptr[rdi+40], 3;"    // TCP ttl
        "jmp        miss2%=;"
"miss3%=:;"
        "mov        byte ptr[rdi+40], 2;"
"miss2%=:;"
        "mov        esi, dword ptr[rdi+60];"
        "bswap      edx;"
        "bswap      esi;"
        "bswap      ecx;"
        "sub        esi, edx;"
        "test       esi, esi;"
        "jg         miss1%=;"
        "bswap      edx;"
        "mov        word ptr[rdi+66], bx;"
        "mov        dword ptr[rdi+60], edx;"
        "or         eax, "STRINGIZE(ACKUPDATE)";"
"miss1%=:;"
        "mov        dword ptr[rdi+56], ecx;"
        "jmp        done%=;"
"mismatch%=:;"
        "mov        dx, word ptr[rsi+34];"
        "not        ebx;"
        "rol        dx, 8;"
        "test       ebx, ebx;"
        "jz         second%=;"
        "bsf        eax, ebx;"
        "add        eax, 1;"
        "jmp        misdone%=;"
"second%=:;"
        "not        ecx;"
        "bsf        eax, ecx;"
        "add        eax, 33;"
"misdone%=:;"
        "sub        dx, "STRINGIZE(IPOVERHEAD)";"
        "shl        eax, 16;"
        "mov        ax, dx;"
"done%=:;"
        ".att_syntax prefix"
        : "=a"(result) , "=S" (src), "=D" (predicted)
        : "1" (src), "2" (predicted)
        : "memory",
          "ebx", "ecx", "edx",
          "bx", "dx",
#if !LINUX_KERNEL_26
          "xmm0", "xmm1", "xmm2", "xmm3",
#endif
          "flags"
    );

    return result;
}

inline void copy_avsm_header(AlignedHeader* dest, AlignedHeader* source) 
{
    asm (
        ".intel_syntax noprefix;"
        "movdqa     xmm0, [rsi];"      // rsi <- source
        "movdqa     xmm1, [rsi+16];"
        "movdqa     xmm2, [rsi+32];"
        "movdqa     xmm3, [rsi+48];"
        "movdqa     xmm4, [rsi+64];"
        "movdqa     xmm5, [rsi+80];"
        "movdqa     xmm6, [rsi+96];"
        "movdqa     xmm7, [rsi+112];"

        "movntdq    [rdi], xmm0;"      // rdi <- dest
        "movntdq    [rdi+16], xmm1;"
        "movntdq    [rdi+32], xmm2;"
        "movntdq    [rdi+48], xmm3;"
        "movntdq    [rdi+64], xmm4;"
        "movntdq    [rdi+80], xmm5;"
        "movntdq    [rdi+96], xmm6;"
        "movntdq    [rdi+112], xmm7;"

        "sfence;"
        ".att_syntax prefix"
        :
        : "S"(source), "D"(dest)
#if !LINUX_KERNEL_26
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7
#endif
        );
}


inline void replicate_avsm_header(AlignedHeader* dest, AlignedHeader* source, int count) 
{
    __asm__ __volatile__ (
        ".intel_syntax noprefix;"
        "mov        rdx, "STRINGIZE(CACHELINESIZE)";"
        "shl        rdx, 1;"        // AVSM header occupies two cachelines
        "movdqa     xmm0, [rsi];"   // rsi <-- source
        "movdqa     xmm1, [rsi+16];"
        "movdqa     xmm2, [rsi+32];"
        "movdqa     xmm3, [rsi+48];"
        "movdqa     xmm4, [rsi+64];"
        "movdqa     xmm5, [rsi+80];"
        "movdqa     xmm6, [rsi+96];"
        "movdqa     xmm7, [rsi+112];"
    "rloop%=:;"
        "movntdq    [rdi], xmm0;"
        "movntdq    [rdi+16], xmm1;"
        "movntdq    [rdi+32], xmm2;"
        "movntdq    [rdi+48], xmm3;"
        "movntdq    [rdi+64], xmm4;"
        "movntdq    [rdi+80], xmm5;"
        "movntdq    [rdi+96], xmm6;"
        "movntdq    [rdi+112], xmm7;"
        "add        rdi, rdx;"
        "dec        ecx;"           // ecx <-- count
        "jnz        rloop%=;"
        "sfence;"
        ".att_syntax prefix"
        : "=S"(source), "=D"(dest), "=c"(count)
        : "0"(source), "1"(dest), "2"(count)
        : "rdx",
#if !LINUX_KERNEL_26
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
#endif
          "flags", "memory"
    );
}


inline void distribute_avsm_header(AlignedHeader* dest, AlignedHeader* source, 
                                   unsigned int count, unsigned int place)
{
    __asm__ __volatile__ (
        ".intel_syntax noprefix;"
        "movdqa     xmm0, [rsi];"    // rsi <-- source
        "movdqa     xmm1, [rsi+16];"
        "movdqa     xmm2, [rsi+32];"
        "movdqa     xmm3, [rsi+48];"
        "movdqa     xmm4, [rsi+64];"
        "movdqa     xmm5, [rsi+80];"
        "movdqa     xmm6, [rsi+96];"
        "movdqa     xmm7, [rsi+112];"
    "rloop%=:;"
        "movntdq    [rdi], xmm0;"    // rdi <-- dest
        "movntdq    [rdi+16], xmm1;"
        "movntdq    [rdi+32], xmm2;"
        "movntdq    [rdi+48], xmm3;"
        "movntdq    [rdi+64], xmm4;"
        "movntdq    [rdi+80], xmm5;"
        "movntdq    [rdi+96], xmm6;"
        "movntdq    [rdi+112], xmm7;"

        "add        rdi, rdx;"
        "dec        ecx;"
        "jnz        rloop%=;"
        "sfence;"
        ".att_syntax prefix"
        : "=S"(source), "=D"(dest), "=c"(count)
        : "0"(source), "1"(dest), "2"(count), "d"(place)
        : "flags", "memory"
#if !LINUX_KERNEL_26
          , "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
#endif
    );
}



int main (void)
{
  unsigned int miss;

#if 0
  printf("\nmiss should be 2a0000:\n");
  receive = received;
  miss = client_validate_header(predicted);
  printf("miss=0x%x miss&MISSMASK=0x%x miss&ACKMASK=0x%x\n", 
         miss, miss & MISSMASK, miss & ACKMASK);

  printf("\nmiss should be 3701d9:\n");
  receive = received1;
  miss = client_validate_header(predicted1);
  printf("miss=0x%x miss&MISSMASK=0x%x miss&ACKMASK=0x%x\n", 
         miss, miss & MISSMASK, miss & ACKMASK);
#endif

#if 1
  printf("\nmiss should be 2a0000:\n");
  receive = received2;
  miss = client_validate_header(predicted2);
  printf("miss=0x%x miss&MISSMASK=0x%x miss&ACKMASK=0x%x\n", 
         miss, miss & MISSMASK, miss & ACKMASK);

  printf("\nmiss should be 800005a6:\n");
  receive = received3;
  miss = client_validate_header(predicted3);
  printf("miss=0x%x miss&MISSMASK=0x%x miss&ACKMASK=0x%x\n", 
         miss, miss & MISSMASK, miss & ACKMASK);
#endif

#if 0
  printf("\nmiss should be 5a6:\n");
  receive = received4;
  miss = client_validate_header(predicted4);
  printf("miss=0x%x miss&MISSMASK=0x%x miss&ACKMASK=0x%x\n", 
         miss, miss & MISSMASK, miss & ACKMASK);
#endif

  printf("\nAVSM: miss should be :\n");
  receive = receivedAVSM;
  miss = client_validate_avsm_header(predictedAVSM);
  printf("miss=0x%x miss&MISSMASK=0x%x miss&ACKMASK=0x%x\n", 
         miss, miss & MISSMASK, miss & ACKMASK);

  printf("\nAVSM: miss should be :\n");
  receive = receivedAVSM2;
  miss = client_validate_avsm_header(predictedAVSM2);
  printf("miss=0x%x;  miss&MISSMASK=0x%x -- 0x%x;  miss&ACKMASK=0x%x\n", 
         miss, miss & MISSMASK, ((miss & MISSMASK)>>16) + 32, miss & ACKMASK);

  // Test dynamic header
  printf("\nISM dynamic header: miss should be :\n");
  receive = receivedDynHdr;
  
  //unsigned char *ptr = (unsigned char *)predictedDynHdr;
  //unsigned char offset = ptr[0] + ptr[1] + ptr[2];

  dump(receive, 128, "Receive header");
  dump(predictedDynHdr, 128, "Predicted header");

  //miss = client_validate_dyn_header((unsigned char*)predictedDynHdr + offset);
  miss = client_validate_dyn_header(predictedDynHdr);
  printf("miss=0x%x miss&MISSMASK=0x%x miss&ACKMASK=0x%x\n", 
         miss, miss & MISSMASK, miss & ACKMASK);



  printf("\nAVSM: copy header\n");
  dump(receivedAVSM2, 128, "Source header");
  copy_avsm_header(predictedAVSM2, receivedAVSM2);
  dump(predictedAVSM2, 128, "Predicted header");

  printf("\nAVSM: replicate N headers\n");
  AlignedHeader replicated[4*128];
  int count = 3;
  dump(receivedAVSM2, 128, "Source header");
  replicate_avsm_header(replicated, receivedAVSM2, count);
  dump(replicated, count*128, "Replicated headers");



  //distribute_header(&headers[0].hdr, header, HTTPPERPAGE, sizeof(HTTPBuffer));



  return 0;
}



#if 0  // received on appliance streamer
1289871397.496912 ---- Received: 5550   (Transmitted: 28592) ------
Received packet #5522 size 1500 0x5dc
  on adapter 1 00:1b:21:58:ca:bd  20.0.13.2
EthernetHeader offset 0x000-0x00e
  0x000 Destination MAC address 00:1b:21:58:ca:bd
  0x006 Source MAC address      18:ef:63:e2:02:35
  0x00c Protocol type           2048 0x800  EtherType IPv4
IP header offset 0x00e-0x022
  0x00e Version                 4 0x4 valid
  0x00e IP header length        5 0x5 valid
  0x00f DiffServ                0 0x0  dscp 0 0x0  ecn 0 0x0
  0x010 Total length            1486 0x5ce
  0x012 Fragment id             0 0x0
  0x014 Fragment offset         0 0x0  Flags 0x2  Do not fragment
  0x016 Time to live            127 0x7f
  0x017 Protocol type           6 0x6  TCP
  0x018 Header checksum         0x26b1 valid
  0x01a Source IP address       20.0.16.2
  0x01e Destination IP address  20.0.13.2
TCP header offset 0x022-0x036   Length 20 0x14
  0x022 Source port             80 0x50   HTTP
  0x024 Destination port        49153 0xc001
  0x026 Sequence number         4203378198 0xfa8a7616
  0x02a Acknowledgement number  4245430699 0xfd0c21ab
  0x02e Data offset             5 0x5
  0x02e Reserved                0 0x0
  0x02f Control Bits            16 0x10
    cwr ece urg ACK psh rst syn fin
  0x030 Window Size             65535 0xffff
  0x032 Checksum                1357 0x54d
  0x034 Urgent Pointer          0 0x0
  0x036-0x036 Options
HTTP Data 0x0036-0x05dc
  Chunk Header: Length 059E
0000: 00 1b 21 58 ca bd 18 ef 63 e2 02 35 08 00 45 00
0010: 05 ce 00 00 40 00 7f 06 b1 26 14 00 10 02 14 00
0020: 0d 02 00 50 c0 01 fa 8a 76 16 fd 0c 21 ab 50 10
0030: ff ff 05 4d 00 00 0d 0a 30 35 39 45 0d 0a fd 8b
0040: 47 d4 0b f6 4c 38 ed 54 e6 6d fb b1 ba b6 f7 82
0050: 0e 0f 03 24 ae f3 8f c6 11 5e e2 f2 c0 b8 83 5a
0060: 59 17 db 1a 26 d9 ad c0 c3 22 d2 e8 20 80 b8 25
0070: 35 c9 f0 26 e6 89 2d 43 52 dc 5b 5a 5a 6d 9b a5
0080: 81 5b dd 5b 7c e2 f7 4d 17 2e 60 50 48 19 80 00
0090: 4a 99 03 19 ff 4d af a7 8f d0 eb d0 f6 b6 94 3d
00A0: ce e6 77 39 e8 20 d8 77 f1 f1 63 1a be 20 27 44
00B0: a8 21 52 1d 8b 99 77 d5 ba d8 30 27 06 a1 04 02
00C0: 3a ad c6 f9 15 21 a7 e5 44 78 d7 20 32 f9 4e bc
00D0: c6 18 ac d2 a4 55 5a 58 61 d6 c7 4d 19 2d 4e f4
00E0: 27 12 b2 e9 47 01 e1 11 e4 45 93 4e 66 46 8a 01
00F0: ad 30 b5 1f 2b 88 ac 8d a4 0e 1c 75 de f4 28 c0
0100: d4 e8 73 5a 50 36 00 0c 1a 82 c6 1c 03 95 e4 71
0110: df 90 0c 1d 01 1a 75 80 80 c4 6c 0d 7f ff 24 5f
0120: 98 e6 38 7b 1f e2 11 6d c0 b2 1c 38 c8 cd 89 d4
0130: 68 8a 1d 26 66 03 e2 0c 5e 14 17 40 50 99 c9 ae
0140: 82 12 09 41 98 07 24 3e e4 b4 f0 01 46 e4 7e 1d
0150: 00 68 03 b2 1a 08 40 84 00 e4 22 b9 69 02 48 e4
0160: f2 d2 7c 81 60 03 fb 01 75 3b a1 23 d6 e4 62 05
0170: f6 38 9e 78 50 1a 88 40 30 c9 0c 28 06 e0 31 49
0180: 30 98 38 31 0b fc 4d 92 92 01 9f 72 19 69 40 0e
0190: 8a 24 6e 90 0b 08 40 92 00 68 f9 25 f0 15 85 50
01A0: 47 01 e1 12 aa 77 07 b0 31 ff fc 05 4b 40 14 db
01B0: 14 42 10 35 80 06 86 61 32 58 ac ea 23 a4 2f c8
01C0: 1c 78 5d 0e 8d 82 81 09 b5 01 18 ae eb 30 4f 75
01D0: 08 3a 18 3c 8e b8 7d 38 81 60 d4 e6 1f 24 47 80
01E0: 3d a7 10 0c 0c 49 80 71 44 68 fb 75 46 13 a0 ba
01F0: d5 db 20 97 d4 32 82 a3 eb 94 0a b2 00 d4 c1 d0
0200: 0b c5 59 c7 4f 5c da 9b b3 69 c4 02 03 38 b1 72
0210: 63 55 3f 36 4e 1c 3e b9 41 37 f0 ad 5c cc 30 0e
0220: 38 50 e8 9a 03 74 a5 00 36 46 41 cb fc 63 8e e7
0230: c7 91 89 c4 19 63 03 c2 b1 c2 c7 0e d1 e3 88 80
0240: 74 9f 10 08 f4 3a 46 03 1f b4 78 10 78 9a 86 0c
0250: 51 09 49 49 24 b1 87 28 05 f2 4b ce 47 01 e1 13
0260: 02 54 a7 81 33 bd 0e 8e 80 19 6e 40 98 9a 35 04
0270: a4 8c 72 8d 03 ec 45 6a 9c e0 19 d3 98 22 85 c2
0280: 60 20 e2 0e 8e ae 6e 22 c2 55 18 4e a7 2c 95 af
0290: dc 9d 87 89 b3 3d c0 00 00 01 09 1a e5 83 85 dd
02A0: 91 6b 94 3f 32 02 10 1a 92 6d 39 81 83 cc 27 cd
02B0: 8f 12 e1 cb 80 d6 22 bd 46 53 99 88 13 0c 3f 13
02C0: c4 43 4e fc f7 20 43 80 07 30 10 ba 75 80 80 c5
02D0: 38 0c c8 24 6b ea 12 00 34 c0 1d c5 00 9c 07 68
02E0: 29 0c fb 00 4b ff 08 c8 00 e6 02 80 0f 00 a2 40
02F0: 2f 02 e4 b0 0a 80 1e 14 12 59 0f 84 16 05 78 e4
0300: 67 01 c0 90 b9 0e fd 9d a6 c1 f4 3a 4c cf 59 18
0310: 2c 9d 00 ec 0a 80 ed 24 47 01 e1 14 d2 80 2a 01
0320: 30 21 00 30 0e b0 09 7e 09 48 8c 6f 17 e0 07 c5
0330: 92 80 60 e0 63 12 cb e6 96 93 47 f2 06 a2 04 26
0340: f5 80 81 14 ed 40 3b 72 8a cc 56 58 13 4b 38 51
0350: 92 71 4b e9 65 f3 d2 ac 4f 88 41 88 60 9e b4 e3
0360: 55 b6 10 e4 75 f8 98 57 28 35 2b 48 b5 77 e1 fe
0370: e8 74 68 06 06 00 cb 4d 01 52 51 43 0e c1 eb f1
0380: 7d 22 c0 3c a2 34 07 6b 8c b5 6e 61 27 80 b6 c5
0390: 0e 93 4e 30 59 b3 3e ee 9c 41 a1 31 6d 6d 6a 3b
03A0: 1f 3b c2 6a 9c 60 21 dd eb 88 06 02 11 10 c9 85
03B0: 00 0f e0 0d a0 a0 37 5c 60 20 04 ee 4c f5 c4 03
03C0: 13 07 ae 48 b0 1a d8 0f ec 80 47 9c e8 80 f5 ca
03D0: 00 53 a2 05 47 01 e1 15 09 88 95 cc 20 d0 e8 08
03E0: 80 ca 40 07 d0 0d 68 a1 2c d0 69 0c 0d e0 14 b8
03F0: cf d3 b2 9b 2c 13 00 13 91 e9 01 9d 0a 00 df 3c
0400: 1a 34 9a 18 34 0c 72 91 83 5b 16 94 95 8b 0f 09
0410: dd 82 a4 e4 81 e1 bf f6 5f 66 47 10 cc 46 95 60
0420: cf ff f7 b4 6c 26 68 25 04 76 7e dc 2a 2f e6 3e
0430: 75 0f 17 0c 5e 84 50 e0 ec 01 05 ac 54 a3 42 a1
0440: c4 78 45 71 80 c3 07 53 96 4a f9 3e 48 d7 50 7c
0450: 5c 75 c0 00 00 01 0a 1a 72 8b ff dc 00 3d 92 dd
0460: 55 ca 0e 26 d6 4e 83 e2 29 06 9c b2 33 58 33 00
0470: 0c b1 4a 88 33 9c 7f c4 6a 30 74 3e 9d 60 22 30
0480: ad c6 84 87 66 f2 3f 18 78 32 80 0c 4b 09 18 83
0490: 47 01 e1 16 36 09 63 52 07 22 8b 0d 41 30 98 58
04A0: 69 7d 08 28 ac e9 49 7d b6 c0 2e d2 55 39 86 53
04B0: b6 28 0d 38 e3 d6 be 46 ca 23 5b 64 0b e0 4d 88
04C0: c0 46 00 c0 06 04 c0 28 01 7f 00 ac 0a 92 f8 09
04D0: b2 ca 26 23 0c 48 09 f8 00 a5 3c 90 42 14 6c 02
04E0: 10 1b 92 c9 48 00 b1 08 26 21 00 58 98 87 25 23
04F0: 63 80 d8 54 c4 7a 1d 26 0e 4f 5c 43 49 7c 94 71
0500: 00 39 06 f1 2f 10 b0 14 0d 2c b7 ea d4 7d 0e 8f
0510: 41 67 63 94 02 fb fc 06 40 1d 68 01 60 03 fc 02
0520: 64 24 06 c0 54 9a 43 01 d8 69 64 80 13 10 9d 25
0530: fc 20 a2 42 05 ab 0b 80 60 01 80 14 00 3e 28 20
0540: 07 41 80 30 41 85 12 4b 0f 18 a7 01 47 01 e1 17
0550: 1a 60 c3 4f 59 ad d5 80 21 7a 71 00 c0 11 c4 13
0560: b3 c1 d1 e3 c2 64 51 bf 8e 12 da 51 49 1b 3b 8b
0570: 94 b0 16 36 8c 06 1f ff 6a 96 46 fa 25 44 a0 03
0580: f6 f0 74 b5 8d 38 c2 40 0d 87 47 c2 65 d7 28 0e
0590: dc 75 ea e5 8b 49 de 4c eb 53 d7 28 00 c1 d1 e4
05A0: 48 d8 05 d3 57 28 05 bb 8c ae 30 28 cf b3 45 58
05B0: 07 51 f0 91 35 c4 03 13 b9 e9 c4 24 15 56 69 c6
05C0: 1e 04 63 f5 5e 30 18 d0 18 07 d0 02 e9 89 03 7a
05D0: 09 1c 07 3c c2 34 30 90 97 58 88 07
#endif  // HTTP packet on appliance (w/ ether header)



#if 0 // Streamer received HTTP packet on AVSM
1289834049.195273 ---- Received: 1019   (Transmitted: 67951) ------
Received packet #1019 size 72 0x48
  on adapter 0 00:22:bd:d4:53:83  20.4.1.2
AVSM header offset 0x000-0x020
  0x000 AVSM Header:
   000: 00 00 00 00 00 11 00 00 02 00 01 0c 00 00 05 00
   010: 81 00 80 81 00 00 01 00 00 04 40 00 00 00 00 00
IP header offset 0x020-0x034
  0x020 Version                 4 0x4 valid
  0x020 IP header length        5 0x5 valid
  0x021 DiffServ                0 0x0  dscp 0 0x0  ecn 0 0x0
  0x022 Total length            40 0x28
  0x024 Fragment id             0 0x0
  0x026 Fragment offset         0 0x0  Flags 0x2  Do not fragment
  0x028 Time to live            127 0x7f
  0x029 Protocol type           6 0x6  TCP
  0x02a Header checksum         0xc8c3 valid
  0x02c Source IP address       20.0.15.2
  0x030 Destination IP address  20.4.1.2
TCP header offset 0x034-0x048   Length 20 0x14
  0x034 Source port             80 0x50   HTTP
  0x036 Destination port        49152 0xc000
  0x038 Sequence number         0 0x0
  0x03c Acknowledgement number  165224081 0x9d91e91
  0x040 Data offset             5 0x5
  0x040 Reserved                0 0x0
  0x041 Control Bits            20 0x14
    cwr ece urg ACK psh RST syn fin
  0x042 Window Size             65535 0xffff
  0x044 Checksum                36622 0x8f0e
  0x046 Urgent Pointer          0 0x0
  0x048-0x048 Options
HTTP Data 0x0048-0x0048
0000: 00 00 00 00 00 11 00 00 02 00 01 0c 00 00 05 00
0010: 81 00 80 81 00 00 01 00 00 04 40 00 00 00 00 00
0020: 45 00 00 28 00 00 40 00 7f 06 c3 c8 14 00 0f 02
0030: 14 04 01 02 00 50 c0 00 00 00 00 00 09 d9 1e 91
0040: 50 14 ff ff 8f 0e 00 00


#endif // Streamer received HTTP packet on AVSM
