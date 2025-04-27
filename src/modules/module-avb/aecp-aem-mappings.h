#ifndef __AECP_AEM_MAPPINGS_H__
#define __AECP_AEM_MAPPINGS_H__

#include <stdint.h>

#define AVB_AEM_AUDIO_MAPPING_FORMAT_OFFSET (8)

/**
 * @ref: Mapping format 1722.1-2021 Table 7.33.
 * Generic to both the aecm and entity descriptor
 */
struct avb_aem_audio_mapping_format {
	uint16_t mapping_stream_index;
	uint16_t mapping_stream_channel;
	uint16_t mapping_cluster_offset;
	uint16_t mapping_cluster_channel;
} __attribute__ ((__packed__));

#endif //__AECP_AEM_MAPPINGS_H__