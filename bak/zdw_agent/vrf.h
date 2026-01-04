#ifndef NTB_VRF_H
#define NTB_VRF_H

#include <sys/queue.h>

#include "ntb_dp.h"
#include "neigh.h"

struct ntb_tunnel_info;
typedef struct vrf_dp_i{
    /* IP routing.*/
    struct rte_hash *rib;
    struct rte_fib *fib;
    struct rte_fbarray *fib_res_array;
    int32_t rib_cnt;

    /* ND proxys.*/
    //struct ol_arp_proxy arp_proxys;

    /* Tunnel reverse mapping.*/
    SLIST_HEAD(, ntb_tunnel_info) tunnel_rmap;
    uint32_t tunnel_cnt;
    // uint32_t eg_tunnel_cnt;
    uint32_t neigh_cnt;
    uint32_t internal_ip;

    struct rte_ether_addr route_mac;

    int32_t vrf_id; /* Read only.*/
    // uint8_t tgre_version; /* gre version, support 0 or 1 */
} vrf_dp_t;

typedef struct {
    char name[NTB_VRF_NAME_LEN];
    int valid;

    vrf_dp_t dp;
} ntb_vrf_t;

typedef int
(*vrf_info_cb_t)(const ntb_vrf_t *vrf, void *arg1, void *arg2);


extern int
ntb_vrf_init(void);

extern int
ntb_vrf_deinit(void);

extern ntb_vrf_t *
ntb_vrf_lookup(const char *vrf_name);

extern int32_t
ntb_vrf_id_lookup(const char *vrf_name);

extern int
ntb_vrf_add(const char *vrf_name);

extern int
ntb_vrf_del(const char *vrf_name);

extern vrf_dp_t *
ntb_vrf_dp_get_by_id(uint32_t vrfid);

extern int
ntb_vrf_count(void);

extern int
ntb_vrf_intip_set(const char *vrf_name, uint32_t intip);

extern int
ntb_vrf_intip_unset(const char *vrf_name);

extern int
ntb_vrf_intip_get(const char *vrf_name, uint32_t *vrf_ip);

// extern int
// ntb_vrf_tgre_version_set(vrf_dp_t *vrf_dp);

// extern int
// ntb_vrf_tgre_version_unset(vrf_dp_t *vrf_dp);

// extern int
// ntb_global_tgre_version_set();

// extern int
// ntb_global_tgre_version_unset();

// extern int
// ntb_global_tgre_version_get(int *version_ret);

ntb_vrf_t *
ntb_vrf_get_by_id(uint32_t vrfid);

// extern int
// ntb_vrf_info_walk(vrf_info_cb_t cb, void *arg1, void *arg2);

extern int
ntb_vrf_rmac_set(const char *vrf_name, const struct rte_ether_addr *route_mac);

extern int
ntb_vrf_rmac_unset(const char *vrf_name, const struct rte_ether_addr *route_mac);

extern int
ntb_vrf_rmac_get(const char *vrf_name, struct rte_ether_addr *route_mac);

#endif
