#ifndef __AECP_AEM_CMD_ADDREM_AUDIO_MAPPING_HELPER_H__
#define __AECP_AEM_CMD_ADDREM_AUDIO_MAPPING_HELPER_H__

#include <stdint.h>
#include <stdlib.h>
#include "aecp-aem-types.h"
#include "aecp-aem-unsol-helper.h"

#include "../aecp-aem-mappings.h"
#include "../aecp-aem-state.h"

static inline int aecp_aem_unsol_addrem_mapping(struct aecp *aecp, uint16_t type)
{
    // Milan authorize a way to go beyon the 512 bytes for a AECP packet
    uint8_t buf[2048];
    struct aecp_aem_dynamic_audio_mappings_state dyn_maps_st = {0};
    struct avb_ethernet_header *h = (struct avb_ethernet_header *) buf;
    struct avb_packet_aecp_aem *p = SPA_PTROFF(h, sizeof(*h), void);
    struct avb_packet_aecp_aem_addrem_mappings *amap =
        (struct avb_packet_aecp_aem_addrem_mappings *) p->payload;
    struct avb_aem_audio_mapping_format *formats;
    struct descriptor *desc;
    uint16_t addremove_mappings = 0;

    size_t len;
    int rc;

    rc = aecp_aem_get_state_var(aecp, aecp->server->entity_id,
            aecp_aem_dynamic_audio_mappings, 0, &dyn_maps_st);
    if (rc) {
        spa_assert(rc);
    }

    if (!dyn_maps_st.base_desc.base_info.needs_update) {
        return 0;
    }

    dyn_maps_st.base_desc.base_info.needs_update = false;
    memset(buf, 0, sizeof(buf));

    len = sizeof(*p) + sizeof(*h) + sizeof(*amap);

    pw_log_info("size of the buffer %u %u", dyn_maps_st.mappings_max_count , dyn_maps_st.mapping_free_count);
    if (len > sizeof(buf)) {
        pw_log_error("buffer space too small\n");
        spa_assert(0);
    }
    formats = &amap->mappings[0];

    for (size_t map_idx = 0; map_idx < (size_t)dyn_maps_st.mappings_max_count;
            map_idx++) {
        if (type == AVB_AECP_AEM_CMD_ADD_AUDIO_MAPPINGS) {
            if (dyn_maps_st.marked_as_added[map_idx]) {
                memcpy(formats, &dyn_maps_st.formats[map_idx], sizeof(*formats));
                formats++;
                dyn_maps_st.marked_as_added[map_idx] = false;
                addremove_mappings++;
            }
        } else if (type == AVB_AECP_AEM_CMD_REMOVE_AUDIO_MAPPINGS) {
            if (dyn_maps_st.marked_for_removal[map_idx]) {
                memcpy(formats, &dyn_maps_st.formats[map_idx], sizeof(*formats));
                formats++;
                dyn_maps_st.marked_for_removal[map_idx] = false;
                addremove_mappings++;
            }
        } else {
            spa_assert(0);
        }
    }

    len +=  addremove_mappings * sizeof( struct avb_aem_audio_mapping_format);

    desc = dyn_maps_st.base_desc.desc;
    amap->descriptor_id = htons(desc->index);
    amap->descriptor_type = htons(desc->type);
    amap->number_of_mappings = htons(addremove_mappings);
    AVB_PACKET_AEM_SET_COMMAND_TYPE(p, type);
    rc = reply_unsolicited_notifications(aecp, &dyn_maps_st.base_desc.base_info,
            buf, len, false);
    if (rc) {
        spa_assert(0);
    }
    rc = aecp_aem_refresh_state_var(aecp, aecp->server->entity_id,
                aecp_aem_dynamic_audio_mappings, 0, &dyn_maps_st);

    return rc;
}
#endif // __AECP_AEM_CMD_ADDREM_AUDIO_MAPPING_HELPER_H__
