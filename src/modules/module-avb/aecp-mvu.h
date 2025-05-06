#ifndef __AECP_MVU_H__
#define __AECP_MVU_H__

#include <stdio.h>
#include "aecp.h"


#define AECP_AVB_VENDOR_UNIQUE_PROTOCOL_ID_MILAN (0x001BC50AC100ULL)


/* Milan v1.2 Clause 5.4.3.2.1 protocol_id field */
#define AECP_MVU_MILAN_VENDOR_UNIQUE	((uint64_t)0x1bc50ac100)

/** These are hardcoded value for now */
/* Milan v1.2 Table 5.20: GET_MILAN */
#define AECP_MVU_GET_MILAN_INFO_PROTO_VERSION	(1)
#define AECP_MVU_GET_MILAN_INFO_CERT_VERSION	(0x01020000)
#define AECP_MVU_GET_MILAN_INFO_FLAGS			(0)


// Milan v1.2 Table 5.18
#define AECP_MILAN_VENDOR_UNIQUE_GET_MILAN_INFO             (0x0000)
#define AECP_MILAN_VENDOR_UNIQUE_SET_SYSTEM_UNIQUE_ID       (0x0001)
#define AECP_MILAN_VENDOR_UNIQUE_GET_SYSTEM_UNIQUE_ID       (0x0002)
#define AECP_MILAN_VENDOR_UNIQUE_SET_MEDIA_CLOCK_REF_INFO   (0x0003)
#define AECP_MILAN_VENDOR_UNIQUE_GET_MEDIA_CLOCK_REF_INFO   (0x0004)


struct avb_packet_mvu_get_milan_info {
	uint32_t protocol_version;
	uint32_t feature_flags;
	uint32_t certification_version;
} __attribute__ ((__packed__));

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