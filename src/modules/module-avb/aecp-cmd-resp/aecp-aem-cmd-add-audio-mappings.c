
#include "../aecp-aem-mappings.h"
#include "../aecp-aem-state.h"
#include "../aecp-aem-descriptors.h"

#include "aecp-aem-cmd-addrem-audio-mappings-helper.h"
#include "aecp-aem-cmd-add-audio-mappings.h"
#include "aecp-aem-helpers.h"
#include "aecp-aem-types.h"

#define CHANNELS_FROM_MAPPINGS(x)   (((uint64_t)x>>22) & 0x3FF)

static struct avb_aem_audio_mapping_format *aecp_aem_retrieve_free_slot(
    struct aecp_aem_dynamic_audio_mappings_state *dyn_map_st
)
{
    for (int maps = 0; maps < dyn_map_st->mappings_max_count; maps++) {
        if (!dyn_map_st->format_slot_allocated[maps]) {
            dyn_map_st->mapping_free_count--;
            dyn_map_st->format_slot_allocated[maps] = true;
            return &dyn_map_st->formats[maps];
        }
    }

    return NULL;
}

int aecp_aem_cmd_add_audio_mappings(struct aecp *aecp, int64_t now,
    const void *m, int len)
{
    int rc;
    struct aecp_aem_dynamic_audio_mappings_state dyn_maps_st = {0};
	struct server *server = aecp->server;
	const struct avb_ethernet_header *h = m;
	const struct avb_packet_aecp_aem *p = SPA_PTROFF(h, sizeof(*h), void);
    struct avb_packet_aecp_aem_addrem_mappings *amap;
    struct descriptor *desc;
    struct avb_aem_audio_mapping_format *formats, *format_slot;
    struct avb_aem_desc_stream_port *stream_port;
    struct avb_aem_desc_stream *stream;

    /** Information in the packet */
    uint16_t desc_type;
    uint16_t desc_index;
    uint16_t maps_count;
    uint64_t ctrler_index;
    uint64_t stream_format;
    bool has_failed = false;

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

    stream_port = (struct avb_aem_desc_stream_port*) desc->ptr;
    dyn_maps_st.base_desc.desc = desc;

    if (stream_port->number_of_maps) {
        pw_log_warn("maps are statics for desc %d type %d", desc_index,
            desc_type);
        reply_bad_arguments(aecp, m, len);
    }

#ifdef USE_MILAN
// TODO this has to be moved to a common file, when the remove mappings exists
    if (desc_type == AVB_AEM_DESC_STREAM_PORT_INPUT) {
        /* Verify if the number of cluster or input is fine*/
        for (uint32_t mapping_idx = 0; mapping_idx < maps_count; mapping_idx++) {
            /** Check if the number of cluster is not over the max number */
            if ((ntohs(formats[mapping_idx].mapping_cluster_offset) + ntohs(formats[mapping_idx].mapping_cluster_channel))>=
                (ntohs((stream_port->base_cluster) + ntohs(stream_port->number_of_clusters)))) {
                pw_log_warn("Cluster offset does not fit in the base + number of cluster");
                has_failed = true;
                break;
            }

            /** Check if the cluster offiset is not below the base cluster*/
            if ((ntohs(formats[mapping_idx].mapping_cluster_offset) + ntohs(formats[mapping_idx].mapping_cluster_channel)) <
                ntohs(stream_port->base_cluster)) {
                pw_log_warn("Mapping of cluster is below the base cluster");
                has_failed = true;
                break;
            }

            /* now check if the stream exists and that the number of channels
                are correct */
            desc = server_find_descriptor(server, AVB_AEM_DESC_STREAM_INPUT,
                    ntohs(formats[mapping_idx].mapping_stream_index));
            if (desc == NULL) {
                has_failed = true;
                break;
            }
            stream = desc->ptr;
            // TODO PR stream format: when the stream format PR is merged use or
            // create a macro
            stream_format = htobe64(stream->current_format);
            /* Also asumuption is that there a stream is carried only by one frame*/
            if (ntohs(formats[mapping_idx].mapping_stream_channel) >=
                CHANNELS_FROM_MAPPINGS(stream_format)) {
                pw_log_warn("Invalid count of channels");
                has_failed = true;
                break;
            }
        }
    }
#endif //#ifdef USE_MILAN
    if (!dyn_maps_st.formats) {
        dyn_maps_st.formats =
            calloc(1, ntohs(stream_port->number_of_clusters) * sizeof(*formats));
        dyn_maps_st.format_slot_allocated =
            calloc(1, ntohs(stream_port->number_of_clusters)* sizeof(bool));
        dyn_maps_st.marked_for_removal =
            calloc(1, ntohs(stream_port->number_of_clusters)* sizeof(bool));
        dyn_maps_st.marked_as_added =
            calloc(1, ntohs(stream_port->number_of_clusters)* sizeof(bool));

        dyn_maps_st.mapping_free_count = ntohs(stream_port->number_of_clusters);
        dyn_maps_st.mappings_max_count = ntohs(stream_port->number_of_clusters);
    }

    if (dyn_maps_st.mapping_free_count < maps_count) {
        has_failed = true;
    }

    if (has_failed) {
        return reply_bad_arguments(aecp, m, len);
    }

    for (uint32_t mapping_idx = 0; mapping_idx < maps_count; mapping_idx++) {
        format_slot = aecp_aem_retrieve_free_slot(&dyn_maps_st);
        if (!format_slot) {
            return reply_bad_arguments(aecp, m, len);
        }
        dyn_maps_st.marked_as_added[mapping_idx] = true;
        memcpy(format_slot, &formats[mapping_idx], sizeof(*format_slot));
    }

    rc = aecp_aem_set_state_var(aecp, aecp->server->entity_id, ctrler_index,
                aecp_aem_dynamic_audio_mappings, 0, &dyn_maps_st);
    if (rc) {
        spa_assert(0);
    }
    return reply_success(aecp, m, len);
}

int aecp_aem_unsol_add_audio_mappings(struct aecp *aecp, int64_t now)
{
    return aecp_aem_unsol_addrem_mapping(aecp,
                                    AVB_AECP_AEM_CMD_ADD_AUDIO_MAPPINGS);
}
