#ifndef __AECP_MVU_H__
#define __AECP_MVU_H__

#include <stdio.h>
#include "aecp.h"


#define AECP_AVB_VENDOR_UNIQUE_PROTOCOL_ID_MILAN (0x001BC50AC100ULL)

// Milan Specification 1.2 Table 5.18
#define AECP_MILAN_VENDOR_UNIQUE_GET_MILAN_INFO             (0x0000)
#define AECP_MILAN_VENDOR_UNIQUE_SET_SYSTEM_UNIQUE_ID       (0x0001)
#define AECP_MILAN_VENDOR_UNIQUE_GET_SYSTEM_UNIQUE_ID       (0x0002)
#define AECP_MILAN_VENDOR_UNIQUE_SET_MEDIA_CLOCK_REF_INFO   (0x0003)
#define AECP_MILAN_VENDOR_UNIQUE_GET_MEDIA_CLOCK_REF_INFO   (0x0004)

#define AVB_PACKET_AECP_MVU_SET_MESSAGE_TYPE(p,v)	(&(p)->r = v)

struct avb_packet_aecp_milan_vendor_unique {
	struct avb_packet_aecp_header aecp;
	uint16_t proto1;
    uint32_t proto2;
#if __BYTE_ORDER == __BIG_ENDIAN
	unsigned r:1;
	unsigned command_type:15;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	unsigned command_type:15;
	unsigned r:1;
#endif
	uint8_t payload[0];
} __attribute__ ((__packed__));


int avb_aecp_vendor_unique_command(struct aecp *aecp, const void *m, int len);
int avb_aecp_vendor_unique_response(struct aecp *aecp, const void *m, int len);

#endif // __AECP_MVU_H__