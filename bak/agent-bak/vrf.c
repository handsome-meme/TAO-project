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
#include "ntb_spec.h"
#include <hal_table_type.h>
#include <hal_table_api.h>

static ntb_vrf_t g_ntb_vrfs[NTB_NR_VRF];
static int g_ntb_vrf_n = 0;
static struct rte_fbarray g_fib_res_array;
static int f_res_blackhole = -1;
static uint8_t g_tgre_version = 0;

ntb_vrf_t *
ntb_vrf_get_by_id(uint32_t vrfid)
{
    int index = vrfid - 1;
    if (index < NTB_NR_VRF && g_ntb_vrfs[index].valid)
        return &g_ntb_vrfs[index];

    return NULL;
}

vrf_dp_t *
ntb_vrf_dp_get_by_id(uint32_t vrfid)
{
    ntb_vrf_t *vrf = ntb_vrf_get_by_id(vrfid);
    return !!vrf ? &vrf->dp : NULL;
}

static int
ntb_vrf_dp_ctor(vrf_dp_t *vrf_dp)
{
    SLIST_INIT(&vrf_dp->tunnel_rmap);
    vrf_dp->tunnel_cnt = 0;
    // vrf_dp->eg_tunnel_cnt = 0;
    vrf_dp->neigh_cnt = 0;
    // vrf_dp->tgre_version = 0;
    return 0;
}


int
ntb_vrf_add(const char *vrf_name)
{
    int i, ret;
    int free_ent = -1;
    ntb_vrf_t *vrf;

    for (i = 0; i < RTE_DIM(g_ntb_vrfs); i++) {
        if (g_ntb_vrfs[i].valid) {
            /* Checks if the VRF has been created already.*/
            if (!strcmp(g_ntb_vrfs[i].name, vrf_name)) //若有相同的，则无法插入
                return 0;
        } else {
            if (-1 == free_ent) //找到第一个空位，插入
                free_ent = i;
        }
    }

    if (-1 == free_ent)
        return NTB_VRF_NOENT;

    vrf = &g_ntb_vrfs[free_ent];

    ntb_vrf_dp_ctor(&vrf->dp);

    rte_ether_addr_copy(ntb_fake_mac_get(), &vrf->dp.route_mac);
    /* notify all overlay neigh according to vrf*/
    // ntb_vrf_neigh_update(vrf->dp.vrf_id);

    strncpy(vrf->name, vrf_name, sizeof(vrf->name));
    vrf->valid = 1;
    g_ntb_vrf_n++;
    return 0;
}

static int
ntb_vrf_dp_dtor(vrf_dp_t *vrf_dp)
{
    ntb_route_cleanup(vrf_dp);
    ntb_tunnel_map_cleanup(vrf_dp);
    // ntb_arp_proxy_cleanup(&vrf_dp->arp_proxys);
    vrf_dp->internal_ip = 0;
    return 0;
}


int
ntb_vrf_del(const char *vrf_name)
{
    int i, ret;

    for (i = 0; i < RTE_DIM(g_ntb_vrfs); i++)
        if (g_ntb_vrfs[i].valid
            && !strcmp(g_ntb_vrfs[i].name, vrf_name))
            break;

    if (i >= RTE_DIM(g_ntb_vrfs))
        return 0;

    ntb_vrf_dp_dtor(&g_ntb_vrfs[i].dp);
    g_ntb_vrfs[i].name[0] = '\0';
    g_ntb_vrfs[i].valid = 0;
    RTE_ASSERT(g_ntb_vrf_n > 0);
    g_ntb_vrf_n--;
    return 0;
}

int
ntb_vrf_count(void)
{
    return g_ntb_vrf_n;
}

ntb_vrf_t *
ntb_vrf_lookup(const char *vrf_name)
{
    int i;

    for (i = 0; i < RTE_DIM(g_ntb_vrfs); i++)
        if (g_ntb_vrfs[i].valid
            && !strcmp(g_ntb_vrfs[i].name, vrf_name))
            break;

    if (i >= RTE_DIM(g_ntb_vrfs))
        return NULL; /* Not found.*/

    return &g_ntb_vrfs[i];
}

int32_t
ntb_vrf_id_lookup(const char *vrf_name)
{
    ntb_vrf_t * vrf = ntb_vrf_lookup(vrf_name);
    if (NULL == vrf)
        return NTB_VRF_NOT_EXIST;

    return vrf->dp.vrf_id;
}

int
ntb_vrf_intip_set(const char *vrf_name, uint32_t intip)
{
    ntb_vrf_t *vrf = ntb_vrf_lookup(vrf_name);
    if (NULL == vrf)
        return NTB_VRF_NOT_EXIST;

    vrf->dp.internal_ip = intip;
    return 0;
}

int
ntb_vrf_intip_unset(const char *vrf_name)
{
    return ntb_vrf_intip_set(vrf_name, 0);
}

int
ntb_vrf_intip_get(const char *vrf_name, uint32_t *vrf_ip)
{
    ntb_vrf_t *vrf = ntb_vrf_lookup(vrf_name);
    if (NULL == vrf)
        return NTB_VRF_NOT_EXIST;

    *vrf_ip = vrf->dp.internal_ip;
    return 0;
}



int
ntb_vrf_rmac_set(const char *vrf_name, const struct rte_ether_addr *route_mac)
{
    ntb_vrf_t *vrf = ntb_vrf_lookup(vrf_name);
    if (NULL == vrf)
        return NTB_VRF_NOT_EXIST;
    
    /* load default fake mac to P4 */
    // int ret = _ntb_hal_inner_smac_update(vrf->dp.vrf_id, route_mac, false);
    // if (ret < 0) 
    //     return ret;

    /* update local route mac */
    rte_ether_addr_copy(route_mac, &vrf->dp.route_mac);

    /* notify all overlay neigh */
    // ntb_vrf_neigh_update(vrf->dp.vrf_id);

    return 0;
}

int
ntb_vrf_rmac_unset(const char *vrf_name, const struct rte_ether_addr *route_mac)
{
    ntb_vrf_t *vrf = ntb_vrf_lookup(vrf_name);
    if (NULL == vrf)
        return NTB_VRF_NOT_EXIST;

    /* check the route_mac to be deleted */
    if (!rte_is_same_ether_addr(route_mac, &vrf->dp.route_mac))
        return NTB_COMM_INVAL_PARAM;
    
    /* load default fake mac to P4 */
    // int ret = _ntb_hal_inner_smac_update(vrf->dp.vrf_id, ntb_fake_mac_get(), false);
    // if (ret < 0) 
    //     return ret;

    /* update local route mac */
    rte_ether_addr_copy(ntb_fake_mac_get(), &vrf->dp.route_mac);

    /* notify all overlay neigh */
    // ntb_vrf_neigh_update(vrf->dp.vrf_id);

    return 0;
}

int
ntb_vrf_rmac_get(const char *vrf_name, struct rte_ether_addr *route_mac)
{
    ntb_vrf_t *vrf = ntb_vrf_lookup(vrf_name);
    if (NULL == vrf)
        return NTB_VRF_NOT_EXIST;

    rte_ether_addr_copy(&vrf->dp.route_mac, route_mac);
    return 0;
}

int ntb_vrf_init(void)
{
    int i;    
    int ret;

    ret = rte_fbarray_init(&g_fib_res_array, "fib_res_shared",
                NTB_NR_FIB_RES_ALL + 1, /* Reserve 1 for blackhole route.*/
                sizeof(struct fib_res));
    RTE_ASSERT(ret == 0);

    /* Pre-allocate fib result for blackhole route.*/
    f_res_blackhole = rte_fbarray_find_next_free(&g_fib_res_array, 0);
    RTE_ASSERT(f_res_blackhole >= 0);
    ret = rte_fbarray_set_used(&g_fib_res_array, f_res_blackhole);
    RTE_ASSERT(0 == ret);
    fib_res_set_flags(rte_fbarray_get(&g_fib_res_array, f_res_blackhole),
                FRF_REJECT);

    for (i = 0; i < RTE_DIM(g_ntb_vrfs); i++) {
        ntb_vrf_t *vrf = &g_ntb_vrfs[i];
        memset(vrf, 0, sizeof(*vrf));

        vrf->dp.vrf_id = i + 1;
        vrf->dp.fib_res_array = &g_fib_res_array;

        ret = ntb_vrf_route_init(&vrf->dp, f_res_blackhole);
        RTE_ASSERT(ret == 0);
    }
    ntb_nhop_init();

    return 0;
}

int ntb_vrf_deinit(void)
{
    return 0;
}

