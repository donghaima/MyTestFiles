#include <stdio.h>
#include <stdint.h>
#include <asm/byteorder.h>
#include <assert.h>

#define CONFIG_AVSM 0


class MACAddress {
public:
    unsigned char addr[6];

    inline unsigned int macstart();
    inline unsigned short macend();
    inline void setstart(unsigned int start);
    inline void setend(unsigned short end);

    inline uint64_t get64();

    inline bool operator == (MACAddress a);

    inline bool operator != (MACAddress a) ;

    inline bool iszero();

    inline bool isnotzero();

    inline void setaddr(uint64_t mac64);

    inline MACAddress();
};

#pragma pack(1)

class MACHeader {
public:
    MACAddress destination;
    MACAddress source;
    unsigned short type;
};


// MACOVERHEAD attempts to account for other overhead associated with the MAC such as SFD, EFD, FCS, IPG and padding
// These bytes effectively comsume wire bandwidth and this is an approximate accounting. Approxmate because
// some device extend the effective size of the SFD, EFD and/or IPG and padding varies.
#if !CONFIG_AVSM
#define MACHEADERSIZE (sizeof(MACHeader))
#else
#define MACHEADERSIZE (sizeof(AvsmHeader))
#endif // !CONFIG_AVSM

#define MACOVERHEAD (22)

#define MACTOTAL (MACOVERHEAD + MACHEADERSIZE)
#define ETHDEFAULTPDU 1514
#define MACDEFAULTPDU (ETHDEFAULTPDU + MACOVERHEAD)



#define CHUNKHEADERSIZE 8  

#if CONFIG_AVSM
#define UNUSEDPADSIZE      46
#else
#define UNUSEDPADSIZE      0
#endif  // CONFIG_AVSM

#define HTTPSIZE (sizeof(Header) + CHUNKHEADERSIZE)


#define CACHELINESIZE 64
#define CACHELINEMASK (CACHELINESIZE - 1)
#define PAGESIZE 8192
#define PAGEMASK (PAGESIZE - 1)


class IPv4Header {
public:
    unsigned char version_ihl;
    unsigned char tos;
    unsigned short length;
    unsigned short identification;
    unsigned short flags_offset;
    unsigned char ttl;
    unsigned char protocol;
    volatile unsigned short checksum;
    unsigned int source;
    unsigned int destination;

    static unsigned char s_httpTOS;

    unsigned short compute_checksum();

    void set_checksum();

    void set_tos();
};

class TCPHeader {
public:
    unsigned short source;
    unsigned short destination;
    unsigned int sequence;
    unsigned int acknowledge;
    unsigned char offset_res;
    unsigned char flags;
    unsigned short window;
    unsigned short checksum;
    unsigned short urgent_ptr;
};


class SynOptions {
public:
    unsigned short mss;
    unsigned char window_scale;
    unsigned char sack;

    inline SynOptions ();
};


class AvsmHeader
{
   //--------------------------------------------------------------------------
   // Public enumerations
public:
   enum 
   {
     AVSM_HEADER_SIZE = 32,
   };

   //--------------------------------------------------------------------------
   // Public static methods
public:
  
private:
  unsigned char m_avsmHeader[AVSM_HEADER_SIZE];
};




class Header {
public:
#if CONFIG_AVSM
    AvsmHeader RBEH;
#else
    MACHeader MAC;
#endif // CONFIG_AVSM
    IPv4Header IPv4;
    TCPHeader TCP;

    inline void* operator new(size_t num_bytes);

    inline void operator delete (void *ptr);
};

class AlignedHeader {
public:
    Header header;
    unsigned char chunk[CHUNKHEADERSIZE];
    unsigned char pad;
    volatile unsigned char pad2;
    unsigned char unused[UNUSEDPADSIZE];

    inline void* operator new(size_t num_bytes);

};


#define HEADERSPERPAGE (PAGESIZE / sizeof(AlignedHeader))

class HeaderPage {
public:
    AlignedHeader headers[HEADERSPERPAGE];

    inline void* operator new(size_t num_bytes);

    inline void operator delete (void *ptr);

    inline HeaderPage();

    inline HeaderPage(AlignedHeader* header);

    inline static AlignedHeader* inc_header(AlignedHeader* current_header);
};

#define HEADERPAGES (MAXTCPSESSIONS / HEADERSPERPAGE)
#define HEADERMASK (HEADERSPERPAGE - 1)
#define HEADERSHIFT 7

class HTTPBuffer {
public:
    AlignedHeader hdr;
    unsigned char http[1536 - sizeof(AlignedHeader)];
    volatile unsigned char sent;
    volatile unsigned char formatted;
    unsigned char pad[510];
};

#define HTTPPERPAGE (PAGESIZE / sizeof(HTTPBuffer))

class HTTPPage {
public:
    HTTPBuffer headers[HTTPPERPAGE];

    inline void* operator new(size_t num_bytes);

    inline void operator delete (void *ptr);

    inline HTTPPage(AlignedHeader* header);
};


#define PACKETOVERHEAD (sizeof(Header) + MACOVERHEAD)
#define HEADERSPERPAGE (PAGESIZE / sizeof(AlignedHeader))

#define HTTPHEADERSIZE 960

#define MAXTCPDATA (PAGESIZE - sizeof(AlignedHeader))
#define ASAPSHIFT 3

#define REDIRALLOC ((PAGESIZE << ASAPSHIFT) + ((PAGESIZE << ASAPSHIFT) >> 6))

#define ASSERT assert

int main (void)
{
  printf("CONFIG_AVSM = %d\n", CONFIG_AVSM);
  printf("CHUNKHEADERSIZE = %d\n", CHUNKHEADERSIZE);
  printf("MACOVERHEAD = %d\n", MACOVERHEAD);
  printf("MACTOTAL = %d\n", MACTOTAL);
  printf("PACKETOVERHEAD = %d\n", PACKETOVERHEAD);

  printf("\n");
  printf("sizeof MACHeader  = %d\n", sizeof(MACHeader));
  printf("sizeof IPv4Header = %d\n", sizeof(IPv4Header));
  printf("sizeof TCPHeader  = %d\n", sizeof(TCPHeader));
  printf("sizeof AvsmHeader = %d\n", sizeof(AvsmHeader));

  printf("\n");
  printf("sizeof Header  = %d\n", sizeof(Header));
  printf("sizeof AlignedHeader  = %d\n", sizeof(AlignedHeader));
  printf("PAGESIZE = %d\n", PAGESIZE);
  printf("HEADERSPERPAGE (PAGESIZE / sizeof(AlignedHeader)) = %d\n",
         HEADERSPERPAGE);
  printf("MAXTCPDATA = PAGESIZE - sizeof(AlignedHeader) = %d\n", MAXTCPDATA);

  printf("sizeof(HTTPBuffer) = %d\n", sizeof(HTTPBuffer));
  printf("HTTPPERPAG = (PAGESIZE / sizeof(HTTPBuffer)) = %d\n", HTTPPERPAGE);
  printf("HTTPSIZE = (sizeof(Header) + CHUNKHEADERSIZE) = %d\n", HTTPSIZE);

    ASSERT(sizeof(MACHeader) == 14);
    ASSERT(sizeof(IPv4Header) == 20);
    ASSERT(sizeof(TCPHeader) == 20);
#if CONFIG_AVSM
    ASSERT(sizeof(Header) == 72);   // Need to align to the cacheline??
#else
    ASSERT(sizeof(Header) == 54);
    ASSERT(sizeof(Header) <= CACHELINESIZE);
    ASSERT(sizeof(AlignedHeader) == CACHELINESIZE);
#endif  // CONFIG_AVSM

    //ASSERT(sizeof(TransmitDescriptor) == 8);
    ASSERT(sizeof(HTTPBuffer) == PAGESIZE / 4);
    //ASSERT(sizeof(DescriptorPage) == PAGESIZE);
    ASSERT(sizeof(HeaderPage) == PAGESIZE);
    ASSERT(sizeof(HTTPPage) == PAGESIZE);
    //ASSERT(sizeof(MasterSessionPage) == PAGESIZE);
    //ASSERT((1<<MASTERSHIFT) == MASTERPERPAGE);
    //ASSERT(sizeof(BufferPage) == PAGESIZE);
    //ASSERT(sizeof(bw_remote_status) <= PAGESIZE);
    //ASSERT(sizeof(bw_header)%4 == 0);

  return 0;
}


#if 0
[doma@bxb-cds-005:/ws/doma-bxb/test]$ ./cds_tcp_hdr
CONFIG_AVSM = 1
CHUNKHEADERSIZE = 8
MACOVERHEAD = 22
MACTOTAL = 54
PACKETOVERHEAD = 94

sizeof MACHeader  = 14
sizeof IPv4Header = 20
sizeof TCPHeader  = 20
sizeof AvsmHeader = 32

sizeof Header  = 72
sizeof AlignedHeader  = 128
PAGESIZE = 8192
HEADERSPERPAGE (PAGESIZE / sizeof(AlignedHeader)) = 64
MAXTCPDATA = PAGESIZE - sizeof(AlignedHeader) = 8064
sizeof(HTTPBuffer) = 2048
HTTPPERPAG = (PAGESIZE / sizeof(HTTPBuffer)) = 4
HTTPSIZE = (sizeof(Header) + CHUNKHEADERSIZE) = 80

doma@bxb-cds-005:/ws/doma-bxb/test]$ vi cds_tcp_hdr.cpp
[doma@bxb-cds-005:/ws/doma-bxb/test]$ g++ -g -o cds_tcp_hdr cds_tcp_hdr.cpp
[doma@bxb-cds-005:/ws/doma-bxb/test]$ ./cds_tcp_hdr
CONFIG_AVSM = 0
CHUNKHEADERSIZE = 8
MACOVERHEAD = 22
MACTOTAL = 36
PACKETOVERHEAD = 76

sizeof MACHeader  = 14
sizeof IPv4Header = 20
sizeof TCPHeader  = 20
sizeof AvsmHeader = 32

sizeof Header  = 54
sizeof AlignedHeader  = 64
PAGESIZE = 8192
HEADERSPERPAGE (PAGESIZE / sizeof(AlignedHeader)) = 128
MAXTCPDATA = PAGESIZE - sizeof(AlignedHeader) = 8128
sizeof(HTTPBuffer) = 2048
HTTPPERPAG = (PAGESIZE / sizeof(HTTPBuffer)) = 4
HTTPSIZE = (sizeof(Header) + CHUNKHEADERSIZE) = 62

#endif
