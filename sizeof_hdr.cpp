#include <stdio.h>
#include <stdint.h>
#include <asm/byteorder.h>

#define CONFIG_AVSM 1

#pragma pack(1)

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
#pragma pack()


#pragma pack(1)

class EthernetHeader
{
public:
   enum 
   {
      MAC_ADDRESS_LENGTH            = 6,

      // MAC_OVERHEAD attempts to account for other overhead associated with
      // the MAC such as SFD, EFD, FCS, IPG and padding.
      // These bytes effectively comsume wire bandwidth and this is an
      // approximate accounting. Approxmate because some device extend the
      // effective size of the SFD, EFD and/or IPG and padding varies.
      MAC_OVERHEAD                  = 22,
   };

public:
   unsigned char   m_destinationMacAddress[MAC_ADDRESS_LENGTH];
   unsigned char   m_sourceMacAddress[MAC_ADDRESS_LENGTH];
   unsigned short  m_ethernetProtocolType;            // packet type ID field
};


class IpHeader
{

public:
#if defined(__LITTLE_ENDIAN_BITFIELD)
   unsigned char     m_ipHeaderLength     : 4;
   unsigned char     m_ipVersion : 4;
#elif defined (__BIG_ENDIAN_BITFIELD)
   unsigned char     m_ipVersion : 4;
   unsigned char     m_ipHeaderLength     : 4;
#else
#error   "Please fix <asm/byteorder.h>"
#endif
   union
   {
      struct
      {
         unsigned char  m_explicitCongestionNotification : 2;
         unsigned char  m_differentiatedServicesCodePoint : 6;
      };
      unsigned char     m_diffServField;
   };
   unsigned short    m_totalLength;
   unsigned short    m_fragmentId;
   unsigned short    m_fragmentOffsetAndFlags;
   unsigned char     m_timeToLive;
   unsigned char     m_ipProtocolType;
   unsigned short    m_ipHeaderChecksum;
   unsigned int      m_sourceIpAddress;
   unsigned int      m_destinationIpAddress;
};

class UdpHeader
{

private:
   unsigned short   m_sourcePort;
   unsigned short   m_destinationPort;
   unsigned short   m_lengthInBytes;
   unsigned short   m_checksum;
};

#pragma pack()



#pragma pack(1)

struct StandardPacketHeader
{
#if CONFIG_AVSM
  AvsmHeader       m_avsmHeader;            // RBEH
#else
  EthernetHeader   m_ethernetHeader;
#endif  // CONFIG_AVSM
  IpHeader m_ipHeader;
  UdpHeader m_udpHeader;

  void dumpFirst64Bytes(char *name)
  {
    uint8_t *data = (uint8_t *)&this[0];

    printf("\nDump first 64 bytes for %s:\n", name);
    for (unsigned int i=0; i<64; i++) {
      if (!(i%16))
        printf("%s   %03x: ", i?"\n":"", i);
      printf("%02x ", data[i]);
    }
  }

};

#pragma pack()


#pragma pack(1)
// cache control protocol header
struct AlignedStandardPacketHeader : public StandardPacketHeader
{
  enum {
    PADDING_SIZE = 2,
  };
  char  m_padToAlignCalypsoHeader[PADDING_SIZE];
};

#pragma pack()

typedef unsigned int        handle_t;

class CalypsoHeader
{

  //--------------------------------------------------------------------------
  // Public data (TODO: make these private with accessors)
public:
  unsigned int    m_opcode;
  handle_t    m_messageId;  // connection specific--contains handle to CCP request
  union
  {
    struct
    {
         unsigned int   m_pathSequence : 16;
         unsigned int   m_pathIndex : 8;
         unsigned int   m_unused : 8;
      };
      unsigned int      m_pathFields;
   };
   unsigned int    m_length;     // length of following data (including this header)
};

// CHANGED from pack(4) to pack(1)
#pragma pack(1)
struct CalypsoPacketHeader : public AlignedStandardPacketHeader
{
    CalypsoHeader  m_calypsoHeader;
};
#pragma pack()



typedef unsigned int        ticks_t;
typedef uint64_t    goid_t;
typedef uint64_t    target_t;
typedef uint64_t    offset_t;
typedef unsigned int        ipAddress_t;
typedef unsigned short      ipPort_t;
typedef unsigned short      udpPort_t;
typedef unsigned int        megabitRate_t;
typedef unsigned int        kilobitRate_t;

typedef unsigned int        LengthInSectors_t;
typedef unsigned int        iooffset_t;

typedef unsigned int        TimeInMilliseconds_t;

typedef unsigned int        bitrate_t;
typedef unsigned int        rate_t;

#pragma pack(4)

class AdapterInfo
{
public:
  target_t         m_macAddress;           // this is in network order
  ipAddress_t      m_ipAddress;
  ipPort_t         m_streamUdpPort;
  udpPort_t        m_fillUdpPort;
  megabitRate_t    m_activeBandwidthInMegabits;

  AdapterInfo& operator = ( const AdapterInfo& a);
};
#pragma pack()


#define AND_MAX_NIC 16
#define MAX_NICS  AND_MAX_NIC


#pragma pack(1)

// HUGE WARNING!!! THIS PACKET CAN'T GROW BIGGER THAN A control 'cause of the way
// it stamps the packet with a signature!!

struct mp_hdr
{
  unsigned char      m_version;
  unsigned char      m_revision;

  static unsigned char getCurrentVersion();
  static unsigned char getCurrentRevision();
  static unsigned char getOldestSupportedVersion();
  static unsigned char getOldestSupportedRevision();
};


struct L3mp_hdr : public mp_hdr
{
  enum
    {
      // Can't talk to older multicast boxes
      MONITOR_VERSION = 2,
      MONITOR_REVISION = 1,
      OLDEST_SUPPORTED_MONITOR_VERSION = 2,
      OLDEST_SUPPORTED_MONITOR_REVISION = 1,
    };
  
  unsigned short     m_groupId;
  unsigned short     m_serverId;
  
  union
  {
    struct
    {
      unsigned short m_isVault : 1;
      unsigned short m_isStreamer : 1;
      unsigned short m_isCache : 1;
      unsigned short m_isGoingOffline : 1;
      unsigned short m_discardsUnsupportedRequests : 1;
    };
    unsigned short m_flags;
  };
  
  unsigned int      m_arrayId;
  goid_t        m_masterGoid;
  
  unsigned int      m_numberOfAdapters;
  AdapterInfo  m_adapterInfo[MAX_NICS];
  
  ticks_t       m_firstSecondsSince1970;
  ticks_t       m_startSecondsSince1970;
  ticks_t       m_lastStartSecondsSince1970;
  ticks_t       m_lastLastSecondsSince1970;
};

struct L3monitor_packet : public CalypsoPacketHeader
{
       L3mp_hdr         mpheader;
};

#pragma pack()




#pragma pack(4)     // CHANGED from pack(4) to pack(1)

// data transfer header
class DataHeader
{
   // Private data
private:
  goid_t              m_goid;
  iooffset_t          m_sectorOffset;
  LengthInSectors_t   m_lengthInSectors;
  handle_t            m_handle;
  unsigned int            m_raidSize : 4;   // target raid group size
  unsigned int            m_raidSequence : 4;  // target raid group sequence position
  unsigned int            m_unusedFlagBits : 8;
  unsigned int            m_serverID : 16;
};
#pragma pack()



#pragma pack(1)  // CHANGED from pack(4) to pack(1)

class DataPacketHeader : public CalypsoPacketHeader
{
   // Public data
public:
   DataHeader  m_dataHeader;
};

#pragma pack()

typedef uint32_t uint;

/** \struct WrappingUint
 * \brief 32 bit unsigned integer with comparison operators based on shortest distance
 *
 * 32 bit unsigned integer with comparison operators based on shortest distance.  For
 * example, 0xffffffff < 0 because it is shorter to get from 0xfffffff to 0 by increasing
 * than be decreasing.
 */
struct WrappingUint
{
   uint m_value;

   WrappingUint ()       : m_value (0) {}
   WrappingUint (uint n) : m_value (n) {}

   bool operator < (uint n)   { return diff (n) < 0; }
   bool operator <= (uint n)  { return diff (n) <= 0; }
   bool operator > (uint n)   { return diff (n) > 0; }
   bool operator >= (uint n)  { return diff (n) >= 0; }
   bool operator == (uint n)  { return diff (n) == 0; }
   bool operator != (uint n)  { return diff (n) != 0; }

   operator uint ()           { return m_value; }
   uint operator ++ ()        { return ++m_value; }
   WrappingUint& operator = (uint n)   { m_value = n; return *this; }

   int diff (uint b)          { return diff (m_value, b); }

   // returns  < 0 if a < b
   //          > 0 if a > b
   //            0 if a == b
   //
   static int diff (uint a, uint b)          { return (int) (a - b); }

   static bool lessThan (uint a, uint b)     { return diff (a, b) < 0; }
   static bool greaterThan (uint a, uint b)  { return diff (a, b) > 0; }
   static bool equal (uint a, uint b)        { return a == b; }
};

typedef WrappingUint PacketSequence_t;

// IOX::ControlPacket
//
// This class and those that follow are placed on the wire.  They must not be 
// Polymorph, etc.

#pragma pack(1)

class ControlPacket : public AlignedStandardPacketHeader
{
  // Public methods 

   //---------------------------------------------------------------------------
   // Protected data
   //
   protected:

   uint8_t           m_legacyOpcode;      // Must be CALYPSO_CONTROL2
   uint8_t           m_protocolLevel;     // Outgoing must be CURRENT_CTL_PROTOCOL_LEVEL
   union
   {
      struct
      {
         uint16_t    m_isAckOnly : 1;              // don't ack this packet
         uint16_t    m_killPartnerRequested : 1;   // don't ack this packet
      };
      uint16_t       m_flags;
   };
   
   uint16_t          m_payloadOffset;     // start of messages
   uint16_t          m_packetLength;      // total length, including aligned_std_hdr
   
   // Someday CN::GroupId_t and CN::ServerId_t will probably have to change to
   // be (at least) 32 bits.  We try to anticipate that day.
   //
   uint32_t          m_sourceGroupId;     // Group ID of the source server
   uint32_t          m_sourceServerId;    // Server ID of the source server
   uint32_t          m_sourceEpoch;       // source server's epoch
   uint32_t          m_destEpoch;         // source server's view of partner's epoch
   PacketSequence_t  m_packetSequence;    // this packet sequence
   PacketSequence_t  m_myDataAck;         // source server has seen partner's data to here, inclusive
   PacketSequence_t  m_remoteDataAck;     // source server has seen partner's acks to here, inclusive

   uint32_t          m_reserved1;         // initialized to 0
   uint32_t          m_reserved2;         // initialized to 0

   static uint       s_packetsSent;
   static uint       s_packetsResent;
};

#pragma pack()




#pragma pack(4)  // CHANGED from pack(4) to pack(1)

// cache control protocol (control info) header

#define  OBJECT_DOESNT_EXIST  0xffffffff
//#define  MAX_PAYLOAD_SIZE (1024*7)
#define  MAX_PAYLOAD_SIZE (1024*2)

extern char*   PriorityStreamNames[];
extern char*   OldPriorityStreamNames[];
extern char    oldToNewPriority[];

struct ccp_data
{
// 0x00
    unsigned short      m_opcode;     // opcode used for CCP
    unsigned short      __padding;    // 
    union
    {
       struct
       {
// 0x04
         iooffset_t      offset;     // target offset
// 0x08
         LengthInSectors_t      length;     // target length
// 0x0c
         ticks_t         deadline;   // target deadline
// 0x10
         rate_t          minbw;      // target min bw
// 0x14
         rate_t          maxbw;      // target max bw
// 0x18
          union
          {
            TimeInMilliseconds_t  startTime;               // request
            unsigned int      outgoingCapacity;                // response
          };
// 0x0c
          union
          {
            TimeInMilliseconds_t  maximumStartDelayTime;   // request
            iooffset_t    currentEndOffset;                // response
          };
// 0x20
          union
          {
            TimeInMilliseconds_t  burstAheadTime;         // request
            bitrate_t             bitrate;                // response
          };
// 0x24
         rate_t                burstTransferRate;      // request

// 0x28
          union
          {
            goid_t          target;                 // request target object
            iooffset_t      lengthOfObject;         // respone (locate; new CP only)
          };
// 0x30
          union
          {
             struct
             {
                unsigned int     extendedStreamPriority  : 4;  // REQ if non-zero then use this
                unsigned int     isDataDownload          : 1;  // RSP is this a data download object?
                unsigned int     isDecommissioned        : 1;  // RSP
                unsigned int     adjustStartOffsetToAllocationUnitBoundaries  : 1;  // REQ
                unsigned int     adjustEndOffsetToAllocationUnitBoundaries  : 1;  // REQ
                unsigned int     isCannotBeRepaired      : 1;  // RSP this object can not be repaired
                unsigned int     verifyObjectAvailable   : 1;  // REQ verify that a cache can indead fill an object
                unsigned int     objectAvailableRemotely : 1;  // RSP the server can get the object from a remote fill source
                unsigned int     isIGate                 : 1;
                unsigned int     skipBits                : 4;  // default zeroed; can be used later to extend
                unsigned int     fillFromServerId        : 16; // REQ if non-zero, then which server fill pull fills from
             };                                                       
             unsigned int  zeroFlags1;
          };
// 0x34
          unsigned int        realbitrate;
      //    CN::target_t        daddr;      // dest addr (can be master_goid)
// 0x38
          union
          {
             struct
             {
                unsigned int     raidSize          : 4;   // target raid group size
                unsigned int     raidSequence      : 4;  // target raid group sequence position
                unsigned int     raidFinalSectors  : 6;  // to note how many sectors in the final raid block
                unsigned int     streamPriority    : 2;  // is a low priority request
                unsigned int     lastByteOffset    : 9; // how much at end (or beginning of reverse) is skipped
                unsigned int     reverse           : 1; // is this a reverse object?
                unsigned int     isLengthOfObject  : 1;  // does this communicate the length of the object
                unsigned int     isCanceled        : 1;  // was this close response due to a cancel?
                unsigned int     noCapacity        : 1;  // one open read rate response, not available due to capacity problems
                unsigned int     isDamaged         : 1;  // set on locate response
                unsigned int     isBeingFilled     : 1;  // set on locate response
#define SUPPORT_KILL_PARTNERS 1
         #if SUPPORT_KILL_PARTNERS
                unsigned int     killYourself      : 1;
         #else
                unsigned int     flags             : 1; // for later
         #endif
             };
             unsigned int  zeroFlags;
          };
       };
    };

}; // ccp_data

#pragma pack()


/************************************************************************/
#pragma pack(1)  // XXX CHANGED from pack(4)

// cache control protocol (data) header
struct ccp_hdr
{
    unsigned int        transfer_id;    // source specific id
    unsigned int        sequence_id;
  offset_t        offset;
    unsigned int        length; // length of protocol data (including header)
  handle_t        response_id;
    unsigned short      server_id;
  unsigned short      __padding;    // explicit padding
    unsigned int        flags;          // may steal space from here

};
#pragma pack()



struct OldControlPacketHeader : public CalypsoPacketHeader
{
    ccp_hdr    m_ccpHeader;
    ccp_data   m_ccpData;
};


/////////////////////////////////////////////////////////////////////////////////////////////


#pragma pack(1)
struct kfa_rbih_hdr_t {
    // Word 0
    uint8_t ecpu_mcast;
    uint8_t vqi;
    uint16_t mgid;

    //Word 1
    uint8_t de_cos;
    uint8_t rsvd1_23_16;
    uint8_t nph_type;
    uint8_t flags_and_len; //lower 5 bits is length

    //Word 2
    uint32_t rsvd2;

    //Word 3
    uint16_t ethertype;
    uint8_t usr_pri;
    uint8_t fpga_flags;

    //7 more bytes
    uint8_t rsvd4[7];

    /* NP header extension follows. The length is variable based on
     * NPH_type:
     *  -IPv4/IPv6 Unicast: 9 bytes
     *  -IPv4/IPv6 Multicast: 5 bytes
     *  -L2 Unicast: 9 bytes
     *  -L2 Flood: 9 bytes
     *  -IPC: 19 bytes
     *  -MPLS: 5 bytes
     */
};

#pragma pack()


/////////////////////////////////////////////////////////////////////////////////////////////
uint8_t testPacket[] = 
  { 0x01, 0x02, 0x03, 0x05, 0x04, 00, 00, 0x02, 00, 0x01, 0x0c, 00, 00, 00, 00,
    0x81, 00, 0x80, 0x40, 00, 00, 00, 00, 00, 0x02, 0xc0, 00, 00, 00, 00, 00,
    0x45, 00, 0x01, 0x96, 00, 00, 0x40, 00, 0x0e, 0x11, 0x30, 0x4f, 0x0d, 0x04, 0x1d, 0x02,
    0x10, 0x01, 0x01, 0x02, 0xbe, 0xef, 0xbe, 0xdf, 0x01, 0x82, 00, 00, 0x12, 0x12, 0xc5, 0xc0, 0xff };

int main (void)
{
  printf("CONFIG_AVSM = %d\n", CONFIG_AVSM);  
  printf("sizeof EthernetHeader = %d\n", sizeof(EthernetHeader));
  printf("sizeof IpHeader       = %d\n", sizeof(IpHeader));
  printf("sizeof UdpHeader      = %d\n", sizeof(UdpHeader));
  printf("sizeof AvsmHeader     = %d\n", sizeof(AvsmHeader));

  printf("sizeof StandardPacketHeader        = %d\n", 
         sizeof(StandardPacketHeader));
  printf("sizeof AlignedStandardPacketHeader = %d\n",
         sizeof(AlignedStandardPacketHeader));

  printf("sizeof CalypsoHeader       = %d\n", sizeof(CalypsoHeader));
  printf("sizeof CalypsoPacketHeader = %d\n", sizeof(CalypsoPacketHeader));
  printf("sizeof L3mp_hdr = %d\n", sizeof(L3mp_hdr));
  printf("sizeof L3monitor_packet = %d\n", sizeof(L3monitor_packet));
  printf("sizeof DataHeader = %d\n", sizeof(DataHeader));
  printf("sizeof DataPacketHeader = %d\n", sizeof(DataPacketHeader));
  printf("sizeof ControlPacket = %d\n", sizeof(ControlPacket));

  printf("sizeof ccp_hdr = %d\n", sizeof(ccp_hdr));
  printf("sizeof ccp_data = %d\n", sizeof(ccp_data));
  printf("sizeof OldControlPacketHeader = %d\n", sizeof(OldControlPacketHeader));
  

  printf("sizeof(char) = %d, aligned padding size=%d\n", 
         sizeof(char), AlignedStandardPacketHeader::PADDING_SIZE);
  printf("sizeof(signed char) = %d, sizeof(unsigned char)=%d, sizeof(int)=%d\n", 
         sizeof(signed char), sizeof(unsigned char), sizeof(int));
  printf("sizeof(signed int) = %d, sizeof(unsigned int)=%d, sizeof(long)=%d, sizeof(ulong)=%d\n", 
         sizeof(int), sizeof(unsigned int), sizeof(long), sizeof(unsigned long));

  printf("\n");
  printf("sizeof(struct kfa_rbih_hdr_t) = %d\n", sizeof(struct kfa_rbih_hdr_t));

  printf("\nTest packet:\n");
  for (unsigned int i=0; i<sizeof(testPacket); i++) {
    if (!(i%16))
      printf("%s   %03x: ", i?"\n":"", i);
    printf("%02x ", testPacket[i]);
  }

  StandardPacketHeader *packet = (StandardPacketHeader *)testPacket;
  packet->dumpFirst64Bytes("StandardPacketHeader");

  L3monitor_packet *l3monPacket = (L3monitor_packet *)testPacket;
  l3monPacket->dumpFirst64Bytes("L3monitor_packet");

  DataPacketHeader *dataPacket = (DataPacketHeader *)testPacket;
  dataPacket->dumpFirst64Bytes("DataPacket");
}
