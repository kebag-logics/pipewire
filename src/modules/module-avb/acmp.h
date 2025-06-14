/* AVB support */
/* SPDX-FileCopyrightText: Copyright © 2022 Wim Taymans */
/* SPDX-License-Identifier: MIT */

#ifndef AVB_ACMP_H
#define AVB_ACMP_H

#include "packets.h"
#include "internal.h"

/*IEEE 1722.1-2021, Table 8-2 - message_type field*/
#define AVB_ACMP_MESSAGE_TYPE_CONNECT_TX_COMMAND		0
#define AVB_ACMP_MESSAGE_TYPE_CONNECT_TX_RESPONSE		1
#define AVB_ACMP_MESSAGE_TYPE_DISCONNECT_TX_COMMAND		2
#define AVB_ACMP_MESSAGE_TYPE_DISCONNECT_TX_RESPONSE		3
#define AVB_ACMP_MESSAGE_TYPE_GET_TX_STATE_COMMAND		4
#define AVB_ACMP_MESSAGE_TYPE_GET_TX_STATE_RESPONSE		5
#define AVB_ACMP_MESSAGE_TYPE_CONNECT_RX_COMMAND		6
#define AVB_ACMP_MESSAGE_TYPE_CONNECT_RX_RESPONSE		7
#define AVB_ACMP_MESSAGE_TYPE_DISCONNECT_RX_COMMAND		8
#define AVB_ACMP_MESSAGE_TYPE_DISCONNECT_RX_RESPONSE		9
#define AVB_ACMP_MESSAGE_TYPE_GET_RX_STATE_COMMAND		10
#define AVB_ACMP_MESSAGE_TYPE_GET_RX_STATE_RESPONSE		11
#define AVB_ACMP_MESSAGE_TYPE_GET_TX_CONNECTION_COMMAND		12
#define AVB_ACMP_MESSAGE_TYPE_GET_TX_CONNECTION_RESPONSE	13

/* IEEE 1722.1-2021, Table 8-3 - status field*/
#define AVB_ACMP_STATUS_SUCCESS				0
#define AVB_ACMP_STATUS_LISTENER_UNKNOWN_ID		1
#define AVB_ACMP_STATUS_TALKER_UNKNOWN_ID		2
#define AVB_ACMP_STATUS_TALKER_DEST_MAC_FAIL		3
#define AVB_ACMP_STATUS_TALKER_NO_STREAM_INDEX		4
#define AVB_ACMP_STATUS_TALKER_NO_BANDWIDTH		5
#define AVB_ACMP_STATUS_TALKER_EXCLUSIVE		6
#define AVB_ACMP_STATUS_LISTENER_TALKER_TIMEOUT		7
#define AVB_ACMP_STATUS_LISTENER_EXCLUSIVE		8
#define AVB_ACMP_STATUS_STATE_UNAVAILABLE		9
#define AVB_ACMP_STATUS_NOT_CONNECTED			10
#define AVB_ACMP_STATUS_NO_SUCH_CONNECTION		11
#define AVB_ACMP_STATUS_COULD_NOT_SEND_MESSAGE		12
#define AVB_ACMP_STATUS_TALKER_MISBEHAVING		13
#define AVB_ACMP_STATUS_LISTENER_MISBEHAVING		14
#define AVB_ACMP_STATUS_RESERVED			15
#define AVB_ACMP_STATUS_CONTROLLER_NOT_AUTHORIZED	16
#define AVB_ACMP_STATUS_INCOMPATIBLE_REQUEST		17
#define AVB_ACMP_STATUS_LISTENER_INVALID_CONNECTION	18
#define AVB_ACMP_STATUS_LISTENER_CAN_ONLY_LISTEN_ONCE 	19
#define AVB_ACMP_STATUS_NOT_SUPPORTED			31

/* Milan v1.2, Table 5.5: Probing status values. */
#define AVB_MILAN_ACMP_STATUS_PROBING_DISABLED 		0
#define AVB_MILAN_ACMP_STATUS_PROBING_PASSIVE 		1
#define AVB_MILAN_ACMP_STATUS_PROBING_ACTIVE 		2
#define AVB_MILAN_ACMP_STATUS_PROBING_COMPLETED 	3

/* IEEE 1722.1-2021, Table 8-1 - ACMP command timeouts*/
#define AVB_ACMP_TIMEOUT_CONNECT_TX_COMMAND_MS		2000
#define AVB_ACMP_TIMEOUT_DISCONNECT_TX_COMMAND_MS	200
#define AVB_ACMP_TIMEOUT_GET_TX_STATE_COMMAND		200
#define AVB_ACMP_TIMEOUT_CONNECT_RX_COMMAND_MS		4500
#define AVB_ACMP_TIMEOUT_DISCONNECT_RX_COMMAND_MS	500
#define AVB_ACMP_TIMEOUT_GET_RX_STATE_COMMAND_MS	200
#define AVB_ACMP_TIMEOUT_GET_TX_CONNECTION_COMMAND	200

/* Milan v1.2, Table 5.26: Command timeouts values. */
#define AVB_MILAN_ACMP_TIMEOUT_PROBE_TX_COMMAND_MS 		200
#define AVB_MILAN_ACMP_TIMEOUT_GET_TX_STATE_COMMAND_MS 	200
#define AVB_MILAN_ACMP_TIMEOUT_BIND_RX_COMMAND_MS		200
#define AVB_MILAN_ACMP_TIMEOUT_UNBIND_RX_COMMAND_MS		200
#define AVB_MILAN_ACMP_TIMEOUT_GET_RX_STATE_COMMAND_MS	200


/* IEEE 1722.1-2021 Table 8.4 flags field */

#define AVB_MILAN_ACMP_FLAGS_CLASS_B							(1<<15)
#define AVB_MILAN_ACMP_FLAGS_FAST_CONNECT						(1<<14)
#define AVB_MILAN_ACMP_FLAGS_SAVE_STATE							(1<<13)
#define AVB_MILAN_ACMP_FLAGS_STREAMING_WAIT						(1<<12)
#define AVB_MILAN_ACMP_FLAGS_SUPPORT_ENCRYPTED					(1<<11)
#define AVB_MILAN_ACMP_FLAGS_ENCTRYPED_PDU						(1<<10)
#define AVB_MILAN_ACMP_FLAGS_SRP_REGISTRATION_FAILED			(1<<9)
#define AVB_MILAN_ACMP_FLAGS_CL_ENTRIES_VALID					(1<<8)
#define AVB_MILAN_ACMP_FLAGS_NO_SRP								(1<<7)
#define AVB_MILAN_ACMP_FLAGS_UDP								(1<<6)

enum milan_acmp_talker_sta {
    MILAN_AMCP_TALKER_STA_MAX
};

/** Milan v1.2 ACMP */
enum milan_acmp_listener_sta {
    MILAN_ACMP_LISTENER_STA_UNBOUND,
    MILAN_ACMP_LISTENER_STA_PRB_W_AVAIL,
    MILAN_ACMP_LISTENER_STA_PRB_W_DELAY,
    MILAN_ACMP_LISTENER_STA_PRB_W_RESP,
    MILAN_ACMP_LISTENER_STA_PRB_W_RESP2,
    MILAN_ACMP_LISTENER_STA_PRB_W_RETRY,
    MILAN_ACMP_LISTENER_STA_SETTLED_NO_RSV,
    MILAN_ACMP_LISTENER_STA_SETTLED_RSV_OK,

    MILAN_ACMP_LISTENER_STA_MAX,
};

struct avb_packet_acmp {
    struct avb_packet_header hdr;
    uint64_t stream_id;
    uint64_t controller_guid;
    uint64_t talker_guid;
    uint64_t listener_guid;
    uint16_t talker_unique_id;
    uint16_t listener_unique_id;
    char stream_dest_mac[6];
    uint16_t connection_count;
    uint16_t sequence_id;
    uint16_t flags;
    uint16_t stream_vlan_id;
    uint16_t reserved;
} __attribute__ ((__packed__));

#define AVB_PACKET_ACMP_SET_MESSAGE_TYPE(p,v)		AVB_PACKET_SET_SUB1(&(p)->hdr, v)
#define AVB_PACKET_ACMP_SET_STATUS(p,v)			AVB_PACKET_SET_SUB2(&(p)->hdr, v)

#define AVB_PACKET_ACMP_GET_MESSAGE_TYPE(p)		AVB_PACKET_GET_SUB1(&(p)->hdr)
#define AVB_PACKET_ACMP_GET_STATUS(p)			AVB_PACKET_GET_SUB2(&(p)->hdr)
struct fsm_state_talker {
    struct spa_list link;

    uint64_t stream_id;
    enum milan_acmp_talker_sta current_state;
    int64_t timeout;
};

struct fsm_binding_parameters {
    uint32_t status;
    uint64_t controller_entity_id;
    uint64_t talker_entity_id;
    uint64_t listener_entity_id;

    uint16_t talker_unique_id;
    uint16_t listener_unique_id;

    uint16_t sequence_id;

    uint64_t stream_id;
    char stream_dest_mac[6];
    uint8_t stream_vlan_id;
};

struct fsm_state_listener {
    struct spa_list link;

    struct fsm_binding_parameters binding_parameters;

    enum milan_acmp_listener_sta current_state;
    int64_t timeout;
    uint16_t flags;
    uint8_t probing_status;
    uint16_t connection_count;
    uint8_t STREAMING_WAIT;

    // FIXME: Is it necessary? remove if not
    uint8_t buf[2048];
};

struct avb_acmp *avb_acmp_register_listener(struct server *server,
        struct fsm_state_listener *fsm_state_listener);

struct avb_acmp *avb_acmp_register_talker(struct server *server,
        struct fsm_state_talker *fsm_talker);

struct avb_acmp *avb_acmp_register(struct server *server);
void avb_acmp_unregister(struct avb_acmp *acmp);

#endif /* AVB_ACMP_H */
