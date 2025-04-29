#ifndef __AECP_AECM_CMD_GET_STREAM_INFO_H__
#define __AECP_AECM_CMD_GET_STREAM_INFO_H__

#include "aecp-aem-cmd-resp-common.h"

int aecp_aem_cmd_get_stream_info(struct aecp *aecp, int64_t now,
    const void *m, int len);


#endif //__AECP_AECM_CMD_GET_STREAM_INFO_H__