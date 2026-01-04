#ifndef SHC_VRF_H
#define SHC_VRF_H

#include <sys/queue.h>
#include <rte_ether.h>
#include <stdio.h>
#include <stdlib.h>

#include "shc_dp.h"
#include "neigh.h"

struct shc_tunnel_info;
// struct lpm_route_info
// {
//     int8_t lpm_counter;
//     uint32_t spc_route_info;
// };
// typedef struct {
//     uint32_t lpm_counter;//该表项被引用次数
//     uint32_t rout_info;//具体的ip
//     struct ListNode* next;
// }ListNode;

typedef struct ListNode{//定义链表
    uint32_t lpm_counter;//该表项被引用次数
    uint32_t rout_info;//具体的ip
    struct ListNode* next;
}linkNode;

struct ListNode_t{
    linkNode* List_head;
    int List_len;//链表的长度
};

typedef struct vrf_dp_i{
    /* IP routing.*/
    struct rte_hash *rib;
    struct rte_fib *fib;
    struct rte_fbarray *fib_res_array;
    int32_t rib_cnt;

    /* ND proxys.*/
    //struct ol_arp_proxy arp_proxys;

    /* Tunnel reverse mapping.*/
    SLIST_HEAD(, shc_tunnel_info) tunnel_rmap;
    uint32_t tunnel_cnt;
    // uint32_t eg_tunnel_cnt;
    uint32_t neigh_cnt;
    uint32_t internal_ip;

    struct rte_ether_addr route_mac;

    int32_t vrf_id; /* Read only.*/

    int8_t exit_flag;
    // struct lpm_route_info *route_info[ROUTE_MAX_ECMP];//存放具体路由信息的结构体
    // int8_t route_info_array_i;//结构体route_info的括号中的值
    struct ListNode_t* List_Node;

} vrf_dp_t;

typedef struct {
    char name[SHC_VRF_NAME_LEN];
    int valid;

    vrf_dp_t dp;
} shc_vrf_t;

typedef int
(*vrf_info_cb_t)(const shc_vrf_t *vrf, void *arg1, void *arg2);


extern int
shc_vrf_init(void);

extern int
shc_vrf_deinit(void);

extern shc_vrf_t *
shc_vrf_lookup(const char *vrf_name);

extern int32_t
shc_vrf_id_lookup(const char *vrf_name);

extern int
shc_vrf_add(const char *vrf_name);

extern int
shc_vrf_del(const char *vrf_name);

extern vrf_dp_t *
shc_vrf_dp_get_by_id(uint32_t vrfid);

extern int
shc_vrf_count(void);

extern int
shc_vrf_intip_set(const char *vrf_name, uint32_t intip);

extern int
shc_vrf_intip_unset(const char *vrf_name);

extern int
shc_vrf_intip_get(const char *vrf_name, uint32_t *vrf_ip);

// extern int
// shc_vrf_tgre_version_set(vrf_dp_t *vrf_dp);

// extern int
// shc_vrf_tgre_version_unset(vrf_dp_t *vrf_dp);

// extern int
// shc_global_tgre_version_set();

// extern int
// shc_global_tgre_version_unset();

// extern int
// shc_global_tgre_version_get(int *version_ret);

shc_vrf_t *
shc_vrf_get_by_id(uint32_t vrfid);

// extern int
// shc_vrf_info_walk(vrf_info_cb_t cb, void *arg1, void *arg2);

extern int
shc_vrf_rmac_set(const char *vrf_name, const struct rte_ether_addr *route_mac);

extern int
shc_vrf_rmac_unset(const char *vrf_name, const struct rte_ether_addr *route_mac);

extern int
shc_vrf_rmac_get(const char *vrf_name, struct rte_ether_addr *route_mac);

#endif
