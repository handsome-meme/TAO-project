#ifndef NTB_NEIGH_H
#define NTB_NEIGH_H

#include "ntb_dp.h"

#define PROXY_IP_PER_VRF 8

// struct ol_arp_proxy_entry {
//     uint32_t proxy_ip; /* key.*/
//     struct rte_ether_addr mac_addr;
// };

// struct ol_arp_proxy {
//     struct {
//         int valid;
//         struct ol_arp_proxy_entry ent;
//     } entrys[PROXY_IP_PER_VRF];
// };

struct ol_neigh_lookup_key {
    uint32_t vrfid; /*starting from 1*/
    uint32_t nexthop;
}__rte_packed;

struct ol_neigh_entry {
    uint8_t F_arp_valid:1;
    uint8_t F_fast_arp:1;
    uint8_t F_arp_hot:1;
    uint8_t F_entry_valid:1;
    uint8_t F_arp_check:1;
    uint8_t F_arp_static:1;
    uint8_t F_unused:2;
    
    uint8_t hh_len;
    uint8_t hh[22]; /*ether [+ vlan][ + vlan]*/
    uint32_t neigh_oid; /*neigh object id correspond to p4 table */
    uint32_t refcnt;
    
    uint64_t tx_tsc;
    uint64_t retry_interval;
    uint64_t lastest_report_time;
};

// typedef int 
// (*neigh_entry_cb_t)(struct ol_neigh_lookup_key *key,
//                 struct ol_neigh_entry *entry,
//                 void *arg1, void *arg2, void *arg3);

// extern int
// ntb_neigh_walk(neigh_entry_cb_t cb, void *arg1, void *arg2, void *arg3);

// typedef int 
// (*arp_proxy_entry_cb_t)(struct ol_arp_proxy_entry *entry,
//                 void *arg1, void *arg2);

// extern int
// ntb_arp_proxy_walk(struct ol_arp_proxy *arp_proxy, arp_proxy_entry_cb_t cb, void *arg1, void *arg2);

/* neighor entry event.*/
typedef enum {
    NEIGH_NOTIFY_CREATE,
    NEIGH_NOTIFY_DELETE,
} neigh_event_type_t;

typedef int
(*neigh_event_notify_cb_t)(neigh_event_type_t type,
                const struct ol_neigh_lookup_key *key,
                const struct ol_neigh_entry *nhop);

extern int
ntb_api_neigh_ent_chg_notify(neigh_event_type_t type,
                const struct ol_neigh_lookup_key *key,
                const struct ol_neigh_entry *nhop);

/* ARP proxy.*/
// extern int
// ntb_nd_proxy_lookup(const struct ol_arp_proxy *arp_proxy, 
//                 uint32_t proxy_ip, struct ol_arp_proxy_entry *entry);

// extern int
// ntb_arp_proxy_add(struct ol_arp_proxy *arp_proxy, 
//                 const struct ol_arp_proxy_entry *entry);

// extern int
// ntb_arp_proxy_del(struct ol_arp_proxy *arp_proxy, 
//                 uint32_t proxy_ip);

// extern int
// ntb_arp_proxy_ent_cnt(const struct ol_arp_proxy *arp_proxy);

// extern int
// ntb_arp_proxy_cleanup(struct ol_arp_proxy *arp_proxy);

extern int
ntb_neigh_init(ntb_pmd_ctx_t *ctx);

extern int
ntb_neigh_deinit(void);

typedef struct vrf_dp_i vrf_dp_t;
extern int
ntb_neigh_add(vrf_dp_t *vrf_dp, uint32_t gw, uint32_t *neigh_oid_ret);

extern int
ntb_neigh_del(vrf_dp_t *vrf_dp, uint32_t gw);

extern int
ntb_neigh_lookup(uint32_t vrfid, uint32_t gw,
                struct rte_ether_addr *hw_addr);

// extern int
// ntb_neigh_learning(ntb_pmd_ctx_t *ctx,
//                 uint32_t vrfid, uint32_t gw,
//                 const struct rte_ether_addr *hw_addr,
//                 int notify);

// extern int
// ntb_vrf_neigh_update(uint32_t vrfid);

// extern int
// ntb_neigh_click_set(uint32_t vrfid, uint32_t ip);

extern int
ntb_neigh_discover_timer(void *ctx);

// extern int
// ntb_nd_sip_pick(const struct ol_arp_proxy *arp_proxy,
//                 uint32_t dip, uint32_t *sip);

extern int
neigh_probe_xmit(void *ctx, uint32_t vrfid, uint32_t nexthop);

// extern int
// ol_neigh_add_static(uint32_t vrfid, uint32_t gw,
//                 const struct rte_ether_addr *hw_addr);

extern const struct rte_ether_addr *
ntb_fake_mac_get(void);

extern int
ntb_fake_mac_set(const struct rte_ether_addr *ether_addr);

extern const struct rte_ether_addr *
ntb_def_gw_mac_get(void);

extern int
ntb_def_gw_mac_set(const struct rte_ether_addr *ether_addr);

#endif

