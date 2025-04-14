/* AVB support */
/* SPDX-FileCopyrightText: Copyright © 2025 Kebag-Logic */
/* SPDX-FileCopyrightText: Copyright © 2025 Alex Malki <alexandre.malki@kebag-logic.com> */
/* SPDX-License-Identifier: MIT  */

#ifndef __AVB_AECP_AEM_HELPERS_H__
#define __AVB_AECP_AEM_HELPERS_H__


#include "aecp-aem-types.h"

static inline int reply_status(struct aecp *aecp, int status, const void *m, int len)
{
	struct server *server = aecp->server;
	uint8_t buf[len];
	struct avb_ethernet_header *h = (void*)buf;
	struct avb_packet_aecp_header *reply = SPA_PTROFF(h, sizeof(*h), void);

	memcpy(buf, m, len);
	AVB_PACKET_AECP_SET_MESSAGE_TYPE(reply, AVB_AECP_MESSAGE_TYPE_AEM_RESPONSE);
	AVB_PACKET_AECP_SET_STATUS(reply, status);

	return avb_server_send_packet(server, h->src, AVB_TSN_ETH, buf, len);
}

static inline int reply_not_implemented(struct aecp *aecp, const void *m, int len)
{
	return reply_status(aecp, AVB_AECP_AEM_STATUS_NOT_IMPLEMENTED, m, len);
}

static inline int reply_not_supported(struct aecp *aecp, const void *m, int len)
{
	return reply_status(aecp, AVB_AECP_AEM_STATUS_NOT_SUPPORTED, m, len);
}

static inline int reply_locked(struct aecp *aecp, const void *m, int len, uint64_t locked_id)
{
	uint8_t buf[len];
	struct avb_ethernet_header *h = (void*)buf;
	struct avb_packet_aecp_header *reply = SPA_PTROFF(h, sizeof(*h), void);
	struct avb_packet_aecp_aem *p_reply = (void*)reply;
	struct avb_packet_aecp_aem_lock *ae_reply;

	memcpy(buf, m, len);

	AVB_PACKET_AECP_SET_MESSAGE_TYPE(reply, AVB_AECP_MESSAGE_TYPE_AEM_RESPONSE);
	AVB_PACKET_AECP_SET_STATUS(reply, AVB_AECP_AEM_STATUS_ENTITY_LOCKED);

	ae_reply = (struct avb_packet_aecp_aem_lock*)p_reply->payload;
	/* IEEE 1722.1-2021, Sec. 7.4.2.1:
	* The locked_id field is set to zero (0) for a command, and is set to the
	* Entity ID of the ATDECC Controller that is holding the lock in a response.
	*/
	ae_reply->locked_guid = htobe64(locked_id);

	return avb_server_send_packet(aecp->server, h->src, AVB_TSN_ETH, buf, len);
}

static inline int reply_no_resources(struct aecp *aecp, const void *m, int len)
{
	return reply_status(aecp, AVB_AECP_AEM_STATUS_NO_RESOURCES, m, len);
}

static inline int reply_success(struct aecp *aecp, const void *m, int len)
{
	return reply_status(aecp, AVB_AECP_AEM_STATUS_SUCCESS, m, len);
}

#endif //__AVB_AECP_AEM_HELPERS_H__