#include "aecp-mvu.h"
#include "aecp-cmd-resp/aecp-aem-helpers.h"
#include "utils.h"

static int reply_mvu_status(struct aecp *aecp, int status, const void *m,
    int len)
{
	struct server *server = aecp->server;
	uint8_t buf[2048];
    char debug_str[1024];
	struct avb_ethernet_header *h = (void*)buf;
	struct avb_packet_aecp_milan_vendor_unique *reply = SPA_PTROFF(h, sizeof(*h), void);
    int rc;
    int written = 0;
	memcpy(buf, m, len);
	AVB_PACKET_AECP_SET_MESSAGE_TYPE(&reply->aecp,
        AVB_AECP_MESSAGE_TYPE_VENDOR_UNIQUE_RESPONSE);

	AVB_PACKET_AECP_SET_STATUS(&reply->aecp, status);

    pw_log_warn("Sending packet of size %d \n", len);

    for (int i = 0; i < len; i++) {
        rc = sprintf(&debug_str[written], "%c%02x",
                                 !(len % 16) ? '\n' : ':', ((uint8_t*)buf)[i]);
        if (rc < 0) {
            break;
        }
        written += rc;
    //    pw_log_error("[%u]%02x", i, buf[i]);
    }

    pw_log_error("%s", debug_str);

	return avb_server_send_packet(server, h->src, AVB_TSN_ETH, buf, len);
}

static int reply_mvu_bad_arguments(struct aecp *aecp, const void *m , int len)
{
    return reply_mvu_status(aecp, AVB_AECP_AEM_STATUS_BAD_ARGUMENTS, m, len);
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

    /* This is fake we do not have certification ahah */
    gmi.certification_version = htonl(AECP_MVU_GET_MILAN_INFO_CERT_VERSION);
    gmi.feature_flags = htonl(AECP_MVU_GET_MILAN_INFO_FLAGS);
    gmi.protocol_version = htonl(AECP_MVU_GET_MILAN_INFO_PROTO_VERSION);

    memcpy(gmi_cpy, &gmi, sizeof(gmi));

    len = len + sizeof(gmi);
    AVB_PACKET_SET_LENGTH(&p->aecp.hdr, control_data_length + sizeof(gmi));

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

    if (len < (int) (sizeof(*h) + sizeof(*p))) {
        pw_log_error("packet size too small %d\n", len);
        return reply_mvu_bad_arguments(aecp, m, len);
    }

	pw_log_info("Vendor unique 0x%lx\n", (mvu));
    cmd_type = htons(p->command_type);

    if (mvu == AECP_MVU_MILAN_VENDOR_UNIQUE) {
        if (cmd_type >= ARRAY_SIZE(mvu_cmd)) {
            pw_log_error("invalid command %d\n", cmd_type);
            return reply_mvu_not_implemented(aecp, m, len);
        }

        pw_log_info("Milan VU command %u\n", cmd_type);
        return mvu_cmd[cmd_type].handle_command(aecp, 0, m, len);
    } else {
        pw_log_warn("Unsupported Vendor Unique protocol_id 0x%lx\n", mvu);
    }

	return reply_mvu_not_supported(aecp, m, len);
}

int avb_aecp_vendor_unique_response(struct aecp *aecp, const void *m, int len)
{
	return reply_not_supported(aecp, m, len);
}