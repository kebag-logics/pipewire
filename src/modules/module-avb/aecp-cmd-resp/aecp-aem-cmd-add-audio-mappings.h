#ifndef __AECP_AEM_CMD_ADD_AUDIO_MAPPINGS_H__
#define __AECP_AEM_CMD_ADD_AUDIO_MAPPINGS_H__

#include "aecp-aem-helpers.h"

int aecp_aem_cmd_add_audio_mappings(struct aecp *aecp, int64_t now,
        const void *m, int len);

int aecp_aem_unsol_add_audio_mappings(struct aecp *aecp, int64_t now);

#endif // __AECP_AEM_CMD_ADD_AUDIO_MAPPINGS_H__