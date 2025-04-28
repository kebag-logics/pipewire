#ifndef __AECP_AECM_CMD_SET_STREAM_INFO_H__
#define __AECP_AECM_CMD_SET_STREAM_INFO_H__

#include "aecp-aem-cmd-resp-common.h"

int aecp_aem_cmd_set_stream_info(struct aecp *aecp, int64_t now, const void *m,
    int len);

int aecp_aem_unsol_set_stream_info(struct aecp *aecp, int64_t now, const void *m,
    int len);

#endif //__AECP_AECM_CMD_SET_STREAM_INFO_H__