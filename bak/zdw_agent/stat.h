#ifndef NTB_STAT_H
#define NTB_STAT_H

#include <rte_debug.h>


#define foreach_swf_evt_type \
    _ (NETIO_MBUF_ALLOC_FAILED, 1) \
    _ (NETIO_PKT_RECV_FAILED, 1) \
    _ (NETIO_PKT_RECV_SUCCEED, 0) \
    _ (IP_TUNNEL_TYPE_UNSUPPORTED, 1) \
    _ (VXLAN_RX_NOT_UDP, 1) \
    _ (VXLAN_RX_NOT_VXLAN_PORT, 1) \
    _ (VXLAN_NOT_MAP_VRF, 1) \
    _ (VXLAN_TX_MBUF_PREPARE_NOSPACE, 1) \
    _ (VXLAN_TX_SEND_FAILED, 1) \
    _ (TGRE_INNER_NOT_IP4, 1) \
    _ (TGRE_NOT_MAP_VRF, 1) \
    _ (TGRE_TX_MBUF_PREPARE_NOSPACE, 1) \
    _ (TGRE_TX_SEND_FAILED, 1) \
    _ (OVERLAY_IP_RECEIVED, 0) \
    _ (OVERLAY_IP_LOOKUP_ROUTE_FAILED, 1) \
    _ (OVERLAY_IP_ROUTE_REJECT, 1) \
    _ (OVERLAY_IP_TTL_EXCEEDED, 1) \
    _ (OVERLAY_IP_FORWARD_NOARP, 1) \
    _ (OVERLAY_IP_FORWARD_SEND, 0) \
    _ (OVERLAY_ICMP_INTIP_NOT_SET, 1) \
    _ (OVERLAY_ICMP_NOT_FIRST_FRAG, 1) \
    _ (OVERLAY_ICMP_MBUF_ALLOC_FAILED, 1) \
    _ (OVERLAY_ICMP_MBUF_PREP_NOSPACE, 1) \
    _ (OVERLAY_ICMP_LOOKUP_ROUTE_FAILED, 1) \
    _ (OVERLAY_ICMP_NEXTHOP_REJECT, 1) \
    _ (OVERLAY_ICMP_NEXTHOP_NOARP, 1) \
    _ (OVERLAY_ICMP_SEND, 0) \
    _ (OVERLAY_ARP_RECEIVED, 0) \
    _ (OVERLAY_ARP_REQ_RECEIVED, 0) \
    _ (OVERLAY_ARP_REQ_NOT_TO_ME, 1) \
    _ (OVERLAY_ARP_REPLAY_SEND, 0) \
    _ (OVERLAY_ARP_PROBE_MBUF_ALLOC_FAILED, 1) \
    _ (OVERLAY_ARP_PROBE_VRF_LOOKUP_FAILED, 1) \
    _ (OVERLAY_ARP_PROBE_ROUTE_LOOKUP_FAILED, 1) \
    _ (OVERLAY_ARP_PROBE_ROUTE_RESULT_REJECT, 1) \
    _ (OVERLAY_ARP_PROBE_SELECT_SIP_FAILED, 1) \
    _ (OVERLAY_ARP_PROBE_SEND, 0) \
    _ (OVERLAY_ARP_LEARN_NOTIFY, 0) \
    _ (OVERLAY_ARP_LEARN_NOTIFY_FAILED, 1) \
    _ (PACKET_MAX, 0)

    /* Notes: 
     *   Append new ntb error code before PACKET_MAX
     */ 

enum {
#define _(f, e) f,
    foreach_swf_evt_type 
#undef _
};

#define NTB_SWF_EVT_TYPE_VALID(t) \
    ((t) >= 0 && (t) < PACKET_MAX)


typedef struct {
    uint64_t stat[PACKET_MAX];
} ntb_swf_evt_stat_t;

extern __thread ntb_swf_evt_stat_t *cur_thr_stats;

#define SWF_EVT_STAT_INC(type) ({        \
    RTE_ASSERT((type) < PACKET_MAX); \
    cur_thr_stats->stat[(type)]++;   \
})

extern int
ntb_swf_evt_stat_total_get(ntb_swf_evt_stat_t *total);

extern int
ntb_swf_evt_stat_clear(void);

extern const char *
ntb_swf_evt_str_get(int type);

extern const char
ntb_swf_evt_is_err(int type);

/* Memory pool statistics.*/
typedef struct {
    char *mp_name;
    uint32_t count_avail;
    uint32_t count_inuse;
    uint32_t count_all;
} mempool_stat_t;

typedef int
(*mempool_walk_cb_t)(const mempool_stat_t *mp_stat, void *arg);

extern uint32_t
ntb_mempool_cnt_get(void);

extern void
ntb_mempool_stat_walk(mempool_walk_cb_t cb, void *arg);

extern int
ntb_mempool_stat_get(mempool_stat_t *mp_stat);

/* Socket memory statistics.*/
typedef struct {
    int node_id;
    size_t total_bytes; /**< Total bytes on heap */
    size_t free_bytes;  /**< Total free bytes on heap */
    size_t alloc_bytes; /**< Total allocated bytes on heap */
} ntb_socket_mem_stat_t;

extern int
ntb_socket_mem_stat_get(int node_n, ntb_socket_mem_stat_t *node_mem_stats);

#endif
