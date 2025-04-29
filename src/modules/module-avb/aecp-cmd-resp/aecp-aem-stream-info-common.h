#ifndef __AECP_AECM_STREAM_INFO_COMMON_H__
#define __AECP_AECM_STREAM_INFO_COMMON_H__

#include "aecp-aem-cmd-resp-common.h"
#include "../descriptors.h"
#include "../aecp-aem-descriptors.h"

static inline int get_index_from_descriptor(struct aecp *aecp, uint16_t desc_id,
    uint16_t desc_type, uint16_t *index)
{
    struct descriptor *desc;
    struct avb_aem_desc_entity *entity_desc;
    struct avb_aem_desc_configuration *conf_desc;
    struct server *server = aecp->server;
    uint16_t desc_type_idx_offset = 0;

    if (desc_type != AVB_AEM_DESC_STREAM_OUTPUT) {
        *index = desc_id;
        return  0;
    }

    // TODO FIXME this is very hacky now we have to calculate th index using
    // The number of stream output descriptors and input descriptors...
    // This is not the right way to do it

    desc = server_find_descriptor(server, AVB_AEM_DESC_ENTITY, 0);
    if (desc == NULL)
        return -1;

    entity_desc = (struct avb_aem_desc_entity *)desc->ptr;
    desc = server_find_descriptor(server, AVB_AEM_DESC_CONFIGURATION,
                    ntohs(entity_desc->current_configuration));
    if (desc == NULL)
        return -1;

    conf_desc = (struct avb_aem_desc_configuration *)desc->ptr;

    for(int desc_idx = 0; desc_idx < ntohs(conf_desc->descriptor_counts_count); desc_idx++) {
        if (htons(conf_desc->descriptor_counts[desc_idx].descriptor_type)
                == AVB_AEM_DESC_STREAM_OUTPUT) {

            desc_type_idx_offset++;
        }
    }

    *index = desc_id + desc_type_idx_offset;

    return 0;
}

#endif //__AECP_AECM_STREAM_INFO_COMMON_H__