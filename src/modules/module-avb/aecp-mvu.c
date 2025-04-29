#include "aecp-mvu.h"
#include "aecp-cmd-resp/aecp-aem-helpers.h"
#include "utils.h"

static int reply_mvu_status(struct aecp *aecp, int status, const void *m,
    int len)
{
	struct server *server = aecp->server;
	uint8_t buf[2048];
	struct avb_ethernet_header *h = (void*)buf;
	struct avb_packet_aecp_milan_vendor_unique *reply = SPA_PTROFF(h, sizeof(*h), void);

	memcpy(buf, m, len);
	AVB_PACKET_AECP_SET_MESSAGE_TYPE(&reply->aecp,
        AVB_AECP_MESSAGE_TYPE_VENDOR_UNIQUE_RESPONSE);

    AVB_PACKET_AECP_SET_MESSAGE_TYPE(&reply->aecp,
            AVB_AECP_MESSAGE_TYPE_VENDOR_UNIQUE_RESPONSE);
	AVB_PACKET_AECP_SET_STATUS(&reply->aecp, status);

    /* Set reply */
    reply->r = 1;

	return avb_server_send_packet(server, h->src, AVB_TSN_ETH, buf, len);
}

static int reply_mvu_not_implemented(struct aecp *aecp, const void *m, int len)
{
	return reply_mvu_status(aecp, AVB_AECP_AEM_STATUS_NOT_IMPLEMENTED, m, len);
}

static int reply_mvu_not_supported(struct aecp *aecp, const void *m, int len)
{
	return reply_mvu_status(aecp, AVB_AECP_AEM_STATUS_NOT_SUPPORTED, m, len);
}

static int reply_mvu_success(struct aecp *aecp, const void *m, int len)
{
	return reply_mvu_status(aecp, AVB_AECP_AEM_STATUS_SUCCESS, m, len);
}

static int mvu_get_milan_info(struct aecp *aecp, int64_t now, const void *m,
    int len)
{
    uint8_t buf[128];
    struct avb_ethernet_header *h = (struct avb_ethernet_header *)buf;
    struct avb_packet_aecp_milan_vendor_unique *p = SPA_PTROFF(h, sizeof(*h), void);
    struct avb_packet_mvu_get_milan_info gmi = {0}, *gmi_cpy;
    gmi_cpy = (struct avb_packet_mvu_get_milan_info *) p->payload;
    uint16_t control_data_length;

    memset(buf, 0, sizeof(buf));
    memcpy(buf, m, len);

    control_data_length = AVB_PACKET_GET_LENGTH(&p->aecp.hdr);
    control_data_length += sizeof(gmi);

    /* This is fake */
    gmi.certification_version = AECP_MVU_GET_MILAN_INFO_CERT_VERSION;
    gmi.feature_flags = AECP_MVU_GET_MILAN_INFO_FLAGS;
    gmi.protocol_version = AECP_MVU_GET_MILAN_INFO_PROTO_VERSION;

    memcpy(gmi_cpy, &gmi, sizeof(gmi));

    len += sizeof(gmi);
    return reply_mvu_success(aecp, buf, len);
}

static int mvu_set_system_unique_id(struct aecp *aecp, int64_t now, const void *m,
    int len)
{
    return reply_mvu_not_supported(aecp, m, len);
}

static int mvu_get_system_unique_id(struct aecp *aecp, int64_t now, const void *m,
    int len)
{
    return reply_mvu_not_supported(aecp, m, len);
}

static int mvu_set_media_clk_ref_info(struct aecp *aecp, int64_t now, const void *m,
    int len)
{
    return reply_mvu_not_supported(aecp, m, len);
}

static int mvu_get_media_clk_ref_info(struct aecp *aecp, int64_t now, const void *m,
    int len)
{
    return reply_mvu_not_supported(aecp, m, len);
}

struct cmd_info {
	const char *name;
	const bool is_readonly;
	int (*handle_command) (struct aecp *aecp, int64_t now, const void *m,
		 int len);
	int (*handle_response) (struct aecp *aecp, int64_t now, const void *m,
		 int len);
};

#define AECP_AEM_HANDLE_CMD(cmd, readonly_desc, name_str, handle_exec)			\
	[cmd] = { .name = name_str, .is_readonly = readonly_desc, 	 				\
				.handle_command = handle_exec }

static const struct cmd_info mvu_cmd [] = {
    AECP_AEM_HANDLE_CMD(AECP_MILAN_VENDOR_UNIQUE_GET_MILAN_INFO, true,
        "milan-vu-get-info", mvu_get_milan_info),

    AECP_AEM_HANDLE_CMD(AECP_MILAN_VENDOR_UNIQUE_SET_SYSTEM_UNIQUE_ID, true,
        "milan-vu-set-system-unique-id", mvu_set_system_unique_id),

    AECP_AEM_HANDLE_CMD(AECP_MILAN_VENDOR_UNIQUE_GET_SYSTEM_UNIQUE_ID, true,
        "milan-vu-get-system-unique-id", mvu_get_system_unique_id),

    AECP_AEM_HANDLE_CMD(AECP_MILAN_VENDOR_UNIQUE_SET_MEDIA_CLOCK_REF_INFO, true,
        "milan-vu-set-clock-ref-info", mvu_set_media_clk_ref_info),

    AECP_AEM_HANDLE_CMD(AECP_MILAN_VENDOR_UNIQUE_GET_MEDIA_CLOCK_REF_INFO, true,
        "milan-vu-get-clock-ref-info", mvu_get_media_clk_ref_info)
};

int avb_aecp_vendor_unique_command(struct aecp *aecp, const void *m, int len)
{
	const struct avb_ethernet_header *h = m;
	const struct avb_packet_aecp_milan_vendor_unique *p = SPA_PTROFF(h, sizeof(*h), void);
	uint64_t mvu = AECP_VENDOR_UNIQUE_PROTO_ID(p);
    uint16_t cmd_type;

	pw_log_warn("MVU: Retrieve value of 0x%lx\n", (mvu));
    pw_log_warn("htobe64 MVU: Retrieve value of 0x%lx\n", htobe64(mvu));

	pw_log_warn("proto1: Retrieve value of 0x%x\n", (p->proto1));
	pw_log_warn("proto2: Retrieve value of 0x%x\n", (p->proto2));

    cmd_type = p->command_type;

    if (cmd_type > ARRAY_SIZE(mvu_cmd)) {
        pw_log_error("invalid command %d\n", cmd_type);
        return reply_mvu_not_implemented(aecp, m, len);
    }

	return mvu_cmd[cmd_type].handle_command(aecp, 0, p, len);
}

int avb_aecp_vendor_unique_response(struct aecp *aecp, const void *m, int len)
{
	return reply_not_supported(aecp, m, len);
}