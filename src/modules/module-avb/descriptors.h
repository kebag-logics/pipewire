/* PipeWire */
/* SPDX-FileCopyrightText: Copyright © 2022 Wim Taymans */
/* SPDX-FileCopyrightText: Copyright © 2025 Kebag-Logic */
/* SPDX-FileCopyrightText: Copyright © 2025 Alexandre Malki <alexandre.malki@kebag-logic.com> */
/* SPDX-FileCopyrightText: Copyright © 2025 Simon Gapp <simon.gapp@kebag-logic.com> */
/* SPDX-License-Identifier: MIT */

#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

#include "adp.h"
#include "aecp-aem.h"
#include "aecp-aem-descriptors.h"
#include "aecp-aem-stream-format-defs.h"
#include "aecp-aem-controls.h"
#include "internal.h"
#include "descriptor-field-value-types.h"
#include "entity_model.h"

static inline void init_descriptors(struct server *server)
{
	// TODO PERSISTENCE: retrieve the saved buffers.
	/**************************************************************************************/
	/* IEEE 1722.1-2021, Sec. 7.2.12 - STRINGS Descriptor 
	* Up to 7 localized strings
	*/
	server_add_descriptor(server, AVB_AEM_DESC_STRINGS, 0,
			sizeof(struct avb_aem_desc_strings),
			&(struct avb_aem_desc_strings)
	{
		.string_0 = DSC_STRINGS_0_DEVICE_NAME,
		.string_1 = DSC_STRINGS_1_CONFIGURATION_NAME,
		.string_2 = DSC_STRINGS_2_MANUFACTURER_NAME,
		.string_3 = DSC_STRINGS_3_GROUP_NAME
	});

	/**************************************************************************************/
	/* IEEE 1722.1-2021, Sec. 7.2.11 - LOCALE Descriptor */
	server_add_descriptor(server, AVB_AEM_DESC_LOCALE, 0,
			sizeof(struct avb_aem_desc_locale),
			&(struct avb_aem_desc_locale)
	{
		.locale_identifier = DSC_LOCALE_LANGUAGE_CODE,
		.number_of_strings = htons(DSC_LOCALE_NO_OF_STRINGS),
		.base_strings = htons(DSC_LOCALE_BASE_STRINGS)
	});

	/**************************************************************************************/
	/* IEEE 1722.1-2021, Sec. 7.2.1 - ENTITY Descriptor */
	/* Milan v1.2, Sec. 5.3.3.1 */

	server_add_descriptor(server, AVB_AEM_DESC_ENTITY, 0,
			sizeof(struct avb_aem_desc_entity),
			&(struct avb_aem_desc_entity)
	{
		.entity_id = htobe64(DSC_ENTITY_MODEL_ENTITY_ID),
		.entity_model_id = htobe64(DSC_ENTITY_MODEL_ID),
		.entity_capabilities = htonl(DSC_ENTITY_MODEL_ENTITY_CAPABILITIES),

		.talker_stream_sources = htons(DSC_ENTITY_MODEL_TALKER_STREAM_SOURCES),
		.talker_capabilities = htons(DSC_ENTITY_MODEL_TALKER_CAPABILITIES),

		.listener_stream_sinks = htons(DSC_ENTITY_MODEL_LISTENER_STREAM_SINKS),
		.listener_capabilities = htons(DSC_ENTITY_MODEL_LISTENER_CAPABILITIES),

		.controller_capabilities = htons(DSC_ENTITY_MODEL_CONTROLLER_CAPABILITIES),

		.available_index = htonl(DSC_ENTITY_MODEL_AVAILABLE_INDEX),
		.association_id = htobe64(DSC_ENTITY_MODEL_ASSOCIATION_ID),

		.entity_name = DSC_ENTITY_MODEL_ENTITY_NAME,
		.vendor_name_string = htons(DSC_ENTITY_MODEL_VENDOR_NAME_STRING),
		.model_name_string = htons(DSC_ENTITY_MODEL_MODEL_NAME_STRING),
		.firmware_version = DSC_ENTITY_MODEL_FIRMWARE_VERSION,
		.group_name = DSC_ENTITY_MODEL_GROUP_NAME,
		.serial_number = DSC_ENTITY_MODEL_SERIAL_NUMBER,
		.configurations_count = htons(DSC_ENTITY_MODEL_CONFIGURATIONS_COUNT),
		.current_configuration = htons(DSC_ENTITY_MODEL_CURRENT_CONFIGURATION)
	});

	/**************************************************************************************/
	/* IEEE 1722.1-2021, Sec. 7.2.2 - CONFIGURATION Descriptor*/
	/* Milan v1.2, Sec. 5.3.3.2 */

	struct {
		struct avb_aem_desc_configuration desc;
		struct avb_aem_desc_descriptor_count descriptor_counts[DSC_CONFIGURATION_DESCRIPTOR_COUNTS_COUNT];
	} __attribute__ ((__packed__)) config =
	{
		{
		.object_name = DSC_CONFIGURATION_OBJECT_NAME,
		.localized_description = htons(DSC_CONFIGURATION_LOCALIZED_DESCRIPTION),
		.descriptor_counts_count = htons(DSC_CONFIGURATION_DESCRIPTOR_COUNTS_COUNT),
		// TODO: Does it work? Was the commented out lines, now replaced with hard coded value from IEEE.
		// .descriptor_counts_offset = htons(
		// 	4 + sizeof(struct avb_aem_desc_configuration)),
		// },
		.descriptor_counts_offset = htons(DSC_CONFIGURATION_DESCRIPTOR_COUNTS_OFFSET),
		},
		.descriptor_counts = {
			{ htons(AVB_AEM_DESC_AUDIO_UNIT), htons(DSC_CONFIGURATION_NO_OF_AUDIO_UNITS) },
			{ htons(AVB_AEM_DESC_STREAM_INPUT), htons(DSC_CONFIGURATION_NO_OF_STREAM_INPUTS) },
			{ htons(AVB_AEM_DESC_STREAM_OUTPUT), htons(DSC_CONFIGURATION_NO_OF_STREAM_OUTPUTS) },
			{ htons(AVB_AEM_DESC_AVB_INTERFACE), htons(DSC_CONFIGURATION_NO_OF_AVB_INTERFACES) },
			{ htons(AVB_AEM_DESC_CLOCK_DOMAIN), htons(DSC_CONFIGURATION_NO_OF_CLOCK_DOMAINS) },
			{ htons(AVB_AEM_DESC_CLOCK_SOURCE), htons(DSC_CONFIGURATION_NO_OF_CLOCK_SOURCES) },
			{ htons(AVB_AEM_DESC_CONTROL), htons(DSC_CONFIGURATION_NO_OF_CONTROLS) },
			{ htons(AVB_AEM_DESC_LOCALE), htons(DSC_CONFIGURATION_NO_OF_LOCALES) },
		}
	};
	server_add_descriptor(server, AVB_AEM_DESC_CONFIGURATION, 0,
			sizeof(config), &config);

	/* Second configuration*/
	struct {
		struct avb_aem_desc_configuration desc;
		struct avb_aem_desc_descriptor_count descriptor_counts[8];
	} __attribute__ ((__packed__)) config1 =
	{
		{
		.object_name = "Non - redundant - 79kHz",
		.localized_description = htons(1),
		.descriptor_counts_count = htons(8),
		.descriptor_counts_offset = htons(
			4 + sizeof(struct avb_aem_desc_configuration)),
		},
		.descriptor_counts = {
			{ htons(AVB_AEM_DESC_AUDIO_UNIT), htons(1) },
			{ htons(AVB_AEM_DESC_STREAM_INPUT), htons(2) },
			{ htons(AVB_AEM_DESC_STREAM_OUTPUT), htons(1) },
			{ htons(AVB_AEM_DESC_AVB_INTERFACE), htons(1) },
			{ htons(AVB_AEM_DESC_CLOCK_DOMAIN), htons(1) },
			{ htons(AVB_AEM_DESC_CLOCK_SOURCE), htons(3) },
			{ htons(AVB_AEM_DESC_CONTROL), htons(1) },
			{ htons(AVB_AEM_DESC_LOCALE), htons(1) },
		}
	};
	server_add_descriptor(server, AVB_AEM_DESC_CONFIGURATION, 1,
			sizeof(config), &config1);

	/**************************************************************************************/
	/* IEEE 1722.1-2021, Sec. 7.2.22 CONTROL Descriptor*/
	/* Milan v1.2, Sec. 5.3.3.10 */

	struct {
		struct avb_aem_desc_control desc;
		struct avb_aem_desc_value_format value_inf;
	} __attribute__ ((__packed__)) ctrl =
	{
		{
			.object_name = DSC_CONTROL_OBJECT_NAME,
			.localized_description = htons(DSC_CONTROL_LOCALIZED_DESCRIPTION),

			.block_latency = htons(DSC_CONTROL_BLOCK_LATENCY),
			.control_latency = htons(DSC_CONTROL_CONTROL_LATENCY),
			.control_domain = htons(DSC_CONTROL_CONTROL_DOMAIN),
			.control_value_type = htons(DSC_CONTROL_CONTROL_VALUE_TYPE),
			.control_type = htobe64(DSC_CONTROL_CONTROL_TYPE),
			.reset_time = htonl(DSC_CONTROL_RESET_TIME),
			// TODO: This is not specified in Table 7-38
			.descriptor_counts_offset = htons(
				4 + sizeof(struct avb_aem_desc_control)),
			.number_of_values = htons(1),
			.signal_type = htons(0xffff),
			.signal_index = htons(0),
			.signal_output = htons(0),
		},
		{
			.minimum = DSC_CONTROL_IDENTIFY_MIN,
			.maximum = DSC_CONTROL_IDENTIFY_MAX,
			.step = DSC_CONTROL_IDENTIFY_STEP,
			.default_value = DSC_CONTROL_IDENTIFY_DEFAULT_VALUE,
			.current_value = DSC_CONTROL_IDENTIFY_CURRENT_VALUE,
			.localized_description = htons(DSC_CONTROL_LOCALIZED_DESCRIPTION),
		}
	};
	server_add_descriptor(server, AVB_AEM_DESC_CONTROL, 0,
			sizeof(ctrl), &ctrl);

	/**************************************************************************************/
	/* IEEE 1722.1-2021, Sec. 7.2.19 AUDIO_MAP Descriptor */
	/* Milan v1.2, Sec. 5.3.3.9 */
	
	struct {
		struct avb_aem_desc_audio_map desc;
		struct avb_aem_audio_mapping_format maps[DSC_AUDIO_MAPS_NO_OF_MAPPINGS];
	} __attribute__((__packed__)) maps_input = {
		.desc = {
			.mapping_offset = htons(AVB_AEM_AUDIO_MAPPING_FORMAT_OFFSET),
			.number_of_mappings = htons(DSC_AUDIO_MAPS_NO_OF_MAPPINGS),
		},
	};

	for (uint32_t map_idx = 0; map_idx < DSC_AUDIO_MAPS_NO_OF_MAPPINGS; map_idx++) {
		maps_input.maps[map_idx].mapping_stream_index    = htons(DSC_AUDIO_MAPS_MAPPING_STREAM_INDEX);
		maps_input.maps[map_idx].mapping_cluster_channel = htons(DSC_AUDIO_MAPS_MAPPING_CLUSTER_CHANNEL);
		maps_input.maps[map_idx].mapping_cluster_offset  = htons(map_idx);
		maps_input.maps[map_idx].mapping_stream_channel  = htons(map_idx);
	}
	server_add_descriptor(server, AVB_AEM_DESC_AUDIO_MAP, 0,
		 sizeof(maps_input), &maps_input);

	struct {
		struct avb_aem_desc_audio_map desc;
		struct avb_aem_audio_mapping_format maps[DSC_AUDIO_MAPS_NO_OF_MAPPINGS];
	} __attribute__((__packed__)) maps_output= {
		.desc = {
			.mapping_offset = htons(AVB_AEM_AUDIO_MAPPING_FORMAT_OFFSET),
			.number_of_mappings = htons(DSC_AUDIO_MAPS_NO_OF_MAPPINGS),
		},
	};

	for (uint32_t map_idx = 0; map_idx < DSC_AUDIO_MAPS_NO_OF_MAPPINGS; map_idx++) {
		maps_output.maps[map_idx].mapping_stream_index    = htons(DSC_AUDIO_MAPS_MAPPING_STREAM_INDEX);
		maps_output.maps[map_idx].mapping_cluster_channel = htons(DSC_AUDIO_MAPS_MAPPING_CLUSTER_CHANNEL);
		maps_output.maps[map_idx].mapping_cluster_offset  = htons(DSC_AUDIO_MAPS_NO_OF_MAPPINGS+map_idx);
		maps_output.maps[map_idx].mapping_stream_channel  = htons(DSC_AUDIO_MAPS_NO_OF_MAPPINGS+map_idx);
	}

	server_add_descriptor(server, AVB_AEM_DESC_AUDIO_MAP, 1,
		 sizeof(maps_output), &maps_output);

	/**************************************************************************************/
	/* IEEE 1722.1-2021, Sec. 7.2.16 AUDIO_CLUSTER Descriptor */
	/* Milan v1.2, Sec. 5.3.3.8 */
	
	struct avb_aem_desc_audio_cluster clusters[DSC_AUDIO_CLUSTER_NO_OF_CLUSTERS];

	for (uint32_t cluster_idx = 0; cluster_idx < DSC_AUDIO_CLUSTER_NO_OF_CLUSTERS; cluster_idx++) {
		memset(clusters[cluster_idx].object_name, 0,
			sizeof(clusters[cluster_idx].object_name));
		// TODO: Make this scale automatically
		if (cluster_idx < 8) {
			snprintf(clusters[cluster_idx].object_name, DSC_AUDIO_CLUSTER_OBJECT_NAME_LEN_IN_OCTET-1,
						"Input %2u", cluster_idx);
		} else {
			snprintf(clusters[cluster_idx].object_name, DSC_AUDIO_CLUSTER_OBJECT_NAME_LEN_IN_OCTET-1,
					 "Output %2u", cluster_idx);
		}

		clusters[cluster_idx].localized_description = htons(DSC_AUDIO_CLUSTER_LOCALIZED_DESCRIPTION);
		clusters[cluster_idx].signal_type = htons(DSC_AUDIO_CLUSTER_SIGNAL_TYPE);
		clusters[cluster_idx].signal_index = htons(DSC_AUDIO_CLUSTER_SIGNAL_INDEX);
		clusters[cluster_idx].signal_output = htons(DSC_AUDIO_CLUSTER_SIGNAL_OUTPUT);
		clusters[cluster_idx].path_latency = htonl(DSC_AUDIO_CLUSTER_PATH_LATENCY_IN_NS);
		clusters[cluster_idx].block_latency = htonl(DSC_AUDIO_CLUSTER_BLOCK_LATENCY_IN_NS);
		clusters[cluster_idx].channel_count = htons(DSC_AUDIO_CLUSTER_CHANNEL_COUNT);
		clusters[cluster_idx].format = DSC_AUDIO_CLUSTER_FORMAT;
		clusters[cluster_idx].aes3_data_type_ref = DSC_AUDIO_CLUSTER_AES3_DATA_TYPE_REF;
		clusters[cluster_idx].aes3_data_type = htons(DSC_AUDIO_CLUSTER_AES3_DATA_TYPE);

		server_add_descriptor(server, AVB_AEM_DESC_AUDIO_CLUSTER, cluster_idx,
				sizeof(clusters[0]), &clusters[cluster_idx]);
	}

	/**************************************************************************************/
	/* IEEE 1722.1-2021, Sec. 7.2.13 STREAM_PORT_INPUT Descriptor */
	/* Milan v1.2, Sec. 5.3.3.7 */

	struct avb_aem_desc_stream_port stream_port_input0 = {
		.clock_domain_index = htons(DSC_STREAM_PORT_INPUT_CLOCK_DOMAIN_INDEX),
		.port_flags = htons(DSC_STREAM_PORT_INPUT_PORT_FLAGS),
		.number_of_controls = htons(DSC_STREAM_PORT_INPUT_NUMBER_OF_CONTROLS),
		.base_control = htons(DSC_STREAM_PORT_INPUT_BASE_CONTROL),
		.number_of_clusters = htons(DSC_STREAM_PORT_INPUT_NUMBER_OF_CLUSTERS),
		.base_cluster = htons(DSC_STREAM_PORT_INPUT_BASE_CLUSTER),
		.number_of_maps = htons(DSC_STREAM_PORT_INPUT_NUMBER_OF_MAPS),
		.base_map = htons(DSC_STREAM_PORT_INPUT_BASE_MAP),
	};
	server_add_descriptor(server, AVB_AEM_DESC_STREAM_PORT_INPUT, 0,
			sizeof(stream_port_input0), &stream_port_input0);

	/**************************************************************************************/
	/* IEEE 1722.1-2021, Sec. 7.2.13 STREAM_PORT_OUTPUT Descriptor */
	/* Milan v1.2, Sec. 5.3.3.7 */
#if TALKER_ENABLE
	struct avb_aem_desc_stream_port stream_port_output0 = {
		.clock_domain_index = htons(DSC_STREAM_PORT_OUTPUT_CLOCK_DOMAIN_INDEX),
		.port_flags = htons(DSC_STREAM_PORT_OUTPUT_PORT_FLAGS),
		.number_of_controls = htons(DSC_STREAM_PORT_OUTPUT_NUMBER_OF_CONTROLS),
		.base_control = htons(DSC_STREAM_PORT_OUTPUT_BASE_CONTROL),
		.number_of_clusters = htons(DSC_STREAM_PORT_OUTPUT_NUMBER_OF_CLUSTERS),
		.base_cluster = htons(DSC_STREAM_PORT_OUTPUT_BASE_CLUSTER),
		.number_of_maps = htons(DSC_STREAM_PORT_OUTPUT_NUMBER_OF_MAPS),
		.base_map = htons(DSC_STREAM_PORT_OUTPUT_BASE_MAP),
	};
	server_add_descriptor(server, AVB_AEM_DESC_STREAM_PORT_OUTPUT, 0,
			sizeof(stream_port_output0), &stream_port_output0);
#endif
	
	/**************************************************************************************/
	/* IEEE 1722.1-2021, Sec. 7.2.3 AUDIO_UNIT Descriptor */
	/* Milan v1.2, Sec. 5.3.3.3 */

	struct {
		struct avb_aem_desc_audio_unit desc;
		struct avb_aem_desc_sampling_rate sampling_rates[DSC_AUDIO_UNIT_SUPPORTED_SAMPLING_RATE_COUNT];
	} __attribute__ ((__packed__)) audio_unit =
	{
		{
		.object_name = DSC_AUDIO_UNIT_OBJECT_NAME,
		.localized_description = htons(DSC_AUDIO_UNIT_LOCALIZED_DESCRIPTION),
		.clock_domain_index = htons(DSC_AUDIO_UNIT_CLOCK_DOMAIN_INDEX),
		.number_of_stream_input_ports = htons(DSC_AUDIO_UNIT_NUMBER_OF_STREAM_INPUT_PORTS),
		.base_stream_input_port = htons(DSC_AUDIO_UNIT_BASE_STREAM_INPUT_PORT),
		.number_of_stream_output_ports = htons(DSC_AUDIO_UNIT_NUMBER_OF_STREAM_OUTPUT_PORTS),
		.base_stream_output_port = htons(DSC_AUDIO_UNIT_BASE_STREAM_OUTPUT_PORT),
		.number_of_external_input_ports = htons(DSC_AUDIO_UNIT_NUMBER_OF_EXTERNAL_INPUT_PORTS),
		.base_external_input_port = htons(DSC_AUDIO_UNIT_BASE_EXTERNAL_INPUT_PORT),
		.number_of_external_output_ports = htons(DSC_AUDIO_UNIT_NUMBER_OF_EXTERNAL_OUTPUT_PORTS),
		.base_external_output_port = htons(DSC_AUDIO_UNIT_BASE_EXTERNAL_OUTPUT_PORT),
		.number_of_internal_input_ports = htons(DSC_AUDIO_UNIT_NUMBER_OF_INTERNAL_INPUT_PORTS),
		.base_internal_input_port = htons(DSC_AUDIO_UNIT_BASE_INTERNAL_INPUT_PORT),
		.number_of_internal_output_ports = htons(DSC_AUDIO_UNIT_NUMBER_OF_INTERNAL_OUTPUT_PORTS),
		.base_internal_output_port = htons(DSC_AUDIO_UNIT_BASE_INTERNAL_OUTPUT_PORT),
		.number_of_controls = htons(DSC_AUDIO_UNIT_NUMBER_OF_CONTROLS),
		.base_control = htons(DSC_AUDIO_UNIT_BASE_CONTROL),
		.number_of_signal_selectors = htons(DSC_AUDIO_UNIT_NUMBER_OF_SIGNAL_SELECTORS),
		.base_signal_selector = htons(DSC_AUDIO_UNIT_BASE_SIGNAL_SELECTOR),
		.number_of_mixers = htons(DSC_AUDIO_UNIT_NUMBER_OF_MIXERS),
		.base_mixer = htons(DSC_AUDIO_UNIT_BASE_MIXER),
		.number_of_matrices = htons(DSC_AUDIO_UNIT_NUMBER_OF_MATRICES),
		.base_matrix = htons(DSC_AUDIO_UNIT_BASE_MATRIX),
		.number_of_splitters = htons(DSC_AUDIO_UNIT_NUMBER_OF_SPLITTERS),
		.base_splitter = htons(DSC_AUDIO_UNIT_BASE_SPLITTER),
		.number_of_combiners = htons(DSC_AUDIO_UNIT_NUMBER_OF_COMBINERS),
		.base_combiner = htons(DSC_AUDIO_UNIT_BASE_COMBINER),
		.number_of_demultiplexers = htons(DSC_AUDIO_UNIT_NUMBER_OF_DEMULTIPLEXERS),
		.base_demultiplexer = htons(DSC_AUDIO_UNIT_BASE_DEMULTIPLEXER),
		.number_of_multiplexers = htons(DSC_AUDIO_UNIT_NUMBER_OF_MULTIPLEXERS),
		.base_multiplexer = htons(DSC_AUDIO_UNIT_BASE_MULTIPLEXER),
		.number_of_transcoders = htons(DSC_AUDIO_UNIT_NUMBER_OF_TRANSCODERS),
		.base_transcoder = htons(DSC_AUDIO_UNIT_BASE_TRANSCODER),
		.number_of_control_blocks = htons(DSC_AUDIO_UNIT_NUMBER_OF_CONTROL_BLOCKS),
		.base_control_block = htons(DSC_AUDIO_UNIT_BASE_CONTROL_BLOCK),
		.current_sampling_rate = htonl(DSC_AUDIO_UNIT_CURRENT_SAMPLING_RATE_IN_HZ),
		.sampling_rates_offset = htons(DSC_AUDIO_UNIT_SAMPLING_RATES_OFFSET),
		.sampling_rates_count = htons(DSC_AUDIO_UNIT_SUPPORTED_SAMPLING_RATE_COUNT),
		},
		.sampling_rates = {
			// Set the list of supported audio unit sample rate
			{ .pull_frequency = htonl(DSC_AUDIO_UNIT_SUPPORTED_SAMPLING_RATE_IN_HZ_0) },
		}
	};
	server_add_descriptor(server, AVB_AEM_DESC_AUDIO_UNIT, 0,
			sizeof(audio_unit), &audio_unit);

    /**************************************************************************************/
	/* IEEE 1722.1-2021, Sec. 7.2.6 STREAM_INPUT Descriptor */
	/* Milan v1.2, Sec. 5.3.3.4 */

    // TODO: 1722.1 lists redundant parameters that are not mentioned here.

	struct {
		struct avb_aem_desc_stream desc;
		uint64_t stream_formats[DSC_STREAM_INPUT_NUMBER_OF_FORMATS];
	} __attribute__ ((__packed__)) stream_input_0 =
	{
		{
		.object_name = DSC_STREAM_INPUT_OBJECT_NAME,
		.localized_description = htons(DSC_STREAM_INPUT_LOCALIZED_DESCRIPTION),
		.clock_domain_index = htons(DSC_STREAM_INPUT_CLOCK_DOMAIN_INDEX),
		.stream_flags = htons(DSC_STREAM_INPUT_STREAM_FLAGS),
		.current_format = htobe64(DSC_STREAM_INPUT_CURRENT_FORMAT),
		.formats_offset = htons(DSC_STREAM_INPUT_FORMATS_OFFSET),
		.number_of_formats = htons(DSC_STREAM_INPUT_NUMBER_OF_FORMATS),
		.backup_talker_entity_id_0 = htobe64(DSC_STREAM_INPUT_BACKUP_TALKER_ENTITY_ID_0),
		.backup_talker_unique_id_0 = htons(DSC_STREAM_INPUT_BACKUP_TALKER_UNIQUE_ID_0),
		.backup_talker_entity_id_1 = htobe64(DSC_STREAM_INPUT_BACKUP_TALKER_ENTITY_ID_1),
		.backup_talker_unique_id_1 = htons(DSC_STREAM_INPUT_BACKUP_TALKER_UNIQUE_ID_1),
		.backup_talker_entity_id_2 = htobe64(DSC_STREAM_INPUT_BACKUP_TALKER_ENTITY_ID_2),
		.backup_talker_unique_id_2 = htons(DSC_STREAM_INPUT_BACKUP_TALKER_UNIQUE_ID_2),
		.backedup_talker_entity_id = htobe64(DSC_STREAM_INPUT_BACKEDUP_TALKER_ENTITY_ID),
		.backedup_talker_unique = htons(DSC_STREAM_INPUT_BACKEDUP_TALKER_UNIQUE_ID),
		.avb_interface_index = htons(DSC_STREAM_INPUT_AVB_INTERFACE_INDEX),
		.buffer_length = htonl(DSC_STREAM_INPUT_BUFFER_LENGTH_IN_NS)
		},
		.stream_formats = {
			htobe64(DSC_STREAM_INPUT_FORMATS_0),
			htobe64(DSC_STREAM_INPUT_FORMATS_1),
			htobe64(DSC_STREAM_INPUT_FORMATS_2),
			htobe64(DSC_STREAM_INPUT_FORMATS_3),
			htobe64(DSC_STREAM_INPUT_FORMATS_4),
		},
	};
	server_add_descriptor(server, AVB_AEM_DESC_STREAM_INPUT, 0,
			sizeof(stream_input_0), &stream_input_0);

	/**************************************************************************************/
    /* IEEE 1722.1-2021, Sec. 7.2.6 STREAM_INPUT Descriptor */
    /* Milan v1.2, Sec. 5.3.3.4 */

    struct {
        struct avb_aem_desc_stream desc;
        uint64_t stream_formats[DSC_STREAM_INPUT_CRF_NUMBER_OF_FORMATS];
    } __attribute__ ((__packed__)) stream_input_crf_0 =
    {
        {
            .object_name = DSC_STREAM_INPUT_CRF_OBJECT_NAME,
            .localized_description = htons(DSC_STREAM_INPUT_CRF_LOCALIZED_DESCRIPTION),
            .clock_domain_index = htons(DSC_STREAM_INPUT_CRF_CLOCK_DOMAIN_INDEX),
            .stream_flags = htons(DSC_STREAM_INPUT_CRF_STREAM_FLAGS),
            .current_format = htobe64(DSC_STREAM_INPUT_CRF_CURRENT_FORMAT),
            .formats_offset = htons(DSC_STREAM_INPUT_CRF_FORMATS_OFFSET),
            .number_of_formats = htons(DSC_STREAM_INPUT_CRF_NUMBER_OF_FORMATS),
            .backup_talker_entity_id_0 = htobe64(DSC_STREAM_INPUT_CRF_BACKUP_TALKER_ENTITY_ID_0),
            .backup_talker_unique_id_0 = htons(DSC_STREAM_INPUT_CRF_BACKUP_TALKER_UNIQUE_ID_0),
            .backup_talker_entity_id_1 = htobe64(DSC_STREAM_INPUT_CRF_BACKUP_TALKER_ENTITY_ID_1),
            .backup_talker_unique_id_1 = htons(DSC_STREAM_INPUT_CRF_BACKUP_TALKER_UNIQUE_ID_1),
            .backup_talker_entity_id_2 = htobe64(DSC_STREAM_INPUT_CRF_BACKUP_TALKER_ENTITY_ID_2),
            .backup_talker_unique_id_2 = htons(DSC_STREAM_INPUT_CRF_BACKUP_TALKER_UNIQUE_ID_2),
            .backedup_talker_entity_id = htobe64(DSC_STREAM_INPUT_CRF_BACKEDUP_TALKER_ENTITY_ID),
            .backedup_talker_unique = htons(DSC_STREAM_INPUT_CRF_BACKEDUP_TALKER_UNIQUE_ID),
            .avb_interface_index = htons(DSC_STREAM_INPUT_CRF_AVB_INTERFACE_INDEX),
            .buffer_length = htonl(DSC_STREAM_INPUT_CRF_BUFFER_LENGTH_IN_NS)
        },
        .stream_formats = {
            htobe64(DSC_STREAM_INPUT_CRF_FORMATS_0),
        },
    };
    server_add_descriptor(server, AVB_AEM_DESC_STREAM_INPUT, 1,
            sizeof(stream_input_crf_0), &stream_input_crf_0);


	/**************************************************************************************/
    /* IEEE 1722.1-2021, Sec. 7.2.6 STREAM_OUTPUT Descriptor */
    /* Milan v1.2, Sec. 5.3.3.4 */

#if TALKER_ENABLE
    struct {
        struct avb_aem_desc_stream desc;
        uint64_t stream_formats[DSC_STREAM_OUTPUT_NUMBER_OF_FORMATS];
    } __attribute__ ((__packed__)) stream_output_0 =
    {
        {
            .object_name = DSC_STREAM_OUTPUT_OBJECT_NAME,
            .localized_description = htons(DSC_STREAM_OUTPUT_LOCALIZED_DESCRIPTION),
            .clock_domain_index = htons(DSC_STREAM_OUTPUT_CLOCK_DOMAIN_INDEX),
            .stream_flags = htons(DSC_STREAM_OUTPUT_STREAM_FLAGS),
            .current_format = htobe64(DSC_STREAM_OUTPUT_CURRENT_FORMAT),
            .formats_offset = htons(DSC_STREAM_OUTPUT_FORMATS_OFFSET),
            .number_of_formats = htons(DSC_STREAM_OUTPUT_NUMBER_OF_FORMATS),
            .backup_talker_entity_id_0 = htobe64(DSC_STREAM_OUTPUT_BACKUP_TALKER_ENTITY_ID_0),
            .backup_talker_unique_id_0 = htons(DSC_STREAM_OUTPUT_BACKUP_TALKER_UNIQUE_ID_0),
            .backup_talker_entity_id_1 = htobe64(DSC_STREAM_OUTPUT_BACKUP_TALKER_ENTITY_ID_1),
            .backup_talker_unique_id_1 = htons(DSC_STREAM_OUTPUT_BACKUP_TALKER_UNIQUE_ID_1),
            .backup_talker_entity_id_2 = htobe64(DSC_STREAM_OUTPUT_BACKUP_TALKER_ENTITY_ID_2),
            .backup_talker_unique_id_2 = htons(DSC_STREAM_OUTPUT_BACKUP_TALKER_UNIQUE_ID_2),
            .backedup_talker_entity_id = htobe64(DSC_STREAM_OUTPUT_BACKEDUP_TALKER_ENTITY_ID),
            .backedup_talker_unique = htons(DSC_STREAM_OUTPUT_BACKEDUP_TALKER_UNIQUE_ID),
            .avb_interface_index = htons(DSC_STREAM_OUTPUT_AVB_INTERFACE_INDEX),
            .buffer_length = htons(DSC_STREAM_OUTPUT_BUFFER_LENGTH_IN_NS)
        },
        .stream_formats = {
            htobe64(DSC_STREAM_OUTPUT_FORMATS_0),
            htobe64(DSC_STREAM_OUTPUT_FORMATS_1),
            htobe64(DSC_STREAM_OUTPUT_FORMATS_2),
            htobe64(DSC_STREAM_OUTPUT_FORMATS_3),
            htobe64(DSC_STREAM_OUTPUT_FORMATS_4),
        },
    };
    server_add_descriptor(server, AVB_AEM_DESC_STREAM_OUTPUT, 0,
            sizeof(stream_output_0), &stream_output_0);
#endif

    /**************************************************************************************/
    /* IEEE 1722.1-2021, Sec. 7.2.8 AVB Interface Descriptor */
    /* Milan v1.2, Sec. 5.3.3.5 */

	struct avb_aem_desc_avb_interface avb_interface = {
		.localized_description = htons(DSC_AVB_INTERFACE_LOCALIZED_DESCRIPTION),
		.interface_flags = htons(DSC_AVB_INTERFACE_INTERFACE_FLAGS),
		.clock_identity = htobe64(DSC_AVB_INTERFACE_CLOCK_IDENTITY),
		.priority1 = DSC_AVB_INTERFACE_PRIORITY1,
		.clock_class = DSC_AVB_INTERFACE_CLOCK_CLASS,
		.offset_scaled_log_variance = htons(DSC_AVB_INTERFACE_OFFSET_SCALED_LOG_VARIANCE),
		.clock_accuracy = DSC_AVB_INTERFACE_CLOCK_ACCURACY,
		.priority2 = DSC_AVB_INTERFACE_PRIORITY2,
		.domain_number = DSC_AVB_INTERFACE_DOMAIN_NUMBER,
		.log_sync_interval = DSC_AVB_INTERFACE_LOG_SYNC_INTERVAL,
		.log_announce_interval = DSC_AVB_INTERFACE_LOG_ANNOUNCE_INTERVAL,
		.log_pdelay_interval = DSC_AVB_INTERFACE_PDELAY_INTERVAL,
		.port_number = DSC_AVB_INTERFACE_PORT_NUMBER,
	};

	memset(avb_interface.object_name, 0, sizeof(avb_interface.object_name));
	strncpy(avb_interface.object_name, "", 63);
	memcpy(avb_interface.mac_address, server->mac_addr, 6);
	server_add_descriptor(server, AVB_AEM_DESC_AVB_INTERFACE, 0,
			sizeof(avb_interface), &avb_interface);

    /**************************************************************************************/
    /* IEEE 1722.1-2021, Sec. 7.2.9 CLOCK_SOURCE Descriptor */
    /* Milan v1.2, Sec. 5.3.3.6 */

    // Internal Clock Descriptor
    struct avb_aem_desc_clock_source clock_source_internal = {
        .object_name = DSC_CLOCK_SOURCE_INTERNAL_OBJECT_NAME,
        .localized_description = htons(DSC_CLOCK_SOURCE_INTERNAL_LOCALIZED_DESCRIPTION),
        .clock_source_flags = htons(DSC_CLOCK_SOURCE_INTERNAL_FLAGS),
        .clock_source_type = htons(DSC_CLOCK_SOURCE_INTERNAL_TYPE),
        .clock_source_identifier = htobe64(DSC_CLOCK_SOURCE_INTERNAL_IDENTIFIER),
        .clock_source_location_type = htons(DSC_CLOCK_SOURCE_INTERNAL_LOCATION_TYPE),
        .clock_source_location_index = htons(DSC_CLOCK_SOURCE_INTERNAL_LOCATION_INDEX),
    };
    server_add_descriptor(server, AVB_AEM_DESC_CLOCK_SOURCE, 0,
            sizeof(clock_source_internal), &clock_source_internal);

    // AAF Clock Descriptor
    struct avb_aem_desc_clock_source clock_source_aaf = {
        .object_name = DSC_CLOCK_SOURCE_AAF_OBJECT_NAME,
        .localized_description = htons(DSC_CLOCK_SOURCE_AAF_LOCALIZED_DESCRIPTION),
        .clock_source_flags = htons(DSC_CLOCK_SOURCE_AAF_FLAGS),
        .clock_source_type = htons(DSC_CLOCK_SOURCE_AAF_TYPE),
        .clock_source_identifier = htobe64(DSC_CLOCK_SOURCE_AAF_IDENTIFIER),
        .clock_source_location_type = htons(DSC_CLOCK_SOURCE_AAF_LOCATION_TYPE),
        .clock_source_location_index = htons(DSC_CLOCK_SOURCE_AAF_LOCATION_INDEX),
    };
    server_add_descriptor(server, AVB_AEM_DESC_CLOCK_SOURCE, 1,
            sizeof(clock_source_aaf), &clock_source_aaf);

    // CRF Clock Descriptor
    struct avb_aem_desc_clock_source clock_source_crf = {
        .object_name = DSC_CLOCK_SOURCE_CRF_OBJECT_NAME,
        .localized_description = htons(DSC_CLOCK_SOURCE_CRF_LOCALIZED_DESCRIPTION),
        .clock_source_flags = htons(DSC_CLOCK_SOURCE_CRF_FLAGS),
        .clock_source_type = htons(DSC_CLOCK_SOURCE_CRF_TYPE),
        .clock_source_identifier = htobe64(DSC_CLOCK_SOURCE_CRF_IDENTIFIER),
        .clock_source_location_type = htons(DSC_CLOCK_SOURCE_CRF_LOCATION_TYPE),
        .clock_source_location_index = htons(DSC_CLOCK_SOURCE_CRF_LOCATION_INDEX),
    };
    server_add_descriptor(server, AVB_AEM_DESC_CLOCK_SOURCE, 2,
            sizeof(clock_source_crf), &clock_source_crf);

    /**************************************************************************************/
    /* IEEE 1722.1-2021, Sec. 7.2.32 CLOCK_DOMAIN Descriptor */
    /* Milan v1.2, Sec. 5.3.3.11 */

    struct {
        struct avb_aem_desc_clock_domain desc;
        uint16_t clock_sources_idx[DSC_CLOCK_DOMAIN_CLOCK_SOURCES_COUNT];
    } __attribute__ ((__packed__)) clock_domain = {
        .desc = {
            .object_name = DSC_CLOCK_DOMAIN_OBJECT_NAME,
            .localized_description = htons(DSC_CLOCK_DOMAIN_LOCALIZED_DESCRIPTION),
            .clock_source_index = htons(DSC_CLOCK_DOMAIN_CLOCK_SOURCE_INDEX),
            .descriptor_counts_offset = htons(DSC_CLOCK_DOMAIN_DESCRIPTOR_COUNTS_OFFSET),
            .clock_sources_count = htons(DSC_CLOCK_DOMAIN_CLOCK_SOURCES_COUNT),
        },
        .clock_sources_idx = {
            htons(DSC_CLOCK_DOMAIN_SOURCES_0),
            htons(DSC_CLOCK_DOMAIN_SOURCES_1),
            htons(DSC_CLOCK_DOMAIN_SOURCES_2),
        },
    };

    server_add_descriptor(server, AVB_AEM_DESC_CLOCK_DOMAIN, 0,
            sizeof(clock_domain), &clock_domain);
}

#endif