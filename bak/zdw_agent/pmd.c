#include <event.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stat.h>

#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/prctl.h>

#include <linux/icmp.h>

#include <rte_common.h>
#include <rte_byteorder.h>
#include <rte_log.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_launch.h>
#include <rte_atomic.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_debug.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_malloc.h>
#include <rte_ip.h>
#include <rte_arp.h>
#include <rte_udp.h>
#include <rte_icmp.h>
#include <rte_vxlan.h>
#include <rte_gre.h>
#include "rte_hash.h"
#include "rte_hash_crc.h"
#include "rte_jhash.h"

#include <rte_ip.h>
#include <rte_log.h>
#include <rte_fib.h>

#include "ntb_dp.h"
#include "netio.h"
#include "neigh.h"
#include "vrf.h"
#include "route.h"


#include "tunnel_map.h"

// #include "ntb_api.h"

#define NB_MBUF   (1024*8)

/* NTB datapath running context.*/
typedef int
(*pmd_run_cb_t)(void *arg);

typedef struct {
    int nd_notify_fd;
    pmd_run_cb_t run_prepare;
    pmd_run_cb_t run_post;
} ntb_pmd_thread_args_t; 

struct key_value {
    //key info
    int32_t  vrf_id;
    uint32_t vni;
    uint32_t inner_src_ip;
    uint32_t inner_dst_ip;
    uint64_t next_hop;

    //encap value info
    uint32_t vni_encp;
    struct rte_ether_addr  outer_src_mac;
    struct rte_ether_addr  outer_dst_mac;
    uint32_t outer_src_ip;
    uint32_t outer_dst_ip;

    struct rte_ether_addr  inner_src_mac;
    struct rte_ether_addr  inner_dst_mac;

}key_value_info;

static struct rte_ether_addr outer_fake_smac = {
    .addr_bytes = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66}
};
static struct rte_ether_addr outer_fake_dmac = {
    .addr_bytes = {0x11, 0x22, 0x33, 0x44, 0x55, 0x67}
};

struct ntb_mbuf_metadata {
    struct ip_tunnel_info in_tunnel_info;
    void *vrf_dp;
} __rte_cache_aligned;

static inline size_t
ntb_mbuf_metadata_size(void)
{
    return sizeof(struct ntb_mbuf_metadata);
}

static inline struct ntb_mbuf_metadata *
ntb_get_mbuf_priv(struct rte_mbuf *m)
{
    return rte_mbuf_to_priv(m);
}

static int
ntb_dp_mbuf_init(ntb_pmd_ctx_t *ctx)
{
    struct rte_mempool *mbuf_pool;

    mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NB_MBUF, 32,
                ntb_mbuf_metadata_size(),
                RTE_MBUF_DEFAULT_BUF_SIZE,
                rte_socket_id());
    RTE_ASSERT(mbuf_pool != NULL);
    ctx->mp = mbuf_pool;
    return 0;    
}

static int
ntb_dp_mbuf_deinit(ntb_pmd_ctx_t *ctx)
{
#if 0
    struct rte_mempool *mbuf_pool;

    mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NB_MBUF, 32,
                                        ntb_mbuf_metadata_size(), 
                                        RTE_MBUF_DEFAULT_BUF_SIZE,
                                        rte_socket_id());
    if (mbuf_pool == NULL)
        return -1;

    ctx->mp = mbuf_pool;
#endif
    return 0;    
}

typedef enum {
    PTS_INIT = 0,
    PTS_RUNNING,
    PTS_IDEL,
} pmd_thr_stat_t;
static pmd_thr_stat_t ntb_pmd_thr_stat = PTS_INIT;

static inline int
ntb_pmd_thr_stat_set(pmd_thr_stat_t stat)
{
    ntb_pmd_thr_stat = stat;
    return 0;
}

static int
ntb_dp_eal_init(ntb_pmd_ctx_t *ctx)
{
    /* 2GB memory for DPDK by default.*/
    char msize[16] = "2048"; 
    char *argv[] = { 
        "swf",
        "--no-huge",
        "-m",
        msize,
        "--syslog",
        "syslog",
        NULL };
    int argc = RTE_DIM(argv) - 1;
    int ret;

    if (ctx->msize != 0)
        snprintf(msize, sizeof(msize), "%d", ctx->msize);

    /* PMD0 is master which is responsible for DPDK setup.*/
    if (ctx->pid != 0)
        return 0;

    /* Init DPDK EAL */
    ret = rte_eal_init(argc, argv);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");

    return 0;
}

static int
ntb_dp_eal_deinit(ntb_pmd_ctx_t *ctx)
{
    return rte_eal_cleanup();
}

static int
ntb_dp_init(ntb_pmd_ctx_t *ctx)
{
    if (ntb_dp_eal_init(ctx) < 0)
        rte_exit(EXIT_FAILURE, "Initialize EAL failed\n");

    if (ntb_dp_mbuf_init(ctx) < 0)
        rte_exit(EXIT_FAILURE, "Initialize mbuf pool failed\n");

    if (ntb_vrf_init() < 0)
        rte_exit(EXIT_FAILURE, "Initialize VRF failed\n");

    if (ntb_tunnel_map_vrf_init() < 0)
        rte_exit(EXIT_FAILURE, "Initialize VRF mapping failed\n");

    if (ntb_neigh_init(ctx) < 0)
        rte_exit(EXIT_FAILURE, "Initialize overlay neigh failed\n");

    if (ntb_netio_init(ctx) < 0)
        rte_exit(EXIT_FAILURE, "Initialize raw socket failed\n"); //done at 4.22

    return 0;
}

static int ntb_dp_deinit(ntb_pmd_ctx_t *ctx)
{
    if (ntb_netio_deinit(ctx) < 0)
        rte_exit(EXIT_FAILURE, "finish raw socket failed\n");

    if (ntb_neigh_deinit() < 0)
        rte_exit(EXIT_FAILURE, "finish overlay neigh failed\n");

    if (ntb_tunnel_map_vrf_deinit() < 0)
        rte_exit(EXIT_FAILURE, "finish VRF mapping failed\n");

    if (ntb_vrf_deinit() < 0)
        rte_exit(EXIT_FAILURE, "finish VRF failed\n");

    if (ntb_dp_mbuf_deinit(ctx) < 0)
        rte_exit(EXIT_FAILURE, "finish mbuf pool failed\n");
    
    if (ntb_dp_eal_deinit(ctx) < 0)
        rte_exit(EXIT_FAILURE, "finish EAL failed\n");

    return 0;
}

const uint8_t ip_template[] = {
    0x45, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0xff, 0x11, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
};

// static int
// encap_vxlan_tx(struct rte_mbuf *pkt, void *ctx,
//             uint32_t remote, uint32_t local, uint32_t vni, uint8_t dscp)
// {
//     struct rte_ipv4_hdr *iphdr;
//     struct rte_udp_hdr *udphdr;
//     struct rte_vxlan_hdr *vxlanhdr;

//     ntb_pmd_ctx_t *dp_ctx = ctx;
//     int nbytes;

//     /* Build VXLAN header.*/
//     vxlanhdr = (typeof(vxlanhdr))rte_pktmbuf_prepend(pkt, sizeof(*vxlanhdr));
//     if (NULL == vxlanhdr) {
//         SWF_EVT_STAT_INC(VXLAN_TX_MBUF_PREPARE_NOSPACE);
//         return -ENOSPC;
//     }

//     vxlanhdr->vx_flags = rte_cpu_to_be_32(VXLAN_HF_VNI);
//     vxlanhdr->vx_vni = rte_cpu_to_be_32(vni << 8);

//     /* Build UDP header.*/
//     udphdr = (typeof(udphdr))rte_pktmbuf_prepend(pkt, sizeof(*udphdr));
//     if (NULL == udphdr) {
//         SWF_EVT_STAT_INC(VXLAN_TX_MBUF_PREPARE_NOSPACE);
//         return -ENOSPC;
//     }

//     udphdr->dst_port = rte_cpu_to_be_16(RTE_VXLAN_DEFAULT_PORT);
//     udphdr->src_port = pkt->hash.rss ? rte_cpu_to_be_16(pkt->hash.rss)
//                                      : rte_cpu_to_be_16(VXLAN_DEFAULT_SPORT);
//     udphdr->dgram_cksum = 0;
//     udphdr->dgram_len = rte_cpu_to_be_16(rte_pktmbuf_pkt_len(pkt));

//     /* Build IP header.*/
//     iphdr = (typeof(iphdr))rte_pktmbuf_prepend(pkt, sizeof(*iphdr));
//     if (NULL == iphdr) {
//         SWF_EVT_STAT_INC(VXLAN_TX_MBUF_PREPARE_NOSPACE);
//         return -ENOSPC;
//     }

//     rte_memcpy(iphdr, ip_template, sizeof(*iphdr));
//     iphdr->src_addr = local;
//     iphdr->dst_addr = remote;
//     iphdr->packet_id = rte_cpu_to_be_16(dp_ctx->ip_pktid++);           
//     iphdr->type_of_service = dscp << 2;
//     iphdr->total_length = rte_cpu_to_be_16(rte_pktmbuf_pkt_len(pkt));
//     iphdr->hdr_checksum = rte_ipv4_cksum(iphdr);

//     /* Packet transmite.*/
//     struct sockaddr_in dst_addr = {
//         .sin_family = AF_INET,
//         .sin_port = rte_cpu_to_be_16(RTE_VXLAN_DEFAULT_PORT),
//         .sin_addr.s_addr = remote,
//     };

//     nbytes = sendto(dp_ctx->udp_sock,
//                     rte_pktmbuf_mtod(pkt, const void *),
//                     rte_pktmbuf_pkt_len(pkt), 0, 
//                     (struct sockaddr *)&dst_addr, sizeof(dst_addr));
//     if (nbytes == -1) {
//         SWF_EVT_STAT_INC(VXLAN_TX_SEND_FAILED);
//     }

//     return 0;
// }

static inline int
ip_tunnel_inner_ether_need(const struct ip_tunnel_info *tunnel_i)
{
    int inner_ether_need = 0;

    switch (tunnel_i->type) {
    case NTB_ENCAP_VXLAN:
        inner_ether_need = 1;
        break;

    case NTB_ENCAP_TGRE:
        inner_ether_need = 0;
        break;

    default:
        break;
    }

    return inner_ether_need;
}

static inline int
ip_tunnel_encap_size(const struct ip_tunnel_info *tunnel_i)
{
    int size = 0;

    switch (tunnel_i->type) {
    case NTB_ENCAP_VXLAN:
        size = sizeof(struct rte_ether_hdr)  /* inner ethernet.*/
             + sizeof(struct rte_vxlan_hdr)  /* outter vxlan.  */
             + sizeof(struct rte_udp_hdr)    /* outter udp.    */
             + sizeof(struct rte_ipv4_hdr);  /* outter ip.     */
        break;

    case NTB_ENCAP_TGRE:
        size = sizeof(struct rte_gre_hdr)    /* outter gre.    */
             + sizeof(struct rte_ipv4_hdr);  /* outter ip.     */
        break;

    default:
        break;
    }

    return size;
}

// int
// ip_tunnel_xmit(struct rte_mbuf *pkt, void *ctx,
//                 const struct ip_tunnel_info *tunnel_i)
// {
//     uint32_t localip;
//     int ret = -1;

//     switch (tunnel_i->type) {
//     case NTB_ENCAP_VXLAN:
//         localip = tunnel_i->vxlan.local;
//         if (localip == 0)
//             ntb_dev_forward_vip_get(&localip);

//         ret = encap_vxlan_tx(pkt, ctx,
//                 tunnel_i->vxlan.remote, localip,
//                 tunnel_i->vxlan.vni, tunnel_i->vxlan.dscp);
//         break;

//     case NTB_ENCAP_TGRE:
//         ntb_dev_forward_vip_get(&localip);
//         ret = encap_tgre_tx(pkt, ctx,
//                 tunnel_i->tgre.remote, localip,
//                 tunnel_i->tgre.vpcid, tunnel_i->tgre.vmip);
//         break;

//     default:
//         SWF_EVT_STAT_INC(IP_TUNNEL_TYPE_UNSUPPORTED);
//         break;
//     }

//     return ret;
// }

static inline void
ip_forward(void *ctx, vrf_dp_t *vrf_dp, struct rte_mbuf *pkt,
                struct fib_res *f_res)
{
    struct rte_ether_hdr *ethhdr;
    int nhsel, ret;

    nhsel = fib_res_select_mpath(f_res,  pkt->hash.rss);

    if (ip_tunnel_inner_ether_need(&f_res->mpath[nhsel]->tunnel_info)) {
        /* Build innner L2 header.*/
        ethhdr = (typeof(ethhdr))rte_pktmbuf_prepend(pkt, sizeof(*ethhdr));
        if (NULL == ethhdr) {
            SWF_EVT_STAT_INC(VXLAN_TX_MBUF_PREPARE_NOSPACE);
            return;
        }

        if (!IP4_ADDR_IS_ZERO(f_res->mpath[nhsel]->gateway)) {
            if (ntb_neigh_lookup(vrf_dp->vrf_id,
                    f_res->mpath[nhsel]->gateway,
                    &ethhdr->d_addr) < 0) {
                SWF_EVT_STAT_INC(OVERLAY_IP_FORWARD_NOARP);
                neigh_probe_xmit(ctx, vrf_dp->vrf_id,
                        f_res->mpath[nhsel]->gateway);
                return;
            }
        } else {
            rte_ether_addr_copy(ntb_def_gw_mac_get(), &ethhdr->d_addr);
        }
        rte_ether_addr_copy(ntb_fake_mac_get(), &ethhdr->s_addr);

        rte_ether_addr_copy(&ethhdr->d_addr, &key_value_info.inner_dst_mac);
        rte_ether_addr_copy(&ethhdr->s_addr, &key_value_info.inner_src_mac);

        key_value_info.vni_encp = rte_cpu_to_be_32(f_res->mpath[nhsel]->tunnel_info.vxlan.vni << 8); //mycode

        key_value_info.outer_src_ip = f_res->mpath[nhsel]->tunnel_info.vxlan.local;//my code
        key_value_info.outer_dst_ip = f_res->mpath[nhsel]->tunnel_info.vxlan.remote;//my code
        // key_value_info.outer_tos = f_res->mpath[nhsel]->tunnel_info.vxlan.dscp;
        
        rte_ether_addr_copy(&outer_fake_dmac, &key_value_info.outer_dst_mac);
        rte_ether_addr_copy(&outer_fake_smac, &key_value_info.outer_src_mac);

    }
    return;
}

static inline void
ip_forward_m(void *ctx, vrf_dp_t *vrf_dp, struct rte_mbuf *pkt, 
                struct fib_res *f_res)
{
    struct rte_ether_hdr *ethhdr;
    uint16_t encap_size;
    int nhsel;

    for (nhsel = 0; nhsel < f_res->mpath_cnt; nhsel++) {
        if (ip_tunnel_inner_ether_need(&f_res->mpath[nhsel]->tunnel_info)) {
            /* Build innner L2 header.*/
            ethhdr = (typeof(ethhdr))rte_pktmbuf_prepend(pkt, sizeof(*ethhdr));
            if (NULL == ethhdr) {
                SWF_EVT_STAT_INC(VXLAN_TX_MBUF_PREPARE_NOSPACE);
                return;
            }

            if (!IP4_ADDR_IS_ZERO(f_res->mpath[nhsel]->gateway)) {
                if (ntb_neigh_lookup(vrf_dp->vrf_id,
                        f_res->mpath[nhsel]->gateway,
                        &ethhdr->d_addr) < 0) {
                    SWF_EVT_STAT_INC(OVERLAY_IP_FORWARD_NOARP);
                    neigh_probe_xmit(ctx, vrf_dp->vrf_id,
                            f_res->mpath[nhsel]->gateway);
                    return;
                }
            } else {
                rte_ether_addr_copy(ntb_def_gw_mac_get(), &ethhdr->d_addr);
            }
            rte_ether_addr_copy(ntb_fake_mac_get(), &ethhdr->s_addr);
        }

        /* Packet send via tunnel.*/
        // encap_size = ip_tunnel_encap_size(&f_res->mpath[nhsel]->tunnel_info);
        // ip_tunnel_xmit(pkt, ctx, &f_res->mpath[nhsel]->tunnel_info);
        // rte_pktmbuf_adj(pkt, encap_size);
    }

    // SWF_EVT_STAT_INC(OVERLAY_IP_FORWARD_SEND);
    return;
}

static void
ol_ip_rcv(struct rte_mbuf *pkt, void *ctx)
{
    struct ntb_mbuf_metadata *mbuf_md;
    vrf_dp_t *vrf_dp;
    struct rte_ipv4_hdr *iphdr;
    struct fib_res *f_res;
    int ret;

    mbuf_md = ntb_get_mbuf_priv(pkt);
    vrf_dp = mbuf_md->vrf_dp; 
    RTE_ASSERT(vrf_dp != NULL);

    iphdr = rte_pktmbuf_mtod(pkt, struct rte_ipv4_hdr *);
    ret = ntb_route_lookup(vrf_dp, iphdr->dst_addr, &f_res); //check vrf(route) table find the exact route path
    if (ret != 0) {
        // SWF_EVT_STAT_INC(OVERLAY_IP_LOOKUP_ROUTE_FAILED);
        return;
    }

    if (fib_res_is_reject(f_res)) {
        // SWF_EVT_STAT_INC(OVERLAY_IP_ROUTE_REJECT);
        return;
    }


    if (fib_res_is_mroute(f_res))
        ip_forward_m(ctx, vrf_dp, pkt, f_res);
    else
        ip_forward(ctx, vrf_dp, pkt, f_res);

    return;

}

static void
ol_ether_rcv(struct rte_mbuf *pkt, void *ctx)
{
    struct rte_ipv4_hdr *iphdr;

    iphdr = (struct rte_ipv4_hdr *)rte_pktmbuf_adj(pkt,
                sizeof(struct rte_ether_hdr));
    RTE_SET_USED(iphdr);
    ol_ip_rcv(pkt, ctx);
}

static int
overlay_pkt_rcv(struct rte_mbuf *pkt, void *arg)
{
    struct rte_ether_hdr *ethhdr;

    ethhdr = rte_pktmbuf_mtod(pkt, struct rte_ether_hdr *);
    switch (ntohs(ethhdr->ether_type)) {
    case RTE_ETHER_TYPE_IPV4:
        // SWF_EVT_STAT_INC(OVERLAY_IP_RECEIVED);
        ol_ether_rcv(pkt, arg);
        break;

    default:
        /* update error statistics: not supported.*/
        break;
    }
    return 0;
}

static void
vxlan_tunnel_rcv(struct rte_mbuf *pkt, void *ctx)
{
    struct rte_ipv4_hdr *iphdr;
    struct rte_udp_hdr *udphdr;
    struct rte_vxlan_hdr *vxlanhdr;
    struct ntb_mbuf_metadata *mbuf_md;
    vrf_dp_t *vrf_dp;
    ntb_tunnel_id_t tunnel_id;
    uint32_t vni;

    iphdr = rte_pktmbuf_mtod(pkt, struct rte_ipv4_hdr *);
    pkt->outer_l3_len = (iphdr->version_ihl & 0xf) << 2;

    udphdr = (struct rte_udp_hdr *)rte_pktmbuf_adj(pkt, pkt->outer_l3_len);
    if (unlikely(udphdr->dst_port != rte_cpu_to_be_16(RTE_VXLAN_DEFAULT_PORT))) {
        // SWF_EVT_STAT_INC(VXLAN_RX_NOT_VXLAN_PORT);
        return;
    }

    vxlanhdr = (struct rte_vxlan_hdr *)rte_pktmbuf_adj(pkt, sizeof(*udphdr));
    tunnel_id.vni = vni = rte_be_to_cpu_32(vxlanhdr->vx_vni) >> 8;

    /* Lookup VRF with ingress VNI.*/
    vrf_dp = ntb_tunnel_map_vrf_lookup(NTB_TUNNEL_VXLAN, tunnel_id); //check tunnel map vrf table
    if (!vrf_dp) {
        // SWF_EVT_STAT_INC(VXLAN_NOT_MAP_VRF);
        return;
    }

    /* Remember the inbound tunnel info.*/
    mbuf_md = ntb_get_mbuf_priv(pkt);
    mbuf_md->in_tunnel_info.type = NTB_ENCAP_VXLAN;
    mbuf_md->in_tunnel_info.vxlan.remote = iphdr->dst_addr;
    mbuf_md->in_tunnel_info.vxlan.local = iphdr->src_addr;
    mbuf_md->in_tunnel_info.vxlan.vni = vni;
    mbuf_md->in_tunnel_info.vxlan.dscp = iphdr->type_of_service >> 2;
    mbuf_md->vrf_dp = vrf_dp;

    /* Calculate outter flow rss. 核间负载均衡*/
    //pkt->hash.rss = calc_l3_flow_rss(iphdr, 0);  //how to set the rss value at a single cpu core
    // pkt->hash.rss = rte_hash_crc(iphdr, 0);

    rte_pktmbuf_adj(pkt, sizeof(*vxlanhdr));//将vxlan头部截取后，得到overlay的头部，即原始的以太网数据帧
    overlay_pkt_rcv(pkt, ctx);
}

static void
tunnel_rcv(struct rte_mbuf *pkt, void *ctx)
{
    struct rte_ipv4_hdr *iphdr;

    iphdr = rte_pktmbuf_mtod(pkt, struct rte_ipv4_hdr *);
    pkt->outer_l3_len = (iphdr->version_ihl & 0xf) << 2;

    switch (iphdr->next_proto_id) {
    case IPPROTO_UDP:
        vxlan_tunnel_rcv(pkt, ctx);
        break;

    default:
        // SWF_EVT_STAT_INC(IP_TUNNEL_TYPE_UNSUPPORTED);
        break;
    }
}

static void
netio_rx(const int sock, void *arg)
{
    int nbytes;
    struct rte_mbuf *mbuf;
    ntb_pmd_ctx_t *dp_ctx = arg;
    int data_off = RTE_PKTMBUF_HEADROOM;

    /* allocate the mbuf */
    mbuf = rte_pktmbuf_alloc(dp_ctx->mp);
    if (mbuf == NULL) {
        // SWF_EVT_STAT_INC(NETIO_MBUF_ALLOC_FAILED);
        return;
    }

    memset(ntb_get_mbuf_priv(mbuf), 0, ntb_mbuf_metadata_size());
    nbytes = recv(sock, rte_pktmbuf_mtod(mbuf, void *),
                  mbuf->buf_len - data_off, 0);
    if (nbytes < 0) {
        // SWF_EVT_STAT_INC(NETIO_PKT_RECV_FAILED);
        event_loopbreak();
    }
    rte_pktmbuf_pkt_len(mbuf) = rte_pktmbuf_data_len(mbuf) = nbytes;

    rte_rwlock_read_lock(dp_ctx->dp_locker);
    tunnel_rcv(mbuf, arg);
    rte_rwlock_read_unlock(dp_ctx->dp_locker);

    rte_pktmbuf_free(mbuf);

    // SWF_EVT_STAT_INC(NETIO_PKT_RECV_SUCCEED);
}

#define NTB_PMD_THR_MAX   (8)
static ntb_pmd_ctx_t *ntb_pmd_ctxs[NTB_PMD_THR_MAX];

int
ntb_pmd_thread_walk(pmd_thr_cb_t cb, void *arg1, void *arg2)
{
    int i;

    for (i = 0; i < RTE_DIM(ntb_pmd_ctxs); i++)
        if (ntb_pmd_ctxs[i])
            if (cb(ntb_pmd_ctxs[i], arg1, arg2) < 0)
                break;

    return 0;
}

static ntb_pmd_ctx_t *
ntb_pmd_ctx_create(void)
{
    ntb_pmd_ctx_t *pmd_ctx;
    int i, pid;

    /* Allocate a pmd ID.*/
    for (i = 0; i < RTE_DIM(ntb_pmd_ctxs); i++)
        if (NULL == ntb_pmd_ctxs[i])
            break;
    if (i >= RTE_DIM(ntb_pmd_ctxs))
        return NULL; /* Too many pmd threads created.*/

    pid = i;
    pmd_ctx = calloc(sizeof(*pmd_ctx), 1);
    if (NULL == pmd_ctx)
        return NULL;

    pmd_ctx->pid = pid;
    // pmd_ctx->neigh_notify_fd_tx = -1;
    //cur_thr_stats = &pmd_ctx->evt_stat;

    /* Register the new pmd thread.*/
    ntb_pmd_ctxs[pid] = pmd_ctx;
    return pmd_ctx;
}

void *
ntb_pmd_thread(void *arg)
{
    ntb_pmd_thread_args_t *thr_args = arg; //ntb_api.h 全局变量

    prctl(PR_SET_NAME, "ntb_pmd_thread");

    ntb_pmd_ctx_t *pmd_ctx = ntb_pmd_ctx_create();
    if (NULL == pmd_ctx)
        rte_exit(EXIT_FAILURE, "Create NTB pmd context failed\n");

    pmd_ctx->netio_rx = netio_rx;
    // pmd_ctx->neigh_notify_fd_tx = thr_args->nd_notify_fd;

    if (ntb_dp_init(pmd_ctx) < 0)
        rte_exit(EXIT_FAILURE, "Initialize NTB dataplane failed\n");

    ntb_pmd_thr_stat_set(PTS_RUNNING);

    if (thr_args->run_prepare)
        thr_args->run_prepare(arg);

    /* netio running; does not return. */
    ntb_netio_run(pmd_ctx);
    ntb_dp_deinit(pmd_ctx);

    if (thr_args->run_post)
        thr_args->run_post(arg);

    return 0;
}

int
ntb_api_ntb_pmd_thread_ready_get(void)
{
    return PTS_RUNNING == ntb_pmd_thr_stat;
}

