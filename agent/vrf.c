#include <sys/queue.h>

#include <rte_common.h>
#include <rte_byteorder.h>
#include <rte_log.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_eal.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_malloc.h>
#include <rte_ip.h>
#include <rte_rib.h>
#include <rte_arp.h>
#include <rte_udp.h>
#include <rte_vxlan.h>
#include "rte_hash.h"
#include "rte_hash_crc.h"
#include "rte_jhash.h"

#include <rte_ip.h>
#include <rte_log.h>
#include <rte_fib.h>


#include "vrf.h"
#include "route.h"
#include "tunnel_map.h"
#include "shc_errno.h"
// #include "shc_spec.h"
// #include <hal_table_type.h>
// #include <hal_table_api.h>

static shc_vrf_t g_shc_vrfs[SHC_NR_VRF];
static int g_shc_vrf_n = 0;
static struct rte_fbarray g_fib_res_array;
static int f_res_blackhole = -1;
// static uint8_t g_tgre_version = 0;

// #define VRF_NOT_EXIST "VRF does not exist."
// #define VRF_NOENT "VRF was used up."
// #define COMM_INVAL_PARPAM "invalid parameter."
shc_vrf_t *
shc_vrf_get_by_id(uint32_t vrfid)
{
    int index = vrfid - 1;
    if (index < SHC_NR_VRF && g_shc_vrfs[index].valid)
        return &g_shc_vrfs[index];

    return NULL;
}

vrf_dp_t *
shc_vrf_dp_get_by_id(uint32_t vrfid)
{
    shc_vrf_t *vrf = shc_vrf_get_by_id(vrfid);
    return !!vrf ? &vrf->dp : NULL;
}

static int
shc_vrf_dp_ctor(vrf_dp_t *vrf_dp)
{
    SLIST_INIT(&vrf_dp->tunnel_rmap);
    vrf_dp->tunnel_cnt = 0;
    // vrf_dp->eg_tunnel_cnt = 0;
    vrf_dp->neigh_cnt = 0;
    // vrf_dp->tgre_version = 0;
    return 0;
}


int
shc_vrf_add(const char *vrf_name)
{
    // int i, ret;
    int i;
    int free_ent = -1;
    shc_vrf_t *vrf;

    for (i = 0; i < RTE_DIM(g_shc_vrfs); i++) {
        if (g_shc_vrfs[i].valid) {
            /* Checks if the VRF has been created already.*/
            if (!strcmp(g_shc_vrfs[i].name, vrf_name)) //若有相同的，则无法插入
                return 0;
        } else {
            if (-1 == free_ent) //找到第一个空位，插入
                free_ent = i;
        }
    }

    if (-1 == free_ent)
        return SHC_VRF_NOENT;
        // return VRF_NOENT;

    vrf = &g_shc_vrfs[free_ent];

    shc_vrf_dp_ctor(&vrf->dp);

    rte_ether_addr_copy(shc_fake_mac_get(), &vrf->dp.route_mac);
    /* notify all overlay neigh according to vrf*/
    // shc_vrf_neigh_update(vrf->dp.vrf_id);

    strncpy(vrf->name, vrf_name, sizeof(vrf->name));
    vrf->valid = 1;
    g_shc_vrf_n++;
    return 0;
}

static int
shc_vrf_dp_dtor(vrf_dp_t *vrf_dp)
{
    shc_route_cleanup(vrf_dp);
    shc_tunnel_map_cleanup(vrf_dp);
    // shc_arp_proxy_cleanup(&vrf_dp->arp_proxys);
    vrf_dp->internal_ip = 0;
    return 0;
}


int
shc_vrf_del(const char *vrf_name)
{
    // int i, ret;
    int i;
    for (i = 0; i < RTE_DIM(g_shc_vrfs); i++)
        if (g_shc_vrfs[i].valid
            && !strcmp(g_shc_vrfs[i].name, vrf_name))
            break;

    if (i >= RTE_DIM(g_shc_vrfs))
        return 0;

    shc_vrf_dp_dtor(&g_shc_vrfs[i].dp);
    g_shc_vrfs[i].name[0] = '\0';
    g_shc_vrfs[i].valid = 0;
    RTE_ASSERT(g_shc_vrf_n > 0);
    g_shc_vrf_n--;
    return 0;
}

int
shc_vrf_count(void)
{
    return g_shc_vrf_n;
}

shc_vrf_t *
shc_vrf_lookup(const char *vrf_name)
{
    int i;

    for (i = 0; i < RTE_DIM(g_shc_vrfs); i++)
        if (g_shc_vrfs[i].valid
            && !strcmp(g_shc_vrfs[i].name, vrf_name))
            break;

    if (i >= RTE_DIM(g_shc_vrfs))
        return NULL; /* Not found.*/

    return &g_shc_vrfs[i];
}

int32_t
shc_vrf_id_lookup(const char *vrf_name)
{
    shc_vrf_t * vrf = shc_vrf_lookup(vrf_name);
    if (NULL == vrf)
        return SHC_VRF_NOT_EXIST;
        // return VRF_NOT_EXIST;

    return vrf->dp.vrf_id;
}

int
shc_vrf_intip_set(const char *vrf_name, uint32_t intip)
{
    shc_vrf_t *vrf = shc_vrf_lookup(vrf_name);
    if (NULL == vrf)
        return SHC_VRF_NOT_EXIST;
        // return VRF_NOT_EXIST;

    vrf->dp.internal_ip = intip;
    return 0;
}

int
shc_vrf_intip_unset(const char *vrf_name)
{
    return shc_vrf_intip_set(vrf_name, 0);
}

int
shc_vrf_intip_get(const char *vrf_name, uint32_t *vrf_ip)
{
    shc_vrf_t *vrf = shc_vrf_lookup(vrf_name);
    if (NULL == vrf)
        return SHC_VRF_NOT_EXIST;
        // return VRF_NOT_EXIST;

    *vrf_ip = vrf->dp.internal_ip;
    return 0;
}



int
shc_vrf_rmac_set(const char *vrf_name, const struct rte_ether_addr *route_mac)
{
    shc_vrf_t *vrf = shc_vrf_lookup(vrf_name);
    if (NULL == vrf)
        return SHC_VRF_NOT_EXIST;
        // return VRF_NOT_EXIST;
    
    /* load default fake mac to P4 */
    // int ret = _shc_hal_inner_smac_update(vrf->dp.vrf_id, route_mac, false);
    // if (ret < 0) 
    //     return ret;

    /* update local route mac */
    rte_ether_addr_copy(route_mac, &vrf->dp.route_mac);

    /* notify all overlay neigh */
    // shc_vrf_neigh_update(vrf->dp.vrf_id);

    return 0;
}

int
shc_vrf_rmac_unset(const char *vrf_name, const struct rte_ether_addr *route_mac)
{
    shc_vrf_t *vrf = shc_vrf_lookup(vrf_name);
    if (NULL == vrf)
        return SHC_VRF_NOT_EXIST;
        // return VRF_NOT_EXIST;

    /* check the route_mac to be deleted */
    if (!rte_is_same_ether_addr(route_mac, &vrf->dp.route_mac))
        return SHC_COMM_INVAL_PARAM;
        // return COMM_INVAL_PARPAM;
    
    /* load default fake mac to P4 */
    // int ret = _shc_hal_inner_smac_update(vrf->dp.vrf_id, shc_fake_mac_get(), false);
    // if (ret < 0) 
    //     return ret;

    /* update local route mac */
    rte_ether_addr_copy(shc_fake_mac_get(), &vrf->dp.route_mac);

    /* notify all overlay neigh */
    // shc_vrf_neigh_update(vrf->dp.vrf_id);

    return 0;
}

int
shc_vrf_rmac_get(const char *vrf_name, struct rte_ether_addr *route_mac)
{
    shc_vrf_t *vrf = shc_vrf_lookup(vrf_name);
    if (NULL == vrf)
        return SHC_VRF_NOT_EXIST;
        // return VRF_NOT_EXIST;

    rte_ether_addr_copy(&vrf->dp.route_mac, route_mac);
    return 0;
}

int shc_vrf_init(void)
{
    int i;    
    int ret;

    ret = rte_fbarray_init(&g_fib_res_array, "fib_res_shared",
                SHC_NR_FIB_RES_ALL + 1, /* Reserve 1 for blackhole route.*/
                sizeof(struct fib_res));
    RTE_ASSERT(ret == 0);

    /* Pre-allocate fib result for blackhole route.*/
    f_res_blackhole = rte_fbarray_find_next_free(&g_fib_res_array, 0);
    RTE_ASSERT(f_res_blackhole >= 0);
    ret = rte_fbarray_set_used(&g_fib_res_array, f_res_blackhole);
    RTE_ASSERT(0 == ret);
    fib_res_set_flags(rte_fbarray_get(&g_fib_res_array, f_res_blackhole),
                FRF_REJECT);

    for (i = 0; i < RTE_DIM(g_shc_vrfs); i++) {
        shc_vrf_t *vrf = &g_shc_vrfs[i];
        memset(vrf, 0, sizeof(*vrf));

        vrf->dp.vrf_id = i + 1;
        vrf->dp.fib_res_array = &g_fib_res_array;

        ret = shc_vrf_route_init(&vrf->dp, f_res_blackhole);
        RTE_ASSERT(ret == 0);
    }
    shc_nhop_init();

    return 0;
}

int shc_vrf_deinit(void)
{
    return 0;
}
