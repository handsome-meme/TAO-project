#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h> /* offsetof() */

#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>

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
#include <rte_vxlan.h>
#include "rte_hash.h"
#include "rte_hash_crc.h"
#include "rte_jhash.h"
#include "rte_rib.h"
#include "rte_fib.h"

#include <rte_ip.h>
#include <rte_log.h>
#include <rte_fib.h>

#include <rte_rwlock.h>

#include "ntb_errno.h"
#include "ntb_dp.h"
#include "netio.h"
#include "neigh.h"
#include "vrf.h"
#include "route.h"
#include "tunnel_map.h"
// #include "ntb_spec.h"
// #include <hal_table_type.h>
// #include <hal_table_api.h>


static struct rte_hash *g_nhop_hash_tbl;

struct ntb_nhop_hash_key {
    uint32_t gateway;
    struct ip_tunnel_info tunnel_info;
} __rte_packed;


static inline struct fib_res *
fib_res_get(struct rte_fbarray *fib_res_array, int f_res_id)
{
    return rte_fbarray_get(fib_res_array, f_res_id);
}

static inline rib_type_t
fib_res_rtype_get(const struct fib_res *f_res)
{
    if (f_res->fr_flags & FRF_MROUTE)
        return RT_MROUTE;

    return RT_DEFAULT;
}

static prefix_type_t
rt_prefix_type_get(const vrf_dp_t *vrf_dp, 
                    const struct ntb_rib_key *pfx, int64_t f_res_id)
{
    struct fib_res *f_res;

    f_res = fib_res_get(vrf_dp->fib_res_array, f_res_id);
    RTE_ASSERT(f_res != NULL);
    if (f_res->mpath_cnt == 1 && 
            f_res->mpath[0]->tunnel_info.type == NTB_ENCAP_TGRE &&
            pfx->prefixlen == 32) {
        return RT_PREFIX_HOST;
    }

    return RT_PREFIX_LPM;
}

static inline int
fib_res_mpath_nospace(const struct fib_res *fib_res)
{
    return fib_res->mpath_cnt >= fib_res->mpath_max;
}

static inline int
fib_nhop_equal(const struct fib_nhop *nhop_args,
               const struct fib_nhop_encap_data *match_key)
{
    int is_equal = 0;

    switch (nhop_args->tunnel_info.type) {
    case NTB_ENCAP_VXLAN:
        is_equal = (nhop_args->gateway == match_key->gateway) && 
                    (!memcmp(&nhop_args->tunnel_info,
                        &match_key->tunnel_info,
                        sizeof(nhop_args->tunnel_info)));
        break;

    case NTB_ENCAP_TGRE:
        is_equal = !memcmp(&nhop_args->tunnel_info,
                &match_key->tunnel_info,
                sizeof(nhop_args->tunnel_info));
        break;

    default:
        break;
    }

    return is_equal;
}


static int
nhop_obj_create(vrf_dp_t *vrf_dp, 
                    const struct fib_nhop_encap_data *nhop_encap_data,
                    struct fib_nhop** fib_nhop_ret)
{
    struct ntb_nhop_hash_key nhop_key;
    struct fib_nhop *f_nhop;
    uint32_t nh_oid = 0;
    uint32_t neigh_oid = 0;
    ntb_tunnel_type_t tun_type;
    ntb_tunnel_id_t tun_id;
    int ret = 0;

    nhop_key.gateway = nhop_encap_data->gateway;
    nhop_key.tunnel_info = nhop_encap_data->tunnel_info;
    if (rte_hash_lookup_data(g_nhop_hash_tbl, &nhop_key, (void**)&f_nhop) < 0) { //no match entry
        f_nhop = rte_zmalloc_socket(NULL, sizeof(*f_nhop), 0, SOCKET_ID_ANY);
        if (f_nhop == NULL) {
            ret = NTB_COMM_NOMEM;
            return ret;
        }
        f_nhop->ref_count = 0;
        f_nhop->gateway = nhop_encap_data->gateway;
        f_nhop->tunnel_info = nhop_encap_data->tunnel_info;

        if (rte_hash_add_key_data(g_nhop_hash_tbl, &nhop_key, ( void **)f_nhop) < 0) {
            ret = -EFAULT;
            goto rollback5;
        }
        /* ntb overlay neigh add */
        if (!IP4_ADDR_IS_ZERO(f_nhop->gateway)) { // 添加了一条路由，需要把overlay的neigh信息完善
            ret = ntb_neigh_add(vrf_dp, f_nhop->gateway, &neigh_oid); 
            if (ret < 0) {
                goto rollback4;
            }
            f_nhop->neigh_oid = neigh_oid; //overlay中neigh会自行创建oid，在硬件中对应
        }

        /* alloc oid */
        //ret = hal_tunnel_nexthop_alloc_oid(&nh_oid);//---------------待修改------------
        if (ret < 0) {
            ret = NTB_ROUTE_NH_ALLOC_OID_FAIL; 
            goto rollback3;
        }
        f_nhop->nh_oid = nh_oid;

        /* call hal func */
        // ret = _ntb_hal_nhop_add(f_nhop); //根据
        // if (ret < 0) {
        //     goto rollback2;
        // }
    }
    RTE_ASSERT(f_nhop != NULL);

    // /* ntb egress tunnel map */
    // if (f_nhop->tunnel_info.type == NTB_ENCAP_VXLAN) {
    //     tun_type = NTB_TUNNEL_VXLAN; 
    //     tun_id.vni = f_nhop->tunnel_info.vxlan.vni;
    // } else {
    //     tun_type = NTB_TUNNEL_TGRE; 
    //     tun_id.vni = f_nhop->tunnel_info.tgre.vpcid;
    // }
    // ret = ntb_eg_tunnel_map(vrf_dp, tun_type, tun_id);
    // if (ret < 0) {
    //     /* f_nhop is already referenced by other routes, just return */
    //     if (f_nhop->ref_count > 0) 
    //         return ret;
    //     goto rollback1;
    // }

    f_nhop->ref_count++;
    *fib_nhop_ret = f_nhop;
    return ret;

// rollback1:
//     _ntb_hal_nhop_del(f_nhop->nh_oid);
// rollback2:
//     hal_tunnel_nexthop_free_oid(nh_oid);
rollback3:
    if (!IP4_ADDR_IS_ZERO(f_nhop->gateway)) {
        ntb_neigh_del(vrf_dp, f_nhop->gateway);
    }
rollback4:
    rte_hash_del_key(g_nhop_hash_tbl, &nhop_key);
rollback5:
    rte_free(f_nhop);
    f_nhop = NULL;
    return ret;
}

static inline int
fib_res_md_set(struct fib_res *f_res, struct fib_res *f_res_old,
                rib_type_t rib_type)
{
    int ret = 0;

    if (f_res_old)
        fib_res_clone_flags(f_res, f_res_old);

    switch (rib_type) {
    case RT_DEFAULT:
        f_res->mpath_max = ROUTE_MAX_ECMP;
        fib_res_unset_flags(f_res, FRF_MROUTE);
        break;

    case RT_MROUTE:
        f_res->mpath_max = MROUTE_MAX_ECMP;
        fib_res_set_flags(f_res, FRF_MROUTE);
        break;

    default:
        RTE_ASSERT(0);
        ret = -1;
        break;
    }
    return ret;
}

static inline void
fib_res_reset(struct fib_res *f_res)
{
    f_res->fr_flags = 0;
    f_res->mpath_cnt = 0;
    f_res->mpath_max = ROUTE_MAX_ECMP;
    memset(f_res->mpath, 0, sizeof(f_res->mpath));
}

static int
fib_res_id_alloc(struct rte_fbarray *fib_res_array)
{
    struct fib_res *fib_res_new;
    int f_res_id;
    int ret;

    f_res_id = rte_fbarray_find_next_free(fib_res_array, 0);
    if (f_res_id < 0)
        return -ENOSPC;

    ret = rte_fbarray_set_used(fib_res_array, f_res_id);
    RTE_ASSERT(0 == ret);

    fib_res_new = rte_fbarray_get(fib_res_array, f_res_id);
    RTE_ASSERT(fib_res_new != NULL);
    fib_res_reset(fib_res_new);

    return f_res_id;
}


static inline void
rib_res_nhop_ctor(const vrf_dp_t *vrf_dp,
                struct fib_res *f_res, int nhsel,
                struct fib_nhop *nhop) //copy nhop to f_res
{
    f_res->mpath[nhsel] = nhop;
    f_res->mpath_cnt++;
}

static int
fib_res_obj_create(vrf_dp_t *vrf_dp, 
                    struct fib_nhop** fib_nhops, 
                    int nhop_num, 
                    struct fib_res *f_res_old, /* if new, then set NULL */
                    rib_type_t rib_type)
{   
    struct fib_res *f_res;
    int64_t f_res_id;
    int i, ret = 0;

    f_res_id = fib_res_id_alloc(vrf_dp->fib_res_array); //找到下一个可用f_res_id并初始化f_res
    if (f_res_id < 0)
        return NTB_ROUTE_RESULT_NOSPACE;
    f_res = fib_res_get(vrf_dp->fib_res_array, f_res_id);
    for (i = 0; i < nhop_num; i++) { //copy nhop to f_res
        rib_res_nhop_ctor(vrf_dp, f_res, i, fib_nhops[i]);
    }

    if (rib_type == RT_DEFAULT) {
        if (nhop_num == 1) {
            f_res->f_res_oid = fib_nhops[0]->nh_oid;
        } else {
            /* alloc ecmp group oid */
            // ret = hal_ecmp_alloc_oid(&f_res->f_res_oid);
            // if (ret < 0) {
            //     goto rollback2;
            // }
            /* call hal crete ecmp group */
            // ret = _ntb_hal_ecmp_group_add(f_res);
            // if (ret < 0) {
            //     goto rollback1;
            // }
        }
    } else { 
        /* mroute f_res_oid = mgid = nhop_num */
        f_res->f_res_oid = nhop_num;
    }

    /* If f_res_old exists, inherit the original attributes, such as nottl */
    if (f_res_old) {
        f_res->fr_flags = f_res_old->fr_flags;
    }
    /* when transroute/transmroute, will overwrite the attribute */
    fib_res_md_set(f_res, NULL, rib_type); //设置fib_res中mpath_max和fr_flag字段
    return f_res_id;

// rollback1:
//     hal_ecmp_free_oid(f_res->f_res_oid);
// rollback2:
//     fib_res_id_free(vrf_dp->fib_res_array, f_res_id);
//     return ret;
}

static int
rt_prefix_obj_update(vrf_dp_t *vrf_dp, 
                    const struct ntb_rib_key *pfx,
                    int64_t f_res_id, int64_t f_res_id_new) 
{
    int ret = 0;
    prefix_type_t rt_ptype, rt_ptype_new;
    struct fib_res *f_res;
    struct fib_res *f_res_new;
    int is_add = 0;

    rt_ptype = RT_PREFIX_INVALID;
    if (f_res_id > 0)
        rt_ptype = rt_prefix_type_get(vrf_dp, pfx, f_res_id);
    rt_ptype_new = rt_prefix_type_get(vrf_dp, pfx, f_res_id_new);

    /* get f_res_oid */
    f_res_new = fib_res_get(vrf_dp->fib_res_array, f_res_id_new);
    RTE_ASSERT(f_res_new != NULL);

    /* update rib */
    if (rte_hash_add_key_data(vrf_dp->rib, pfx, (void *)(long)f_res_id_new) < 0) { //直接覆盖f_res_id
        ret = NTB_ROUTE_PREFIX_NOSPACE; 
        return ret;
    }
    /* update fib.*/
    if (rte_fib_add(vrf_dp->fib, rte_be_to_cpu_32(pfx->prefix4), pfx->prefixlen, f_res_id_new)) {
        ret = NTB_ROUTE_PREFIX_NOSPACE; 
        goto rollback2;
    }
    if ((rt_ptype != RT_PREFIX_INVALID 
                && rt_ptype != rt_ptype_new) || f_res_id < 0) {
        
        is_add = 1;
    }
    /* update hal prefix */ //根据flag进行更新
    // ret = _ntb_hal_rt_prefix_update(pfx, rt_ptype_new, f_res_new, is_add);
    // if (ret < 0) {
    //     goto rollback1;
    // }

    // if (rt_ptype != RT_PREFIX_INVALID && 
    //                 rt_ptype != rt_ptype_new) { //协议不一致就删除旧的表项
    //     f_res = fib_res_get(vrf_dp->fib_res_array, f_res_id);
    //     RTE_ASSERT(f_res != NULL);
    //     ret = _ntb_hal_rt_prefix_delete(pfx, rt_ptype, f_res);
    //     RTE_ASSERT(ret == 0);
    // }

    if (f_res_id < 0) {
        vrf_dp->rib_cnt++; //路由数目
    }

    return ret;

// rollback1:
//     if (f_res_id < 0)
//         rte_fib_delete(vrf_dp->fib, 
//                     rte_be_to_cpu_32(pfx->prefix4), pfx->prefixlen);
//     else 
//         rte_fib_add(vrf_dp->fib, rte_be_to_cpu_32(pfx->prefix4), pfx->prefixlen, f_res_id);
rollback2:
    if (f_res_id < 0)
        rte_hash_del_key(vrf_dp->rib, pfx);
    else
        rte_hash_add_key_data(vrf_dp->rib, pfx, (void *)(long)f_res_id); 
    return ret;
}

static int
nhop_obj_delete(vrf_dp_t *vrf_dp, struct fib_nhop *f_nhop)
{
    int ret = 0;
    struct ntb_nhop_hash_key nhop_key;
    ntb_tunnel_type_t tun_type;
    ntb_tunnel_id_t tun_id;

    /* egress tunnel unmap */
    if (f_nhop->tunnel_info.type == NTB_ENCAP_VXLAN) {
        tun_type = NTB_TUNNEL_VXLAN; 
        tun_id.vni = f_nhop->tunnel_info.vxlan.vni;
    } else {
        tun_type = NTB_TUNNEL_TGRE; 
        tun_id.vni = f_nhop->tunnel_info.tgre.vpcid;
    }
    // ret = ntb_eg_tunnel_unmap(vrf_dp, tun_type, tun_id);
    // RTE_ASSERT(0 == ret);

    /* nhop del */
    f_nhop->ref_count--;
    if (f_nhop->ref_count > 0) { //若表项被引用数目为0，就删除表项；否则直接返回
        return ret;
    }

    nhop_key.gateway = f_nhop->gateway;
    nhop_key.tunnel_info = f_nhop->tunnel_info;
    ret = rte_hash_del_key(g_nhop_hash_tbl, &nhop_key); 
    RTE_ASSERT(0 <= ret);

    /* ntb overlay neigh del */
    if (!IP4_ADDR_IS_ZERO(f_nhop->gateway)) {
        ret = ntb_neigh_del(vrf_dp, f_nhop->gateway);
        RTE_ASSERT(0 == ret);
    }

    /* call hal func */
    // ret = _ntb_hal_nhop_del(f_nhop->nh_oid);
    // RTE_ASSERT(0 == ret);

    /* free oid */
    //ret = hal_tunnel_nexthop_free_oid(f_nhop->nh_oid); //------------修改成自己table中的函数
    RTE_ASSERT(0 == ret);

    rte_free(f_nhop);
    return 0;
}

static int
fib_res_id_free(struct rte_fbarray *fib_res_array, int f_res_id)
{
    return rte_fbarray_set_free(fib_res_array, f_res_id);
}

static int 
fib_res_obj_delete(vrf_dp_t *vrf_dp, int64_t fib_res_id)
{
    int ret;
    struct fib_res *f_res;

    f_res = fib_res_get(vrf_dp->fib_res_array, fib_res_id);
    RTE_ASSERT(f_res != NULL);

    if (f_res->mpath_cnt == 1) {
        /* only one nexthop, just return */
        fib_res_id_free(vrf_dp->fib_res_array, fib_res_id);
        return 0;
    }

    /* mroute already config mgroup in init, no need delete */
    if (fib_res_rtype_get(f_res) == RT_DEFAULT) {
        /* call hal func */
        // ret = _ntb_hal_ecmp_group_del(f_res);
        // RTE_ASSERT(ret == 0);
        /* free oid */
        // ret = hal_ecmp_free_oid(f_res->f_res_oid);
        // RTE_ASSERT(ret == 0);
    }

    fib_res_id_free(vrf_dp->fib_res_array, fib_res_id);
    return 0;
}

//增加路由 由dst vni得到remoteip gateway
int
ntb_route_nhop_insert(vrf_dp_t *vrf_dp, 
                const struct ntb_rib_key *pfx, 
                const struct fib_nhop_encap_data *nhop_encap_data,
                rib_type_t rib_type) //add a new route info
{
    struct fib_res *f_res = NULL;
    int64_t f_res_id = -1, f_res_id_new;
    int ret = 0, i;
    struct fib_nhop *nhop_new = NULL;
    struct fib_nhop *f_nhops[MAX_NHOPS_PER_RTOUE];
    int fib_res_rpath_num = 0;
    int prefix_is_exits = 0;

    /* sanity check */
    ret = rte_hash_lookup_data(vrf_dp->rib, pfx, (void**)&f_res_id);//if there is a same key in rib
    if (ret >= 0) {//exist
        f_res = fib_res_get(vrf_dp->fib_res_array, f_res_id);// get the result of fib_res_array
        if (fib_res_rtype_get(f_res) != rib_type) { //ignore
            return NTB_IPV4_ROUTETYPE_NO_SAME;
        }
        if (fib_res_mpath_nospace(f_res)) {//
            return NTB_ROUTE_NEXTHOP_ECMP_NOSPACE;
        }
        for (i = 0; i < f_res->mpath_cnt; i++) {
            if (fib_nhop_equal(f_res->mpath[i], nhop_encap_data)) {
                return 0;
            }
        }
        prefix_is_exits = 1; //there is a same key
    }

    /* create new nhop object */
    ret = nhop_obj_create(vrf_dp, nhop_encap_data, &nhop_new); //ol_neigh一并创建
    if (ret < 0) {
        return ret;
    }

    /* create new fib_res object */
    if (!prefix_is_exits) {
        fib_res_rpath_num = 1;
        f_nhops[0] = nhop_new; 
    } else {
        fib_res_rpath_num = f_res->mpath_cnt + 1;
        for (i = 0; i < f_res->mpath_cnt; i++) {
            f_nhops[i] = f_res->mpath[i];
        }
        f_nhops[i] = nhop_new;
    }
    f_res_id_new = fib_res_obj_create(vrf_dp, f_nhops, fib_res_rpath_num, f_res, rib_type);
    if (f_res_id_new < 0) {
        ret = f_res_id_new; 
        goto rollback2;
    }

    /* update route prefix */
    ret = rt_prefix_obj_update(vrf_dp, pfx, f_res_id, f_res_id_new);//更新rib、fib
    if (ret < 0) {
        goto rollback1;
    }

    /* delete old fib_res */
    if (prefix_is_exits) {
        ret = fib_res_obj_delete(vrf_dp, f_res_id);
        RTE_ASSERT(ret == 0);
    }
    return ret;

rollback1:
    fib_res_obj_delete(vrf_dp, f_res_id_new);
rollback2:
    nhop_obj_delete(vrf_dp, nhop_new);
    return ret;
}


static int
rt_prefix_obj_delete(vrf_dp_t *vrf_dp, 
                    const struct ntb_rib_key *pfx, 
                    int64_t f_res_id) 
{
    int ret = 0;
    prefix_type_t rt_ptype;
    struct fib_res *f_res;

    rt_ptype = rt_prefix_type_get(vrf_dp, pfx, f_res_id);

    ret = rte_fib_delete(vrf_dp->fib,
                rte_be_to_cpu_32(pfx->prefix4), pfx->prefixlen);
    RTE_ASSERT(0 == ret);
    ret = rte_hash_del_key(vrf_dp->rib, pfx);
    RTE_ASSERT(0 <= ret);
    vrf_dp->rib_cnt--; 
    RTE_ASSERT(vrf_dp->rib_cnt >= 0);

    f_res = fib_res_get(vrf_dp->fib_res_array, f_res_id);
    RTE_ASSERT(f_res != NULL);

    // ret = _ntb_hal_rt_prefix_delete(pfx, rt_ptype, f_res);
    // RTE_ASSERT(ret == 0);

    return ret;
}


//删除路由--根据详细路由删
//表中存在的路由都有对应的完整表项，没有无对应信息的路由
int
ntb_route_nhop_delete(vrf_dp_t *vrf_dp,
                const struct ntb_rib_key *pfx, 
                const struct fib_nhop_encap_data *nhop_encap_data,
                rib_type_t rib_type)
{
    struct ntb_nhop_hash_key nhop_key;
    int i, rpath_index, ret = 0;
    struct fib_res *f_res = NULL;
    int64_t f_res_id, f_res_id_new;
    struct fib_nhop *f_nhop;
    struct fib_nhop *f_nhops[MAX_NHOPS_PER_RTOUE];
    int fib_res_rpath_num = 0;

    /* Lookup fib result by prefix.*/
    if (rte_hash_lookup_data(vrf_dp->rib, pfx, (void**)&f_res_id) < 0)
        return 0; /* no exist, just return ok */

    f_res = fib_res_get(vrf_dp->fib_res_array, f_res_id); 
    RTE_ASSERT(f_res != NULL); //若无封装信息，则报错

    if (fib_res_rtype_get(f_res) != rib_type) {
        return NTB_IPV4_ROUTETYPE_NO_SAME;
    }

    nhop_key.gateway = nhop_encap_data->gateway;
    nhop_key.tunnel_info = nhop_encap_data->tunnel_info;
    if (rte_hash_lookup_data(g_nhop_hash_tbl, &nhop_key, (void**)&f_nhop) < 0)
        return 0; /* no exist, just return ok */ //若无相应的封装表项，则无需删除。直接返回
    for (i = 0; i < f_res->mpath_cnt; i++)
        if (f_res->mpath[i] == f_nhop) //f_res中是否有与f_nhop相同的结果（指针指向同一地址）
            break;
    if (i >= f_res->mpath_cnt)
        return 0;/* no exist, just return ok */

    /* nhop_num == 1, delete rotue prefix */ 
    if (f_res->mpath_cnt == 1) {
        ret = rt_prefix_obj_delete(vrf_dp, pfx, f_res_id);
        RTE_ASSERT(ret == 0);
        ret = fib_res_obj_delete(vrf_dp, f_res_id); 
        RTE_ASSERT(ret == 0);
        ret = nhop_obj_delete(vrf_dp, f_nhop);
        RTE_ASSERT(ret == 0);
        return ret;
    } 

    /* create new fib_res */ //先加后删
    fib_res_rpath_num = f_res->mpath_cnt - 1;
    rpath_index = 0;
    for (i = 0; i < f_res->mpath_cnt; i++) {  //for ecmp
        if (f_res->mpath[i] == f_nhop)
            continue;
        f_nhops[rpath_index++] = f_res->mpath[i];
    }
    f_res_id_new = fib_res_obj_create(vrf_dp, f_nhops, fib_res_rpath_num, f_res, rib_type);
    if (f_res_id_new < 0) {
        ret = f_res_id_new;
        return ret;
    }

    /* update route prefix */
    ret = rt_prefix_obj_update(vrf_dp, pfx, f_res_id, f_res_id_new);
    if (ret < 0) {
        goto rollback;
    }

    /* delete old fib_res and nhop */
    ret = fib_res_obj_delete(vrf_dp, f_res_id);
    RTE_ASSERT(ret == 0);
    ret = nhop_obj_delete(vrf_dp, f_nhop);
    RTE_ASSERT(ret == 0);
    return ret;

rollback:
    fib_res_obj_delete(vrf_dp, f_res_id_new);
    return ret;
}

//删除路由--根据key删
int
ntb_route_delete(vrf_dp_t *vrf_dp,
                const struct ntb_rib_key *pfx)
{
    struct fib_res *f_res;
    int64_t f_res_id;
    int i, ret = -1;
    int nhop_num;
    struct fib_nhop *f_nhops[MAX_NHOPS_PER_RTOUE];

    /* Lookup fib result by prefix.*/
    if (rte_hash_lookup_data(vrf_dp->rib, pfx, (void**)&f_res_id) < 0)
        return NTB_ROUTE_PREFIX_NOT_FOUND;

    f_res = fib_res_get(vrf_dp->fib_res_array, f_res_id);
    RTE_ASSERT(f_res != NULL);

    /* record nhops */
    nhop_num = f_res->mpath_cnt;
    for (i = 0; i < f_res->mpath_cnt; i++) {
        f_nhops[i] = f_res->mpath[i];
    }
    /* delete route prefix */
    ret = rt_prefix_obj_delete(vrf_dp, pfx, f_res_id);
    RTE_ASSERT(ret == 0);

    /* delete fib_res object */
    ret = fib_res_obj_delete(vrf_dp, f_res_id);
    RTE_ASSERT(ret == 0);

    /* Lookup route nexthop to delete.*/
    for (i = 0; i < nhop_num; i++) {
        ret = nhop_obj_delete(vrf_dp, f_nhops[i]);
        RTE_ASSERT(ret == 0);
    }
    return 0;
}

//根据dst查找路由，为pmd提供查找功能
int
ntb_route_lookup(const vrf_dp_t *vrf_dp,
                uint32_t dip, struct fib_res **f_res)
{    
    uint64_t next_hop;

    dip = rte_be_to_cpu_32(dip);
    rte_fib_lookup_bulk(vrf_dp->fib, &dip, &next_hop, 1);
    *f_res = fib_res_get(vrf_dp->fib_res_array, next_hop);
    
    return 0;
}

//ecmp hash选路
int
fib_res_select_mpath(struct fib_res *f_res, int hash)
{
    int nhsel = 0;

    if (f_res->mpath_cnt > 0)
        nhsel = hash % f_res->mpath_cnt;

    return nhsel;
}

//清理路由 所有表信息rib、fib、f_res、nhop、neigh
int
ntb_route_cleanup(vrf_dp_t *vrf_dp)
{
    const struct ntb_rib_key *rib_key;
    uint32_t iter = 0;
    int64_t f_res_id;

    while (rte_hash_iterate(vrf_dp->rib, (const void **)&rib_key,
                (void **)&f_res_id, &iter) >= 0)
        if (vrf_dp->vrf_id == rib_key->vrf_id)
            ntb_route_delete(vrf_dp, rib_key);

    return 0;
}

//nhop表初始化
void
ntb_nhop_init()
{
    struct rte_hash_parameters nhop_tbl_params = {
        .name = RTE_STR(NEXTHOP_HASH_TBL),
        .entries = NTB_NR_NHOP_ALL,
        .hash_func = rte_jhash,
        .key_len = sizeof(struct ntb_nhop_hash_key),
    };

    g_nhop_hash_tbl = rte_hash_create(&nhop_tbl_params);
    RTE_ASSERT(!!g_nhop_hash_tbl);
    rte_hash_reset(g_nhop_hash_tbl);    
}

int
ntb_vrf_route_init(vrf_dp_t *vrf_dp, int f_res_bh)
{
    char name[64];
    static struct rte_hash *ntb_rib_hash_tbl = NULL;

    struct rte_fib_conf fib_params = {
        .max_routes = NTB_NR_ROUTES_ALL,
        .type = RTE_FIB_DUMMY,
        .default_nh = f_res_bh,
        .shr_fib_pool_name = "SHR_FIB_NPOOL",
    };

    if (ntb_rib_hash_tbl == NULL) {
        struct rte_hash_parameters rib_params = {
            .name = RTE_STR(NTB_RIB_HASH_TBL),
            .entries = NTB_NR_ROUTES_ALL * 2,
            .hash_func = rte_jhash,
            .key_len = sizeof(struct ntb_rib_key),
        };
        ntb_rib_hash_tbl = rte_hash_create(&rib_params);
        RTE_ASSERT(!!ntb_rib_hash_tbl);
        rte_hash_reset(ntb_rib_hash_tbl);    
    }

    vrf_dp->rib = ntb_rib_hash_tbl;
    RTE_ASSERT(!!vrf_dp->rib);

    name[0] = '\0';
    snprintf(name, sizeof(name), "fib_%d", vrf_dp->vrf_id);
    vrf_dp->fib = rte_fib_create(name, SOCKET_ID_ANY, &fib_params);
    RTE_ASSERT(!!vrf_dp->fib);

    return 0;
}


//according to flag
static int
nhop_add_to_hardware(){
    return 0;
}

static int
route_add_to_hardware(){
    return 0;
}

