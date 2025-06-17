/* AVB support */
/* SPDX-FileCopyrightText: Copyright © 2025 Kebag-Logic */
/* SPDX-FileCopyrightText: Copyright © 2025 Alexandre Malki <alexandre.malki@kebag-logic.com> */
/* SPDX-License-Identifier: MIT */

#ifndef AVB_AECP_AEM_STATE_H
#define AVB_AECP_AEM_STATE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "internal.h"

struct aem_state_var_info {
    /** The name of the var for debug */
    const char *var_name;
    /** persisted */
    const bool is_persited;
    /** The descriptor type it belongs to */
    // FIXME Remove once the refactoring is finished
    const uint16_t desc_type;

    /** Not all var can time out */
    bool expires;

    /** The counts of the vars per entity */
    // FIXME Remove once the refactoring is finished
    size_t      count;
    /** Element size */
    size_t      el_sz;
};

/**
 * Basic information about the last time it was updated, or will expired
 * and the controller that last accessed it
 */
struct aecp_aem_base_info {
    struct aem_state_var_info var_info;
    /** Originator of the control
     * This is needed so the unsoolictied notification does not send back SUCCESS
     * to the originator of of the unsolicited notification */
    // FIXME the only case when this is needed is when a registred controller,
    // Has already provided the necessary information
    uint64_t controller_entity_id;

    /** Check the need for an update, that is usually updated when setting var. */
    bool needs_update;

    /** timeout absolute time*/
    int64_t expire_timeout;
};

struct aecp_aem_lock_state {
    struct aecp_aem_base_info base_info;
    /**
     * The entity id that is locking this system
     */
    uint64_t locked_id;

    /**
     * Actual value of the lock
     */
    bool is_locked;
};

struct aecp_aem_unsol_notification_state {
    struct aecp_aem_base_info base_info;
    /**
     * The controller is that is locking this system
     */
    uint64_t ctrler_endity_id;

    /**
     * mac Address of the controller
     */
    uint8_t ctrler_mac_addr[6];

    /**
     * Port where the registeration originated from
     */
    uint8_t port_id;

    /***
     * The sequence ID of the next unsolicited notification
     */

    uint16_t next_seq_id;
    /**
     * Actual value of the lock, get removed when unregistere or expired.
     */
    bool is_registered;

};

#define AECP_AEM_MILAN_MAX_CONTROLLER 16
struct aecp_aem_entity_state {
    struct aecp_aem_lock_state lock_state;
    struct aecp_aem_unsol_notification_state unsol_notif_state[AECP_AEM_MILAN_MAX_CONTROLLER];
    struct avb_aem_desc_entity desc;
};

struct aecp_aem_configuration_state {
    struct aecp_aem_base_info base_info;
    uint16_t cfg_idx;
};

/**
 * The aecp_aem_desc_base inherites from the base
 */
struct aecp_aem_desc_base {
    struct aecp_aem_base_info base_info;
    // TODO clean, config_index is held in the descriptor.
    uint16_t config_index;
    void *desc;
};

/**
 * The aecp_aem_desc_base inherites from the base
 */
struct aecp_aem_desc {
    struct aecp_aem_desc_base base_desc;
};

/** The control information to keep track of the latest changes */
struct aecp_aem_control_state {
    struct aecp_aem_desc_base base;
    struct avb_aem_desc_control desc;
};

/**
 * To keep track of which desciptor has chaanged
 */
struct aecp_aem_name_state {
    struct aecp_aem_desc_base base_desc;
    uint16_t name_index;
};

/**
 * To keep track of which desciptor has changed  */
struct aecp_aem_stream_format_state {
    struct aecp_aem_desc_base base_desc;
};

/** To keep track of the unsolicited notifications */
struct aecp_aem_clock_domain_state {
    struct aecp_aem_desc_base base_desc;
};

/** To keep track of the unsolicited notifications */
struct aecp_aem_sampling_rate_state {
    struct aecp_aem_desc_base base_desc;
};

/**
 * Milan v1.2 Table 5.13: GET_COUNTERSmandatory AVB Interface counters
 * Table 5.14: GET_COUNTERS optional AVB Interface counters
 */
struct aecp_aem_counter_avb_interface_state {
    struct aecp_aem_desc_base base_desc;

    uint32_t link_up;
    uint32_t link_down;
    uint32_t gptp_gm_changed;
    // optional AVB Interface counters
    uint32_t frame_tx;
    uint32_t frame_rx;
    uint32_t error_crc;
};

struct aecp_aem_avb_interface_state {
    struct aecp_aem_counter_avb_interface_state counters;
    struct avb_aem_desc_avb_interface desc;
};

/**
 * Milan v1.2 Table 5.15: GET_COUNTERS mandatory AVB Interface counters
 */
 struct aecp_aem_counter_clock_domain_state {
    struct aecp_aem_desc_base base_desc;

    uint32_t locked;
    uint32_t unlocked;
};

/**
 * Milan v1.2 Table 5.16: GET_COUNTERS Stream Input counters
 */
struct aecp_aem_counter_stream_input_state {
    struct aecp_aem_desc_base base_desc;

    uint32_t media_locked;
    uint32_t media_unlocked;
    uint32_t stream_interrupted;
    uint32_t seq_mistmatch;
    uint32_t media_reset;
    uint32_t tu;
    uint32_t unsupported_format;
    uint32_t late_timestamp;
    uint32_t early_timestamp;
    uint32_t frame_rx;
};

struct aecp_aem_stream_output_state {
    struct aecp_aem_counter_stream_input_state counters;
    struct stream stream;
    struct avb_aem_desc_stream desc;
};


/**
 * Milan v1.2 Table 5.17: GET_COUNTERS Stream Output counters
 */
struct aecp_aem_counter_stream_output_state {
    struct aecp_aem_desc_base base_desc;
    uint32_t stream_start;
    uint32_t stream_stop;
    uint32_t media_reset;
    uint32_t tu;
    uint32_t frame_tx;
};

struct aecp_aem_stream_output_state {
    struct aecp_aem_counter_stream_output_state counters;
    struct stream stream;
    struct avb_aem_desc_stream desc;
};

struct aecp_aem_stream_input_state {
    struct aecp_aem_counter_stream_input_state counters;
    struct stream stream;
    struct avb_aem_desc_stream desc;
};

#endif // AVB_AECP_AEM_STATE_H
