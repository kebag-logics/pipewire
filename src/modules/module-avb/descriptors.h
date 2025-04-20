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

static inline void init_descriptors(struct server *server)
{
	// TODO PERSISTENCE: retrieve the saved buffers.
	// TODO: Add Milan references
	/**************************************************************************************/
	/* IEEE 1722.1-2021, Sec. 7.2.12 - STRINGS Descriptor 
	* Up to 7 localized strings
	*/
	#define DSC_STRINGS_0 "DSXYZ"
	#define DSC_STRINGS_1 "NON - redundant - 48kHz"
	#define DSC_STRINGS_2 "Alexandre Malki"
	#define DSC_STRINGS_3 "Kebag Logic"
	server_add_descriptor(server, AVB_AEM_DESC_STRINGS, 0,
			sizeof(struct avb_aem_desc_strings),
			&(struct avb_aem_desc_strings)
	{
		.string_0 = DSC_STRINGS_0,
		.string_1 = DSC_STRINGS_1,
		.string_2 = DSC_STRINGS_2,
		.string_3 = DSC_STRINGS_3
	});

	/**************************************************************************************/
	/* IEEE 1722.1-2021, Sec. 7.2.11 - LOCALE Descriptor */
	#define DSC_LOCALE_LANGUAGE_CODE "en-EN"
	#define DSC_LOCALE_NO_OF_STRINGS 1
	#define DSC_LOCALE_BASE_STRINGS 0
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
	#define DSC_ENTITY_MODEL_ENTITY_ID 0xDEAD00BEEF00FEED
	#define DSC_ENTITY_MODEL_ID 0
	#define DSC_ENTITY_MODEL_ENTITY_CAPABILITIES (AVB_ADP_ENTITY_CAPABILITY_AEM_SUPPORTED | \
			AVB_ADP_ENTITY_CAPABILITY_CLASS_A_SUPPORTED | \
			AVB_ADP_ENTITY_CAPABILITY_GPTP_SUPPORTED | \
			AVB_ADP_ENTITY_CAPABILITY_AEM_IDENTIFY_CONTROL_INDEX_VALID | \
			AVB_ADP_ENTITY_CAPABILITY_AEM_INTERFACE_INDEX_VALID)
	/* IEEE 1722.1-2021, Table 7-2 - ENTITY Descriptor
	 * This is the maximum number of STREAM_OUTPUT 
	 * descriptors the ATDECC Entity has for 
	 * Output Streams in any of its Configurations */
	#define DSC_ENTITY_MODEL_TALKER_STREAM_SOURCES 8
	#define DSC_ENTITY_MODEL_TALKER_CAPABILITIES (AVB_ADP_TALKER_CAPABILITY_IMPLEMENTED | \
			AVB_ADP_TALKER_CAPABILITY_AUDIO_SOURCE)
	#define DSC_ENTITY_MODEL_LISTENER_STREAM_SINKS 8
	#define DSC_ENTITY_MODEL_LISTENER_CAPABILITIES (AVB_ADP_LISTENER_CAPABILITY_IMPLEMENTED | \
			AVB_ADP_LISTENER_CAPABILITY_AUDIO_SINK)
	#define DSC_ENTITY_MODEL_CONTROLLER_CAPABILITIES 0
	/* IEEE 1722.1-2021, Table 7-2 ENTITY Descriptor
	* The available index of the ATDECC Entity.
	* This is the same as the available_index field 
	* in ATDECC Discovery Protocol.*/
	#define DSC_ENTITY_MODEL_AVAILABLE_INDEX 0
	/* IEEE 1722.1-2021, Table 7-2 ENTITY Descriptor 
	 * The association ID for the ATDECC Entity. 
	 * This is the same as association_id field 
	 * in ATDECC Discovery Protocol*/
	#define DSC_ENTITY_MODEL_ASSOCIATION_ID 0
	#define DSC_ENTITY_MODEL_ENTITY_NAME DSC_STRINGS_0
	/* IEEE 1722.1-2021, Table 7-2 - ENTITY Descriptor
	 * The localized string reference pointing to the 
	 * localized vendor name. See 7.3.7. */
	#define DSC_ENTITY_MODEL_VENDOR_NAME_STRING 2
	/* IEEE 1722.1-2021, Table 7-2 - ENTITY Descriptor 
	 * The localized string reference pointing to the 
	 * localized model name. See 7.3.7. */
	#define DSC_ENTITY_MODEL_MODEL_NAME_STRING 0
	#define DSC_ENTITY_MODEL_FIRMWARE_VERSION "0.3.48"
	#define DSC_ENTITY_MODEL_GROUP_NAME DSC_STRINGS_3
	#define DSC_ENTITY_MODEL_SERIAL_NUMBER "0xBEBEDEAD"
	#define DSC_ENTITY_MODEL_CONFIGURATIONS_COUNT 2
	#define DSC_ENTITY_MODEL_CURRENT_CONFIGURATION 0

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
	#define DSC_CONFIGURATION_DESCRIPTOR_COUNTS_COUNT 8
	#define DSC_CONFIGURATION_OBJECT_NAME DSC_STRINGS_1
	/* IEEE 1722.1-2021, Table 7-3 CONFIGURATION Descriptor
	 * The localized string reference pointing to the 
	 * localized Configuration name. */
	#define DSC_CONFIGURATION_LOCALIZED_DESCRIPTION 1
	/* IEEE 1722.1-2021, Table 7-3 CONFIGURATION Descriptor
	 * The offset to the descriptor_counts field from the 
	 * start of the descriptor. This field is set to 74 for
	 * this version of AEM. */
	#define DSC_CONFIGURATION_DESCRIPTOR_COUNTS_OFFSET 74

	#define DSC_CONFIGURATION_NO_OF_AUDIO_UNITS 1
	#define DSC_CONFIGURATION_NO_OF_STREAM_INPUTS 2
	#define DSC_CONFIGURATION_NO_OF_STREAM_OUTPUTS 1
	#define DSC_CONFIGURATION_NO_OF_AVB_INTERFACES 1
	#define DSC_CONFIGURATION_NO_OF_CLOCK_DOMAINS 1
	#define DSC_CONFIGURATION_NO_OF_CLOCK_SOURCES 3
	#define DSC_CONFIGURATION_NO_OF_CONTROLS 1
	#define DSC_CONFIGURATION_NO_OF_LOCALES 1
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
	#define DSC_CONTROL_OBJECT_NAME "Identify"
	#define DSC_CONTROL_LOCALIZED_DESCRIPTION AVB_AEM_DESC_INVALID
	#define DSC_CONTROL_BLOCK_LATENCY 500
	#define DSC_CONTROL_CONTROL_LATENCY 500
	#define DSC_CONTROL_CONTROL_DOMAIN 0
	#define DSC_CONTROL_CONTROL_VALUE_TYPE AVB_AEM_CONTROL_LINEAR_UINT8
	#define DSC_CONTROL_CONTROL_TYPE AEM_CTRL_TYPE_IDENTIFY
	/* IEEE 1722.1-2021, Table 7-39 - CONTROL Descriptor
	 * The time period in microseconds from when a control
	 * is set with the SET_CONTROL command till it automatically
	 * resets to its default values.
	 * When this is set to zero (0) automatic resets do not happen. */
	// TODO: Milan v1.2: The PAAD remains in identification mode until the value of the “IDENTIFY” CONTROL descriptor is set back to 0.
	#define DSC_CONTROL_RESET_TIME 3
	#define DSC_CONTROL_NUMBER_OF_VALUES 1
	#define DSC_CONTROL_SIGNAL_TYPE AVB_AEM_DESC_INVALID
	#define DSC_CONTROL_SIGNAL_INDEX 0
	#define DSC_CONTROL_SIGNAL_OUTPUT 0

	#define DSC_CONTROL_IDENTIFY_MIN 0
	#define DSC_CONTROL_IDENTIFY_MAX 255
	#define DSC_CONTROL_IDENTIFY_STEP 255
	#define DSC_CONTROL_IDENTIFY_DEFAULT_VALUE 0
	#define DSC_CONTROL_IDENTIFY_CURRENT_VALUE 0
	#define DSC_CONTROL_LOCALIZED_DESCRIPTION AVB_AEM_DESC_INVALID

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
	// TODO: Prepared for for loop over total number of audio maps
	#define DSC_AUDIO_MAPS_TOTAL_NO_OF_MAPS 2

	#define DSC_AUDIO_MAPS_NO_OF_MAPPINGS 8
	#define DSC_AUDIO_MAPS_MAPPING_STREAM_INDEX 0
	#define DSC_AUDIO_MAPS_MAPPING_CLUSTER_CHANNEL 0
	
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
	/* Milan v1.2, Sec. 5.3.3.9*/
	#define DSC_AUDIO_CLUSTER_NO_OF_CLUSTERS 16
	#define DSC_AUDIO_CLUSTER_OBJECT_NAME_LEN_IN_OCTET 64
	#define DSC_AUDIO_CLUSTER_OBJECT_NAME_INPUT "Input"
	#define DSC_AUDIO_CLUSTER_OBJECT_NAME_OUTPUT "Output"

	#define DSC_AUDIO_CLUSTER_LOCALIZED_DESCRIPTION AVB_AEM_DESC_INVALID

	/* The signal_type and signal_index fields indicate the 
	 * object providing the signal destined for the channels 
	 * of the streams mapped to the port. For a signal which 
	 * is sourced internally from the Unit, the signal_type 
	 * is set to AUDIO_UNIT and signal_index is set to the 
	 * index of the Unit. For a Cluster attached to a 
	 * STREAM_PORT_INPUT the signal_type and signal_index 
	 * fields is set to INVALID and zero (0) respectively. */
	#define DSC_AUDIO_CLUSTER_SIGNAL_TYPE 0
	#define DSC_AUDIO_CLUSTER_SIGNAL_INDEX 0
	/* The index of the output of the signal source of the 
	 * cluster. For a signal_type of SIGNAL_SPLITTER or 
	 * SIGNAL_DEMULTIPLEXER this is which output of the 
	 * object it is being sourced from, for a signal_type 
	 * of MATRIX this is the column the signal is from 
	 * and for any other signal_type this is zero (0). */
	#define DSC_AUDIO_CLUSTER_SIGNAL_OUTPUT 0
	#define DSC_AUDIO_CLUSTER_PATH_LATENCY_IN_NS 500
	#define DSC_AUDIO_CLUSTER_BLOCK_LATENCY_IN_NS 500
	#define DSC_AUDIO_CLUSTER_CHANNEL_COUNT 1
	#define DSC_AUDIO_CLUSTER_FORMAT AVB_AEM_AUDIO_CLUSTER_TYPE_MBLA
	#define DSC_AUDIO_CLUSTER_AES3_DATA_TYPE_REF 0
	#define DSC_AUDIO_CLUSTER_AES3_DATA_TYPE 0

	
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
	/* Milan v1.2, Sec. 5.3.3.7*/

	//TODO: Put in aecp-aem-descriptors.h
	/* IEEE 1722.1-2021, Table 7-24 - Port Flags */
	#define AVB_AEM_PORT_FLAG_CLOCK_SYNC_SOURCE 0x0001
	#define AVB_AEM_PORT_FLAG_ASYNC_SAMPLE_RATE_CONV 0x0002
	#define AVB_AEM_PORT_FLAG_SYNC_SAMPLE_RATE_CONV 0x0004

	#define DSC_STREAM_PORT_INPUT_CLOCK_DOMAIN_INDEX 0x0000
	#define DSC_STREAM_PORT_INPUT_PORT_FLAGS AVB_AEM_PORT_FLAG_CLOCK_SYNC_SOURCE
	/* The number of clusters within the Port. This corresponds to the number of 
	 * AUDIO_CLUSTER, VIDEO_CLUSTER or SENSOR_CLUSTER descriptors which represent 
	 * these clusters. */
	// TODO: Validate value
	#define DSC_STREAM_PORT_INPUT_NUMBER_OF_CONTROLS 0
	#define DSC_STREAM_PORT_INPUT_BASE_CLUSTER 0
	// TODO: Validate value
	#define DSC_STREAM_PORT_INPUT_NUMBER_OF_MAPS 8
	#define DSC_STREAM_PORT_INPUT_BASE_MAP 0

	struct avb_aem_desc_stream_port stream_port_input0 = {
		.clock_domain_index = htons(0),
		.port_flags = htons(1),
		.number_of_controls = htons(0),
		.base_control = htons(0),
		.number_of_clusters = htons(8),
		.base_cluster = htons(0),
		.number_of_maps = htons(1),
		.base_map = htons(0),
	};
	server_add_descriptor(server, AVB_AEM_DESC_STREAM_PORT_INPUT, 0,
			sizeof(stream_port_input0), &stream_port_input0);

	struct avb_aem_desc_stream_port stream_port_output0 = {
		.clock_domain_index = htons(0),
		.port_flags = htons(0),
		.number_of_controls = htons(0),
		.base_control = htons(0),
		.number_of_clusters = htons(8),
		.base_cluster = htons(8),
		.number_of_maps = htons(1),
		.base_map = htons(1),
	};

	server_add_descriptor(server, AVB_AEM_DESC_STREAM_PORT_OUTPUT, 0,
			sizeof(stream_port_output0), &stream_port_output0);
	struct {
		struct avb_aem_desc_audio_unit desc;
		struct avb_aem_desc_sampling_rate sampling_rates[2];
	} __attribute__ ((__packed__)) audio_unit =
	{
		{
		.object_name = "",
		.localized_description = htons(0xffff),
		.clock_domain_index = htons(0),
		.number_of_stream_input_ports = htons(1),
		.base_stream_input_port = htons(0),
		.number_of_stream_output_ports = htons(1),
		.base_stream_output_port = htons(0),
		.number_of_external_input_ports = htons(8),
		.base_external_input_port = htons(0),
		.number_of_external_output_ports = htons(8),
		.base_external_output_port = htons(0),
		.number_of_internal_input_ports = htons(0),
		.base_internal_input_port = htons(0),
		.number_of_internal_output_ports = htons(0),
		.base_internal_output_port = htons(0),
		.number_of_controls = htons(0),
		.base_control = htons(0),
		.number_of_signal_selectors = htons(0),
		.base_signal_selector = htons(0),
		.number_of_mixers = htons(0),
		.base_mixer = htons(0),
		.number_of_matrices = htons(0),
		.base_matrix = htons(0),
		.number_of_splitters = htons(0),
		.base_splitter = htons(0),
		.number_of_combiners = htons(0),
		.base_combiner = htons(0),
		.number_of_demultiplexers = htons(0),
		.base_demultiplexer = htons(0),
		.number_of_multiplexers = htons(0),
		.base_multiplexer = htons(0),
		.number_of_transcoders = htons(0),
		.base_transcoder = htons(0),
		.number_of_control_blocks = htons(0),
		.base_control_block = htons(0),
		.current_sampling_rate = htonl(48000),
		.sampling_rates_offset = htons(
			4 + sizeof(struct avb_aem_desc_audio_unit)),
		.sampling_rates_count = htons(2),
		},
		.sampling_rates = {
			// Set the list of supported audio unit sample rate
			{ .pull_frequency = htonl(48000) },
			{ .pull_frequency = htonl(96000) },
		}
	};
	server_add_descriptor(server, AVB_AEM_DESC_AUDIO_UNIT, 0,
			sizeof(audio_unit), &audio_unit);

	struct {
		struct avb_aem_desc_stream desc;
		uint64_t stream_formats[5];
	} __attribute__ ((__packed__)) stream_input_0 =
	{
		{
		.object_name = "Stream 1",
		.localized_description = htons(0xffff),
		.clock_domain_index = htons(0),
		.stream_flags = htons(
				AVB_AEM_DESC_STREAM_FLAG_SYNC_SOURCE |
				AVB_AEM_DESC_STREAM_FLAG_CLASS_A),
		.current_format = htobe64(0x0205022000406000ULL),
		.formats_offset = htons(
			4 + sizeof(struct avb_aem_desc_stream)),
		.number_of_formats = htons(5),
		.backup_talker_entity_id_0 = htobe64(0),
		.backup_talker_unique_id_0 = htons(0),
		.backup_talker_entity_id_1 = htobe64(0),
		.backup_talker_unique_id_1 = htons(0),
		.backup_talker_entity_id_2 = htobe64(0),
		.backup_talker_unique_id_2 = htons(0),
		.backedup_talker_entity_id = htobe64(0),
		.backedup_talker_unique = htons(0),
		.avb_interface_index = htons(0),
		.buffer_length = htonl(2126000)
		},
		.stream_formats = {
			htobe64(AECP_AEM_HELPER_FORMAT_STD_AVTP(48KHZ, 32BIT_INT, 32, 1, 6)),
			htobe64(AECP_AEM_HELPER_FORMAT_STD_AVTP(48KHZ, 32BIT_INT, 32, 2, 6)),
			htobe64(AECP_AEM_HELPER_FORMAT_STD_AVTP(48KHZ, 32BIT_INT, 32, 4, 6)),
			htobe64(AECP_AEM_HELPER_FORMAT_STD_AVTP(48KHZ, 32BIT_INT, 32, 6, 6)),
			htobe64(AECP_AEM_HELPER_FORMAT_STD_AVTP(48KHZ, 32BIT_INT, 32, 8, 6)),
		},
	};
	server_add_descriptor(server, AVB_AEM_DESC_STREAM_INPUT, 0,
			sizeof(stream_input_0), &stream_input_0);

	struct {
		struct avb_aem_desc_stream desc;
		uint64_t stream_formats[1];
	} __attribute__ ((__packed__)) stream_input_crf_0 =
	{
		{
		.object_name = "CRF",
		.localized_description = htons(0xffff),
		.clock_domain_index = htons(0),
		.stream_flags = htons( AVB_AEM_DESC_STREAM_FLAG_SYNC_SOURCE |
				AVB_AEM_DESC_STREAM_FLAG_CLASS_A),
		.current_format = htobe64(0x041060010000BB80ULL),
		.formats_offset = htons(
			4 + sizeof(struct avb_aem_desc_stream)),
		.number_of_formats = htons(1),
		.backup_talker_entity_id_0 = htobe64(0),
		.backup_talker_unique_id_0 = htons(0),
		.backup_talker_entity_id_1 = htobe64(0),
		.backup_talker_unique_id_1 = htons(0),
		.backup_talker_entity_id_2 = htobe64(0),
		.backup_talker_unique_id_2 = htons(0),
		.backedup_talker_entity_id = htobe64(0),
		.backedup_talker_unique = htons(0),
		.avb_interface_index = htons(0),
		.buffer_length = htonl(2126000)
		},
		.stream_formats = {
			htobe64(0x041060010000BB80ULL),
		},
	};
	server_add_descriptor(server, AVB_AEM_DESC_STREAM_INPUT, 1,
			sizeof(stream_input_crf_0), &stream_input_crf_0);

	struct {
		struct avb_aem_desc_stream desc;
		uint64_t stream_formats[6];
	} __attribute__ ((__packed__)) stream_output_0 =
	{
		{
		.object_name = "Stream output 1",
		.localized_description = htons(0xffff),
		.clock_domain_index = htons(0),
		.stream_flags = htons(
				AVB_AEM_DESC_STREAM_FLAG_CLASS_A),
		.current_format = htobe64(0x0205022002006000ULL),
		.formats_offset = htons(
			4 + sizeof(struct avb_aem_desc_stream)),
		.number_of_formats = htons(5),
		.backup_talker_entity_id_0 = htobe64(0),
		.backup_talker_unique_id_0 = htons(0),
		.backup_talker_entity_id_1 = htobe64(0),
		.backup_talker_unique_id_1 = htons(0),
		.backup_talker_entity_id_2 = htobe64(0),
		.backup_talker_unique_id_2 = htons(0),
		.backedup_talker_entity_id = htobe64(0),
		.backedup_talker_unique = htons(0),
		.avb_interface_index = htons(0),
		.buffer_length = htons(8)
		},
		.stream_formats = {
			htobe64(0x0205022000406000ULL),
			htobe64(0x0205022000806000ULL),
			htobe64(0x0205022001006000ULL),
			htobe64(0x0205022001806000ULL),
			htobe64(0x0205022002006000ULL),
		},
	};
	server_add_descriptor(server, AVB_AEM_DESC_STREAM_OUTPUT, 0,
			sizeof(stream_output_0), &stream_output_0);

	struct avb_aem_desc_avb_interface avb_interface = {
		.localized_description = htons(0xffff),
		.interface_flags = htons(
				AVB_AEM_DESC_AVB_INTERFACE_FLAG_GPTP_GRANDMASTER_SUPPORTED |
				AVB_AEM_DESC_AVB_INTERFACE_FLAG_GPTP_SUPPORTED |
				AVB_AEM_DESC_AVB_INTERFACE_FLAG_SRP_SUPPORTED),
		.clock_identity = htobe64(0x3cc0c6FFFE000641),
		.priority1 = 0xF8,
		.clock_class = 0xF8,
		.offset_scaled_log_variance = htons(0x436A),
		.clock_accuracy = 0x21,
		.priority2 = 0xf8,
		.domain_number = 0,
		.log_sync_interval = 0,
		.log_announce_interval = 0,
		.log_pdelay_interval = 0,
		.port_number = 0,
	};

	memset(avb_interface.object_name, 0, sizeof(avb_interface.object_name));
	strncpy(avb_interface.object_name, "", 63);
	memcpy(avb_interface.mac_address, server->mac_addr, 6);
	server_add_descriptor(server, AVB_AEM_DESC_AVB_INTERFACE, 0,
			sizeof(avb_interface), &avb_interface);

	struct avb_aem_desc_clock_source clock_source_internal = {
		.object_name = "Internal",
		.localized_description = htons(0xffff),
		.clock_source_flags = htons(2),
		.clock_source_type = htons(
				AVB_AEM_DESC_CLOCK_SOURCE_TYPE_INTERNAL),
		.clock_source_identifier = htobe64(0),
		.clock_source_location_type = htons(AVB_AEM_DESC_CLOCK_SOURCE),
		.clock_source_location_index = htons(0),
	};
	server_add_descriptor(server, AVB_AEM_DESC_CLOCK_SOURCE, 0,
			sizeof(clock_source_internal), &clock_source_internal);

	struct avb_aem_desc_clock_source clock_source_aaf = {
		.object_name = "Stream Clock",
		.localized_description = htons(0xffff),
		.clock_source_flags = htons(2),
		.clock_source_type = htons(
				AVB_AEM_DESC_CLOCK_SOURCE_TYPE_INPUT_STREAM),
		.clock_source_identifier = htobe64(0),
		.clock_source_location_type = htons(AVB_AEM_DESC_STREAM_INPUT),
		.clock_source_location_index = htons(0),
	};
	server_add_descriptor(server, AVB_AEM_DESC_CLOCK_SOURCE, 1,
			sizeof(clock_source_aaf), &clock_source_aaf);

	struct avb_aem_desc_clock_source clock_source_crf = {
		.object_name = "CRF Clock",
		.localized_description = htons(0xffff),
		.clock_source_flags = htons(2),
		.clock_source_type = htons(
				AVB_AEM_DESC_CLOCK_SOURCE_TYPE_INPUT_STREAM),
		.clock_source_identifier = htobe64(0),
		.clock_source_location_type = htons(AVB_AEM_DESC_STREAM_INPUT),
		.clock_source_location_index = htons(1),
	};
	server_add_descriptor(server, AVB_AEM_DESC_CLOCK_SOURCE, 2,
			sizeof(clock_source_crf), &clock_source_crf);

	struct {
		struct avb_aem_desc_clock_domain desc;
		uint16_t clock_sources[3];
	} __attribute__ ((__packed__)) clock_domain = {
		.desc = {
			.object_name = "Clock Reference Format",
			.localized_description = htons(0xffff),
			.clock_source_index = htons(0),
			.descriptor_counts_offset = htons(
			4 + sizeof(struct avb_aem_desc_clock_domain)),
			.clock_sources_count = htons(3),
		},
		.clock_sources = {
		    htons(0),
		    htons(1),
		    htons(2)
		},
	};

	server_add_descriptor(server, AVB_AEM_DESC_CLOCK_DOMAIN, 0,
			sizeof(clock_domain), &clock_domain);
}

#endif