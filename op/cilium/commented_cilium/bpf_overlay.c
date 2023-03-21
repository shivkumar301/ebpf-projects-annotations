// SPDX-License-Identifier: (GPL-2.0-only OR BSD-2-Clause)
/* Copyright Authors of Cilium */

#include <bpf/ctx/skb.h>
#include <bpf/api.h>

#include <node_config.h>
#include <netdev_config.h>

#define IS_BPF_OVERLAY 1

/* Controls the inclusion of the CILIUM_CALL_HANDLE_ICMP6_NS section in the
 * bpf_lxc object file.
 */
#define SKIP_ICMPV6_NS_HANDLING

/* Controls the inclusion of the CILIUM_CALL_SEND_ICMP6_ECHO_REPLY section in
 * the bpf_lxc object file.
 */
#define SKIP_ICMPV6_ECHO_HANDLING

/* Controls the inclusion of the CILIUM_CALL_SRV6 section in the object file.
 */
#define SKIP_SRV6_HANDLING

#include "lib/tailcall.h"
#include "lib/common.h"
#include "lib/edt.h"
#include "lib/maps.h"
#include "lib/ipv6.h"
#include "lib/eth.h"
#include "lib/dbg.h"
#include "lib/trace.h"
#include "lib/l3.h"
#include "lib/drop.h"
#include "lib/identity.h"
#include "lib/nodeport.h"

#ifdef ENABLE_VTEP
#include "lib/arp.h"
#include "lib/encap.h"
#include "lib/eps.h"
#endif /* ENABLE_VTEP */

#ifdef ENABLE_IPV6
/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "pkt_go_to_next_module",
      "pkt_go_to_next_module": [
        {
          "Project": "cilium",
          "Return Type": "int",
          "Input Params": [],
          "Function Name": "TC_ACT_OK",
          "Return": 0,
          "Description": "will terminate the packet processing pipeline and allows the packet to proceed. Pass the skb onwards either to upper layers of the stack on ingress or down to the networking device driver for transmission on egress, respectively. TC_ACT_OK sets skb->tc_index based on the classid the tc BPF program set. The latter is set out of the tc BPF program itself through skb->tc_classid from the BPF context.",
          "compatible_hookpoints": [
            "xdp",
            "sched_cls",
            "sched_act"
          ],
          "capabilities": [
            "pkt_go_to_next_module"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 46,
  "endLine": 180,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/bpf_overlay.c",
  "funcName": "handle_ipv6",
  "developer_inline_comments": [
    {
      "start_line": 1,
      "end_line": 1,
      "text": "// SPDX-License-Identifier: (GPL-2.0-only OR BSD-2-Clause)"
    },
    {
      "start_line": 2,
      "end_line": 2,
      "text": "/* Copyright Authors of Cilium */"
    },
    {
      "start_line": 12,
      "end_line": 14,
      "text": "/* Controls the inclusion of the CILIUM_CALL_HANDLE_ICMP6_NS section in the\n * bpf_lxc object file.\n */"
    },
    {
      "start_line": 17,
      "end_line": 19,
      "text": "/* Controls the inclusion of the CILIUM_CALL_SEND_ICMP6_ECHO_REPLY section in\n * the bpf_lxc object file.\n */"
    },
    {
      "start_line": 22,
      "end_line": 23,
      "text": "/* Controls the inclusion of the CILIUM_CALL_SRV6 section in the object file.\n */"
    },
    {
      "start_line": 43,
      "end_line": 43,
      "text": "/* ENABLE_VTEP */"
    },
    {
      "start_line": 56,
      "end_line": 56,
      "text": "/* verifier workaround (dereference of modified ctx ptr) */"
    },
    {
      "start_line": 81,
      "end_line": 84,
      "text": "/* Any node encapsulating will map any HOST_ID source to be\n\t\t * presented as REMOTE_NODE_ID, therefore any attempt to signal\n\t\t * HOST_ID as source from a remote node can be dropped.\n\t\t */"
    },
    {
      "start_line": 88,
      "end_line": 91,
      "text": "/* Maybe overwrite the REMOTE_NODE_ID with\n\t\t * KUBE_APISERVER_NODE_ID to support upgrade. After v1.12,\n\t\t * this should be removed.\n\t\t */"
    },
    {
      "start_line": 95,
      "end_line": 97,
      "text": "/* Look up the ipcache for the src IP, it will give us\n\t\t\t * the real identity of that IP.\n\t\t\t */"
    },
    {
      "start_line": 110,
      "end_line": 112,
      "text": "/* IPSec is not currently enforce (feature coming soon)\n\t\t * so for now just handle normally\n\t\t */"
    },
    {
      "start_line": 119,
      "end_line": 119,
      "text": "/* Decrypt \"key\" is determined by SPI */"
    },
    {
      "start_line": 122,
      "end_line": 127,
      "text": "/* To IPSec stack on cilium_vxlan we are going to pass\n\t\t * this up the stack but eth_type_trans has already labeled\n\t\t * this as an OTHERHOST type packet. To avoid being dropped\n\t\t * by IP stack before IPSec can be processed mark as a HOST\n\t\t * packet.\n\t\t */"
    },
    {
      "start_line": 140,
      "end_line": 140,
      "text": "/* Lookup IPv6 address in list of local endpoints */"
    },
    {
      "start_line": 145,
      "end_line": 147,
      "text": "/* Let through packets to the node-ip so they are processed by\n\t\t * the local ip stack.\n\t\t */"
    },
    {
      "start_line": 160,
      "end_line": 162,
      "text": "/* A packet entering the node from the tunnel and not going to a local\n\t * endpoint has to be going to the local host.\n\t */"
    }
  ],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct  __ctx_buff *ctx",
    " __u32 *identity"
  ],
  "output": "static__always_inlineint",
  "helper": [
    "CTX_ACT_OK"
  ],
  "compatibleHookpoints": [
    "xdp",
    "sched_act",
    "sched_cls"
  ],
  "source": [
    "static __always_inline int handle_ipv6 (struct  __ctx_buff *ctx, __u32 *identity)\n",
    "{\n",
    "    int ret, l3_off = ETH_HLEN, hdrlen;\n",
    "    void *data_end, *data;\n",
    "    struct ipv6hdr *ip6;\n",
    "    struct bpf_tunnel_key key = {}\n",
    "    ;\n",
    "    struct endpoint_info *ep;\n",
    "    bool decrypted;\n",
    "    if (!revalidate_data_pull (ctx, &data, &data_end, &ip6))\n",
    "        return DROP_INVALID;\n",
    "\n",
    "#ifdef ENABLE_NODEPORT\n",
    "    if (!bpf_skip_nodeport (ctx)) {\n",
    "        ret = nodeport_lb6 (ctx, * identity);\n",
    "        if (ret < 0)\n",
    "            return ret;\n",
    "    }\n",
    "\n",
    "#endif\n",
    "    ret = encap_remap_v6_host_address (ctx, false);\n",
    "    if (unlikely (ret < 0))\n",
    "        return ret;\n",
    "    if (!revalidate_data (ctx, &data, &data_end, &ip6))\n",
    "        return DROP_INVALID;\n",
    "    decrypted = ((ctx->mark & MARK_MAGIC_HOST_MASK) == MARK_MAGIC_DECRYPT);\n",
    "    if (decrypted) {\n",
    "        *identity = key.tunnel_id = get_identity (ctx);\n",
    "    }\n",
    "    else {\n",
    "        if (unlikely (ctx_get_tunnel_key (ctx, &key, sizeof (key), 0) < 0))\n",
    "            return DROP_NO_TUNNEL_KEY;\n",
    "        *identity = key.tunnel_id;\n",
    "        if (*identity == HOST_ID)\n",
    "            return DROP_INVALID_IDENTITY;\n",
    "        if (identity_is_remote_node (*identity)) {\n",
    "            struct remote_endpoint_info *info;\n",
    "            info = ipcache_lookup6 (& IPCACHE_MAP, (union v6addr *) & ip6 -> saddr, V6_CACHE_KEY_LEN);\n",
    "            if (info)\n",
    "                *identity = info->sec_label;\n",
    "        }\n",
    "    }\n",
    "    cilium_dbg (ctx, DBG_DECAP, key.tunnel_id, key.tunnel_label);\n",
    "\n",
    "#ifdef ENABLE_IPSEC\n",
    "    if (!decrypted) {\n",
    "        if (ip6->nexthdr != IPPROTO_ESP) {\n",
    "            update_metrics (ctx_full_len (ctx), METRIC_INGRESS, REASON_PLAINTEXT);\n",
    "            goto not_esp;\n",
    "        }\n",
    "        ctx->mark = MARK_MAGIC_DECRYPT;\n",
    "        set_identity_mark (ctx, *identity);\n",
    "        ctx_change_type (ctx, PACKET_HOST);\n",
    "        send_trace_notify (ctx, TRACE_TO_STACK, 0, 0, 0, ctx->ingress_ifindex, TRACE_REASON_ENCRYPTED, TRACE_PAYLOAD_LEN);\n",
    "        return CTX_ACT_OK;\n",
    "    }\n",
    "    ctx->mark = 0;\n",
    "not_esp :\n",
    "\n",
    "#endif\n",
    "    ep = lookup_ip6_endpoint (ip6);\n",
    "    if (ep) {\n",
    "        __u8 nexthdr;\n",
    "        if (ep->flags & ENDPOINT_F_HOST)\n",
    "            goto to_host;\n",
    "        nexthdr = ip6->nexthdr;\n",
    "        hdrlen = ipv6_hdrlen (ctx, & nexthdr);\n",
    "        if (hdrlen < 0)\n",
    "            return hdrlen;\n",
    "        return ipv6_local_delivery (ctx, l3_off, *identity, ep, METRIC_INGRESS, false);\n",
    "    }\n",
    "to_host :\n",
    "\n",
    "#ifdef HOST_IFINDEX\n",
    "    if (1) {\n",
    "        union macaddr host_mac = HOST_IFINDEX_MAC;\n",
    "        union macaddr router_mac = NODE_MAC;\n",
    "        ret = ipv6_l3 (ctx, ETH_HLEN, (__u8 *) & router_mac.addr, (__u8 *) & host_mac.addr, METRIC_INGRESS);\n",
    "        if (ret != CTX_ACT_OK)\n",
    "            return ret;\n",
    "        cilium_dbg_capture (ctx, DBG_CAPTURE_DELIVERY, HOST_IFINDEX);\n",
    "        return ctx_redirect (ctx, HOST_IFINDEX, 0);\n",
    "    }\n",
    "\n",
    "#else\n",
    "    return CTX_ACT_OK;\n",
    "\n",
    "#endif\n",
    "}\n"
  ],
  "called_function_list": [
    "cilium_dbg",
    "ipv6_l3",
    "encap_and_redirect_with_nodeid",
    "ipv6_local_delivery",
    "ctx_full_len",
    "set_identity_meta",
    "lookup_ip6_endpoint",
    "bpf_skip_nodeport",
    "icmp6_host_handle",
    "get_identity",
    "rewrite_dmac_to_host",
    "set_encrypt_key_meta",
    "encap_and_redirect_netdev",
    "IS_ERR",
    "set_encrypt_dip",
    "update_metrics",
    "ctx_get_xfer",
    "srv6_lookup_sid",
    "send_trace_notify",
    "unlikely",
    "ipv6_host_policy_ingress",
    "ep_tail_call",
    "likely",
    "identity_is_remote_node",
    "ipv6_host_policy_egress",
    "defined",
    "encap_remap_v6_host_address",
    "is_srv6_packet",
    "ipv6_hdrlen",
    "ipcache_lookup6",
    "ctx_get_tunnel_key",
    "ctx_change_type",
    "set_identity_mark",
    "revalidate_data_pull",
    "ctx_redirect",
    "get_min_encrypt_key",
    "cilium_dbg_capture",
    "revalidate_data",
    "ctx_skip_host_fw",
    "nodeport_lb6"
  ],
  "call_depth": -1,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline int handle_ipv6(struct __ctx_buff *ctx,
				       __u32 *identity)
{
	int ret, l3_off = ETH_HLEN, hdrlen;
	void *data_end, *data;
	struct ipv6hdr *ip6;
	struct bpf_tunnel_key key = {};
	struct endpoint_info *ep;
	bool decrypted;

	/* verifier workaround (dereference of modified ctx ptr) */
	if (!revalidate_data_pull(ctx, &data, &data_end, &ip6))
		return DROP_INVALID;
#ifdef ENABLE_NODEPORT
	if (!bpf_skip_nodeport(ctx)) {
		ret = nodeport_lb6(ctx, *identity);
		if (ret < 0)
			return ret;
	}
#endif
	ret = encap_remap_v6_host_address(ctx, false);
	if (unlikely(ret < 0))
		return ret;

	if (!revalidate_data(ctx, &data, &data_end, &ip6))
		return DROP_INVALID;

	decrypted = ((ctx->mark & MARK_MAGIC_HOST_MASK) == MARK_MAGIC_DECRYPT);
	if (decrypted) {
		*identity = key.tunnel_id = get_identity(ctx);
	} else {
		if (unlikely(ctx_get_tunnel_key(ctx, &key, sizeof(key), 0) < 0))
			return DROP_NO_TUNNEL_KEY;
		*identity = key.tunnel_id;

		/* Any node encapsulating will map any HOST_ID source to be
		 * presented as REMOTE_NODE_ID, therefore any attempt to signal
		 * HOST_ID as source from a remote node can be dropped.
		 */
		if (*identity == HOST_ID)
			return DROP_INVALID_IDENTITY;

		/* Maybe overwrite the REMOTE_NODE_ID with
		 * KUBE_APISERVER_NODE_ID to support upgrade. After v1.12,
		 * this should be removed.
		 */
		if (identity_is_remote_node(*identity)) {
			struct remote_endpoint_info *info;

			/* Look up the ipcache for the src IP, it will give us
			 * the real identity of that IP.
			 */
			info = ipcache_lookup6(&IPCACHE_MAP,
					       (union v6addr *)&ip6->saddr,
					       V6_CACHE_KEY_LEN);
			if (info)
				*identity = info->sec_label;
		}
	}

	cilium_dbg(ctx, DBG_DECAP, key.tunnel_id, key.tunnel_label);

#ifdef ENABLE_IPSEC
	if (!decrypted) {
		/* IPSec is not currently enforce (feature coming soon)
		 * so for now just handle normally
		 */
		if (ip6->nexthdr != IPPROTO_ESP) {
			update_metrics(ctx_full_len(ctx), METRIC_INGRESS,
				       REASON_PLAINTEXT);
			goto not_esp;
		}

		/* Decrypt "key" is determined by SPI */
		ctx->mark = MARK_MAGIC_DECRYPT;
		set_identity_mark(ctx, *identity);
		/* To IPSec stack on cilium_vxlan we are going to pass
		 * this up the stack but eth_type_trans has already labeled
		 * this as an OTHERHOST type packet. To avoid being dropped
		 * by IP stack before IPSec can be processed mark as a HOST
		 * packet.
		 */
		ctx_change_type(ctx, PACKET_HOST);

		send_trace_notify(ctx, TRACE_TO_STACK, 0, 0, 0,
				  ctx->ingress_ifindex, TRACE_REASON_ENCRYPTED,
				  TRACE_PAYLOAD_LEN);

		return CTX_ACT_OK;
	}
	ctx->mark = 0;
not_esp:
#endif

	/* Lookup IPv6 address in list of local endpoints */
	ep = lookup_ip6_endpoint(ip6);
	if (ep) {
		__u8 nexthdr;

		/* Let through packets to the node-ip so they are processed by
		 * the local ip stack.
		 */
		if (ep->flags & ENDPOINT_F_HOST)
			goto to_host;

		nexthdr = ip6->nexthdr;
		hdrlen = ipv6_hdrlen(ctx, &nexthdr);
		if (hdrlen < 0)
			return hdrlen;

		return ipv6_local_delivery(ctx, l3_off, *identity, ep,
					   METRIC_INGRESS, false);
	}

	/* A packet entering the node from the tunnel and not going to a local
	 * endpoint has to be going to the local host.
	 */
to_host:
#ifdef HOST_IFINDEX
	if (1) {
		union macaddr host_mac = HOST_IFINDEX_MAC;
		union macaddr router_mac = NODE_MAC;

		ret = ipv6_l3(ctx, ETH_HLEN, (__u8 *)&router_mac.addr,
			      (__u8 *)&host_mac.addr, METRIC_INGRESS);
		if (ret != CTX_ACT_OK)
			return ret;

		cilium_dbg_capture(ctx, DBG_CAPTURE_DELIVERY, HOST_IFINDEX);
		return ctx_redirect(ctx, HOST_IFINDEX, 0);
	}
#else
	return CTX_ACT_OK;
#endif
}

__section_tail(CILIUM_MAP_CALLS, CILIUM_CALL_IPV6_FROM_OVERLAY)
/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 183,
  "endLine": 192,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/bpf_overlay.c",
  "funcName": "tail_handle_ipv6",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct  __ctx_buff *ctx"
  ],
  "output": "int",
  "helper": [],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "int tail_handle_ipv6 (struct  __ctx_buff *ctx)\n",
    "{\n",
    "    __u32 src_identity = 0;\n",
    "    int ret = handle_ipv6 (ctx, & src_identity);\n",
    "    if (IS_ERR (ret))\n",
    "        return send_drop_notify_error (ctx, src_identity, ret, CTX_ACT_DROP, METRIC_INGRESS);\n",
    "    return ret;\n",
    "}\n"
  ],
  "called_function_list": [
    "ctx_load_meta",
    "IS_ERR",
    "handle_ipv6",
    "send_drop_notify_error",
    "__tail_handle_ipv6",
    "ctx_store_meta"
  ],
  "call_depth": -1,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
int tail_handle_ipv6(struct __ctx_buff *ctx)
{
	__u32 src_identity = 0;
	int ret = handle_ipv6(ctx, &src_identity);

	if (IS_ERR(ret))
		return send_drop_notify_error(ctx, src_identity, ret,
					      CTX_ACT_DROP, METRIC_INGRESS);
	return ret;
}
#endif /* ENABLE_IPV6 */

#ifdef ENABLE_IPV4
/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "pkt_go_to_next_module",
      "pkt_go_to_next_module": [
        {
          "Project": "cilium",
          "Return Type": "int",
          "Input Params": [],
          "Function Name": "TC_ACT_OK",
          "Return": 0,
          "Description": "will terminate the packet processing pipeline and allows the packet to proceed. Pass the skb onwards either to upper layers of the stack on ingress or down to the networking device driver for transmission on egress, respectively. TC_ACT_OK sets skb->tc_index based on the classid the tc BPF program set. The latter is set out of the tc BPF program itself through skb->tc_classid from the BPF context.",
          "compatible_hookpoints": [
            "xdp",
            "sched_cls",
            "sched_act"
          ],
          "capabilities": [
            "pkt_go_to_next_module"
          ]
        }
      ]
    },
    {
      "capability": "map_read",
      "map_read": [
        {
          "Project": "cilium",
          "Return Type": "void*",
          "Description": "Perform a lookup in <[ map ]>(IP: 0) for an entry associated to key. ",
          "Return": " Map value associated to key, or NULL if no entry was found.",
          "Function Name": "map_lookup_elem",
          "Input Params": [
            "{Type: struct map ,Var: *map}",
            "{Type:  const void ,Var: *key}"
          ],
          "compatible_hookpoints": [
            "socket_filter",
            "kprobe",
            "sched_cls",
            "sched_act",
            "tracepoint",
            "xdp",
            "perf_event",
            "cgroup_skb",
            "cgroup_sock",
            "lwt_in",
            "lwt_out",
            "lwt_xmit",
            "sock_ops",
            "sk_skb",
            "cgroup_device",
            "sk_msg",
            "raw_tracepoint",
            "cgroup_sock_addr",
            "lwt_seg6local",
            "sk_reuseport",
            "flow_dissector",
            "cgroup_sysctl",
            "raw_tracepoint_writable"
          ],
          "capabilities": [
            "map_read"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 196,
  "endLine": 333,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/bpf_overlay.c",
  "funcName": "handle_ipv4",
  "developer_inline_comments": [
    {
      "start_line": 204,
      "end_line": 204,
      "text": "/* verifier workaround (dereference of modified ctx ptr) */"
    },
    {
      "start_line": 208,
      "end_line": 211,
      "text": "/* If IPv4 fragmentation is disabled\n * AND a IPv4 fragmented packet is received,\n * then drop the packet.\n */"
    },
    {
      "start_line": 229,
      "end_line": 229,
      "text": "/* If packets are decrypted the key has already been pushed into metadata. */"
    },
    {
      "start_line": 255,
      "end_line": 255,
      "text": "/* See comment at equivalent code in handle_ipv6() */"
    },
    {
      "start_line": 270,
      "end_line": 272,
      "text": "/* IPSec is not currently enforce (feature coming soon)\n\t\t * so for now just handle normally\n\t\t */"
    },
    {
      "start_line": 281,
      "end_line": 286,
      "text": "/* To IPSec stack on cilium_vxlan we are going to pass\n\t\t * this up the stack but eth_type_trans has already labeled\n\t\t * this as an OTHERHOST type packet. To avoid being dropped\n\t\t * by IP stack before IPSec can be processed mark as a HOST\n\t\t * packet.\n\t\t */"
    },
    {
      "start_line": 299,
      "end_line": 299,
      "text": "/* Lookup IPv4 address in list of local endpoints */"
    },
    {
      "start_line": 302,
      "end_line": 304,
      "text": "/* Let through packets to the node-ip so they are processed by\n\t\t * the local ip stack.\n\t\t */"
    },
    {
      "start_line": 312,
      "end_line": 314,
      "text": "/* A packet entering the node from the tunnel and not going to a local\n\t * endpoint has to be going to the local host.\n\t */"
    }
  ],
  "updateMaps": [],
  "readMaps": [
    "  VTEP_MAP"
  ],
  "input": [
    "struct  __ctx_buff *ctx",
    " __u32 *identity"
  ],
  "output": "static__always_inlineint",
  "helper": [
    "CTX_ACT_OK",
    "map_lookup_elem"
  ],
  "compatibleHookpoints": [
    "xdp",
    "sched_act",
    "sched_cls"
  ],
  "source": [
    "static __always_inline int handle_ipv4 (struct  __ctx_buff *ctx, __u32 *identity)\n",
    "{\n",
    "    void *data_end, *data;\n",
    "    struct iphdr *ip4;\n",
    "    struct endpoint_info *ep;\n",
    "    struct bpf_tunnel_key key = {}\n",
    "    ;\n",
    "    bool decrypted;\n",
    "    if (!revalidate_data_pull (ctx, &data, &data_end, &ip4))\n",
    "        return DROP_INVALID;\n",
    "\n",
    "#ifndef ENABLE_IPV4_FRAGMENTS\n",
    "    if (ipv4_is_fragment (ip4))\n",
    "        return DROP_FRAG_NOSUPPORT;\n",
    "\n",
    "#endif\n",
    "\n",
    "#ifdef ENABLE_NODEPORT\n",
    "    if (!bpf_skip_nodeport (ctx)) {\n",
    "        int ret = nodeport_lb4 (ctx, * identity);\n",
    "        if (ret < 0)\n",
    "            return ret;\n",
    "    }\n",
    "\n",
    "#endif\n",
    "    if (!revalidate_data (ctx, &data, &data_end, &ip4))\n",
    "        return DROP_INVALID;\n",
    "    decrypted = ((ctx->mark & MARK_MAGIC_HOST_MASK) == MARK_MAGIC_DECRYPT);\n",
    "    if (decrypted) {\n",
    "        *identity = key.tunnel_id = get_identity (ctx);\n",
    "    }\n",
    "    else {\n",
    "        if (unlikely (ctx_get_tunnel_key (ctx, &key, sizeof (key), 0) < 0))\n",
    "            return DROP_NO_TUNNEL_KEY;\n",
    "        *identity = key.tunnel_id;\n",
    "        if (*identity == HOST_ID)\n",
    "            return DROP_INVALID_IDENTITY;\n",
    "\n",
    "#ifdef ENABLE_VTEP\n",
    "        {\n",
    "            struct vtep_key vkey = {}\n",
    "            ;\n",
    "            struct vtep_value *info;\n",
    "            vkey.vtep_ip = ip4->saddr & VTEP_MASK;\n",
    "            info = map_lookup_elem (& VTEP_MAP, & vkey);\n",
    "            if (!info)\n",
    "                goto skip_vtep;\n",
    "            if (info->tunnel_endpoint) {\n",
    "                if (*identity != WORLD_ID)\n",
    "                    return DROP_INVALID_VNI;\n",
    "            }\n",
    "        }\n",
    "    skip_vtep :\n",
    "\n",
    "#endif\n",
    "        if (identity_is_remote_node (*identity)) {\n",
    "            struct remote_endpoint_info *info;\n",
    "            info = ipcache_lookup4 (& IPCACHE_MAP, ip4 -> saddr, V4_CACHE_KEY_LEN);\n",
    "            if (info)\n",
    "                *identity = info->sec_label;\n",
    "        }\n",
    "    }\n",
    "    cilium_dbg (ctx, DBG_DECAP, key.tunnel_id, key.tunnel_label);\n",
    "\n",
    "#ifdef ENABLE_IPSEC\n",
    "    if (!decrypted) {\n",
    "        if (ip4->protocol != IPPROTO_ESP) {\n",
    "            update_metrics (ctx_full_len (ctx), METRIC_INGRESS, REASON_PLAINTEXT);\n",
    "            goto not_esp;\n",
    "        }\n",
    "        ctx->mark = MARK_MAGIC_DECRYPT;\n",
    "        set_identity_mark (ctx, *identity);\n",
    "        ctx_change_type (ctx, PACKET_HOST);\n",
    "        send_trace_notify (ctx, TRACE_TO_STACK, 0, 0, 0, ctx->ingress_ifindex, TRACE_REASON_ENCRYPTED, TRACE_PAYLOAD_LEN);\n",
    "        return CTX_ACT_OK;\n",
    "    }\n",
    "    ctx->mark = 0;\n",
    "not_esp :\n",
    "\n",
    "#endif\n",
    "    ep = lookup_ip4_endpoint (ip4);\n",
    "    if (ep) {\n",
    "        if (ep->flags & ENDPOINT_F_HOST)\n",
    "            goto to_host;\n",
    "        return ipv4_local_delivery (ctx, ETH_HLEN, *identity, ip4, ep, METRIC_INGRESS, false);\n",
    "    }\n",
    "to_host :\n",
    "\n",
    "#ifdef HOST_IFINDEX\n",
    "    if (1) {\n",
    "        union macaddr host_mac = HOST_IFINDEX_MAC;\n",
    "        union macaddr router_mac = NODE_MAC;\n",
    "        int ret;\n",
    "        ret = ipv4_l3 (ctx, ETH_HLEN, (__u8 *) & router_mac.addr, (__u8 *) & host_mac.addr, ip4);\n",
    "        if (ret != CTX_ACT_OK)\n",
    "            return ret;\n",
    "        cilium_dbg_capture (ctx, DBG_CAPTURE_DELIVERY, HOST_IFINDEX);\n",
    "        return ctx_redirect (ctx, HOST_IFINDEX, 0);\n",
    "    }\n",
    "\n",
    "#else\n",
    "    return CTX_ACT_OK;\n",
    "\n",
    "#endif\n",
    "}\n"
  ],
  "called_function_list": [
    "ipv4_l3",
    "ipv4_is_fragment",
    "cilium_dbg",
    "encap_and_redirect_with_nodeid",
    "ctx_full_len",
    "set_identity_meta",
    "bpf_skip_nodeport",
    "get_identity",
    "eth_store_daddr",
    "rewrite_dmac_to_host",
    "set_encrypt_key_meta",
    "ipv4_host_policy_egress",
    "encap_and_redirect_netdev",
    "IS_ERR",
    "set_encrypt_dip",
    "update_metrics",
    "ipv4_host_policy_ingress",
    "ctx_get_xfer",
    "send_trace_notify",
    "unlikely",
    "ep_tail_call",
    "identity_is_remote_node",
    "defined",
    "ipcache_lookup4",
    "ipv4_local_delivery",
    "ctx_get_tunnel_key",
    "lookup_ip4_endpoint",
    "ctx_change_type",
    "nodeport_lb4",
    "set_identity_mark",
    "revalidate_data_pull",
    "ctx_redirect",
    "get_min_encrypt_key",
    "__encap_and_redirect_with_nodeid",
    "cilium_dbg_capture",
    "revalidate_data",
    "ctx_skip_host_fw",
    "send_drop_notify_error",
    "ctx_store_meta"
  ],
  "call_depth": -1,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline int handle_ipv4(struct __ctx_buff *ctx, __u32 *identity)
{
	void *data_end, *data;
	struct iphdr *ip4;
	struct endpoint_info *ep;
	struct bpf_tunnel_key key = {};
	bool decrypted;

	/* verifier workaround (dereference of modified ctx ptr) */
	if (!revalidate_data_pull(ctx, &data, &data_end, &ip4))
		return DROP_INVALID;

/* If IPv4 fragmentation is disabled
 * AND a IPv4 fragmented packet is received,
 * then drop the packet.
 */
#ifndef ENABLE_IPV4_FRAGMENTS
	if (ipv4_is_fragment(ip4))
		return DROP_FRAG_NOSUPPORT;
#endif

#ifdef ENABLE_NODEPORT
	if (!bpf_skip_nodeport(ctx)) {
		int ret = nodeport_lb4(ctx, *identity);

		if (ret < 0)
			return ret;
	}
#endif
	if (!revalidate_data(ctx, &data, &data_end, &ip4))
		return DROP_INVALID;

	decrypted = ((ctx->mark & MARK_MAGIC_HOST_MASK) == MARK_MAGIC_DECRYPT);
	/* If packets are decrypted the key has already been pushed into metadata. */
	if (decrypted) {
		*identity = key.tunnel_id = get_identity(ctx);
	} else {
		if (unlikely(ctx_get_tunnel_key(ctx, &key, sizeof(key), 0) < 0))
			return DROP_NO_TUNNEL_KEY;
		*identity = key.tunnel_id;

		if (*identity == HOST_ID)
			return DROP_INVALID_IDENTITY;
#ifdef ENABLE_VTEP
		{
			struct vtep_key vkey = {};
			struct vtep_value *info;

			vkey.vtep_ip = ip4->saddr & VTEP_MASK;
			info = map_lookup_elem(&VTEP_MAP, &vkey);
			if (!info)
				goto skip_vtep;
			if (info->tunnel_endpoint) {
				if (*identity != WORLD_ID)
					return DROP_INVALID_VNI;
			}
		}
skip_vtep:
#endif
		/* See comment at equivalent code in handle_ipv6() */
		if (identity_is_remote_node(*identity)) {
			struct remote_endpoint_info *info;

			info = ipcache_lookup4(&IPCACHE_MAP, ip4->saddr,
					       V4_CACHE_KEY_LEN);
			if (info)
				*identity = info->sec_label;
		}
	}

	cilium_dbg(ctx, DBG_DECAP, key.tunnel_id, key.tunnel_label);

#ifdef ENABLE_IPSEC
	if (!decrypted) {
		/* IPSec is not currently enforce (feature coming soon)
		 * so for now just handle normally
		 */
		if (ip4->protocol != IPPROTO_ESP) {
			update_metrics(ctx_full_len(ctx), METRIC_INGRESS,
				       REASON_PLAINTEXT);
			goto not_esp;
		}

		ctx->mark = MARK_MAGIC_DECRYPT;
		set_identity_mark(ctx, *identity);
		/* To IPSec stack on cilium_vxlan we are going to pass
		 * this up the stack but eth_type_trans has already labeled
		 * this as an OTHERHOST type packet. To avoid being dropped
		 * by IP stack before IPSec can be processed mark as a HOST
		 * packet.
		 */
		ctx_change_type(ctx, PACKET_HOST);

		send_trace_notify(ctx, TRACE_TO_STACK, 0, 0, 0,
				  ctx->ingress_ifindex, TRACE_REASON_ENCRYPTED,
				  TRACE_PAYLOAD_LEN);

		return CTX_ACT_OK;
	}
	ctx->mark = 0;
not_esp:
#endif

	/* Lookup IPv4 address in list of local endpoints */
	ep = lookup_ip4_endpoint(ip4);
	if (ep) {
		/* Let through packets to the node-ip so they are processed by
		 * the local ip stack.
		 */
		if (ep->flags & ENDPOINT_F_HOST)
			goto to_host;

		return ipv4_local_delivery(ctx, ETH_HLEN, *identity, ip4, ep,
					   METRIC_INGRESS, false);
	}

	/* A packet entering the node from the tunnel and not going to a local
	 * endpoint has to be going to the local host.
	 */
to_host:
#ifdef HOST_IFINDEX
	if (1) {
		union macaddr host_mac = HOST_IFINDEX_MAC;
		union macaddr router_mac = NODE_MAC;
		int ret;

		ret = ipv4_l3(ctx, ETH_HLEN, (__u8 *)&router_mac.addr,
			      (__u8 *)&host_mac.addr, ip4);
		if (ret != CTX_ACT_OK)
			return ret;

		cilium_dbg_capture(ctx, DBG_CAPTURE_DELIVERY, HOST_IFINDEX);
		return ctx_redirect(ctx, HOST_IFINDEX, 0);
	}
#else
	return CTX_ACT_OK;
#endif
}

__section_tail(CILIUM_MAP_CALLS, CILIUM_CALL_IPV4_FROM_OVERLAY)
/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 336,
  "endLine": 345,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/bpf_overlay.c",
  "funcName": "tail_handle_ipv4",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct  __ctx_buff *ctx"
  ],
  "output": "int",
  "helper": [],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "int tail_handle_ipv4 (struct  __ctx_buff *ctx)\n",
    "{\n",
    "    __u32 src_identity = 0;\n",
    "    int ret = handle_ipv4 (ctx, & src_identity);\n",
    "    if (IS_ERR (ret))\n",
    "        return send_drop_notify_error (ctx, src_identity, ret, CTX_ACT_DROP, METRIC_INGRESS);\n",
    "    return ret;\n",
    "}\n"
  ],
  "called_function_list": [
    "__tail_handle_ipv4",
    "ctx_load_meta",
    "IS_ERR",
    "handle_ipv4",
    "send_drop_notify_error",
    "ctx_store_meta"
  ],
  "call_depth": -1,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
int tail_handle_ipv4(struct __ctx_buff *ctx)
{
	__u32 src_identity = 0;
	int ret = handle_ipv4(ctx, &src_identity);

	if (IS_ERR(ret))
		return send_drop_notify_error(ctx, src_identity, ret,
					      CTX_ACT_DROP, METRIC_INGRESS);
	return ret;
}

#ifdef ENABLE_VTEP
/*
 * ARP responder for ARP requests from VTEP
 * Respond to remote VTEP endpoint with cilium_vxlan MAC
 */
__section_tail(CILIUM_MAP_CALLS, CILIUM_CALL_ARP)
/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "pkt_go_to_next_module",
      "pkt_go_to_next_module": [
        {
          "Project": "cilium",
          "Return Type": "int",
          "Input Params": [],
          "Function Name": "TC_ACT_OK",
          "Return": 0,
          "Description": "will terminate the packet processing pipeline and allows the packet to proceed. Pass the skb onwards either to upper layers of the stack on ingress or down to the networking device driver for transmission on egress, respectively. TC_ACT_OK sets skb->tc_index based on the classid the tc BPF program set. The latter is set out of the tc BPF program itself through skb->tc_classid from the BPF context.",
          "compatible_hookpoints": [
            "xdp",
            "sched_cls",
            "sched_act"
          ],
          "capabilities": [
            "pkt_go_to_next_module"
          ]
        }
      ]
    },
    {
      "capability": "map_read",
      "map_read": [
        {
          "Project": "cilium",
          "Return Type": "void*",
          "Description": "Perform a lookup in <[ map ]>(IP: 0) for an entry associated to key. ",
          "Return": " Map value associated to key, or NULL if no entry was found.",
          "Function Name": "map_lookup_elem",
          "Input Params": [
            "{Type: struct map ,Var: *map}",
            "{Type:  const void ,Var: *key}"
          ],
          "compatible_hookpoints": [
            "socket_filter",
            "kprobe",
            "sched_cls",
            "sched_act",
            "tracepoint",
            "xdp",
            "perf_event",
            "cgroup_skb",
            "cgroup_sock",
            "lwt_in",
            "lwt_out",
            "lwt_xmit",
            "sock_ops",
            "sk_skb",
            "cgroup_device",
            "sk_msg",
            "raw_tracepoint",
            "cgroup_sock_addr",
            "lwt_seg6local",
            "sk_reuseport",
            "flow_dissector",
            "cgroup_sysctl",
            "raw_tracepoint_writable"
          ],
          "capabilities": [
            "map_read"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 353,
  "endLine": 395,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/bpf_overlay.c",
  "funcName": "tail_handle_arp",
  "developer_inline_comments": [
    {
      "start_line": 348,
      "end_line": 351,
      "text": "/*\n * ARP responder for ARP requests from VTEP\n * Respond to remote VTEP endpoint with cilium_vxlan MAC\n */"
    }
  ],
  "updateMaps": [],
  "readMaps": [
    "  VTEP_MAP"
  ],
  "input": [
    "struct  __ctx_buff *ctx"
  ],
  "output": "int",
  "helper": [
    "CTX_ACT_OK",
    "map_lookup_elem"
  ],
  "compatibleHookpoints": [
    "xdp",
    "sched_act",
    "sched_cls"
  ],
  "source": [
    "int tail_handle_arp (struct  __ctx_buff *ctx)\n",
    "{\n",
    "    union macaddr mac = NODE_MAC;\n",
    "    union macaddr smac;\n",
    "    struct trace_ctx trace = {\n",
    "        .reason = TRACE_REASON_CT_REPLY,\n",
    "        .monitor = TRACE_PAYLOAD_LEN,}\n",
    "    ;\n",
    "    __be32 sip;\n",
    "    __be32 tip;\n",
    "    int ret;\n",
    "    struct bpf_tunnel_key key = {}\n",
    "    ;\n",
    "    struct vtep_key vkey = {}\n",
    "    ;\n",
    "    struct vtep_value *info;\n",
    "    if (unlikely (ctx_get_tunnel_key (ctx, &key, sizeof (key), 0) < 0))\n",
    "        return send_drop_notify_error (ctx, 0, DROP_NO_TUNNEL_KEY, CTX_ACT_DROP, METRIC_INGRESS);\n",
    "    if (!arp_validate (ctx, &mac, &smac, &sip, &tip) || !__lookup_ip4_endpoint (tip))\n",
    "        goto pass_to_stack;\n",
    "    vkey.vtep_ip = sip & VTEP_MASK;\n",
    "    info = map_lookup_elem (& VTEP_MAP, & vkey);\n",
    "    if (!info)\n",
    "        goto pass_to_stack;\n",
    "    ret = arp_prepare_response (ctx, & mac, tip, & smac, sip);\n",
    "    if (unlikely (ret != 0))\n",
    "        return send_drop_notify_error (ctx, 0, ret, CTX_ACT_DROP, METRIC_EGRESS);\n",
    "    if (info->tunnel_endpoint)\n",
    "        return __encap_and_redirect_with_nodeid (ctx, info->tunnel_endpoint, SECLABEL, WORLD_ID, &trace);\n",
    "    return send_drop_notify_error (ctx, 0, DROP_UNKNOWN_L3, CTX_ACT_DROP, METRIC_EGRESS);\n",
    "pass_to_stack :\n",
    "    send_trace_notify (ctx, TRACE_TO_STACK, 0, 0, 0, ctx->ingress_ifindex, trace.reason, trace.monitor);\n",
    "    return CTX_ACT_OK;\n",
    "}\n"
  ],
  "called_function_list": [
    "arp_prepare_response",
    "unlikely",
    "send_trace_notify",
    "arp_respond",
    "__encap_and_redirect_with_nodeid",
    "ctx_get_tunnel_key",
    "__lookup_ip4_endpoint",
    "send_drop_notify_error",
    "arp_validate"
  ],
  "call_depth": -1,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
int tail_handle_arp(struct __ctx_buff *ctx)
{
	union macaddr mac = NODE_MAC;
	union macaddr smac;
	struct trace_ctx trace = {
		.reason = TRACE_REASON_CT_REPLY,
		.monitor = TRACE_PAYLOAD_LEN,
	};
	__be32 sip;
	__be32 tip;
	int ret;
	struct bpf_tunnel_key key = {};
	struct vtep_key vkey = {};
	struct vtep_value *info;

	if (unlikely(ctx_get_tunnel_key(ctx, &key, sizeof(key), 0) < 0))
		return send_drop_notify_error(ctx, 0, DROP_NO_TUNNEL_KEY, CTX_ACT_DROP,
										METRIC_INGRESS);

	if (!arp_validate(ctx, &mac, &smac, &sip, &tip) || !__lookup_ip4_endpoint(tip))
		goto pass_to_stack;
	vkey.vtep_ip = sip & VTEP_MASK;
	info = map_lookup_elem(&VTEP_MAP, &vkey);
	if (!info)
		goto pass_to_stack;

	ret = arp_prepare_response(ctx, &mac, tip, &smac, sip);
	if (unlikely(ret != 0))
		return send_drop_notify_error(ctx, 0, ret, CTX_ACT_DROP, METRIC_EGRESS);
	if (info->tunnel_endpoint)
		return __encap_and_redirect_with_nodeid(ctx,
							info->tunnel_endpoint,
							SECLABEL,
							WORLD_ID,
							&trace);

	return send_drop_notify_error(ctx, 0, DROP_UNKNOWN_L3, CTX_ACT_DROP, METRIC_EGRESS);

pass_to_stack:
	send_trace_notify(ctx, TRACE_TO_STACK, 0, 0, 0, ctx->ingress_ifindex,
			  trace.reason, trace.monitor);
	return CTX_ACT_OK;
}
#endif /* ENABLE_VTEP */

#endif /* ENABLE_IPV4 */

#ifdef ENABLE_IPSEC
/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 401,
  "endLine": 428,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/bpf_overlay.c",
  "funcName": "is_esp",
  "developer_inline_comments": [
    {
      "start_line": 398,
      "end_line": 398,
      "text": "/* ENABLE_IPV4 */"
    }
  ],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct  __ctx_buff *ctx",
    " __u16 proto"
  ],
  "output": "static__always_inlinebool",
  "helper": [],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline bool is_esp (struct  __ctx_buff *ctx, __u16 proto)\n",
    "{\n",
    "    void *data, *data_end;\n",
    "    __u8 protocol = 0;\n",
    "    struct ipv6hdr * ip6 __maybe_unused;\n",
    "    struct iphdr * ip4 __maybe_unused;\n",
    "    switch (proto) {\n",
    "\n",
    "#ifdef ENABLE_IPV6\n",
    "    case bpf_htons (ETH_P_IPV6) :\n",
    "        if (!revalidate_data_pull (ctx, &data, &data_end, &ip6))\n",
    "            return false;\n",
    "        protocol = ip6->nexthdr;\n",
    "        break;\n",
    "\n",
    "#endif\n",
    "\n",
    "#ifdef ENABLE_IPV4\n",
    "    case bpf_htons (ETH_P_IP) :\n",
    "        if (!revalidate_data_pull (ctx, &data, &data_end, &ip4))\n",
    "            return false;\n",
    "        protocol = ip4->protocol;\n",
    "        break;\n",
    "\n",
    "#endif\n",
    "    default :\n",
    "        return false;\n",
    "    }\n",
    "    return protocol == IPPROTO_ESP;\n",
    "}\n"
  ],
  "called_function_list": [
    "revalidate_data_pull",
    "bpf_htons"
  ],
  "call_depth": -1,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline bool is_esp(struct __ctx_buff *ctx, __u16 proto)
{
	void *data, *data_end;
	__u8 protocol = 0;
	struct ipv6hdr *ip6 __maybe_unused;
	struct iphdr *ip4 __maybe_unused;

	switch (proto) {
#ifdef ENABLE_IPV6
	case bpf_htons(ETH_P_IPV6):
		if (!revalidate_data_pull(ctx, &data, &data_end, &ip6))
			return false;
		protocol = ip6->nexthdr;
		break;
#endif
#ifdef ENABLE_IPV4
	case bpf_htons(ETH_P_IP):
		if (!revalidate_data_pull(ctx, &data, &data_end, &ip4))
			return false;
		protocol = ip4->protocol;
		break;
#endif
	default:
		return false;
	}

	return protocol == IPPROTO_ESP;
}
#endif /* ENABLE_IPSEC */

/* Attached to the ingress of cilium_vxlan/cilium_geneve to execute on packets
 * entering the node via the tunnel.
 */
__section("from-overlay")
/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "pkt_go_to_next_module",
      "pkt_go_to_next_module": [
        {
          "Project": "cilium",
          "Return Type": "int",
          "Input Params": [],
          "Function Name": "TC_ACT_OK",
          "Return": 0,
          "Description": "will terminate the packet processing pipeline and allows the packet to proceed. Pass the skb onwards either to upper layers of the stack on ingress or down to the networking device driver for transmission on egress, respectively. TC_ACT_OK sets skb->tc_index based on the classid the tc BPF program set. The latter is set out of the tc BPF program itself through skb->tc_classid from the BPF context.",
          "compatible_hookpoints": [
            "xdp",
            "sched_cls",
            "sched_act"
          ],
          "capabilities": [
            "pkt_go_to_next_module"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 435,
  "endLine": 527,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/bpf_overlay.c",
  "funcName": "from_overlay",
  "developer_inline_comments": [
    {
      "start_line": 431,
      "end_line": 433,
      "text": "/* Attached to the ingress of cilium_vxlan/cilium_geneve to execute on packets\n * entering the node via the tunnel.\n */"
    },
    {
      "start_line": 444,
      "end_line": 444,
      "text": "/* Pass unknown traffic to the stack */"
    },
    {
      "start_line": 449,
      "end_line": 468,
      "text": "/* We need to handle following possible packets come to this program\n *\n * 1. ESP packets coming from overlay (encrypted and not marked)\n * 2. Non-ESP packets coming from overlay (plain and not marked)\n * 3. Non-ESP packets coming from stack re-inserted by xfrm (plain\n *    and marked with MARK_MAGIC_DECRYPT and has an identity as\n *    well, IPSec mode only)\n *\n * 1. will be traced with TRACE_REASON_ENCRYPTED\n * 2. will be traced without TRACE_REASON_ENCRYPTED\n * 3. will be traced without TRACE_REASON_ENCRYPTED, and with identity\n *\n * Note that 1. contains the ESP packets someone else generated.\n * In that case, we trace it as \"encrypted\", but it doesn't mean\n * \"encrypted by Cilium\".\n *\n * When IPSec is disabled, we won't use TRACE_REASON_ENCRYPTED even\n * if the packets are ESP, because it doesn't matter for the\n * non-IPSec mode.\n */"
    },
    {
      "start_line": 480,
      "end_line": 482,
      "text": "/* Non-ESP packet marked with MARK_MAGIC_DECRYPT is a packet\n\t\t * re-inserted from the stack.\n\t\t */"
    },
    {
      "start_line": 520,
      "end_line": 520,
      "text": "/* Pass unknown traffic to the stack */"
    }
  ],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct  __ctx_buff *ctx"
  ],
  "output": "int",
  "helper": [
    "CTX_ACT_OK"
  ],
  "compatibleHookpoints": [
    "xdp",
    "sched_act",
    "sched_cls"
  ],
  "source": [
    "int from_overlay (struct  __ctx_buff *ctx)\n",
    "{\n",
    "    __u16 proto;\n",
    "    int ret;\n",
    "    bpf_clear_meta (ctx);\n",
    "    bpf_skip_nodeport_clear (ctx);\n",
    "    if (!validate_ethertype (ctx, &proto)) {\n",
    "        ret = CTX_ACT_OK;\n",
    "        goto out;\n",
    "    }\n",
    "\n",
    "#ifdef ENABLE_IPSEC\n",
    "    if (is_esp (ctx, proto))\n",
    "        send_trace_notify (ctx, TRACE_FROM_OVERLAY, 0, 0, 0, ctx->ingress_ifindex, TRACE_REASON_ENCRYPTED, TRACE_PAYLOAD_LEN);\n",
    "    else\n",
    "\n",
    "#endif\n",
    "        {\n",
    "            __u32 identity = 0;\n",
    "            enum trace_point obs_point = TRACE_FROM_OVERLAY;\n",
    "            if ((ctx->mark & MARK_MAGIC_HOST_MASK) == MARK_MAGIC_DECRYPT) {\n",
    "                identity = get_identity (ctx);\n",
    "                obs_point = TRACE_FROM_STACK;\n",
    "            }\n",
    "            send_trace_notify (ctx, obs_point, identity, 0, 0, ctx->ingress_ifindex, TRACE_REASON_UNKNOWN, TRACE_PAYLOAD_LEN);\n",
    "        }\n",
    "    switch (proto) {\n",
    "    case bpf_htons (ETH_P_IPV6) :\n",
    "\n",
    "#ifdef ENABLE_IPV6\n",
    "        ep_tail_call (ctx, CILIUM_CALL_IPV6_FROM_OVERLAY);\n",
    "        ret = DROP_MISSED_TAIL_CALL;\n",
    "\n",
    "#else\n",
    "        ret = DROP_UNKNOWN_L3;\n",
    "\n",
    "#endif\n",
    "        break;\n",
    "    case bpf_htons (ETH_P_IP) :\n",
    "\n",
    "#ifdef ENABLE_IPV4\n",
    "        ep_tail_call (ctx, CILIUM_CALL_IPV4_FROM_OVERLAY);\n",
    "        ret = DROP_MISSED_TAIL_CALL;\n",
    "\n",
    "#else\n",
    "        ret = DROP_UNKNOWN_L3;\n",
    "\n",
    "#endif\n",
    "        break;\n",
    "\n",
    "#ifdef ENABLE_VTEP\n",
    "    case bpf_htons (ETH_P_ARP) :\n",
    "        ep_tail_call (ctx, CILIUM_CALL_ARP);\n",
    "        ret = DROP_MISSED_TAIL_CALL;\n",
    "        break;\n",
    "\n",
    "#endif\n",
    "    default :\n",
    "        ret = CTX_ACT_OK;\n",
    "    }\n",
    "out :\n",
    "    if (IS_ERR (ret))\n",
    "        return send_drop_notify_error (ctx, 0, ret, CTX_ACT_DROP, METRIC_INGRESS);\n",
    "    return ret;\n",
    "}\n"
  ],
  "called_function_list": [
    "send_trace_notify",
    "validate_ethertype",
    "is_esp",
    "ep_tail_call",
    "bpf_clear_meta",
    "IS_ERR",
    "send_drop_notify_error",
    "get_identity",
    "bpf_htons",
    "bpf_skip_nodeport_clear"
  ],
  "call_depth": -1,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
int from_overlay(struct __ctx_buff *ctx)
{
	__u16 proto;
	int ret;

	bpf_clear_meta(ctx);
	bpf_skip_nodeport_clear(ctx);

	if (!validate_ethertype(ctx, &proto)) {
		/* Pass unknown traffic to the stack */
		ret = CTX_ACT_OK;
		goto out;
	}

/* We need to handle following possible packets come to this program
 *
 * 1. ESP packets coming from overlay (encrypted and not marked)
 * 2. Non-ESP packets coming from overlay (plain and not marked)
 * 3. Non-ESP packets coming from stack re-inserted by xfrm (plain
 *    and marked with MARK_MAGIC_DECRYPT and has an identity as
 *    well, IPSec mode only)
 *
 * 1. will be traced with TRACE_REASON_ENCRYPTED
 * 2. will be traced without TRACE_REASON_ENCRYPTED
 * 3. will be traced without TRACE_REASON_ENCRYPTED, and with identity
 *
 * Note that 1. contains the ESP packets someone else generated.
 * In that case, we trace it as "encrypted", but it doesn't mean
 * "encrypted by Cilium".
 *
 * When IPSec is disabled, we won't use TRACE_REASON_ENCRYPTED even
 * if the packets are ESP, because it doesn't matter for the
 * non-IPSec mode.
 */
#ifdef ENABLE_IPSEC
	if (is_esp(ctx, proto))
		send_trace_notify(ctx, TRACE_FROM_OVERLAY, 0, 0, 0,
				  ctx->ingress_ifindex, TRACE_REASON_ENCRYPTED,
				  TRACE_PAYLOAD_LEN);
	else
#endif
	{
		__u32 identity = 0;
		enum trace_point obs_point = TRACE_FROM_OVERLAY;

		/* Non-ESP packet marked with MARK_MAGIC_DECRYPT is a packet
		 * re-inserted from the stack.
		 */
		if ((ctx->mark & MARK_MAGIC_HOST_MASK) == MARK_MAGIC_DECRYPT) {
			identity = get_identity(ctx);
			obs_point = TRACE_FROM_STACK;
		}

		send_trace_notify(ctx, obs_point, identity, 0, 0,
				  ctx->ingress_ifindex,
				  TRACE_REASON_UNKNOWN, TRACE_PAYLOAD_LEN);
	}

	switch (proto) {
	case bpf_htons(ETH_P_IPV6):
#ifdef ENABLE_IPV6
		ep_tail_call(ctx, CILIUM_CALL_IPV6_FROM_OVERLAY);
		ret = DROP_MISSED_TAIL_CALL;
#else
		ret = DROP_UNKNOWN_L3;
#endif
		break;

	case bpf_htons(ETH_P_IP):
#ifdef ENABLE_IPV4
		ep_tail_call(ctx, CILIUM_CALL_IPV4_FROM_OVERLAY);
		ret = DROP_MISSED_TAIL_CALL;
#else
		ret = DROP_UNKNOWN_L3;
#endif
		break;

#ifdef ENABLE_VTEP
	case bpf_htons(ETH_P_ARP):
		ep_tail_call(ctx, CILIUM_CALL_ARP);
		ret = DROP_MISSED_TAIL_CALL;
		break;
#endif

	default:
		/* Pass unknown traffic to the stack */
		ret = CTX_ACT_OK;
	}
out:
	if (IS_ERR(ret))
		return send_drop_notify_error(ctx, 0, ret, CTX_ACT_DROP, METRIC_INGRESS);
	return ret;
}

/* Attached to the egress of cilium_vxlan/cilium_geneve to execute on packets
 * leaving the node via the tunnel.
 */
__section("to-overlay")
/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "pkt_go_to_next_module",
      "pkt_go_to_next_module": [
        {
          "Project": "cilium",
          "Return Type": "int",
          "Input Params": [],
          "Function Name": "TC_ACT_OK",
          "Return": 0,
          "Description": "will terminate the packet processing pipeline and allows the packet to proceed. Pass the skb onwards either to upper layers of the stack on ingress or down to the networking device driver for transmission on egress, respectively. TC_ACT_OK sets skb->tc_index based on the classid the tc BPF program set. The latter is set out of the tc BPF program itself through skb->tc_classid from the BPF context.",
          "compatible_hookpoints": [
            "xdp",
            "sched_cls",
            "sched_act"
          ],
          "capabilities": [
            "pkt_go_to_next_module"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 533,
  "endLine": 568,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/bpf_overlay.c",
  "funcName": "to_overlay",
  "developer_inline_comments": [
    {
      "start_line": 529,
      "end_line": 531,
      "text": "/* Attached to the egress of cilium_vxlan/cilium_geneve to execute on packets\n * leaving the node via the tunnel.\n */"
    },
    {
      "start_line": 542,
      "end_line": 547,
      "text": "/* In tunneling mode, we should do this as close as possible to the\n\t * phys dev where FQ runs, but the issue is that the aggregate state\n\t * (in queue_mapping) is overridden on tunnel xmit. Hence set the\n\t * timestamp already here. The tunnel dev has noqueue qdisc, so as\n\t * tradeoff it's close enough.\n\t */"
    },
    {
      "start_line": 549,
      "end_line": 549,
      "text": "/* No send_drop_notify_error() here given we're rate-limiting. */"
    }
  ],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct  __ctx_buff *ctx"
  ],
  "output": "int",
  "helper": [
    "CTX_ACT_OK"
  ],
  "compatibleHookpoints": [
    "xdp",
    "sched_act",
    "sched_cls"
  ],
  "source": [
    "int to_overlay (struct  __ctx_buff *ctx)\n",
    "{\n",
    "    int ret;\n",
    "    ret = encap_remap_v6_host_address (ctx, true);\n",
    "    if (unlikely (ret < 0))\n",
    "        goto out;\n",
    "\n",
    "#ifdef ENABLE_BANDWIDTH_MANAGER\n",
    "    ret = edt_sched_departure (ctx);\n",
    "    if (ret == CTX_ACT_DROP) {\n",
    "        update_metrics (ctx_full_len (ctx), METRIC_EGRESS, -DROP_EDT_HORIZON);\n",
    "        return CTX_ACT_DROP;\n",
    "    }\n",
    "\n",
    "#endif\n",
    "\n",
    "#ifdef ENABLE_NODEPORT\n",
    "    if ((ctx->mark & MARK_MAGIC_SNAT_DONE) == MARK_MAGIC_SNAT_DONE) {\n",
    "        ret = CTX_ACT_OK;\n",
    "        goto out;\n",
    "    }\n",
    "    ret = handle_nat_fwd (ctx);\n",
    "\n",
    "#endif\n",
    "out :\n",
    "    if (IS_ERR (ret))\n",
    "        return send_drop_notify_error (ctx, 0, ret, CTX_ACT_DROP, METRIC_EGRESS);\n",
    "    return ret;\n",
    "}\n"
  ],
  "called_function_list": [
    "unlikely",
    "edt_sched_departure",
    "IS_ERR",
    "update_metrics",
    "ctx_full_len",
    "encap_remap_v6_host_address",
    "send_drop_notify_error",
    "handle_nat_fwd"
  ],
  "call_depth": -1,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
int to_overlay(struct __ctx_buff *ctx)
{
	int ret;

	ret = encap_remap_v6_host_address(ctx, true);
	if (unlikely(ret < 0))
		goto out;

#ifdef ENABLE_BANDWIDTH_MANAGER
	/* In tunneling mode, we should do this as close as possible to the
	 * phys dev where FQ runs, but the issue is that the aggregate state
	 * (in queue_mapping) is overridden on tunnel xmit. Hence set the
	 * timestamp already here. The tunnel dev has noqueue qdisc, so as
	 * tradeoff it's close enough.
	 */
	ret = edt_sched_departure(ctx);
	/* No send_drop_notify_error() here given we're rate-limiting. */
	if (ret == CTX_ACT_DROP) {
		update_metrics(ctx_full_len(ctx), METRIC_EGRESS,
			       -DROP_EDT_HORIZON);
		return CTX_ACT_DROP;
	}
#endif

#ifdef ENABLE_NODEPORT
	if ((ctx->mark & MARK_MAGIC_SNAT_DONE) == MARK_MAGIC_SNAT_DONE) {
		ret = CTX_ACT_OK;
		goto out;
	}
	ret = handle_nat_fwd(ctx);
#endif
out:
	if (IS_ERR(ret))
		return send_drop_notify_error(ctx, 0, ret, CTX_ACT_DROP, METRIC_EGRESS);
	return ret;
}

BPF_LICENSE("Dual BSD/GPL");
