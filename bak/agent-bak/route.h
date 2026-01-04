#ifndef NTB_ROUTE_H
#define NTB_ROUTE_H

#include "ntb_dp.h"

enum ntb_encap_type {
    NTB_ENCAP_INVALID = 0,
    NTB_ENCAP_VXLAN,
    NTB_ENCAP_TGRE,
    NTB_ENCAP_MAX
};

#define NTB_ENCAP_TYPE_VALID(proto) \
    ((proto) > NTB_ENCAP_INVALID && (proto) < NTB_ENCAP_MAX)

typedef enum {
    RT_DEFAULT = 0,
    RT_MROUTE,
    RT_MAX,
} rib_type_t;

typedef enum {
    RT_PREFIX_INVALID = 0,
    RT_PREFIX_LPM,
    RT_PREFIX_HOST,
    RT_PREFIX_MAX,
} prefix_type_t;

#define NTB_ROUTE_TYPE_VALID(rtype) \
    ((rtype) >= RT_DEFAULT && (rtype) < RT_MAX)

struct ntb_rib_key {
    int32_t vrf_id;
    uint8_t family;
    uint8_t prefixlen;
    union {
        /* IP prefix in Big Endian(Network Byte order)*/
        uint32_t prefix4;
    };
} __rte_packed;

struct ip_tunnel_info {
    uint8_t family;
    enum ntb_encap_type type;
    union {
        struct {
            /* vxlan tunnel encap info.*/
            uint32_t remote;
            uint32_t local;
            uint32_t vni:24;
            uint32_t dscp:8;
        } vxlan;
        struct {
            uint32_t remote;
            uint32_t vpcid;
            uint32_t vmip;
        } tgre;
    };
}__rte_packed;

struct fib_nhop_encap_data {
    uint32_t gateway;
    struct ip_tunnel_info tunnel_info;
};
struct fib_nhop {
    uint32_t nh_oid;
    uint32_t ref_count;
    uint32_t neigh_oid;
    uint32_t gateway;
    struct ip_tunnel_info tunnel_info;

    /* Route nexthop attribues.*/
    int marker[0];
} __rte_packed;


struct fib_res {
    uint32_t f_res_oid;
    uint32_t fr_flags;
    uint32_t mpath_cnt;
    uint32_t mpath_max;
    struct fib_nhop *mpath[ROUTE_MAX_ECMP];
};

// typedef int 
// (*rt_prefix_entry_cb_t)(struct ntb_rib_key *key,
//                 int64_t f_res_id,
//                 int is_mroute,
//                 int is_nottl,
//                 uint32_t mpath_cnt,
//                 void *arg1, void *arg2, void *arg3);

// typedef int 
// (*rt_nhop_entry_cb_t)(struct fib_nhop *f_nhop,
//                 void *arg1, void *arg2);

/* List of fib result flags.*/
#define FRF_REJECT (1ul << 0)
#define FRF_MROUTE (1ul << 1)
#define FRF_NOTTL  (1ul << 2)

extern int
ntb_vrf_route_init(vrf_dp_t *vrf_dp, int f_res_bh);

extern void ntb_nhop_init();

extern int
ntb_route_nhop_insert(vrf_dp_t *vrf_dp,
                const struct ntb_rib_key *pfx, 
                const struct fib_nhop_encap_data *nhop_encap_data,
                rib_type_t rib_type);

extern int
ntb_route_nhop_delete(vrf_dp_t *vrf_dp,
                const struct ntb_rib_key *pfx, 
                const struct fib_nhop_encap_data *nhop,
                rib_type_t rib_type);
// extern int //暂不支持路由的类型修改 默认路由还是多播路由
// ntb_route_type_trans(vrf_dp_t *vrf_dp,
//                 const struct ntb_rib_key *pfx, 
//                 rib_type_t rib_type);

extern int
ntb_route_delete(vrf_dp_t *vrf_dp,
                const struct ntb_rib_key *pfx);

extern int
ntb_route_cleanup(vrf_dp_t *vrf_dp);

extern int
ntb_route_lookup(const vrf_dp_t *vrf_dp,
                uint32_t dip, struct fib_res **f_res);

// extern int
// fib_res_feat_mod(const vrf_dp_t *vrf_dp,
//                 const struct ntb_rib_key *pfx,
//                 uint32_t flags, int op_is_set);

// extern int
// ntb_route_prefix_walk(vrf_dp_t *vrf_dp,
//                         rt_prefix_entry_cb_t cb, void *arg1, void *arg2, void *arg3);

// int
// ntb_route_nhop_walk(vrf_dp_t *vrf_dp, int64_t f_res_id, 
//                         rt_nhop_entry_cb_t cb, void *arg1, void *arg2);
extern int
fib_res_select_mpath(struct fib_res *f_res, int hash);

/* Utility functions.*/
static inline int
fib_res_is_reject(const struct fib_res *f_res)
{
    return !!(f_res->fr_flags & FRF_REJECT);
}

static inline int
fib_res_is_mroute(const struct fib_res *f_res)
{
    return !!(f_res->fr_flags & FRF_MROUTE);
}

static inline int
fib_res_is_nottl(const struct fib_res *f_res)
{
    return !!(f_res->fr_flags & FRF_NOTTL);
}

static inline void
fib_res_set_flags(struct fib_res *f_res, uint32_t flags)
{
    f_res->fr_flags |= flags;
}

static inline void
fib_res_unset_flags(struct fib_res *f_res, uint32_t flags)
{
    f_res->fr_flags &= ~flags;
}

static inline void
fib_res_clone_flags(struct fib_res *f_res_d, const struct fib_res *f_res_s)
{
    f_res_d->fr_flags = f_res_s->fr_flags;
}

#endif
