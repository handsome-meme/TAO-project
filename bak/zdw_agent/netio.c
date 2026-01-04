#include <event.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/filter.h>

#include <rte_common.h>
#include <rte_byteorder.h>
#include <rte_log.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_eal.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_malloc.h>
#include <rte_rwlock.h>

#include "netio.h"
#include "neigh.h"

static rte_rwlock_t ntb_dp_locker = RTE_RWLOCK_INITIALIZER;


void
ntb_netio_write_lock(void)
{
    return rte_rwlock_write_lock(&ntb_dp_locker);
}

void
ntb_netio_write_unlock(void)
{
    return rte_rwlock_write_unlock(&ntb_dp_locker);
}

void
ntb_netio_read_lock(void)
{
    return rte_rwlock_read_lock(&ntb_dp_locker);
}

void
ntb_netio_read_unlock(void)
{
    return rte_rwlock_read_unlock(&ntb_dp_locker);
}

static void
netio_cb(const int sock, short int which, void *arg)
{
    ntb_pmd_ctx_t *ctx = arg;

    switch (which) {
    case EV_READ:
        if (ctx->netio_rx)
            ctx->netio_rx(sock, arg);
        break;
    case EV_WRITE:
    default:
        /* Log it.*/
        break;
    }
}

static void
timer_cb(const int sock, short int which, void *arg)
{
    ntb_pmd_ctx_t *ctx = arg;

    ntb_neigh_discover_timer(ctx);

    event_add(&ctx->timer_event, &ctx->timer_tv);
}

// static void __rte_unused
// arp_report_cb(const int sock, short int which, void *arg)
// {
//     ntb_pmd_ctx_t *ctx;

//     ctx = arg;

//     switch (which) {
//     case EV_READ:
//         if (ctx->arp_rx)
//             ctx->arp_rx(sock, arg);
//         break;
//     case EV_WRITE:
//     default:
//         break;
//     }
// }

static int
udp_netio_init(ntb_pmd_ctx_t *ctx)
{
    const int on = 1;
    int raw_sock;
    int ret;

    static struct sock_filter insns[] = {
        BPF_STMT(BPF_LD|BPF_B|BPF_ABS, 9),                  /* load 9th byte (protocol) */
        BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, IPPROTO_UDP, 0, 6), /* jump to L1 if it is IPPROTO_UDP, else to L4 */
        BPF_STMT(BPF_LD|BPF_H|BPF_ABS, 6),                  /* L1: load halfword from offset 6 (flags and frag offset) */
        BPF_JUMP(BPF_JMP|BPF_JSET|BPF_K, 0x1fff, 4, 0),     /* jump to L4 if any bits in frag offset field are set, else to L2 */
        BPF_STMT(BPF_LDX|BPF_B|BPF_MSH, 0),                 /* L2: skip IP header (load index reg with header len) */
        BPF_STMT(BPF_LD|BPF_H|BPF_IND, 2),                  /* load udp destination port from halfword[header_len + 2] */
        BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, 4789, 0, 1),        /* jump to L3 if udp dport is vxlan 4789 , else to L4 */
        BPF_STMT(BPF_RET|BPF_K, 0x7fffffff),                /* L3: accept packet  */
        BPF_STMT(BPF_RET|BPF_K, 0)                          /* L4: discard packet ("accept zero bytes") */
    };

    static struct sock_fprog filter = {
        sizeof insns / sizeof(insns[0]),
        insns
    };

    /* Initialize inet raw socket for UDP proto.*/
    raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    RTE_ASSERT(raw_sock >= 0);

    ret = setsockopt(raw_sock, SOL_IP, IP_HDRINCL, &on, sizeof(on));
    RTE_ASSERT(ret == 0);

    ret = setsockopt(raw_sock, SOL_SOCKET, SO_ATTACH_FILTER, 
                &filter, sizeof(filter));
    RTE_ASSERT(ret == 0);

    ctx->udp_sock = raw_sock;

    RTE_ASSERT(!!ctx->ev_base);
    ret = event_assign(&ctx->udp_event, ctx->ev_base, ctx->udp_sock,
                       EV_READ|EV_PERSIST, netio_cb, ctx);
    RTE_ASSERT(ret == 0);

    ret = event_add(&ctx->udp_event, NULL);
    RTE_ASSERT(ret == 0);

    return ret;
}

// static int
// gre_netio_init(ntb_pmd_ctx_t *ctx)
// {
//     const int on = 1;
//     int raw_sock;
//     int ret;

//     /* Initialize inet raw socket for GRE proto.*/
//     raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_GRE);
//     RTE_ASSERT(raw_sock >= 0);

//     ret = setsockopt(raw_sock, SOL_IP, IP_HDRINCL, &on, sizeof(on));
//     RTE_ASSERT(ret == 0);

//     ctx->gre_sock = raw_sock;

//     RTE_ASSERT(!!ctx->ev_base);
//     ret = event_assign(&ctx->gre_event, ctx->ev_base, ctx->gre_sock,
//                        EV_READ|EV_PERSIST, netio_cb, ctx);
//     RTE_ASSERT(ret == 0);

//     ret = event_add(&ctx->gre_event, NULL);
//     RTE_ASSERT(ret == 0);

//     return ret;
// }

int
ntb_netio_init(ntb_pmd_ctx_t *ctx)
{
    int ret;

    /* Initialize dataplane locker.*/
    ctx->dp_locker = &ntb_dp_locker;

    /* Intialize netio event.*/
    ctx->ev_base = event_base_new_with_config(NULL);
    RTE_ASSERT(!!ctx->ev_base);

    ret = udp_netio_init(ctx);
    RTE_ASSERT(0 == ret);

    // ret = gre_netio_init(ctx);
    RTE_ASSERT(0 == ret);

    /* Intialize netio clock source.*/
    ret = event_assign(&ctx->timer_event, ctx->ev_base, -1,
                       0, timer_cb, ctx);
    RTE_ASSERT(ret == 0);

    ctx->timer_tv.tv_sec = 1;
    ctx->timer_tv.tv_usec = 0;
    ret = event_add(&ctx->timer_event, &ctx->timer_tv);
    RTE_ASSERT(ret == 0);

    return 0;
}

int
ntb_netio_deinit(ntb_pmd_ctx_t *ctx)
{
    return 0;
}

int
ntb_netio_run(ntb_pmd_ctx_t *ctx)
{
    return event_base_loop(ctx->ev_base, 0);
}
