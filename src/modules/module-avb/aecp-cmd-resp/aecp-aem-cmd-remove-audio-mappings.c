
#include "../aecp-aem-mappings.h"
#include "../aecp-aem-state.h"
#include "../aecp-aem-descriptors.h"

#include "aecp-aem-cmd-remove-audio-mappings.h"
#include "aecp-aem-helpers.h"
#include "aecp-aem-types.h"
#include "aecp-aem-unsol-helper.h"

#define CHANNELS_FROM_MAPPINGS(x)   (((uint64_t)x>>22) & 0x3FF)

static int find_and_remove_mapping(
    struct aecp_aem_dynamic_audio_mappings_state *dyn_map_st,
    struct avb_aem_audio_mapping_format *formats,
    size_t formats_count
)
{
    size_t found_count = 0;
    int rc;

    pw_log_info("looking for a valid format number of format %ld\n", formats_count );
    /* First fine the mappings, remember if one of them is not found the whole set
       is not removed */
    for (size_t mapping_idx = 0; mapping_idx < formats_count; mapping_idx++) {
        for (size_t avl_mapping_idx = 0;
                avl_mapping_idx < (size_t)dyn_map_st->mappings_max_count; avl_mapping_idx++) {
            if (dyn_map_st->format_slot_allocated[avl_mapping_idx]) {
                rc = memcmp(&formats[mapping_idx],&dyn_map_st->formats[avl_mapping_idx],
                             sizeof(*formats));
                if (!rc) {
                    pw_log_warn("removing memory avl %ld map_idx %ld\n",
                        avl_mapping_idx, mapping_idx);
                    found_count++;
                    dyn_map_st->marked_for_removal[avl_mapping_idx] = true;
                }
            }
        }
    }

    for (size_t avl_mapping_idx = 0;
        avl_mapping_idx < (size_t) dyn_map_st->mappings_max_count; avl_mapping_idx++) {
        pw_log_info("updating for avb %ld\n", avl_mapping_idx);
        if (found_count == formats_count) {
            dyn_map_st->format_slot_allocated[avl_mapping_idx] = false;
            dyn_map_st->mapping_free_count++;
        }
        dyn_map_st->marked_for_removal[avl_mapping_idx] = false;
    }

    return (found_count == formats_count ? 0 : -1);
}

int aecp_aem_cmd_remove_audio_mappings(struct aecp *aecp, int64_t now,
    const void *m, int len)
{
    int rc;
    struct aecp_aem_dynamic_audio_mappings_state dyn_maps_st = {0};
	struct server *server = aecp->server;
	const struct avb_ethernet_header *h = m;
	const struct avb_packet_aecp_aem *p = SPA_PTROFF(h, sizeof(*h), void);
    struct avb_packet_aecp_aem_addrem_mappings *amap;
    struct descriptor *desc;
    struct avb_aem_audio_mapping_format *formats;
    struct avb_aem_desc_stream_port *stream_port;

    /** Information in the packet */
    uint16_t desc_type;
    uint16_t desc_index;
    uint16_t maps_count;
    uint64_t ctrler_index;

    amap = (struct avb_packet_aecp_aem_addrem_mappings *)p->payload;
    desc_type = ntohs(amap->descriptor_type);
    desc_index = ntohs(amap->descriptor_id);
    maps_count = ntohs(amap->number_of_mappings);
    ctrler_index = htobe64(p->aecp.controller_guid);
    formats = amap->mappings;

	desc = server_find_descriptor(server, desc_type, desc_index);
	if (desc == NULL)
		return reply_status(aecp, AVB_AECP_AEM_STATUS_NO_SUCH_DESCRIPTOR, m, len);

    rc = aecp_aem_get_state_var(aecp, aecp->server->entity_id,
            aecp_aem_dynamic_audio_mappings, 0, &dyn_maps_st);
    if (rc) {
        spa_assert(rc);
    }

    if (!dyn_maps_st.formats) {
        pw_log_warn("The mapping data was never created \n");
        reply_bad_arguments(aecp, m, len);
    }

    stream_port = (struct avb_aem_desc_stream_port*) desc->ptr;

    if (stream_port->number_of_maps) {
        pw_log_warn("maps are statics for desc %d type %d", desc_index,
            desc_type);
        return reply_bad_arguments(aecp, m, len);
    }

    if (ntohs(stream_port->number_of_clusters) < maps_count) {
        pw_log_warn("more maps to remove %d vs %d than held", maps_count,
            ntohs(stream_port->number_of_clusters));
        return reply_bad_arguments(aecp, m, len);
    }

    rc = find_and_remove_mapping(&dyn_maps_st, formats, maps_count);
    if (rc) {
        return reply_bad_arguments(aecp, m, len);
    }

    rc = aecp_aem_set_state_var(aecp, aecp->server->entity_id, ctrler_index,
                aecp_aem_dynamic_audio_mappings, 0, &dyn_maps_st);
    if (rc) {
        spa_assert(0);
    }
    return reply_success(aecp, m, len);
}

int aecp_aem_unsol_remove_audio_mappings(struct aecp *aecp, int64_t now)
{
    return 0;
}