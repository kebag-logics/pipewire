#include "aecp-aem.h"
#include "aecp-aem-descriptors.h"
#include "aecp-state-vars.h"

#include "aecp-aem-lock-entity.h"

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

static int check_valid_descriptor(struct aecp *aecp, struct server *server,
	uint16_t desc_type, uint16_t desc_id,
	const struct avb_packet_aecp_aem *p, size_t len)
{
	const struct descriptor *desc = server_find_descriptor(server, desc_type, desc_id);
	if (desc == NULL) {
		return reply_status(aecp, AVB_AECP_AEM_STATUS_NO_SUCH_DESCRIPTOR, p, len);
	}

	// Milan v1.2: Sec. 5.4.2.2
	// Controller can only lock Entity descriptor. Else: not supported
	if (desc_type != AVB_AEM_DESC_ENTITY || desc_id != 0) {
		return reply_not_supported(aecp, p, len);
	}

	return 0;  // Success, descriptor is valid
}

/* LOCK_ENTITY */
int handle_lock_entity(struct aecp *aecp, const void *m, int len)
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
	uint16_t desc_type, desc_id;
	uint32_t flags;
	uint64_t controller_id;

	static struct aecp_aem_lock_state *lock_state = NULL;
	lock_state = (struct aecp_aem_lock_state*)aecp_aem_get_state_var(aecp, server->entity_id, aecp_aem_lock);

    const struct avb_packet_aecp_aem_lock *le = (const struct avb_packet_aecp_aem_lock*)p->payload;
	controller_id = be64toh(le->locked_guid);
	flags = ntohl(le->flags);

	desc_type = ntohs(le->descriptor_type);
	desc_id = ntohs(le->descriptor_id);

	// Scenario 1: Entity is unlocked
	if (!lock_state->is_locked)
	{
		// Check for valid descriptor
		int result = check_valid_descriptor(aecp, server, desc_type, desc_id, p, len);
        if ( result != 0)
		{
            return result;
        }

		// Set the lock
		lock_state->locked_id = controller_id;
		lock_state->is_locked = true;
		// TODO: Is this correct?
		lock_state->timestamp = (uint64_t)time(NULL);
		// The locked_id field is set to zero (0) for a command,
		// and is set to the Entity ID of the AVDECC Controller
		// that is holding the lock in a response.
		pw_log_debug("Entity is locked by %lu", controller_id);
		return reply_lock_entity(aecp, m, len, lock_state->locked_id);
	}
	// Scenario 2: Entity is locked but locking controller wants to unlock
	else if ((lock_state->locked_id == controller_id) && (flags == 1))
	{
		// Check for valid descriptor
		int result = check_valid_descriptor(aecp, server, desc_type, desc_id, p, len);
        if ( result != 0)
		{
            return result;
        }

		lock_state->locked_id = 0;
		lock_state->is_locked = false;
		// TODO: Is this correct?
		lock_state->timestamp = 0;
		pw_log_debug("Entity is unlocked by %lu", controller_id);
		return reply_lock_entity(aecp,	 m, len, lock_state->locked_id);
	}

	// Scenario 3: Entity is locked and requesting controller is not locking controller
	pw_log_debug("Entity locked by %lu. Request from %lu declined.", lock_state->locked_id, controller_id);
	return reply_lock_entity(aecp, m, len, lock_state->locked_id);

	#endif //USE_MILAN
}