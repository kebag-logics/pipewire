#include "aecp-aem-lock-entity.h"
#include "aecp-aem.h"
#include "aecp-aem-descriptors.h"

static int reply_lock_entity(struct aecp *aecp, const void *m, int len, uint64_t locked_guid)
{
    struct server *server = aecp->server;
    uint8_t buf[len];
    struct avb_ethernet_header *h = (void*)buf;
    struct avb_packet_aecp_header *reply = SPA_PTROFF(h, sizeof(*h), void);
    struct avb_packet_aecp_aem_lock *lock_reply;

    // Copy incoming message to preserve original structure
    memcpy(buf, m, len);

    // LOCK_ENTITY uses the same AECPDU format for both command and response
    lock_reply = (struct avb_packet_aecp_aem_lock *)((struct avb_packet_aecp_aem *)reply)->payload;
    lock_reply->locked_guid = htobe64(locked_guid);  // Set the entity ID of the controller holding the lock

    return avb_server_send_packet(server, h->src, AVB_TSN_ETH, buf, len);
}

/* LOCK_ENTITY */
static int handle_lock_entity(struct aecp *aecp, const void *m, int len)
{
	#ifndef USE_MILAN
	struct server *server = aecp->server;
	const struct avb_packet_aecp_aem *p = m;
	const struct avb_packet_aecp_aem_acquire *ae;
	const struct descriptor *desc;
	uint16_t desc_type, desc_id;

	ae = (const struct avb_packet_aecp_aem_acquire*)p->payload;

	desc_type = ntohs(ae->descriptor_type);
	desc_id = ntohs(ae->descriptor_id);

	desc = server_find_descriptor(server, desc_type, desc_id);
	if (desc == NULL)
		return reply_status(aecp, AVB_AECP_AEM_STATUS_NO_SUCH_DESCRIPTOR, p, len);

	if (desc_type != AVB_AEM_DESC_ENTITY || desc_id != 0)
		return reply_not_implemented(aecp, m, len);

	return reply_success(aecp, m, len);
	#else // USE_MILAN
	struct server *server = aecp->server;
	const struct avb_packet_aecp_aem *p = m;
	const struct avb_packet_aecp_aem_acquire *ae;
	const struct descriptor *desc;
	uint16_t desc_type, desc_id;
	uint32_t flags;
	uint64_t controller_id;
	// Store the currently locked controller ID
	static uint64_t locked_id = 0;

    const struct avb_packet_aecp_aem_lock *le = (const struct avb_packet_aecp_aem_lock*)p->payload;
	controller_id = be64toh(le->locked_guid);
	flags = ntohl(le->flags);

	desc_type = ntohs(le->descriptor_type);
	desc_id = ntohs(le->descriptor_id);

	desc = server_find_descriptor(server, desc_type, desc_id);
	if (desc == NULL)
		return reply_status(aecp, AVB_AECP_AEM_STATUS_NO_SUCH_DESCRIPTOR, p, len);

	// Milan v1.2: Sec. 5.4.2.2 LOCK_ENTITY
	if (desc_type != AVB_AEM_DESC_ENTITY || desc_id != 0)
		return reply_not_supported(aecp, m, len);	

	// If no one as locked it, assign the current controller
	if (locked_id == 0)
	{
		locked_id = controller_id;
		// The locked_id field is set to zero (0) for a command, 
		// and is set to the Entity ID of the AVDECC Controller 
		// that is holding the lock in a response.
		return reply_lock_entity(aecp, m, len, locked_id);
	}
	// Entity locked but locking controller wants to UNLOCK
	else if (locked_id == controller_id) && (flags == 1) 
	{
		locked_id = 0;
		return reply_lock_entity(aecp, m, len, locked_id);
	}
	
	// The locked_id field is set to zero (0) for a command, and is set to the Entity ID
	// of the AVDECC Controller that is holding the lock in a response.
	// Controller locked
	return reply_lock_entity(aecp, m, len, locked_id);

	#endif //USE_MILAN
}