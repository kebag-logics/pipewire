#include "aecp-aem-acquire-entity.h"
#include "aecp-aem.h"

/* ACQUIRE_ENTITY */
static int handle_acquire_entity(struct aecp *aecp, const void *m, int len)
{
	const struct avb_packet_aecp_aem *p = m;
#ifndef USE_MILAN
	const struct avb_packet_aecp_aem_acquire *ae;
	struct server *server = aecp->server;

	const struct descriptor *desc;
	uint16_t desc_type, desc_id;

	ae = (const struct avb_packet_aecp_aem_acquire*)p->payload;

	desc_type = ntohs(ae->descriptor_type);
	desc_id = ntohs(ae->descriptor_id);

	desc = server_find_descriptor(server, desc_type, desc_id);
	if (desc == NULL)
		return reply_status(aecp, AVB_AECP_AEM_STATUS_NO_SUCH_DESCRIPTOR, p, len);

#endif

#ifdef USE_MILAN
	return reply_not_supported(aecp, m, len);

#else // USE_MILAN
	if (desc_type != AVB_AEM_DESC_ENTITY || desc_id != 0)
		return reply_not_implemented(aecp, m, len);

	return reply_success(aecp, m, len);
#endif // USE_MILAN
}