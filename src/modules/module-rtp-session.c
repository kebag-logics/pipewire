/* PipeWire */
/* SPDX-FileCopyrightText: Copyright © 2022 Wim Taymans <wim.taymans@gmail.com> */
/* SPDX-License-Identifier: MIT */

#include "config.h"

#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <net/if.h>
#include <ctype.h>

#include <spa/utils/hook.h>
#include <spa/utils/result.h>
#include <spa/utils/ringbuffer.h>
#include <spa/utils/json.h>
#include <spa/param/audio/format-utils.h>
#include <spa/debug/types.h>
#include <spa/debug/mem.h>

#include <pipewire/pipewire.h>
#include <pipewire/impl.h>

#include <module-rtp/rtp.h>
#include <module-rtp/apple-midi.h>
#include <module-rtp/stream.h>

/** \page page_module_rtp_sink PipeWire Module: RTP sink
 *
 * The `rtp-sink` module creates a PipeWire sink that sends audio
 * RTP packets.
 *
 * ## Module Options
 *
 * Options specific to the behavior of this module
 *
 * - `source.ip =<str>`: source IP address, default "0.0.0.0"
 * - `destination.ip =<str>`: destination IP address, default "224.0.0.56"
 * - `destination.port =<int>`: destination port, default random beteen 46000 and 47024
 * - `local.ifname = <str>`: interface name to use
 * - `net.mtu = <int>`: MTU to use, default 1280
 * - `net.ttl = <int>`: TTL to use, default 1
 * - `net.loop = <bool>`: loopback multicast, default false
 * - `sess.min-ptime = <int>`: minimum packet time in milliseconds, default 2
 * - `sess.max-ptime = <int>`: maximum packet time in milliseconds, default 20
 * - `sess.name = <str>`: a session name
 * - `sess.ts-offset = <int>`: an offset to apply to the timestamp, default -1 = random offset
 * - `sess.ts-refclk = <string>`: the name of a reference clock
 * - `rtp.media = <string>`: the media type audio|midi, default audio
 * - `stream.props = {}`: properties to be passed to the stream
 *
 * ## General options
 *
 * Options with well-known behavior:
 *
 * - \ref PW_KEY_REMOTE_NAME
 * - \ref PW_KEY_AUDIO_FORMAT
 * - \ref PW_KEY_AUDIO_RATE
 * - \ref PW_KEY_AUDIO_CHANNELS
 * - \ref SPA_KEY_AUDIO_POSITION
 * - \ref PW_KEY_NODE_NAME
 * - \ref PW_KEY_NODE_DESCRIPTION
 * - \ref PW_KEY_MEDIA_NAME
 * - \ref PW_KEY_NODE_GROUP
 * - \ref PW_KEY_NODE_LATENCY
 * - \ref PW_KEY_NODE_VIRTUAL
 * - \ref PW_KEY_MEDIA_CLASS
 *
 * ## Example configuration
 *\code{.unparsed}
 * context.modules = [
 * {   name = libpipewire-module-rtp-sink
 *     args = {
 *         #local.ifname = "eth0"
 *         #source.ip = "0.0.0.0"
 *         #destination.ip = "224.0.0.56"
 *         #destination.port = 46000
 *         #net.mtu = 1280
 *         #net.ttl = 1
 *         #net.loop = false
 *         #sess.min-ptime = 2
 *         #sess.max-ptime = 20
 *         #sess.name = "PipeWire RTP stream"
 *         #rtp.media = "audio"
 *         #audio.format = "S16BE"
 *         #audio.rate = 48000
 *         #audio.channels = 2
 *         #audio.position = [ FL FR ]
 *         stream.props = {
 *             node.name = "rtp-sink"
 *         }
 *     }
 *}
 *]
 *\endcode
 *
 * \since 0.3.60
 */

#define NAME "rtp-sink"

PW_LOG_TOPIC_STATIC(mod_topic, "mod." NAME);
#define PW_LOG_TOPIC_DEFAULT mod_topic

#define BUFFER_SIZE		(1u<<20)
#define BUFFER_MASK		(BUFFER_SIZE-1)

#define DEFAULT_FORMAT		"S16BE"
#define DEFAULT_RATE		48000
#define DEFAULT_CHANNELS	2
#define DEFAULT_POSITION	"[ FL FR ]"

#define DEFAULT_SOURCE_IP	"0.0.0.0"
#define DEFAULT_SOURCE_PORT	0
#define DEFAULT_DESTINATION_IP	"224.0.0.56"
#define DEFAULT_DESTINATION_PORT	46000
#define DEFAULT_TTL		1
#define DEFAULT_MTU		1280
#define DEFAULT_LOOP		false

#define DEFAULT_TS_OFFSET	-1

#define USAGE	"source.ip=<source IP address, default:"DEFAULT_SOURCE_IP"> "			\
		"destination.ip=<destination IP address, default:"DEFAULT_DESTINATION_IP"> "	\
 		"destination.port=<int, default random beteen 46000 and 47024> "		\
		"local.ifname=<local interface name to use> "					\
		"net.mtu=<desired MTU, default:"SPA_STRINGIFY(DEFAULT_MTU)"> "			\
		"net.ttl=<desired TTL, default:"SPA_STRINGIFY(DEFAULT_TTL)"> "			\
		"net.loop=<desired loopback, default:"SPA_STRINGIFY(DEFAULT_LOOP)"> "		\
		"sess.name=<a name for the session> "						\
		"sess.min-ptime=<minimum packet time in milliseconds, default:2> "		\
		"sess.max-ptime=<maximum packet time in milliseconds, default:20> "		\
 		"rtp.media=<string, the media type audio|midi, default audio> "		\
		"audio.format=<format, default:"DEFAULT_FORMAT"> "				\
		"audio.rate=<sample rate, default:"SPA_STRINGIFY(DEFAULT_RATE)"> "		\
		"audio.channels=<number of channels, default:"SPA_STRINGIFY(DEFAULT_CHANNELS)"> "\
		"audio.position=<channel map, default:"DEFAULT_POSITION"> "			\
		"stream.props= { key=value ... }"

static const struct spa_dict_item module_info[] = {
	{ PW_KEY_MODULE_AUTHOR, "Wim Taymans <wim.taymans@gmail.com>" },
	{ PW_KEY_MODULE_DESCRIPTION, "RTP Sink" },
	{ PW_KEY_MODULE_USAGE, USAGE },
	{ PW_KEY_MODULE_VERSION, PACKAGE_VERSION },
};

struct session {
	struct impl *impl;
	struct spa_list link;

	struct sockaddr_storage ctrl_addr;
	socklen_t ctrl_len;
	struct sockaddr_storage data_addr;
	socklen_t data_len;

	struct rtp_stream *send;
	struct spa_hook send_listener;
	struct rtp_stream *recv;
	struct spa_hook recv_listener;

	char *name;

	uint32_t initiator;
	uint32_t remote_ssrc;

	uint32_t ssrc;
	uint32_t ts_offset;

	unsigned ctrl_ready:1;
	unsigned data_ready:1;
};

struct impl {
	struct pw_context *context;

	struct pw_impl_module *module;
	struct spa_hook module_listener;
	struct pw_properties *props;

	struct pw_properties *stream_props;

	struct pw_loop *loop;
	struct pw_loop *data_loop;

	struct pw_core *core;
	struct spa_hook core_listener;
	struct spa_hook core_proxy_listener;
	unsigned int do_disconnect:1;

	struct spa_source *timer;

	struct spa_source *ctrl_source;
	struct spa_source *data_source;

	char *ifname;
	char *session_name;
	uint32_t mtu;
	bool ttl;
	bool mcast_loop;
	int64_t ts_offset;
	char *ts_refclk;
	int payload;

	struct sockaddr_storage ctrl_addr;
	socklen_t ctrl_len;
	struct sockaddr_storage src_addr;
	socklen_t src_len;

	struct spa_list sessions;
	uint32_t n_sessions;
};

static ssize_t send_packet(int fd, struct msghdr *msg)
{
	ssize_t n;
	n = sendmsg(fd, msg, MSG_NOSIGNAL);
	if (n < 0) {
		switch (errno) {
		case ECONNREFUSED:
		case ECONNRESET:
			pw_log_debug("remote end not listening");
			break;
		default:
			pw_log_debug("sendmsg() failed: %m");
			break;
		}
	}
	return n;
}

static void send_apple_midi_cmd_in(struct session *sess, bool ctrl)
{
	struct impl *impl = sess->impl;
	struct iovec iov[3];
	struct msghdr msg;
	struct rtp_apple_midi hdr;

	spa_zero(hdr);
	hdr.cmd = htonl(APPLE_MIDI_CMD_IN);
	hdr.protocol = htonl(2);
	hdr.initiator = htonl(sess->initiator);
	hdr.ssrc = htonl(sess->ssrc);

	iov[0].iov_base = &hdr;
	iov[0].iov_len = sizeof(hdr);
	iov[1].iov_base = sess->name;
	iov[1].iov_len = strlen(sess->name);

	spa_zero(msg);
	msg.msg_name = ctrl ? &sess->ctrl_addr : &sess->data_addr;
	msg.msg_namelen = ctrl ? sess->ctrl_len : sess->data_len;
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

	send_packet(ctrl ? impl->ctrl_source->fd : impl->data_source->fd, &msg);
}

static void send_apple_midi_cmd_by(struct session *sess, bool ctrl)
{
	struct impl *impl = sess->impl;
	struct iovec iov[3];
	struct msghdr msg;
	struct rtp_apple_midi hdr;

	spa_zero(hdr);
	hdr.cmd = htonl(APPLE_MIDI_CMD_BY);
	hdr.protocol = htonl(2);
	hdr.initiator = htonl(sess->initiator);
	hdr.ssrc = htonl(sess->ssrc);

	iov[0].iov_base = &hdr;
	iov[0].iov_len = sizeof(hdr);

	spa_zero(msg);
	msg.msg_name = ctrl ? &sess->ctrl_addr : &sess->data_addr;
	msg.msg_namelen = ctrl ? sess->ctrl_len : sess->data_len;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	send_packet(ctrl ? impl->ctrl_source->fd : impl->data_source->fd, &msg);
}

static void send_destroy(void *data)
{
}

static void send_state_changed(void *data, bool started, const char *error)
{
	struct session *sess = data;

	pw_log_info("send initiator:%08x state %d", sess->initiator, started);

	if (started) {
		if (!sess->ctrl_ready)
			send_apple_midi_cmd_in(sess, true);
		else if (!sess->data_ready)
			send_apple_midi_cmd_in(sess, false);
	} else {
		if (sess->ctrl_ready)
			send_apple_midi_cmd_by(sess, true);
		if (sess->data_ready)
			send_apple_midi_cmd_by(sess, false);
	}
}

static void send_send_packet(void *data, struct iovec *iov, size_t iovlen)
{
	struct session *sess = data;
	struct impl *impl = sess->impl;
	struct msghdr msg;

	spa_zero(msg);
	msg.msg_name = &sess->data_addr;
	msg.msg_namelen = sess->data_len;
	msg.msg_iov = iov;
	msg.msg_iovlen = iovlen;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;

	send_packet(impl->data_source->fd, &msg);
}

static void recv_destroy(void *data)
{
}
static void recv_state_changed(void *data, bool started, const char *error)
{
	struct session *sess = data;
	pw_log_info("send initiator:%08x state %d", sess->initiator, started);

	if (started) {
		if (!sess->ctrl_ready)
			send_apple_midi_cmd_in(sess, true);
		else if (!sess->data_ready)
			send_apple_midi_cmd_in(sess, false);
	} else {
		if (sess->ctrl_ready)
			send_apple_midi_cmd_by(sess, true);
		if (sess->data_ready)
			send_apple_midi_cmd_by(sess, false);
	}
}

static const struct rtp_stream_events send_stream_events = {
	RTP_VERSION_STREAM_EVENTS,
	.destroy = send_destroy,
	.state_changed = send_state_changed,
	.send_packet = send_send_packet,
};

static const struct rtp_stream_events recv_stream_events = {
	RTP_VERSION_STREAM_EVENTS,
	.destroy = recv_destroy,
	.state_changed = recv_state_changed,
};

static void free_session(struct session *sess)
{
	spa_list_remove(&sess->link);
	sess->impl->n_sessions--;

	if (sess->send)
		rtp_stream_destroy(sess->send);
	if (sess->recv)
		rtp_stream_destroy(sess->recv);
	free(sess);
}

static bool cmp_ip(const struct sockaddr_storage *sa, const struct sockaddr_storage *sb)
{
	if (sa->ss_family == AF_INET && sb->ss_family == AF_INET) {
		struct sockaddr_in *ia = (struct sockaddr_in*)sa;
		struct sockaddr_in *ib = (struct sockaddr_in*)sb;
		return ia->sin_addr.s_addr == ib->sin_addr.s_addr;
	} else if (sa->ss_family == AF_INET6 && sb->ss_family == AF_INET6) {
		struct sockaddr_in6 *ia = (struct sockaddr_in6*)sa;
		struct sockaddr_in6 *ib = (struct sockaddr_in6*)sb;
		return ia->sin6_addr.s6_addr == ib->sin6_addr.s6_addr;
	}
	return false;
}

static int get_ip(const struct sockaddr_storage *sa, char *ip, size_t len, uint16_t *port)
{
	if (sa->ss_family == AF_INET) {
		struct sockaddr_in *in = (struct sockaddr_in*)sa;
		inet_ntop(sa->ss_family, &in->sin_addr, ip, len);
		*port = ntohs(in->sin_port);
	} else if (sa->ss_family == AF_INET6) {
		struct sockaddr_in6 *in = (struct sockaddr_in6*)sa;
		inet_ntop(sa->ss_family, &in->sin6_addr, ip, len);
		*port = ntohs(in->sin6_port);
	} else
		return -EIO;
	return 0;
}

static struct session *make_session(struct impl *impl, const char *name)
{
	struct session *sess;
	char addr[64];
	struct pw_properties *props;
	uint16_t port = 0;

	sess = calloc(1, sizeof(struct session));
	if (sess == NULL)
		return NULL;

	spa_list_append(&impl->sessions, &sess->link);
	impl->n_sessions++;

	sess->impl = impl;
	sess->initiator = pw_rand32();
	sess->ssrc = pw_rand32();
	sess->ts_offset = impl->ts_offset < 0 ? pw_rand32() : impl->ts_offset;
	sess->name = strdup(name);

	props = pw_properties_copy(impl->stream_props);
	if (props == NULL)
		return NULL;

	get_ip(&sess->ctrl_addr, addr, sizeof(addr), &port);
	pw_properties_set(props, "rtp.destination.ip", addr);
	pw_properties_setf(props, "rtp.destination.port", "%u", port);
	pw_properties_setf(props, "rtp.ts-offset", "%u", sess->ts_offset);
	pw_properties_setf(props, "rtp.sender-ssrc", "%u", sess->ssrc);
	pw_properties_setf(props, "rtp.receiver-ssrc", "%u", sess->remote_ssrc);

	sess->send = rtp_stream_new(impl->core,
			PW_DIRECTION_INPUT, pw_properties_copy(props),
			&send_stream_events, sess);
	sess->recv = rtp_stream_new(impl->core,
			PW_DIRECTION_OUTPUT, pw_properties_copy(props),
			&recv_stream_events, sess);

	return sess;
}

static struct session *find_session_by_addr_name(struct impl *impl,
		const struct sockaddr_storage *sa, const char *name)
{
	struct session *sess;
	spa_list_for_each(sess, &impl->sessions, link) {
		if (cmp_ip(sa, &sess->ctrl_addr) &&
		    spa_streq(sess->name, name))
			return sess;
	}
	return NULL;
}
static struct session *find_session_by_initiator(struct impl *impl, uint32_t initiator)
{
	struct session *sess;
	spa_list_for_each(sess, &impl->sessions, link) {
		if (sess->initiator == initiator)
			return sess;
	}
	return NULL;
}

static struct session *find_session_by_ssrc(struct impl *impl, uint32_t ssrc)
{
	struct session *sess;
	spa_list_for_each(sess, &impl->sessions, link) {
		if (sess->remote_ssrc == ssrc)
			return sess;
	}
	return NULL;
}

static void parse_apple_midi_cmd_in(struct impl *impl, bool ctrl, uint8_t *buffer,
		ssize_t len, struct sockaddr_storage *sa, socklen_t salen)
{
	struct rtp_apple_midi *hdr = (struct rtp_apple_midi*)buffer;
	struct iovec iov[3];
	struct msghdr msg;
	struct rtp_apple_midi reply;
	struct session *sess;
	bool success = true;
	uint32_t initiator = ntohl(hdr->initiator);
	char addr[128];
	uint16_t port = 0;

	get_ip(sa, addr, sizeof(addr), &port);
	pw_log_info("IN from %s:%d %s", addr, port, hdr->name);

	sess = find_session_by_initiator(impl, initiator);
	if (sess == NULL)
		sess = find_session_by_addr_name(impl, sa, hdr->name);

	if (ctrl) {
		if (sess != NULL) {
			if (sess->ctrl_ready) {
				pw_log_warn("receive ctrl IN from existing session %08x", initiator);
				success = false;
			}
		} else {
			pw_log_info("got control IN request %08x", initiator);
			sess = make_session(impl, hdr->name);
			if (sess == NULL) {
				pw_log_warn("failed to make session: %m");
				success = false;
			}
		}
		if (success) {
			sess->initiator = initiator;
			sess->remote_ssrc = ntohl(hdr->ssrc);
			sess->ctrl_addr = *sa;
			sess->ctrl_len = salen;
			sess->ctrl_ready = true;
		}
	}
	else {
		if (sess == NULL) {
			pw_log_warn("receive data IN from nonexisting session %08x", initiator);
			success = false;
		} else {
			pw_log_info("got data IN request %08x", initiator);
			sess->data_addr = *sa;
			sess->data_len = salen;
			sess->data_ready = true;
		}
	}

	reply = *hdr;
	if (success) {
		reply.cmd = htonl(APPLE_MIDI_CMD_OK);
		reply.ssrc = htonl(sess->ssrc);
	} else
		reply.cmd = htonl(APPLE_MIDI_CMD_NO);

	iov[0].iov_base = &reply;
	iov[0].iov_len = sizeof(reply);
	iov[1].iov_base = sess->name;
	iov[1].iov_len = strlen(sess->name);

	spa_zero(msg);
	msg.msg_name = sa;
	msg.msg_namelen = salen;
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

	pw_log_debug("send %p %u", msg.msg_name, msg.msg_namelen);

	send_packet(ctrl ? impl->ctrl_source->fd : impl->data_source->fd, &msg);
}

static void parse_apple_midi_cmd_ok(struct impl *impl, bool ctrl, uint8_t *buffer,
		ssize_t len, struct sockaddr_storage *sa, socklen_t salen)
{
	struct rtp_apple_midi *hdr = (struct rtp_apple_midi*)buffer;
	uint32_t initiator = ntohl(hdr->initiator);
	struct session *sess;

	sess = find_session_by_initiator(impl, initiator);
	if (sess == NULL) {
		pw_log_warn("received OK from nonexisting session %u", initiator);
		return;
	}

	if (ctrl) {
		pw_log_info("got ctrl OK %08x %d", initiator, sess->data_ready);
		sess->ctrl_ready = true;
		if (!sess->data_ready)
			send_apple_midi_cmd_in(sess, false);
	} else {
		pw_log_info("got data OK %08x", initiator);
		sess->data_ready = true;
	}
}

static void parse_apple_midi_cmd_ck(struct impl *impl, bool ctrl, uint8_t *buffer,
		ssize_t len, struct sockaddr_storage *sa, socklen_t salen)
{
	struct rtp_apple_midi_ck *hdr = (struct rtp_apple_midi_ck*)buffer;
	struct iovec iov[3];
	struct msghdr msg;
	struct rtp_apple_midi_ck reply;
	struct timespec ts;
	struct session *sess;
	uint64_t now;
	uint32_t ssrc = ntohl(hdr->ssrc);

	sess = find_session_by_ssrc(impl, ssrc);
	if (sess == NULL) {
		pw_log_warn("unknown SSRC %u", ssrc);
		return;
	}

	pw_log_debug("got CK count %d", hdr->count);

	clock_gettime(CLOCK_MONOTONIC, &ts);
	now = SPA_TIMESPEC_TO_NSEC(&ts);
	reply = *hdr;
	reply.ssrc = htonl(sess->ssrc);
	reply.count++;
	iov[0].iov_base = &reply;
	iov[0].iov_len = sizeof(reply);

	switch (hdr->count) {
	case 0:
		reply.ts2_h = htonl(now >> 32);
		reply.ts2_l = htonl(now);
		break;
	case 1:
		reply.ts3_h = htonl(now >> 32);
		reply.ts3_l = htonl(now);
		break;
	case 2:
		return;
	}

	spa_zero(msg);
	msg.msg_name = sa;
	msg.msg_namelen = salen;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	pw_log_debug("send %p %u", msg.msg_name, msg.msg_namelen);

	send_packet(ctrl ? impl->ctrl_source->fd : impl->data_source->fd, &msg);
}

static void parse_apple_midi_cmd(struct impl *impl, bool ctrl, uint8_t *buffer,
		ssize_t len, struct sockaddr_storage *sa, socklen_t salen)
{
	struct rtp_apple_midi *hdr = (struct rtp_apple_midi*)buffer;
	switch (ntohl(hdr->cmd)) {
	case APPLE_MIDI_CMD_IN:
		parse_apple_midi_cmd_in(impl, ctrl, buffer, len, sa, salen);
		break;
	case APPLE_MIDI_CMD_OK:
		parse_apple_midi_cmd_ok(impl, ctrl, buffer, len, sa, salen);
		break;
	case APPLE_MIDI_CMD_CK:
		parse_apple_midi_cmd_ck(impl, ctrl, buffer, len, sa, salen);
		break;
	default:
		break;
	}
}

static void
on_ctrl_io(void *data, int fd, uint32_t mask)
{
	struct impl *impl = data;
	ssize_t len;
	uint8_t buffer[2048];

	if (mask & SPA_IO_IN) {
		struct sockaddr_storage sa;
		socklen_t salen = sizeof(sa);

		if ((len = recvfrom(fd, buffer, sizeof(buffer), 0,
					(struct sockaddr*)&sa, &salen)) < 0)
			goto receive_error;

		if (len < 12)
			goto short_packet;

		if (buffer[0] == 0xff && buffer[1] == 0xff) {
			parse_apple_midi_cmd(impl, true, buffer, len, &sa, salen);
		} else {
			spa_debug_mem(0, buffer, len);
		}
	}
	return;

receive_error:
	pw_log_warn("recv error: %m");
	return;
short_packet:
	pw_log_warn("short packet received");
	return;
}

static void
on_data_io(void *data, int fd, uint32_t mask)
{
	struct impl *impl = data;
	ssize_t len;
	uint8_t buffer[2048];

	if (mask & SPA_IO_IN) {
		struct sockaddr_storage sa;
		socklen_t salen = sizeof(sa);

		if ((len = recvfrom(fd, buffer, sizeof(buffer), 0,
					(struct sockaddr*)&sa, &salen)) < 0)
			goto receive_error;

		if (len < 12)
			goto short_packet;

		if (buffer[0] == 0xff && buffer[1] == 0xff) {
			parse_apple_midi_cmd(impl, false, buffer, len, &sa, salen);
		} else {
			struct rtp_header *hdr = (struct rtp_header*)buffer;
			struct session *sess = find_session_by_ssrc(impl, ntohl(hdr->ssrc));
			if (sess == NULL)
				goto unknown_ssrc;

			rtp_stream_receive_packet(sess->recv, buffer, len);
		}
	}
	return;

receive_error:
	pw_log_warn("recv error: %m");
	return;
short_packet:
	pw_log_warn("short packet received");
	return;
unknown_ssrc:
	pw_log_warn("unknown SSRC");
	return;
}

static int make_socket(const struct sockaddr_storage* sa, socklen_t salen,
		bool loop, int ttl, char *ifname)
{
	int af, fd, val, res;
	struct ifreq req;
	struct sockaddr_storage src = *sa;
	bool is_multicast = false;

	af = sa->ss_family;
	if ((fd = socket(af, SOCK_DGRAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0)) < 0) {
		pw_log_error("socket failed: %m");
		return -errno;
	}
#ifdef SO_TIMESTAMP
	val = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_TIMESTAMP, &val, sizeof(val)) < 0) {
		res = -errno;
		pw_log_error("setsockopt failed: %m");
		goto error;
	}
#endif
	val = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
		res = -errno;
		pw_log_error("setsockopt failed: %m");
		goto error;
	}

	spa_zero(req);
	if (ifname) {
		snprintf(req.ifr_name, sizeof(req.ifr_name), "%s", ifname);
		res = ioctl(fd, SIOCGIFINDEX, &req);
	        if (res < 0)
	                pw_log_warn("SIOCGIFINDEX %s failed: %m", ifname);
	}
	res = 0;
	if (af == AF_INET) {
		static const uint32_t ipv4_mcast_mask = 0xe0000000;
		struct sockaddr_in *sa4 = (struct sockaddr_in*)&src;
		if ((ntohl(sa4->sin_addr.s_addr) & ipv4_mcast_mask) == ipv4_mcast_mask) {
			struct ip_mreqn mr4;
			memset(&mr4, 0, sizeof(mr4));
			mr4.imr_multiaddr = sa4->sin_addr;
			mr4.imr_ifindex = req.ifr_ifindex;
			res = setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mr4, sizeof(mr4));
			is_multicast = true;
		} else {
			sa4->sin_addr.s_addr = INADDR_ANY;
		}
	} else if (af == AF_INET6) {
		struct sockaddr_in6 *sa6 = (struct sockaddr_in6*)&src;
		if (sa6->sin6_addr.s6_addr[0] == 0xff) {
			struct ipv6_mreq mr6;
			memset(&mr6, 0, sizeof(mr6));
			mr6.ipv6mr_multiaddr = sa6->sin6_addr;
			mr6.ipv6mr_interface = req.ifr_ifindex;
			res = setsockopt(fd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mr6, sizeof(mr6));
			is_multicast = true;
		} else {
		        sa6->sin6_addr = in6addr_any;
		}
	} else {
		res = -EINVAL;
		goto error;
	}

	if (res < 0) {
		res = -errno;
		pw_log_error("join mcast failed: %m");
		goto error;
	}
	if (is_multicast) {
		val = loop;
		if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &val, sizeof(val)) < 0)
			pw_log_warn("setsockopt(IP_MULTICAST_LOOP) failed: %m");

		val = ttl;
		if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &val, sizeof(val)) < 0)
			pw_log_warn("setsockopt(IP_MULTICAST_TTL) failed: %m");
	}

	if (bind(fd, (struct sockaddr*)&src, salen) < 0) {
		res = -errno;
		pw_log_error("bind() failed: %m");
		goto error;
	}
	/* FIXME AES67 wants IPTOS_DSCP_AF41 */
	val = IPTOS_LOWDELAY;
	if (setsockopt(fd, IPPROTO_IP, IP_TOS, &val, sizeof(val)) < 0)
		pw_log_warn("setsockopt(IP_TOS) failed: %m");

	return fd;
error:
	return res;
}

static int setup_apple_session(struct impl *impl)
{
	int fd;

	if ((fd = make_socket(&impl->ctrl_addr, impl->ctrl_len,
				impl->mcast_loop, impl->ttl, impl->ifname)) < 0) {
		return fd;
	}
	impl->ctrl_source = pw_loop_add_io(impl->loop, fd,
					SPA_IO_IN, true, on_ctrl_io, impl);

	if (impl->ctrl_source == NULL)
		return -errno;

	if ((fd = make_socket(&impl->src_addr, impl->src_len,
				impl->mcast_loop, impl->ttl, impl->ifname)) < 0)
		return fd;

	impl->data_source = pw_loop_add_io(impl->data_loop, fd,
				SPA_IO_IN, true, on_data_io, impl);
	if (impl->data_source == NULL)
		return -errno;

	return 0;
}

static void core_destroy(void *d)
{
	struct impl *impl = d;
	spa_hook_remove(&impl->core_listener);
	impl->core = NULL;
	pw_impl_module_schedule_destroy(impl->module);
}

static const struct pw_proxy_events core_proxy_events = {
	.destroy = core_destroy,
};

static void impl_destroy(struct impl *impl)
{
	struct session *sess;

	spa_list_consume(sess, &impl->sessions, link)
		free_session(sess);

	if (impl->core && impl->do_disconnect)
		pw_core_disconnect(impl->core);

	if (impl->timer)
		pw_loop_destroy_source(impl->loop, impl->timer);
	if (impl->ctrl_source)
		pw_loop_destroy_source(impl->loop, impl->ctrl_source);
	if (impl->data_source)
		pw_loop_destroy_source(impl->data_loop, impl->data_source);

	pw_properties_free(impl->stream_props);
	pw_properties_free(impl->props);

	free(impl->ifname);
	free(impl->ts_refclk);
	free(impl->session_name);
	free(impl);
}

static void module_destroy(void *d)
{
	struct impl *impl = d;
	spa_hook_remove(&impl->module_listener);
	impl_destroy(impl);
}

static const struct pw_impl_module_events module_events = {
	PW_VERSION_IMPL_MODULE_EVENTS,
	.destroy = module_destroy,
};

static void on_core_error(void *d, uint32_t id, int seq, int res, const char *message)
{
	struct impl *impl = d;

	pw_log_error("error id:%u seq:%d res:%d (%s): %s",
			id, seq, res, spa_strerror(res), message);

	if (id == PW_ID_CORE && res == -EPIPE)
		pw_impl_module_schedule_destroy(impl->module);
}

static const struct pw_core_events core_events = {
	PW_VERSION_CORE_EVENTS,
	.error = on_core_error,
};

static int parse_address(const char *address, uint16_t port,
		struct sockaddr_storage *addr, socklen_t *len)
{
	struct sockaddr_in *sa4 = (struct sockaddr_in*)addr;
	struct sockaddr_in6 *sa6 = (struct sockaddr_in6*)addr;

	if (inet_pton(AF_INET, address, &sa4->sin_addr) > 0) {
		sa4->sin_family = AF_INET;
		sa4->sin_port = htons(port);
		*len = sizeof(*sa4);
	} else if (inet_pton(AF_INET6, address, &sa6->sin6_addr) > 0) {
		sa6->sin6_family = AF_INET6;
		sa6->sin6_port = htons(port);
		*len = sizeof(*sa6);
	} else
		return -EINVAL;

	return 0;
}

static void copy_props(struct impl *impl, struct pw_properties *props, const char *key)
{
	const char *str;
	if ((str = pw_properties_get(props, key)) != NULL) {
		if (pw_properties_get(impl->stream_props, key) == NULL)
			pw_properties_set(impl->stream_props, key, str);
	}
}

SPA_EXPORT
int pipewire__module_init(struct pw_impl_module *module, const char *args)
{
	struct pw_context *context = pw_impl_module_get_context(module);
	struct impl *impl;
	struct pw_properties *props = NULL, *stream_props = NULL;
	uint32_t id = pw_global_get_id(pw_impl_module_get_global(module));
	uint32_t pid = getpid();
	char addr[64];
	uint16_t port;
	const char *str;
	int res = 0;

	PW_LOG_TOPIC_INIT(mod_topic);

	impl = calloc(1, sizeof(struct impl));
	if (impl == NULL)
		return -errno;

	if (args == NULL)
		args = "";

	spa_list_init(&impl->sessions);

	props = pw_properties_new_string(args);
	if (props == NULL) {
		res = -errno;
		pw_log_error( "can't create properties: %m");
		goto out;
	}
	impl->props = props;

	stream_props = pw_properties_new(NULL, NULL);
	if (stream_props == NULL) {
		res = -errno;
		pw_log_error( "can't create properties: %m");
		goto out;
	}
	impl->stream_props = stream_props;

	impl->module = module;
	impl->context = context;
	impl->loop = pw_context_get_main_loop(context);
	impl->data_loop = pw_data_loop_get_loop(pw_context_get_data_loop(context));

	if (pw_properties_get(props, PW_KEY_NODE_VIRTUAL) == NULL)
		pw_properties_set(props, PW_KEY_NODE_VIRTUAL, "true");
	if (pw_properties_get(stream_props, PW_KEY_NODE_NETWORK) == NULL)
		pw_properties_set(stream_props, PW_KEY_NODE_NETWORK, "true");

	if (pw_properties_get(props, PW_KEY_NODE_NAME) == NULL)
		pw_properties_setf(props, PW_KEY_NODE_NAME, "rtp-session-%u-%u", pid, id);
	if (pw_properties_get(props, PW_KEY_NODE_DESCRIPTION) == NULL)
		pw_properties_set(props, PW_KEY_NODE_DESCRIPTION,
				pw_properties_get(props, PW_KEY_NODE_NAME));
	if (pw_properties_get(props, PW_KEY_MEDIA_NAME) == NULL)
		pw_properties_set(props, PW_KEY_MEDIA_NAME, "RTP Session Stream");

	if ((str = pw_properties_get(props, "stream.props")) != NULL)
		pw_properties_update_string(stream_props, str, strlen(str));

	copy_props(impl, props, PW_KEY_AUDIO_FORMAT);
	copy_props(impl, props, PW_KEY_AUDIO_RATE);
	copy_props(impl, props, PW_KEY_AUDIO_CHANNELS);
	copy_props(impl, props, SPA_KEY_AUDIO_POSITION);
	copy_props(impl, props, PW_KEY_NODE_NAME);
	copy_props(impl, props, PW_KEY_NODE_DESCRIPTION);
	copy_props(impl, props, PW_KEY_NODE_GROUP);
	copy_props(impl, props, PW_KEY_NODE_LATENCY);
	copy_props(impl, props, PW_KEY_NODE_VIRTUAL);
	copy_props(impl, props, PW_KEY_NODE_CHANNELNAMES);
	copy_props(impl, props, PW_KEY_MEDIA_NAME);
	copy_props(impl, props, PW_KEY_MEDIA_CLASS);

	str = pw_properties_get(props, "local.ifname");
	impl->ifname = str ? strdup(str) : NULL;

	port = DEFAULT_SOURCE_PORT;
	port = pw_properties_get_uint32(props, "control.port", port);
	if ((str = pw_properties_get(props, "control.ip")) == NULL)
		str = DEFAULT_SOURCE_IP;

	if ((res = parse_address(str, port, &impl->ctrl_addr, &impl->ctrl_len)) < 0) {
		pw_log_error("invalid control.ip %s: %s", str, spa_strerror(res));
		goto out;
	}
	if ((res = parse_address(str, port ? port+1 : 0, &impl->src_addr, &impl->src_len)) < 0) {
		pw_log_error("invalid data.ip %s: %s", str, spa_strerror(res));
		goto out;
	}

	impl->mtu = pw_properties_get_uint32(props, "net.mtu", DEFAULT_MTU);
	impl->ttl = pw_properties_get_uint32(props, "net.ttl", DEFAULT_TTL);
	impl->mcast_loop = pw_properties_get_bool(props, "net.loop", DEFAULT_LOOP);

	impl->ts_offset = pw_properties_get_int64(props,
			"sess.ts-offset", DEFAULT_TS_OFFSET);

	str = pw_properties_get(props, "sess.ts-refclk");
	impl->ts_refclk = str ? strdup(str) : NULL;

	if ((str = pw_properties_get(props, "sess.name")) == NULL)
		pw_properties_setf(props, "sess.name", "PipeWire RTP Stream on %s",
				pw_get_host_name());
	str = pw_properties_get(props, "sess.name");
	impl->session_name = str ? strdup(str) : NULL;

	pw_properties_set(stream_props, "rtp.session", impl->session_name);
	get_ip(&impl->src_addr, addr, sizeof(addr), &port);
	pw_properties_set(stream_props, "rtp.source.ip", addr);
	pw_properties_setf(stream_props, "rtp.source.port", "%u", port);
	pw_properties_setf(stream_props, "rtp.mtu", "%u", impl->mtu);
	pw_properties_setf(stream_props, "rtp.ttl", "%u", impl->ttl);

	if ((str = pw_properties_get(props, "rtp.media")) != NULL)
		pw_properties_set(stream_props, "rtp.media", str);
	if ((str = pw_properties_get(props, "sess.min-ptime")) != NULL)
		pw_properties_set(stream_props, "rtp.min-ptime", str);
	if ((str = pw_properties_get(props, "sess.max-ptime")) != NULL)
		pw_properties_set(stream_props, "rtp.max-ptime", str);
	if (impl->ts_refclk != NULL)
		pw_properties_set(stream_props, "rtp.ts-refclk", impl->ts_refclk);

	impl->core = pw_context_get_object(impl->context, PW_TYPE_INTERFACE_Core);
	if (impl->core == NULL) {
		str = pw_properties_get(props, PW_KEY_REMOTE_NAME);
		impl->core = pw_context_connect(impl->context,
				pw_properties_new(
					PW_KEY_REMOTE_NAME, str,
					NULL),
				0);
		impl->do_disconnect = true;
	}
	if (impl->core == NULL) {
		res = -errno;
		pw_log_error("can't connect: %m");
		goto out;
	}

	pw_proxy_add_listener((struct pw_proxy*)impl->core,
			&impl->core_proxy_listener,
			&core_proxy_events, impl);
	pw_core_add_listener(impl->core,
			&impl->core_listener,
			&core_events, impl);

	str = pw_properties_get(props, "destination.ip");
	if (str != NULL) {
		struct session *sess;

		port = pw_properties_get_uint32(props, "destination.port", 0);
		if (port == 0) {
			pw_log_error("invalid destination.port");
			goto out;
		}
		sess = make_session(impl, impl->session_name);
		if (sess == NULL) {
			res = -errno;
			pw_log_error("can't create session: %m");
			goto out;
		}
		if ((res = parse_address(str, port, &sess->ctrl_addr, &sess->ctrl_len)) < 0) {
			pw_log_error("invalid destination.ip %s: %s", str, spa_strerror(res));
			goto out;
		}
		if ((res = parse_address(str, port+1, &sess->data_addr, &sess->data_len)) < 0) {
			pw_log_error("invalid destination.ip %s: %s", str, spa_strerror(res));
			goto out;
		}
	}
	if ((res = setup_apple_session(impl)) < 0)
		goto out;


	pw_impl_module_add_listener(module, &impl->module_listener, &module_events, impl);

	pw_impl_module_update_properties(module, &SPA_DICT_INIT_ARRAY(module_info));

	return 0;
out:
	impl_destroy(impl);
	return res;
}
