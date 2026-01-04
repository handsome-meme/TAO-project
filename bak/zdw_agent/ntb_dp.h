#ifndef NTB_DP_H
#define NTB_DP_H

#include <event.h>

#include <rte_common.h>
#include <rte_mempool.h>
#include <rte_rwlock.h>

// #include "stat.h"

/* Definication of NTB dataplane specification and helper macro 
 * for specification sanity check
 */

/* VRF spec.*/
#define NTB_NR_VRF              (4096)
#define NTB_NR_ROUTES_ALL       (800 * 1024)
#define NTB_NR_FIB_RES_ALL      (800 * 1024) 
#define NTB_NR_NHOP_ALL         (NTB_NR_FIB_RES_ALL)
#define NTB_NR_TUNNEL_ALL       (8 * 1024)

#define NTB_VRF_NAME_LEN        (64)

#define VRF_NAME_VALID(vrfname)              \
    ((!!vrfname) && ((vrfname)[0] != '\0')   \
    && ((vrfname)[0] != ' ') \
    && (strlen(vrfname) < NTB_VRF_NAME_LEN) \
    && (!find_str_spec_char(vrfname)))

/* Route ECMP spec.*/
#define ROUTE_MAX_ECMP  (1) //from 64 change to 1
#define MROUTE_MAX_ECMP (8)

#define MAX_NHOPS_PER_RTOUE (ROUTE_MAX_ECMP > MROUTE_MAX_ECMP ? ROUTE_MAX_ECMP : MROUTE_MAX_ECMP)

#define ROUTE_ECMP_VALID(e)  ((e) <= ROUTE_MAX_ECMP)
#define MROUTE_ECMP_VALID(e) ((e) <= MROUTE_MAX_ECMP)

/* Route WECMP spec.*/
#define NH_WEIGHT_MIN  (1)
#define NH_WEIGHT_MAX  (99)
#define NH_WEIGHT_DEF  (5)
#define NH_WEIGHT_VALID(w) \
    ((w) >= NH_WEIGHT_MIN && (w) <= NH_WEIGHT_MAX)
#define NH_WEIGHT_ARG_VALID(w) \
    ((w) >= 0 && (w) <= NH_WEIGHT_MAX)

/* Route IPv4 prefix.*/
#define IP4_PREFIX_LEN_VALID(l) ((l) <= 32)
#define IP6_PREFIX_LEN_VALID(l) ((l) <= 128)
#define IP4_ADDR_IS_ZERO(ip4)  ((ip4) == 0)

/* Route path's DSCP.*/
#define MIN_DSCP_ID (0)
#define MAX_DSCP_ID (63)
#define DSCP_ID_VALID(d) \
    ((d) >= MIN_DSCP_ID && (d) <= MAX_DSCP_ID)


/* VXLAN.*/
#define VXLAN_HF_VNI (0x08000000)
#define VXLAN_DEFAULT_SPORT (55555)


/* Overlay gateway spec.*/
#define MAX_VRF_GW_HASH_ENT (10000)


static inline int find_str_spec_char(const char *str)
{
    uint32_t i = 0;
    for(i = 0;i < strlen(str);i++) {
        if(!isprint(str[i])) {
            return 1;
        }
    }
    return 0;
}


typedef void (*io_rx_fn)(const int sock, void *arg);
typedef void (*tmr_fn)(void *ctx);

typedef struct {
    /* DPDK environment.*/
    struct rte_mempool *mp;
    uint32_t msize; /* DPDK memory size #MB.*/
    int pid; /* pmd id: 0 is master.*/

    /* netio event.*/
    struct event_base *ev_base;
    struct event timer_event;
    struct timeval timer_tv;
    struct event udp_event;
    // struct event gre_event;
    int udp_sock;
    // int gre_sock;

    rte_rwlock_t *dp_locker;
    io_rx_fn netio_rx;
    uint16_t ip_pktid;

    /* neighbor discovery timer fn.*/
    // tmr_fn nd_tmr_fn;

    /* ARP report notification.*/
    // int neigh_notify_fd_tx; /* sending end of notify pipe */
    // io_rx_fn arp_rx;

    /* ntb_swf_evt_stat_t evt_stat;
     ntb_swf_evt_stat_t evt_base_stat;*/

    /* Private fields.*/
    int is_valid;
} ntb_pmd_ctx_t;

typedef enum {
    NTB_HAL_OP_ADD,
    NTB_HAL_OP_DEL,
    NTB_HAL_OP_UPDATE,
} ntb_hal_op_type_t;

typedef int 
(*pmd_thr_cb_t)(ntb_pmd_ctx_t *pmd_thr_ctx, void *arg1, void *arg2);

extern int
ntb_pmd_thread_walk(pmd_thr_cb_t cb, void *arg1, void *arg2);

#endif
