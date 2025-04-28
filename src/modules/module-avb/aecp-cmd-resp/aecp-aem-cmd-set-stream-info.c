
#include "../aecp-aem-state.h"
#include "../aecp-aem-stream-info.h"

#include "aecp-aem-helpers.h"
#include "aecp-aem-types.h"
#include "aecp-aem-unsol-helper.h"
#include "aecp-aem-cmd-set-stream-info.h"
#include "aecp-aem-stream-info-common.h"


int aecp_aem_cmd_set_stream_info(struct aecp *aecp, int64_t now, const void *m,
    int len)
{
    struct aecp_aem_stream_info_state sinf_state = {0};
    const struct avb_ethernet_header *h = m;
    const struct avb_packet_aecp_aem *p = SPA_PTROFF(h, sizeof(*h), void);
    struct avb_packet_aecp_aem_setget_stream_info *sinf =
                    (struct avb_packet_aecp_aem_setget_stream_info*)p->payload;

    struct descriptor *desc;
    uint16_t desc_type;
    uint16_t desc_index;
    uint32_t flags;
    uint32_t msrp_accumulated_latency;
    uint16_t index_state = 0;
    uint64_t ctrler_id;
    int rc;

    flags = ntohl(sinf->aem_stream_info_flags);
    desc_type = ntohs(sinf->descriptor_type);
    desc_index = ntohs(sinf->descriptor_index);
    ctrler_id = htobe64(p->aecp.controller_guid);

    if (!(flags & AVB_AEM_STREAM_INFO_GET_MASK(AVB_AEM_STREAM_INFO_FLAG_MSRP_ACC_LAT_VALID)))
     {
        pw_log_error("MSRP is the only one supported for stream input %d\n",
             desc_index);
        return reply_not_supported(aecp, m, len);
    }

    if (desc_type != AVB_AEM_DESC_STREAM_OUTPUT) {
        pw_log_error("descriptor type %d is not supported\n", desc_type);
        return reply_not_supported(aecp, m, len);
    }

    desc = server_find_descriptor(aecp->server, desc_type, desc_index);
    if (desc == NULL) {
        pw_log_error("Could not find the descriptor id %d type %d\n",
            desc_index, desc_type);
        return reply_bad_arguments(aecp, m, len);
    }
    // TODO FIXME check if the stream is hidding IEEE 1722.1-2021 Clause 7.4.15.2

    rc = get_index_from_descriptor(aecp, desc_index, desc_type, &index_state);
    if (rc) {
        pw_log_error("Could not get the index from the descriptor id %d type %d\n",
            desc_index, desc_type);
        spa_assert(0);
    }

    rc = aecp_aem_get_state_var(aecp, aecp->server->entity_id,
        aecp_aem_stream_info, index_state, &sinf_state);
    if (rc) {
        pw_log_error("could not get the stream info\n");
        spa_assert(0);
    }

    msrp_accumulated_latency = ntohl(sinf->msrp_accumulated_latency);

    /* Milan v1.2 Clause 5.4.2.9 SET_STREAM_INFO */
    if (msrp_accumulated_latency > 0x7FFFFFF) {
        pw_log_error("msrp accumulated latency %d is not supported\n",
            msrp_accumulated_latency);
        return reply_bad_arguments(aecp, m, len);
    }

    sinf_state.msrp_accumulated_latency = msrp_accumulated_latency;
    sinf_state.base_desc.desc = desc;

    rc = aecp_aem_set_state_var(aecp, aecp->server->entity_id, ctrler_id,
            aecp_aem_stream_info, desc_index, &sinf_state);
    if (rc) {
        pw_log_error("could not set the stream info\n");
        spa_assert(0);
    }

    return reply_success(aecp, m, len);
}

int aecp_aem_unsol_set_stream_info(struct aecp *aecp, int64_t now)
{
    uint8_t buf[256];
    struct avb_ethernet_header *h = (struct avb_ethernet_header *)buf;
    struct avb_packet_aecp_aem *p = SPA_PTROFF(h, sizeof(*h), void);
    struct avb_packet_aecp_aem_setget_stream_info *sinf =
                    (struct avb_packet_aecp_aem_setget_stream_info*)p->payload;
    struct aecp_aem_stream_info_state sinf_state = {0};
    struct descriptor *desc;
    size_t len;
    uint16_t state_idx = 0;
    int rc = 0;

    len = sizeof(*p) + sizeof(*sinf) + sizeof(*h);
    while (!rc) {
        rc = aecp_aem_get_state_var(aecp, aecp->server->entity_id,
            aecp_aem_stream_info, state_idx, &sinf_state);

        if (sinf_state.base_desc.base_info.var_info.count >= state_idx){
            break;
        }

        state_idx++;
        if (rc) {
            break;
        }

        if (!sinf_state.base_desc.base_info.needs_update) {
            continue;
        }

        desc = (struct descriptor *)sinf_state.base_desc.desc;
        if (desc->type != AVB_AEM_DESC_STREAM_OUTPUT) {
            continue;
        }

        memset(buf, 0, sizeof(buf));
        sinf->descriptor_type = htons(desc->type);
        sinf->descriptor_index = htons(desc->index);
        sinf->aem_stream_info_flags = htonl(AVB_AEM_STREAM_INFO_FLAG_MSRP_ACC_LAT_VALID);
        sinf->msrp_accumulated_latency = htonl(sinf_state.msrp_accumulated_latency);

        AVB_PACKET_AEM_SET_COMMAND_TYPE(p, AVB_AECP_AEM_CMD_SET_NAME);

        rc = reply_unsolicited_notifications(aecp, &sinf_state.base_desc.base_info,
            buf, len, false);

        if (rc) {
            pw_log_error("Could not send the unsolicited notification\n");
            spa_assert(0);
        }

        rc = aecp_aem_refresh_state_var(aecp, aecp->server->entity_id,
            aecp_aem_stream_info, state_idx, &sinf_state);
        if (rc) {
            pw_log_error("Could not refresh the state var\n");
            spa_assert(0);
        }
    }

    return rc;
}