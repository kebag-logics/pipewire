/* PipeWire
 *
 * Copyright © 2022 Wim Taymans
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/filter.h>
#include <linux/net_tstamp.h>
#include <limits.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <spa/support/cpu.h>
#include <spa/debug/mem.h>

#include <pipewire/pipewire.h>

#include "avb.h"
#include "packets.h"
#include "internal.h"
#include "acmp.h"
#include "adp.h"
#include "aecp.h"
#include "maap.h"
#include "mmrp.h"
#include "msrp.h"
#include "mvrp.h"
#include "descriptors.h"
#include "utils.h"

#define DEFAULT_INTERVAL	1

#define server_emit(s,m,v,...) spa_hook_list_call(&s->listener_list, struct server_events, m, v, ##__VA_ARGS__)
#define server_emit_destroy(s)		server_emit(s, destroy, 0)
#define server_emit_message(s,n,m,l)	server_emit(s, message, 0, n, m, l)
#define server_emit_periodic(s,n)	server_emit(s, periodic, 0, n)
#define server_emit_command(s,n,c,a,f)	server_emit(s, command, 0, n, c, a, f)

static void on_timer_event(void *data, uint64_t expirations)
{
	struct server *server = data;
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	server_emit_periodic(server, SPA_TIMESPEC_TO_NSEC(&now));
}

static void on_socket_data(void *data, int fd, uint32_t mask)
{
	struct server *server = data;
	struct timespec now;

	if (mask & SPA_IO_IN) {
		int len;
		uint8_t buffer[2048];

		len = recv(fd, buffer, sizeof(buffer), 0);

		if (len < 0) {
			pw_log_warn("got recv error: %m");
		}
		else if (len < (int)sizeof(struct avb_packet_header)) {
			pw_log_warn("short packet received (%d < %d)", len,
					(int)sizeof(struct avb_packet_header));
		} else {
			clock_gettime(CLOCK_REALTIME, &now);
			server_emit_message(server, SPA_TIMESPEC_TO_NSEC(&now), buffer, len);
		}
	}
}

int avb_server_send_packet(struct server *server, const uint8_t dest[6],
		uint16_t type, void *data, size_t size)
{
	struct avb_ethernet_header *hdr = (struct avb_ethernet_header*)data;
	int res = 0;

	memcpy(hdr->dest, dest, ETH_ALEN);
	memcpy(hdr->src, server->mac_addr, ETH_ALEN);
	hdr->type = htons(type);

	if (send(server->source->fd, data, size, 0) < 0) {
		res = -errno;
		pw_log_warn("got send error: %m");
	}
	return res;
}

static int load_filter(int fd, uint16_t eth, const uint8_t dest[6], const uint8_t mac[6])
{
	struct sock_fprog filter;
	struct sock_filter bpf_code[] = {
		BPF_STMT(BPF_LD|BPF_H|BPF_ABS,  12),
		BPF_JUMP(BPF_JMP|BPF_JEQ,       eth,        0, 8),
		BPF_STMT(BPF_LD|BPF_W|BPF_ABS,  2),
		BPF_JUMP(BPF_JMP|BPF_JEQ,       (dest[2] << 24) |
						(dest[3] << 16) |
						(dest[4] <<  8) |
						(dest[5]),  0, 2),
		BPF_STMT(BPF_LD|BPF_H|BPF_ABS,  0),
		BPF_JUMP(BPF_JMP|BPF_JEQ,       (dest[0] << 8) |
						(dest[1]),  3, 4),
		BPF_JUMP(BPF_JMP|BPF_JEQ,       (mac[2] << 24) |
						(mac[3] << 16) |
						(mac[4] <<  8) |
						(mac[5]),   0, 3),
		BPF_STMT(BPF_LD|BPF_H|BPF_ABS,  0),
		BPF_JUMP(BPF_JMP|BPF_JEQ,       (mac[0] <<  8) |
						(mac[1]), 0, 1),
		BPF_STMT(BPF_RET,               0x00040000),
		BPF_STMT(BPF_RET,               0x00000000),
	};
	filter.len = sizeof(bpf_code) / 8;
	filter.filter = bpf_code;

	if (setsockopt(fd, SOL_SOCKET, SO_ATTACH_FILTER,
				&filter, sizeof(filter)) < 0) {
		pw_log_error("setsockopt(ATTACH_FILTER) failed: %m");
		return -errno;
	}
	return 0;
}

static int setup_socket(struct server *server)
{
	struct impl *impl = server->impl;
	int fd, res;
	struct ifreq req;
	struct packet_mreq mreq;
	struct sockaddr_ll sll;
	struct timespec value, interval;
	static const uint8_t bmac[6] = AVB_BROADCAST_MAC;


	fd = socket(AF_PACKET, SOCK_RAW|SOCK_NONBLOCK, htons(ETH_P_ALL));
	if (fd < 0) {
		pw_log_error("socket() failed: %m");
		return -errno;
	}

	spa_zero(req);
	snprintf(req.ifr_name, sizeof(req.ifr_name), "%s", server->ifname);
	if (ioctl(fd, SIOCGIFINDEX, &req) < 0) {
		res = -errno;
		pw_log_error("SIOCGIFINDEX %s failed: %m", server->ifname);
		goto error_close;
	}
	server->ifindex = req.ifr_ifindex;

	spa_zero(req);
	snprintf(req.ifr_name, sizeof(req.ifr_name), "%s", server->ifname);
	if (ioctl(fd, SIOCGIFHWADDR, &req) < 0) {
		res = -errno;
		pw_log_error("SIOCGIFHWADDR %s failed: %m", server->ifname);
		goto error_close;
	}
	memcpy(server->mac_addr, req.ifr_hwaddr.sa_data, sizeof(server->mac_addr));

	server->entity_id = (uint64_t)server->mac_addr[0] << 56 |
			(uint64_t)server->mac_addr[1] << 48 |
			(uint64_t)server->mac_addr[2] << 40 |
			(uint64_t)0xff << 32 |
			(uint64_t)0xfe << 24 |
			(uint64_t)server->mac_addr[3] << 16 |
			(uint64_t)server->mac_addr[4] << 8 |
			(uint64_t)server->mac_addr[5];

	pw_log_info("%lx %d", server->entity_id, server->ifindex);

	spa_zero(sll);
	sll.sll_family = AF_PACKET;
	sll.sll_protocol = htons(ETH_P_ALL);
	sll.sll_ifindex = server->ifindex;
	sll.sll_halen = ETH_ALEN;
	memcpy(sll.sll_addr, bmac, ETH_ALEN);
	if (bind(fd, (struct sockaddr *) &sll, sizeof(sll)) < 0) {
		res = -errno;
		pw_log_error("bind() failed: %m");
		goto error_close;
	}

	spa_zero(mreq);
	mreq.mr_ifindex = server->ifindex;
	mreq.mr_type = PACKET_MR_MULTICAST;
	mreq.mr_alen = ETH_ALEN;
	memcpy(mreq.mr_address, bmac, ETH_ALEN);

	if (setsockopt(fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP,
				&mreq, sizeof(mreq)) < 0) {
		res = -errno;
		pw_log_error("setsockopt(ADD_MEMBERSHIP) failed: %m");
		goto error_close;
	}

	if ((res = load_filter(fd, AVB_TSN_ETH, bmac, server->mac_addr)) < 0)
		goto error_close;

	server->source = pw_loop_add_io(impl->loop, fd, SPA_IO_IN, true, on_socket_data, server);
	if (server->source == NULL) {
		res = -errno;
		pw_log_error("server %p: can't create server source: %m", impl);
		goto error_close;
	}
	server->timer = pw_loop_add_timer(impl->loop, on_timer_event, server);
	if (server->timer == NULL) {
		res = -errno;
		pw_log_error("server %p: can't create timer source: %m", impl);
		goto error_close;
	}
	value.tv_sec = 0;
	value.tv_nsec = 1;
	interval.tv_sec = DEFAULT_INTERVAL;
	interval.tv_nsec = 0;
	pw_loop_update_timer(impl->loop, server->timer, &value, &interval, false);

	return 0;

error_close:
	close(fd);
	return res;
}

struct server *avdecc_server_new(struct impl *impl, const char *ifname, struct spa_dict *props)
{
	struct server *server;
	int res = 0;

	server = calloc(1, sizeof(*server));
	if (server == NULL)
		return NULL;

	server->impl = impl;
	spa_list_append(&impl->servers, &server->link);
	server->ifname = strdup(ifname);
	spa_hook_list_init(&server->listener_list);
	spa_list_init(&server->descriptors);

	server->debug_messages = false;

	if ((res = setup_socket(server)) < 0)
		goto error_free;

	init_descriptors(server);

	server->mrp = avb_mrp_new(server);
	if (server->mrp == NULL)
		goto error_free;

	avb_aecp_register(server);
	avb_maap_register(server);
	server->mmrp = avb_mmrp_register(server);
	server->msrp = avb_msrp_register(server);
	server->mvrp = avb_mvrp_register(server);
	avb_adp_register(server);
	avb_acmp_register(server);

	server->domain_attr = avb_msrp_attribute_new(server->msrp,
			AVB_MSRP_ATTRIBUTE_TYPE_DOMAIN);
	server->domain_attr->attr.domain.sr_class_id = 6;
	server->domain_attr->attr.domain.sr_class_priority = 3;
	server->domain_attr->attr.domain.sr_class_vid = htons(2);

	avb_mrp_attribute_begin(server->domain_attr->mrp, 0);
	avb_mrp_attribute_join(server->domain_attr->mrp, 0, true);

	server->listener_attr = avb_msrp_attribute_new(server->msrp,
			AVB_MSRP_ATTRIBUTE_TYPE_LISTENER);
	server->listener_attr->attr.listener.stream_id = htobe64(0);
	avb_mrp_attribute_begin(server->listener_attr->mrp, 0);

	return server;

error_free:
	free(server);
	if (res < 0)
		errno = -res;
	return NULL;
}

void avdecc_server_add_listener(struct server *server, struct spa_hook *listener,
		const struct server_events *events, void *data)
{
	spa_hook_list_append(&server->listener_list, listener, events, data);
}

void avdecc_server_free(struct server *server)
{
	struct impl *impl = server->impl;

	spa_list_remove(&server->link);
	if (server->source)
		pw_loop_destroy_source(impl->loop, server->source);
	if (server->timer)
		pw_loop_destroy_source(impl->loop, server->source);
	spa_hook_list_clean(&server->listener_list);
	free(server);
}
