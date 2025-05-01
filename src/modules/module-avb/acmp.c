/* AVB support */
/* SPDX-FileCopyrightText: Copyright © 2022 Wim Taymans */
/* SPDX-FileCopyrightText: Copyright © 2025 Kebag-Logic */
/* SPDX-FileCopyrightText: Copyright © 2025 Alex Malki <alexandre.malki@kebag-logic.com> */
/* SPDX-FileCopyrightText: Copyright © 2025 Simon Gapp <simon.gapp@kebag-logic.com> */
/* SPDX-License-Identifier: MIT */

#include <spa/utils/json.h>
#include <spa/debug/mem.h>

#include <pipewire/pipewire.h>

#include "acmp.h"
#include "msrp.h"
#include "internal.h"
#include "stream.h"
#include "aecp-aem.h"

static const uint8_t mac[6] = AVB_BROADCAST_MAC;


struct pending {
	struct spa_list link;
	uint64_t last_time;
	uint64_t timeout;
	uint16_t old_sequence_id;
	uint16_t sequence_id;
	uint16_t retry;
	size_t size;
	void *ptr;
};

struct fsm_state_listener {
	struct spa_list link;

	uint64_t stream_id;
	enum milan_acmp_listener_sta current_state;
	int64_t timeout;
	size_t size;
	void *ptr;
};

struct acmp {
	struct server *server;
	struct spa_hook server_listener;

#define PENDING_TALKER		0
#define PENDING_LISTENER	1
#define PENDING_CONTROLLER	2
	struct spa_list pending[3];

#define STREAM_LISTENER_FSM 0
#define STREAM_TALKER_FSM 1
	struct spa_list stream_fsm[2];
	uint16_t sequence_id[3];

};


static struct fsm_state_listener *acmp_fsm_find(struct acmp *acmp, int type, uint64_t id)
{
	struct fsm_state_listener *fsm;
	spa_list_for_each(fsm, &acmp->stream_fsm[type], link) {
		if (fsm->stream_id == id )
			return fsm;
	}

	return NULL;
}

#ifdef USE_MILAN

#define AECP_MILAN_ACMP_EVT_TMR_NO_RESP			0
#define AECP_MILAN_ACMP_EVT_TMR_RETRY			1
#define AECP_MILAN_ACMP_EVT_TMR_DELAY			2
#define AECP_MILAN_ACMP_EVT_TMR_NO_TK			3
#define AECP_MILAN_ACMP_EVT_RCV_BIND_RX_CMD		4
#define AECP_MILAN_ACMP_EVT_RCV_PROBE_TX_RESP	5
#define AECP_MILAN_ACMP_EVT_RCV_GET_RX_STATE	6
#define AECP_MILAN_ACMP_EVT_RCV_UNBIND_RX_CMD	7
#define AECP_MILAN_ACMP_EVT_TK_DISCOVERED		8
#define AECP_MILAN_ACMP_EVT_TK_DEPARTED			9
#define AECP_MILAN_ACMP_EVT_TK_REGISTERED		10
#define AECP_MILAN_ACMP_EVT_TK_UNREGISTERED		11

#define AECP_MILAN_ACMP_EVT_MAX					12

// Below are all the state according to the state machine */

struct listener_fsm_cmd {
	int (*state_handler) (struct server *, void *, size_t, int64_t);
};

/** Milan v1.2 5.5.3.5.3 */
int handle_fsm_unbound_rcv_bind_rx_cmd_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.4 */
int handle_fsm_unbound_rcv_get_rx_state_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.5 */
int handle_fsm_unbound_rcv_unbind_rx_cmd_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.6 */
int handle_fsm_prb_w_avail_rcv_bind_rx_cmd_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.7 */
int handle_fsm_prb_w_avail_rcv_get_rx_state_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.8 */
int handle_fsm_prb_w_avail_rcv_unbind_rx_cmd_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.9 */
int handle_fsm_prb_w_avail_evt_tk_discovered_evt(struct server *acmp,struct fsm_state_listener *info, void *m, size_t len,
	int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.10 */
int handle_fsm_prb_w_delay_tmr_delay_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.11 */
int handle_fsm_prb_w_delay_rcv_bind_rx_cmd_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.12 */
int handle_fsm_prb_w_delay_rcv_get_rx_state_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.13 */
int handle_fsm_prb_w_delay_rcv_unbind_rx_cmd_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.14 */
int handle_fsm_prb_w_delay_evt_tk_discovered_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.15 */
int handle_fsm_prb_w_delay_evt_tk_departed_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.16 */
int handle_fsm_prb_w_resp_tmr_no_resp_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.17 */
int handle_fsm_prb_w_resp_rcv_bind_rx_cmd_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.18 */
int handle_fsm_prb_w_resp_rcv_probe_tx_resp_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.19 */
int handle_fsm_prb_w_resp_rcv_get_rx_state_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.20 */
int handle_fsm_prb_w_resp_rcv_unbind_rx_cmd_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.21 */
int handle_fsm_prb_w_resp_evt_tk_discovered_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.22 */
int handle_fsm_prb_w_resp_evt_tk_departed_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.23 */
int handle_fsm_prb_w_resp2_tmr_no_resp_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.24 */
int handle_fsm_prb_w_resp2_rcv_bind_rx_cmd_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.25 */
int handle_fsm_prb_w_resp2_rcv_probe_tx_resp_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.26 */
int handle_fsm_prb_w_resp2_rcv_get_rx_state_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.27 */
int handle_fsm_prb_w_resp2_rcv_unbind_rx_cmd_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.28 */
int handle_fsm_prb_w_resp2_evt_tk_discovered_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.29 */
int handle_fsm_prb_w_resp2_evt_tk_departed_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.30 */
int handle_fsm_prb_w_retry_tmr_retry_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.31 */
int handle_fsm_prb_w_retry_rcv_bind_rx_cmd_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.32 */
int handle_fsm_prb_w_retry_rcv_get_rx_state_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.33 */
int handle_fsm_prb_w_retry_rcv_unbind_rx_cmd_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.34 */
int handle_fsm_prb_w_retry_evt_tk_discovered_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.35 */
int handle_fsm_prb_w_retry_evt_tk_departed_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.36 */
int handle_fsm_settled_no_rsv_tmr_no_tk_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.37 */
int handle_fsm_settled_no_rsv_rcv_bind_rx_cmd_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.38 */
int handle_fsm_settled_no_rsv_rcv_get_rx_state_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.39 */
int handle_fsm_settled_no_rsv_rcv_unbind_rx_cmd_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.40 */
int handle_fsm_settled_no_rsv_evt_tk_discovered_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.41 */
int handle_fsm_settled_no_rsv_evt_tk_departed_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.42 */
int handle_fsm_settled_no_rsv_evt_tk_registered_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.43 */
int handle_fsm_settled_rsv_ok_rcv_bind_rx_cmd_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.44 */
int handle_fsm_settled_rsv_ok_rcv_get_rx_state_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.45 */
int handle_fsm_settled_rsv_ok_rcv_unbind_rx_cmd_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.46 */
int handle_fsm_settled_rsv_ok_evt_tk_discovered_evt(struct server *acmp,
	struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.47 */
int handle_fsm_settled_rsv_ok_evt_tk_departed_evt(struct server *acmp,
	 struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

/** Milan v1.2 5.5.3.5.48 */
int handle_fsm_settled_rsv_ok_evt_tk_unregistered_evt(struct server *acmp,
	 struct fsm_state_listener *info, void *m, size_t len, int64_t now)
{
	return 0;
}

static const struct listener_fsm_cmd listener_unbound[AECP_MILAN_ACMP_EVT_MAX] = {
    [AECP_MILAN_ACMP_EVT_RCV_BIND_RX_CMD] = handle_fsm_unbound_rcv_bind_rx_cmd_evt, // 5.5.3.5.3
    [AECP_MILAN_ACMP_EVT_RCV_GET_RX_STATE] = handle_fsm_unbound_rcv_get_rx_state_evt, // 5.5.3.5.4
    [AECP_MILAN_ACMP_EVT_RCV_UNBIND_RX_CMD] = handle_fsm_unbound_rcv_unbind_rx_cmd_evt, // 5.5.3.5.5
    // Other events have no specific handler in this state per the table
};

static const struct listener_fsm_cmd listener_prb_w_avail[AECP_MILAN_ACMP_EVT_MAX] = {
    [AECP_MILAN_ACMP_EVT_RCV_BIND_RX_CMD] = handle_fsm_prb_w_avail_rcv_bind_rx_cmd_evt, // 5.5.3.5.6
    [AECP_MILAN_ACMP_EVT_RCV_GET_RX_STATE] = handle_fsm_prb_w_avail_rcv_get_rx_state_evt, // 5.5.3.5.7
    [AECP_MILAN_ACMP_EVT_RCV_UNBIND_RX_CMD] = handle_fsm_prb_w_avail_rcv_unbind_rx_cmd_evt, // 5.5.3.5.8
    [AECP_MILAN_ACMP_EVT_TK_DISCOVERED] = handle_fsm_prb_w_avail_evt_tk_discovered_evt, // 5.5.3.5.9
    // Other events have no specific handler in this state per the table
};

static const struct listener_fsm_cmd listener_prb_w_delay[AECP_MILAN_ACMP_EVT_MAX] = {
    [AECP_MILAN_ACMP_EVT_TMR_DELAY] = handle_fsm_prb_w_delay_tmr_delay_evt, // 5.5.3.5.10
    [AECP_MILAN_ACMP_EVT_RCV_BIND_RX_CMD] = handle_fsm_prb_w_delay_rcv_bind_rx_cmd_evt, // 5.5.3.5.11
    [AECP_MILAN_ACMP_EVT_RCV_GET_RX_STATE] = handle_fsm_prb_w_delay_rcv_get_rx_state_evt, // 5.5.3.5.12
    [AECP_MILAN_ACMP_EVT_RCV_UNBIND_RX_CMD] = handle_fsm_prb_w_delay_rcv_unbind_rx_cmd_evt, // 5.5.3.5.13
    [AECP_MILAN_ACMP_EVT_TK_DISCOVERED] = handle_fsm_prb_w_delay_evt_tk_discovered_evt, // 5.5.3.5.14 (Added based on table)
    [AECP_MILAN_ACMP_EVT_TK_DEPARTED] = handle_fsm_prb_w_delay_evt_tk_departed_evt, // 5.5.3.5.15 (Added based on table)
     // Note: Corrected potential duplicate RCV_UNBIND_RX_CMD from template
     // Other events have no specific handler in this state per the table
};

static const struct listener_fsm_cmd listener_prb_w_resp[AECP_MILAN_ACMP_EVT_MAX] = {
    [AECP_MILAN_ACMP_EVT_TMR_NO_RESP] = handle_fsm_prb_w_resp_tmr_no_resp_evt, // 5.5.3.5.16 (Added based on table)
    [AECP_MILAN_ACMP_EVT_RCV_BIND_RX_CMD] = handle_fsm_prb_w_resp_rcv_bind_rx_cmd_evt, // 5.5.3.5.17
    [AECP_MILAN_ACMP_EVT_RCV_PROBE_TX_RESP] = handle_fsm_prb_w_resp_rcv_probe_tx_resp_evt, // 5.5.3.5.18 (Added based on table)
    [AECP_MILAN_ACMP_EVT_RCV_GET_RX_STATE] = handle_fsm_prb_w_resp_rcv_get_rx_state_evt, // 5.5.3.5.19
    [AECP_MILAN_ACMP_EVT_RCV_UNBIND_RX_CMD] = handle_fsm_prb_w_resp_rcv_unbind_rx_cmd_evt, // 5.5.3.5.20
    [AECP_MILAN_ACMP_EVT_TK_DISCOVERED] = handle_fsm_prb_w_resp_evt_tk_discovered_evt, // 5.5.3.5.21 (Added based on table)
    [AECP_MILAN_ACMP_EVT_TK_DEPARTED] = handle_fsm_prb_w_resp_evt_tk_departed_evt, // 5.5.3.5.22 (Added based on table)
    // Other events have no specific handler in this state per the table
};

static const struct listener_fsm_cmd listener_prb_w_resp2[AECP_MILAN_ACMP_EVT_MAX] = {
    [AECP_MILAN_ACMP_EVT_TMR_NO_RESP] = handle_fsm_prb_w_resp2_tmr_no_resp_evt, // 5.5.3.5.23 (Added based on table)
    [AECP_MILAN_ACMP_EVT_RCV_BIND_RX_CMD] = handle_fsm_prb_w_resp2_rcv_bind_rx_cmd_evt, // 5.5.3.5.24
    [AECP_MILAN_ACMP_EVT_RCV_PROBE_TX_RESP] = handle_fsm_prb_w_resp2_rcv_probe_tx_resp_evt, // 5.5.3.5.25 (Added based on table)
    [AECP_MILAN_ACMP_EVT_RCV_GET_RX_STATE] = handle_fsm_prb_w_resp2_rcv_get_rx_state_evt, // 5.5.3.5.26
    [AECP_MILAN_ACMP_EVT_RCV_UNBIND_RX_CMD] = handle_fsm_prb_w_resp2_rcv_unbind_rx_cmd_evt, // 5.5.3.5.27
    [AECP_MILAN_ACMP_EVT_TK_DISCOVERED] = handle_fsm_prb_w_resp2_evt_tk_discovered_evt, // 5.5.3.5.28 (Added based on table)
    [AECP_MILAN_ACMP_EVT_TK_DEPARTED] = handle_fsm_prb_w_resp2_evt_tk_departed_evt, // 5.5.3.5.29 (Added based on table)
    // Other events have no specific handler in this state per the table
};

static const struct listener_fsm_cmd listener_prb_w_retry[AECP_MILAN_ACMP_EVT_MAX] = {
    [AECP_MILAN_ACMP_EVT_TMR_RETRY] = handle_fsm_prb_w_retry_tmr_retry_evt, // 5.5.3.5.30 (Added based on table)
    [AECP_MILAN_ACMP_EVT_RCV_BIND_RX_CMD] = handle_fsm_prb_w_retry_rcv_bind_rx_cmd_evt, // 5.5.3.5.31
    [AECP_MILAN_ACMP_EVT_RCV_GET_RX_STATE] = handle_fsm_prb_w_retry_rcv_get_rx_state_evt, // 5.5.3.5.32
    [AECP_MILAN_ACMP_EVT_RCV_UNBIND_RX_CMD] = handle_fsm_prb_w_retry_rcv_unbind_rx_cmd_evt, // 5.5.3.5.33
    [AECP_MILAN_ACMP_EVT_TK_DISCOVERED] = handle_fsm_prb_w_retry_evt_tk_discovered_evt, // 5.5.3.5.34 (Added based on table)
    [AECP_MILAN_ACMP_EVT_TK_DEPARTED] = handle_fsm_prb_w_retry_evt_tk_departed_evt, // 5.5.3.5.35 (Added based on table)
    // Other events have no specific handler in this state per the table
};

static const struct listener_fsm_cmd listener_settled_no_rsv[AECP_MILAN_ACMP_EVT_MAX] = {
    [AECP_MILAN_ACMP_EVT_TMR_NO_TK] = handle_fsm_settled_no_rsv_tmr_no_tk_evt, // 5.5.3.5.36 (Added based on table)
    [AECP_MILAN_ACMP_EVT_RCV_BIND_RX_CMD] = handle_fsm_settled_no_rsv_rcv_bind_rx_cmd_evt, // 5.5.3.5.37
    [AECP_MILAN_ACMP_EVT_RCV_GET_RX_STATE] = handle_fsm_settled_no_rsv_rcv_get_rx_state_evt, // 5.5.3.5.38
    [AECP_MILAN_ACMP_EVT_RCV_UNBIND_RX_CMD] = handle_fsm_settled_no_rsv_rcv_unbind_rx_cmd_evt, // 5.5.3.5.39
    [AECP_MILAN_ACMP_EVT_TK_DISCOVERED] = handle_fsm_settled_no_rsv_evt_tk_discovered_evt, // 5.5.3.5.40 (Added based on table)
    [AECP_MILAN_ACMP_EVT_TK_DEPARTED] = handle_fsm_settled_no_rsv_evt_tk_departed_evt, // 5.5.3.5.41 (Added based on table)
    [AECP_MILAN_ACMP_EVT_TK_REGISTERED] = handle_fsm_settled_no_rsv_evt_tk_registered_evt, // 5.5.3.5.42 (Added based on table)
    // Other events have no specific handler in this state per the table
};

static const struct listener_fsm_cmd listener_settled_rsv_ok[AECP_MILAN_ACMP_EVT_MAX] = {
    [AECP_MILAN_ACMP_EVT_RCV_BIND_RX_CMD] = handle_fsm_settled_rsv_ok_rcv_bind_rx_cmd_evt, // 5.5.3.5.43
    [AECP_MILAN_ACMP_EVT_RCV_GET_RX_STATE] = handle_fsm_settled_rsv_ok_rcv_get_rx_state_evt, // 5.5.3.5.44
    [AECP_MILAN_ACMP_EVT_RCV_UNBIND_RX_CMD] = handle_fsm_settled_rsv_ok_rcv_unbind_rx_cmd_evt, // 5.5.3.5.45
    [AECP_MILAN_ACMP_EVT_TK_DISCOVERED] = handle_fsm_settled_rsv_ok_evt_tk_discovered_evt, // 5.5.3.5.46 (Added based on table)
    [AECP_MILAN_ACMP_EVT_TK_DEPARTED] = handle_fsm_settled_rsv_ok_evt_tk_departed_evt, // 5.5.3.5.47 (Added based on table)
    [AECP_MILAN_ACMP_EVT_TK_UNREGISTERED] = handle_fsm_settled_rsv_ok_evt_tk_unregistered_evt, // 5.5.3.5.48 (Added based on table)
    // Other events have no specific handler in this state per the table
};

static const struct listener_fsm_cmd *cmd_listeners_states[MILAN_ACMP_LISTENER_STA_MAX] = {
	[MILAN_ACMP_LISTENER_STA_UNBOUND] = listener_unbound,
	[MILAN_ACMP_LISTENER_STA_PRB_W_AVAIL] = listener_prb_w_avail,
	[MILAN_ACMP_LISTENER_STA_PRB_W_DELAY] = listener_prb_w_delay,
	[MILAN_ACMP_LISTENER_STA_PRB_W_RESP] = listener_prb_w_resp,
	[MILAN_ACMP_LISTENER_STA_PRB_W_RESP2] = listener_prb_w_resp2,
	[MILAN_ACMP_LISTENER_STA_PRB_W_RETRY] = listener_prb_w_retry,
	[MILAN_ACMP_LISTENER_STA_SETTLED_NO_RSV] = listener_settled_no_rsv,
	[MILAN_ACMP_LISTENER_STA_SETLED_RSV_OK] = listener_settled_rsv_ok,
};

#endif // USE_MILAN


static void *stream_listener_fsm_new(struct acmp *acmp, uint32_t type, uint64_t now, uint32_t timeout_ms,
	const void *m, size_t size)
{
	struct fsm_state_listener *p;

	p = calloc(1, sizeof(*p) + size);
	if (p == NULL)
		return NULL;

	p->timeout = now + (timeout_ms * SPA_NSEC_PER_MSEC);
	p->size = size;

	spa_list_append(&acmp->stream_fsm[type], &p->link);
}

static void *pending_new(struct acmp *acmp, uint32_t type, uint64_t now, uint32_t timeout_ms,
		const void *m, size_t size)
{
	struct pending *p;
	struct avb_ethernet_header *h;
	struct avb_packet_acmp *pm;

	p = calloc(1, sizeof(*p) + size);
	if (p == NULL)
		return NULL;
	p->last_time = now;
	p->timeout = timeout_ms * SPA_NSEC_PER_MSEC;
	p->sequence_id = acmp->sequence_id[type]++;
	p->size = size;
	p->ptr = SPA_PTROFF(p, sizeof(*p), void);
	memcpy(p->ptr, m, size);

	h = p->ptr;
	pm = SPA_PTROFF(h, sizeof(*h), void);
	p->old_sequence_id = ntohs(pm->sequence_id);
	pm->sequence_id = htons(p->sequence_id);
	spa_list_append(&acmp->pending[type], &p->link);

	return p->ptr;
}

static struct pending *pending_find(struct acmp *acmp, uint32_t type, uint16_t sequence_id)
{
	struct pending *p;
	spa_list_for_each(p, &acmp->pending[type], link)
		if (p->sequence_id == sequence_id)
			return p;
	return NULL;
}

static void pending_free(struct acmp *acmp, struct pending *p)
{
	spa_list_remove(&p->link);
	free(p);
}

struct msg_info {
	uint16_t type;
	const char *name;
	int (*handle) (struct acmp *acmp, uint64_t now, const void *m, int len);
};

static int reply_not_supported(struct acmp *acmp, uint8_t type, const void *m, int len)
{
	struct server *server = acmp->server;
	uint8_t buf[len];
	struct avb_ethernet_header *h = (void*)buf;
	struct avb_packet_acmp *reply = SPA_PTROFF(h, sizeof(*h), void);

	memcpy(h, m, len);
	AVB_PACKET_ACMP_SET_MESSAGE_TYPE(reply, type);
	AVB_PACKET_ACMP_SET_STATUS(reply, AVB_ACMP_STATUS_NOT_SUPPORTED);

	return avb_server_send_packet(server, h->src, AVB_TSN_ETH, buf, len);
}

static int retry_pending(struct acmp *acmp, uint64_t now, struct pending *p)
{
	struct server *server = acmp->server;
	struct avb_ethernet_header *h = p->ptr;
	p->retry++;
	p->last_time = now;
	return avb_server_send_packet(server, h->dest, AVB_TSN_ETH, p->ptr, p->size);
}

static int handle_connect_tx_command(struct acmp *acmp, uint64_t now, const void *m, int len)
{
	struct server *server = acmp->server;
	uint8_t buf[len];
	struct avb_ethernet_header *h = (void*)buf;
	struct avb_packet_acmp *reply = SPA_PTROFF(h, sizeof(*h), void);
	const struct avb_packet_acmp *p = SPA_PTROFF(m, sizeof(*h), void);
	int status = AVB_ACMP_STATUS_SUCCESS;
	struct stream *stream;

	if (be64toh(p->talker_guid) != server->entity_id)
		return 0;

	memcpy(buf, m, len);
	stream = server_find_stream(server, SPA_DIRECTION_OUTPUT,
			reply->talker_unique_id);
	if (stream == NULL) {
		status = AVB_ACMP_STATUS_TALKER_NO_STREAM_INDEX;
		goto done;
	}

	AVB_PACKET_ACMP_SET_MESSAGE_TYPE(reply, AVB_ACMP_MESSAGE_TYPE_CONNECT_TX_RESPONSE);
	reply->stream_id = htobe64(stream->id);

	stream_activate(stream, now);

	memcpy(reply->stream_dest_mac, stream->addr, 6);
	reply->connection_count = htons(1);
	reply->stream_vlan_id = htons(stream->vlan_id);

done:
	AVB_PACKET_ACMP_SET_STATUS(reply, status);
	return avb_server_send_packet(server, h->dest, AVB_TSN_ETH, buf, len);
}

static int handle_connect_tx_response(struct acmp *acmp, uint64_t now, const void *m, int len)
{
	struct server *server = acmp->server;
	struct avb_ethernet_header *h;
	const struct avb_packet_acmp *resp = SPA_PTROFF(m, sizeof(*h), void);
	struct avb_packet_acmp *reply;
	struct pending *pending;
	uint16_t sequence_id;
	struct stream *stream;
	int res;

	if (be64toh(resp->listener_guid) != server->entity_id)
		return 0;

	sequence_id = ntohs(resp->sequence_id);

	pending = pending_find(acmp, PENDING_TALKER, sequence_id);
	if (pending == NULL)
		return 0;

	h = pending->ptr;
	pending->size = SPA_MIN((int)pending->size, len);
	memcpy(h, m, pending->size);

	// In Milan, the BIND_RX_RESPONSE is sent on reception of the bind_rx_cmd
#if !USE_MILAN
	reply = SPA_PTROFF(h, sizeof(*h), void);
	reply->sequence_id = htons(pending->old_sequence_id);
	AVB_PACKET_ACMP_SET_MESSAGE_TYPE(reply, AVB_ACMP_MESSAGE_TYPE_CONNECT_RX_RESPONSE);

	stream = server_find_stream(server, SPA_DIRECTION_INPUT,
			ntohs(reply->listener_unique_id));
	if (stream == NULL)
		return 0;

	stream->peer_id = be64toh(reply->stream_id);
	memcpy(stream->addr, reply->stream_dest_mac, 6);
	stream_activate(stream, now);

	res = avb_server_send_packet(server, h->dest, AVB_TSN_ETH, h, pending->size);

	pending_free(acmp, pending);
#endif

	return res;
}

static int handle_disconnect_tx_command(struct acmp *acmp, uint64_t now, const void *m, int len)
{
	struct server *server = acmp->server;
	uint8_t buf[len];
	struct avb_ethernet_header *h = (void*)buf;
	struct avb_packet_acmp *reply = SPA_PTROFF(h, sizeof(*h), void);
	const struct avb_packet_acmp *p = SPA_PTROFF(m, sizeof(*h), void);
	int status = AVB_ACMP_STATUS_SUCCESS;
	struct stream *stream;

	if (be64toh(p->talker_guid) != server->entity_id)
		return 0;

	memcpy(buf, m, len);
	stream = server_find_stream(server, SPA_DIRECTION_OUTPUT,
			reply->talker_unique_id);
	if (stream == NULL) {
		status = AVB_ACMP_STATUS_TALKER_NO_STREAM_INDEX;
		goto done;
	}

	AVB_PACKET_ACMP_SET_MESSAGE_TYPE(reply, AVB_ACMP_MESSAGE_TYPE_DISCONNECT_TX_RESPONSE);

	stream_deactivate(stream, now);

done:
	AVB_PACKET_ACMP_SET_STATUS(reply, status);
	return avb_server_send_packet(server, h->dest, AVB_TSN_ETH, buf, len);
}

static int handle_disconnect_tx_response(struct acmp *acmp, uint64_t now, const void *m, int len)
{
	struct server *server = acmp->server;
	struct avb_ethernet_header *h;
	struct avb_packet_acmp *reply;
	const struct avb_packet_acmp *resp = SPA_PTROFF(m, sizeof(*h), void);
	struct pending *pending;
	uint16_t sequence_id;
	struct stream *stream;
	int res;

	if (be64toh(resp->listener_guid) != server->entity_id)
		return 0;

	sequence_id = ntohs(resp->sequence_id);

	pending = pending_find(acmp, PENDING_TALKER, sequence_id);
	if (pending == NULL)
		return 0;

	h = pending->ptr;
	pending->size = SPA_MIN((int)pending->size, len);
	memcpy(h, m, pending->size);

	reply = SPA_PTROFF(h, sizeof(*h), void);
	reply->sequence_id = htons(pending->old_sequence_id);
	AVB_PACKET_ACMP_SET_MESSAGE_TYPE(reply, AVB_ACMP_MESSAGE_TYPE_DISCONNECT_RX_RESPONSE);

	stream = server_find_stream(server, SPA_DIRECTION_INPUT,
			reply->listener_unique_id);
	if (stream == NULL)
		return 0;

	stream_deactivate(stream, now);

	res = avb_server_send_packet(server, h->dest, AVB_TSN_ETH, h, pending->size);

	pending_free(acmp, pending);

	return res;
}

/* IEEE 1722.1-2021, Sec. 8.1.1. Connecting a Stream */
/* Milan v1.2, Sec. 5.5.2.4 Controller Bind */
static int handle_connect_rx_command(struct acmp *acmp, uint64_t now, const void *m, int len)
{
	struct server *server = acmp->server;
	struct avb_ethernet_header *h;
	const struct avb_packet_acmp *p = SPA_PTROFF(m, sizeof(*h), void);
	struct avb_packet_acmp *cmd;
	struct avb_packet_acmp *reply;
	int res;
	AVB_ACMP_FLAGS flags;

#if USE_MILAN
	struct listener_fsm_cmd *cmd;
	struct fsm_state_listener *fsm = acmp_fsm_find(acmp, STREAM_LISTENER_FSM,
		be64toh(p->listener_guid));
	int evt = AECP_MILAN_ACMP_EVT_RCV_BIND_RX_CMD;

	if (!fsm) {
		pw_log_error("What to do ? create a stream here ?");
		return 0;
	}
	cmd = &cmd_listeners_states[fsm->current_state][evt];
	if (!cmd) {
		pw_log_error("Non compatible event for this state, state %d evt id %d",
			fsm->current_state, evt);
		__assert(0);
	}
	return 0;
#else


	if (be64toh(p->listener_guid) != server->entity_id)
		return 0;

	// TODO: Check if entity is locked and respond with CONTROLLER_NOT_AUTHORIZED


	h = pending_new(acmp, PENDING_TALKER, now,
		AVB_ACMP_TIMEOUT_CONNECT_TX_COMMAND_MS, m, len);
	if (h == NULL)
	return -errno;

	cmd = SPA_PTROFF(h, sizeof(*h), void);

	// TODO: Continue here
	AVB_PACKET_ACMP_SET_MESSAGE_TYPE(cmd, AVB_ACMP_MESSAGE_TYPE_CONNECT_TX_COMMAND);
	AVB_PACKET_ACMP_SET_STATUS(cmd, AVB_ACMP_STATUS_SUCCESS);

	return avb_server_send_packet(server, h->dest, AVB_TSN_ETH, h, len);
#endif
}

static int handle_ignore(struct acmp *acmp, uint64_t now, const void *m, int len)
{
	return 0;
}

/** Milan v1.2 5.5.2.2 UNBIND_RX_COMMAND */
static int handle_disconnect_rx_command(struct acmp *acmp, uint64_t now, const void *m, int len)
{
	struct server *server = acmp->server;
	struct avb_ethernet_header *h;
	const struct avb_packet_acmp *p = SPA_PTROFF(m, sizeof(*h), void);
	struct avb_packet_acmp *cmd;

	if (be64toh(p->listener_guid) != server->entity_id)
		return 0;

#ifndef USE_MILAN
	h = pending_new(acmp, PENDING_TALKER, now,
			AVB_ACMP_TIMEOUT_DISCONNECT_TX_COMMAND_MS, m, len);
	if (h == NULL)
		return -errno;

	cmd = SPA_PTROFF(h, sizeof(*h), void);
	AVB_PACKET_ACMP_SET_MESSAGE_TYPE(cmd, AVB_ACMP_MESSAGE_TYPE_DISCONNECT_TX_COMMAND);
	AVB_PACKET_ACMP_SET_STATUS(cmd, AVB_ACMP_STATUS_SUCCESS);
#else

#endif

	return avb_server_send_packet(server, h->dest, AVB_TSN_ETH, h, len);
}

static const struct msg_info msg_info[] = {
	{ AVB_ACMP_MESSAGE_TYPE_CONNECT_TX_COMMAND, "connect-tx-command", handle_connect_tx_command, },
	{ AVB_ACMP_MESSAGE_TYPE_CONNECT_TX_RESPONSE, "connect-tx-response", handle_connect_tx_response, },
	{ AVB_ACMP_MESSAGE_TYPE_DISCONNECT_TX_COMMAND, "disconnect-tx-command", handle_disconnect_tx_command, },
	{ AVB_ACMP_MESSAGE_TYPE_DISCONNECT_TX_RESPONSE, "disconnect-tx-response", handle_disconnect_tx_response, },
	{ AVB_ACMP_MESSAGE_TYPE_GET_TX_STATE_COMMAND, "get-tx-state-command", NULL, },
	{ AVB_ACMP_MESSAGE_TYPE_GET_TX_STATE_RESPONSE, "get-tx-state-response", handle_ignore, },
	{ AVB_ACMP_MESSAGE_TYPE_CONNECT_RX_COMMAND, "connect-rx-command", handle_connect_rx_command, },
	{ AVB_ACMP_MESSAGE_TYPE_CONNECT_RX_RESPONSE, "connect-rx-response", handle_ignore, },
	{ AVB_ACMP_MESSAGE_TYPE_DISCONNECT_RX_COMMAND, "disconnect-rx-command", handle_disconnect_rx_command, },
	{ AVB_ACMP_MESSAGE_TYPE_DISCONNECT_RX_RESPONSE, "disconnect-rx-response", handle_ignore, },
	{ AVB_ACMP_MESSAGE_TYPE_GET_RX_STATE_COMMAND, "get-rx-state-command", NULL, },
	{ AVB_ACMP_MESSAGE_TYPE_GET_RX_STATE_RESPONSE, "get-rx-state-response", handle_ignore, },
	{ AVB_ACMP_MESSAGE_TYPE_GET_TX_CONNECTION_COMMAND, "get-tx-connection-command", NULL, },
	{ AVB_ACMP_MESSAGE_TYPE_GET_TX_CONNECTION_RESPONSE, "get-tx-connection-response", handle_ignore, },
};

static inline const struct msg_info *find_msg_info(uint16_t type, const char *name)
{
	SPA_FOR_EACH_ELEMENT_VAR(msg_info, i) {
		if ((name == NULL && type == i->type) ||
		    (name != NULL && spa_streq(name, i->name)))
			return i;
	}
	return NULL;
}

static int acmp_message(void *data, uint64_t now, const void *message, int len)
{
	struct acmp *acmp = data;
	struct server *server = acmp->server;
	const struct avb_ethernet_header *h = message;
	const struct avb_packet_acmp *p = SPA_PTROFF(h, sizeof(*h), void);
	const struct msg_info *info;
	int message_type;

	if (ntohs(h->type) != AVB_TSN_ETH)
		return 0;
	if (memcmp(h->dest, mac, 6) != 0 &&
	    memcmp(h->dest, server->mac_addr, 6) != 0)
		return 0;

	if (AVB_PACKET_GET_SUBTYPE(&p->hdr) != AVB_SUBTYPE_ACMP)
		return 0;

	message_type = AVB_PACKET_ACMP_GET_MESSAGE_TYPE(p);

	info = find_msg_info(message_type, NULL);
	if (info == NULL)
		return 0;

	pw_log_info("got ACMP message %s", info->name);

	if (info->handle == NULL)
		return reply_not_supported(acmp, message_type | 1, message, len);

	return info->handle(acmp, now, message, len);
}

static void acmp_destroy(void *data)
{
	struct acmp *acmp = data;
	spa_hook_remove(&acmp->server_listener);
	free(acmp);
}

static void check_timeout(struct acmp *acmp, uint64_t now, uint16_t type)
{
#ifndef USE_MILAN
	struct pending *p, *t;

	spa_list_for_each_safe(p, t, &acmp->pending[type], link) {
		if (p->last_time + p->timeout > now)
			continue;

		if (p->retry == 0) {
			pw_log_info("%p: pending timeout, retry", p);
			retry_pending(acmp, now, p);
		} else {
			pw_log_info("%p: pending timeout, fail", p);
			pending_free(acmp, p);
		}
	}
#else
	struct fsm_state_listener *p, *t;
	struct listener_fsm_cmd *cmd = NULL;
	int evt;
	spa_list_for_each_safe(p, t, &acmp->stream_fsm[type], link) {
		if (p->timeout > now)
			continue;

		switch (p->current_state) {
			case MILAN_ACMP_LISTENER_STA_PRB_W_DELAY:
				pw_log_warn("PRB_W_DELAY waiting more\n");
				evt = AECP_MILAN_ACMP_EVT_TMR_DELAY;
			break;
			case MILAN_ACMP_LISTENER_STA_PRB_W_RESP:
				pw_log_warn("PRB_W_RESP waiting more\n");
				evt = AECP_MILAN_ACMP_EVT_TMR_NO_RESP;
			break;
			case MILAN_ACMP_LISTENER_STA_PRB_W_RESP2:
				pw_log_warn("PRB_W_RESP2 waiting more\n");
				evt = AECP_MILAN_ACMP_EVT_TMR_NO_RESP;
			break;
			case MILAN_ACMP_LISTENER_STA_PRB_W_RETRY:
				pw_log_warn("PRB_W_RETRY waiting more but failed....\n");
				evt = AECP_MILAN_ACMP_EVT_TMR_RETRY;
			break;
			case MILAN_ACMP_LISTENER_STA_SETTLED_NO_RSV:
				pw_log_warn("PRB_W_RETRY waiting more but failed....\n");
				evt = AECP_MILAN_ACMP_EVT_TMR_NO_TK;
			break;
			default:
				__assert(0);
			break;
		}

		cmd = &cmd_listeners_states[p->current_state][evt];
		if (!cmd) {
			pw_log_error("Should not be here with state %d, event %d\n",
				p->current_state, evt);
			__assert(0);
		}

		return cmd->state_handler(acmp, NULL, 0, now);
#endif // USE_MILAN
	}
}
static void acmp_periodic(void *data, uint64_t now)
{
	struct acmp *acmp = data;
#ifndef USE_MILAN
	check_timeout(acmp, now, PENDING_TALKER);
	check_timeout(acmp, now, PENDING_LISTENER);
	check_timeout(acmp, now, PENDING_CONTROLLER);
#else
	check_timeout(acmp, now, STREAM_LISTENER_FSM);
	check_timeout(acmp, now, STREAM_TALKER_FSM);
#endif // USE_MILAN

}

static int do_help(struct acmp *acmp, const char *args, FILE *out)
{
	fprintf(out, "{ \"type\": \"help\","
			"\"text\": \""
			  "/adp/help: this help \\n"
			"\" }");
	return 0;
}

static int acmp_command(void *data, uint64_t now, const char *command, const char *args, FILE *out)
{
	struct acmp *acmp = data;
	int res;

	if (!spa_strstartswith(command, "/acmp/"))
		return 0;

	command += strlen("/acmp/");

	if (spa_streq(command, "help"))
		res = do_help(acmp, args, out);
	else
		res = -ENOTSUP;

	return res;
}

static const struct server_events server_events = {
	AVB_VERSION_SERVER_EVENTS,
	.destroy = acmp_destroy,
	.message = acmp_message,
	.periodic = acmp_periodic,
	.command = acmp_command
};

struct avb_acmp *avb_acmp_register(struct server *server)
{
	struct acmp *acmp;

	acmp = calloc(1, sizeof(*acmp));
	if (acmp == NULL)
		return NULL;

	acmp->server = server;
#ifndef USE_MILAN
	spa_list_init(&acmp->pending[PENDING_TALKER]);
	spa_list_init(&acmp->pending[PENDING_LISTENER]);
	spa_list_init(&acmp->pending[PENDING_CONTROLLER]);
#else // USE_MILAN
	spa_list_init(&acmp->stream_fsm[STREAM_LISTENER_FSM]);
	spa_list_init(&acmp->stream_fsm[STREAM_TALKER_FSM]);
#endif // USE_MILAN

	avdecc_server_add_listener(server, &acmp->server_listener, &server_events, acmp);

	return (struct avb_acmp*)acmp;
}

void avb_acmp_unregister(struct avb_acmp *acmp)
{
	acmp_destroy(acmp);
}
