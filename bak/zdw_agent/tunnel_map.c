#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/queue.h>

#include <rte_common.h>
#include <rte_byteorder.h>
#include <rte_log.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_eal.h>

#include <rte_branch_prediction.h>
#include <rte_debug.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_malloc.h>
#include "rte_hash.h"
#include "rte_hash_crc.h"
#include "rte_jhash.h"
#include <rte_rwlock.h>

#include "ntb_errno.h"
#include "tunnel_map.h"
// #include "ntb_spec.h"
// #include <hal_table_type.h>
// #include <hal_table_api.h>

static struct rte_hash *vrf_hash_tbl;

struct tunnel_map_key {
    ntb_tunnel_type_t type;
    ntb_tunnel_id_t id;
} __rte_packed;

// create a map table
int
ntb_tunnel_map_vrf_init(void)
{
    struct rte_hash_parameters params1 = {
        .name = RTE_STR(VRF_MAPPING_HASH),
        .entries = NTB_NR_TUNNEL_ALL * 8,
        .hash_func = rte_jhash,
        .key_len = sizeof(struct tunnel_map_key),
    };

    vrf_hash_tbl = rte_hash_create(&params1);
    RTE_ASSERT(!!vrf_hash_tbl);
    rte_hash_reset(vrf_hash_tbl);

    return 0;
}

int
ntb_tunnel_map_vrf_deinit(void)
{
    return 0;
}



// add an entry
int
ntb_tunnel_map_vrf(const char *vrf_name, ntb_tunnel_type_t type,
                ntb_tunnel_id_t id, ntb_tunnel_attr_t tunnel_attr)
{
    struct tunnel_map_key map_key;
    struct ntb_tunnel_info *tunnel_i = NULL;
    ntb_vrf_t *vrf;
    int ret;

    vrf = ntb_vrf_lookup(vrf_name);
    if (NULL == vrf)
        return NTB_VRF_NOT_EXIST;

    /* If the tunnel has been created already.*/
    map_key.type = type;
    map_key.id = id;
    ret = rte_hash_lookup_data(vrf_hash_tbl, &map_key, (void**)&tunnel_i);
    if (ret >= 0) {
        RTE_ASSERT(tunnel_i != NULL);

        /* If the tunnel is mapped to other VRF already?.*/
        if (tunnel_i->vrf_id != vrf->dp.vrf_id)
            return NTB_TUNNEL_INUSED;

        return 0;
    }

    tunnel_i = rte_zmalloc(__func__, sizeof(*tunnel_i), 0);
    if (NULL == tunnel_i)
        return NTB_COMM_NOMEM;

    tunnel_i->vrf_id = vrf->dp.vrf_id;

    if (rte_hash_add_key_data(vrf_hash_tbl, &map_key, tunnel_i) < 0) {
        rte_free(tunnel_i);
        return NTB_TUNNEL_MAP_NOSPACE;
    }

   
    SLIST_INSERT_HEAD(&vrf->dp.tunnel_rmap, tunnel_i, next);
    vrf->dp.tunnel_cnt++;
    return 0;
}

/*
static inline int
__tunnel_unmap(ntb_tunnel_type_t type, ntb_tunnel_id_t id)
{
    struct tunnel_map_key map_key = {
        .type = type,
        .id = id,
    };

    if (rte_hash_del_key(vrf_hash_tbl, &map_key) < 0)
        return -1;  ENOENT 

    return 0;
}*/

// delete an entry
int
ntb_tunnel_unmap_vrf(ntb_tunnel_type_t type, ntb_tunnel_id_t id)
{
    struct ntb_tunnel_info *tunnel_i;
    vrf_dp_t *vrf_dp = NULL;
    int ret;

    struct tunnel_map_key map_key = {
        .type = type,
        .id = id,
    };
    ret = rte_hash_lookup_data(vrf_hash_tbl, &map_key, (void**)&tunnel_i);
    if (ret < 0) {
        return 0;  /* no exist, just return ok */
    }

    vrf_dp = ntb_vrf_dp_get_by_id(tunnel_i->vrf_id);

    /* Tunnel unmapping.*/
    // ret = __tunnel_unmap(type, id);
    if (ret < 0) {
        /* entry not exits, just return ok */
        return 0;
    }

    /* Release the corresponding tunnel info.*/
    RTE_ASSERT(tunnel_i != NULL);
    SLIST_REMOVE(&vrf_dp->tunnel_rmap, tunnel_i, ntb_tunnel_info, next);
    rte_free(tunnel_i);
    vrf_dp->tunnel_cnt--;
    return 0;
}

// for pmd lookup
vrf_dp_t *
ntb_tunnel_map_vrf_lookup(ntb_tunnel_type_t type,
                ntb_tunnel_id_t id)
{
    int ret;
    vrf_dp_t *vrf_dp = NULL;
    struct ntb_tunnel_info *tunnel_i = NULL;
    struct tunnel_map_key map_key = {
        .type = type,
        .id = id,
    };

    ret = rte_hash_lookup_data(vrf_hash_tbl, &map_key, (void**)&tunnel_i);
    if (ret >= 0) {
        RTE_ASSERT(tunnel_i != NULL);

        vrf_dp = ntb_vrf_dp_get_by_id(tunnel_i->vrf_id);
        RTE_ASSERT(vrf_dp != NULL);
    }

    return vrf_dp;
}

int
ntb_tunnel_map_cleanup(vrf_dp_t *vrf_dp)
{
    struct ntb_tunnel_info *tunnel_i;

    while (!SLIST_EMPTY(&vrf_dp->tunnel_rmap)) {
        tunnel_i = SLIST_FIRST(&vrf_dp->tunnel_rmap);
        SLIST_REMOVE_HEAD(&vrf_dp->tunnel_rmap, next);
        //__tunnel_unmap(tunnel_i->type, tunnel_i->id);
        // ntb_tunnel_bundle_cleanup(tunnel_i); 
        // _ntb_hal_tunnel_unmap_vrf(tunnel_i->type, tunnel_i->id);
        // _ntb_hal_ig_tunnel_statistics_del(vrf_dp, tunnel_i->type, tunnel_i->id);
        rte_free(tunnel_i);
        vrf_dp->tunnel_cnt--;
    }
    return 0;
}
