
#include  "../aecp-aem-stream-info.h"
#include  "../aecp-aem-state.h"
#include  "../aecp-aem.h"
#include  "../aecp-aem-descriptors.h"
#include  "../aecp.h"
#include  "../utils.h"


#include "aecp-aem-cmd-get-stream-info.h"
#include "aecp-aem-helpers.h"
#include "aecp-aem-types.h"
#include "aecp-aem-stream-info-common.h"


int aecp_aem_cmd_get_stream_info(struct aecp *aecp, int64_t now,
    const void *m, int len)
{
    uint8_t buf[256];
    const struct avb_ethernet_header *h = m;
    const struct avb_packet_aecp_aem *p = SPA_PTROFF(h, sizeof(*h), void);
    struct avb_packet_aecp_aem_setget_stream_info *sinf;
    // This is used to align the structure to 8 bytes
    struct avb_packet_aecp_aem_setget_stream_info aligned_sinf;
    struct aecp_aem_stream_info_state sinf_state = {0};
    struct avb_ethernet_header *h_reply = (struct avb_ethernet_header *)buf;
    struct avb_packet_aecp_aem *p_reply = SPA_PTROFF(h_reply, sizeof(*h_reply),
         void);

    struct descriptor *desc;
    struct avb_aem_desc_stream *desc_stream;
    uint16_t desc_index;
    uint16_t desc_type;
    uint16_t ctrl_data_length;
    uint16_t index_state;
    uint32_t flags = 0;
    size_t added_size;
    int rc = 0;

    /* Unaligned access happend sometimes, so we need to align the structure */
    memcpy(&aligned_sinf, p->payload, sizeof(aligned_sinf));
    sinf = (struct avb_packet_aecp_aem_setget_stream_info *)&aligned_sinf;

    ctrl_data_length = AVB_PACKET_GET_LENGTH(&p->aecp.hdr);
    desc_type = ntohs(sinf->descriptor_type);
    desc_index = ntohs(sinf->descriptor_index);

    desc = server_find_descriptor(aecp->server, desc_type, desc_index);
    if (desc == NULL) {
        pw_log_error("Could not find the descriptor id %d type %d\n",
            desc_index, desc_type);
        return reply_bad_arguments(aecp, m, len);
    }

    rc = get_index_from_descriptor(aecp, desc_index, desc_type, &index_state);
    if (rc) {
        pw_log_error("Could not get the index from the descriptor id %d type %d\n",
            desc_index, desc_type);
        spa_assert(0);
    }

    rc = aecp_aem_get_state_var(aecp, aecp->server->entity_id,
        aecp_aem_stream_info, index_state, &sinf_state);
    if (rc) {
        pw_log_error("Could not get the stream info\n");
        spa_assert(0);
    }

    /* Prepapre the reply */
    memset(buf, 0, sizeof(buf));
    memcpy(buf, m, len);

    /** Reset the structure, it was used before */
    memset(&aligned_sinf, 0, sizeof(aligned_sinf));
    sinf = (struct avb_packet_aecp_aem_setget_stream_info *)&aligned_sinf;

    // Milan v1.2 Clause 5.4.2.10 GET_STREAM_INFO
    if (desc_type == AVB_AEM_DESC_STREAM_INPUT) {
        /** Milan v1.2 Clause 5.4.2.10.1 Requirements for a Stream Input */
        flags = htonl(
            AVB_AEM_STREAM_INFO_GET_MASK(AVB_AEM_STREAM_INFO_FLAG_FAST_CONNECT));

        if (sinf_state.connected) {
            flags |= htonl(
                AVB_AEM_STREAM_INFO_GET_MASK(AVB_AEM_STREAM_INFO_FLAG_SAVED_STATE));
        }

        if (sinf_state.streaming_wait) {
            flags |= htonl(
                AVB_AEM_STREAM_INFO_GET_MASK(AVB_AEM_STREAM_INFO_FLAG_STREAMING_WAIT));
        }
    }
    desc_stream = (struct avb_aem_desc_stream *)desc->ptr;

    flags |= htonl(
        AVB_AEM_STREAM_INFO_GET_MASK(AVB_AEM_STREAM_INFO_FLAG_STREAM_FORMAT_VALID));
    sinf->stream_format = htobe64(desc_stream->current_format);

    /** TODO FIXME, this expects that the value is set to 0 when not used, and the correct
     * value when used */
    // Milan v1.2 Clause 5.4.2.10 GET_STREAM_INFO called REGISTERING_FAILED
    if (sinf_state.talker_failed) {
        flags |= htonl(
            AVB_AEM_STREAM_INFO_GET_MASK(AVB_AEM_STREAM_INFO_FLAG_TALKER_FAILED));

            sinf->msrp_failure_bridge_id = htobe64(sinf_state.msrp_failure_bridge_id);
    }

    if (sinf_state.stream_vlan_id) {
        flags |= htonl(
            AVB_AEM_STREAM_INFO_GET_MASK(AVB_AEM_STREAM_INFO_FLAG_STREAM_VLAN_ID_VALID));
        sinf->stream_vlan_id = htons(sinf_state.stream_vlan_id);
    }

    // Milan v1.2 Clause 5.4.2.10 GET_STREAM_INFO called bound
    if (sinf_state.connected) {
        flags |= htonl(
            AVB_AEM_STREAM_INFO_GET_MASK(AVB_AEM_STREAM_INFO_FLAG_CONNECTED));
    }

    if (sinf->msrp_failure_code) {
        flags |= htonl(
            AVB_AEM_STREAM_INFO_GET_MASK(AVB_AEM_STREAM_INFO_FLAG_MSRP_FAILURE_VALID));
        sinf->msrp_failure_code = sinf_state.msrp_failure_code;
    }

    if (sinf->stream_dest_mac[0] || sinf->stream_dest_mac[1] ||
        sinf->stream_dest_mac[2] || sinf->stream_dest_mac[3] ||
        sinf->stream_dest_mac[4] || sinf->stream_dest_mac[5]) {
        // Setup the destination mac addresss
        flags |= htonl(
            AVB_AEM_STREAM_INFO_GET_MASK(AVB_AEM_STREAM_INFO_FLAG_STREAM_DEST_MAC_VALID));
        memcpy(sinf->stream_dest_mac, sinf_state.stream_dest_mac,
            sizeof(sinf->stream_dest_mac));
    }

    if (sinf_state.msrp_accumulated_latency) {
        flags |=htonl(
            AVB_AEM_STREAM_INFO_GET_MASK(AVB_AEM_STREAM_INFO_FLAG_MSRP_ACC_LAT_VALID));
        sinf->msrp_accumulated_latency =
            htonl(sinf_state.msrp_accumulated_latency);
    }

    if (sinf_state.stream_id) {
        flags |= htonl(
            AVB_AEM_STREAM_INFO_GET_MASK(AVB_AEM_STREAM_INFO_FLAG_STREAM_ID_VALID));
        sinf->stream_id = htobe64(sinf_state.stream_id);
    }

    sinf->aem_stream_info_flags = flags;

    if (sinf_state.flags_ex &  AVB_AEM_STREAM_INFO_FLAG_EX_REGISTERING) {
        sinf->milan.flag_ex = htonl(AVB_AEM_STREAM_INFO_FLAG_EX_REGISTERING);
    }

    sinf->milan.acmpsta = sinf_state.acmpsta;
    sinf->milan.pbsta = sinf_state.pbsta;

    added_size = sizeof(*sinf) - sizeof(sinf->descriptor_index)
        - sizeof(sinf->descriptor_type);

    AVB_PACKET_SET_LENGTH(&p_reply->aecp.hdr, ctrl_data_length + added_size);

    memcpy(p_reply->payload, sinf, sizeof(*sinf));
    return reply_success(aecp, buf, len + added_size);
}
