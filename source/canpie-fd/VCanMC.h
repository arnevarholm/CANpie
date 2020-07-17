// KONGSBERG PROPRIETARY. This document and its accompanying elements, contain KONGSBERG information
// which is proprietary and confidential. Any disclosure, copying, distribution or use is prohibited
// if not otherwise explicitly agreed with KONGSBERG in writing. Any authorized reproduction, in
// whole or in part, must include this legend. (c) KONGSBERG – All rights reserved.

#if !defined(VCANMC_H)
#define VCANMC_H

// MicroControl message definition
#include "cp_msg.h"

#define VCANMC_NET "192.32.23.0"
#define VCANMC_DEST "239.0.0.0"
#define VCANMC_PORT 62222

#define VCAN_SEGMENT 5  // Segment of Master and Slaves
#define VCAN_NODEID_MASTER 1
#define VCAN_NODEID_SLAVE  6

/*
 * Type of checksum of message
 */
#define CRC32_CHECKSUM 1

#pragma pack(push, 1)
typedef struct CpCanHdr_s
{ 
   uint32_t crc32;			 // CRC32 from (including) nodeId to end of message
   uint8_t nodeId;           // NodeId of sender if known, else 0
   uint8_t segment;          // CAN Segment number (RCU number)
   uint8_t from;             // From medium CAN=0, VCANMC=1
   uint8_t msgCount;         // Number of CpCanMsg
   uint8_t ECR_TEC;          // Error Counter Register: Transmit Error Counter
   uint8_t ECR_REC;          // Error Counter Register: Receive Error Counter
   uint8_t PSR_Bus_Off : 1;  // Protocol Status Register: Bus Off status
   uint8_t PSR_LEC : 3;      // Protocol status Register: Last error code
   uint8_t free1 : 4;        // Free for now, set to 0
   uint8_t free2;            // Free for now, set to 0
} CpCanHdr_ts;

//
// cpCanMsg.ulMsgUser
// Bit 0-3: CAN controller index, 4 bit (0-15)
typedef struct CpCanLanMsg_s
{
   CpCanHdr_ts cpCanHdr;
   CpCanMsg_ts cpCanMsg[1];
} CpCanLanMsg_ts;

// Max number of COBS in one single ethernet UDP frame is at least 1500 bytes
#define ETH_MTU 1500
#define UDP_HEADER_SIZE 8
#define UDP_PAYLOAD (ETH_MTU - UDP_HEADER_SIZE)
#define MAX_COBS ((UDP_PAYLOAD - sizeof(CpCanHdr_ts)) / sizeof(CpCanMsg_ts))

typedef struct CpCanLanMsgMax_s
{
   CpCanHdr_ts cpCanHdr;
   CpCanMsg_ts cpCanMsg[MAX_COBS];
} CpCanLanMsgMax_ts;


#define VCAN_MSGLEN(cobs) (sizeof(CpCanHdr_ts) + sizeof(CpCanMsg_ts) * cobs)

#pragma pack(pop)

#endif
