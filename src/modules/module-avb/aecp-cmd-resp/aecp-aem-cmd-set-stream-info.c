
#include "../aecp-aem-state.h"
#include "../descriptors.h"

#include "aecp-aem-helpers.h"
#include "aecp-aem-types.h"
#include "aecp-aem-unsol-helper.h"
#include "aecp-aem-cmd-set-stream-info.h"


int aecp_aem_cmd_set_stream_info(struct aecp *aecp, int64_t now, const void *m, int len)
{
    struct server *server = aecp->server;
    const struct avb_ethernet_header *h;
    const struct avb_packet_aecp_aem *p;
    struct aecp_aem_stream_info_state sinf_state = {0};
    struct descriptor *desc;

    struct avb_aem_desc_audio_unit *au;
    uint16_t desc_type;
    uint16_t desc_index;
    uint32_t sampling_rate;
    uint64_t ctrler_id;
    int rc;


    return 0;
}

int aecp_aem_unsol_set_stream_info(struct aecp *aecp, int64_t now, const void *m, int len)
{
    struct server *server = aecp->server;
    const struct avb_ethernet_header *h;
    const struct avb_packet_aecp_aem *p;
    struct aecp_aem_stream_info_state sinf_state = {0};
    struct descriptor *desc;

    struct avb_aem_desc_audio_unit *au;
    uint16_t desc_type;
    uint16_t desc_index;
    uint32_t sampling_rate;
    uint64_t ctrler_id;
    int rc;
    uint8_t buf[256];

//TODO
    return 0;
}