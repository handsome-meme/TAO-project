#include <stdio.h>
#include <unistd.h>

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

#include <rte_ip.h>
#include <rte_log.h>
#include <rte_fib.h>

#include "ntb_errno.h"
#include "vrf.h"
#include "neigh.h"
#include "route.h"
#include "stat.h"
// #include "ntb_spec.h"
// #include <hal_table_type.h>
// #include <hal_table_api.h>

static neigh_event_notify_cb_t neigh_ent_mod_notify_fn
                = ntb_api_neigh_ent_chg_notify;

static struct rte_ether_addr fake_mac = {
    .addr_bytes = {0x3c, 0xfd, 0xfe, 0x29, 0xcb, 0xc2}
};

static struct rte_ether_addr def_gw_mac = {
    .addr_bytes = {0x3c, 0xfd, 0xfe, 0x29, 0xcb, 0xc2}
};

const struct rte_ether_addr *
ntb_fake_mac_get(void)
{
    return &fake_mac;
}

int
ntb_fake_mac_set(const struct rte_ether_addr *ether_addr)
{
    rte_ether_addr_copy(ether_addr, &fake_mac);
    return 0;
}

const struct rte_ether_addr *
ntb_def_gw_mac_get(void)
{
    return &def_gw_mac;
}

int
ntb_def_gw_mac_set(const struct rte_ether_addr *ether_addr)
{
    rte_ether_addr_copy(ether_addr, &def_gw_mac);
    return 0;
}

const struct rte_ether_addr *
ntb_broadcast_mac_get(void)
{
    static const struct rte_ether_addr broadcast_mac = {
        .addr_bytes = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
    };

    return &broadcast_mac;
}

const struct rte_ether_addr *
ntb_zero_mac_get(void)
{
    static const struct rte_ether_addr zero_mac = {
        .addr_bytes = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    };

    return &zero_mac;
}

static struct {
    struct rte_hash *hash_tbl;
    uint32_t count;
} ol_neigh_table;

static int __rte_unused
ol_neigh_lookup(uint32_t vrfid, uint32_t gw,
                struct ol_neigh_entry *entry)
{
    struct ol_neigh_entry *hh;

    struct ol_neigh_lookup_key key = {
        .vrfid = vrfid,
        .nexthop = gw
    };

    if (rte_hash_lookup_data(ol_neigh_table.hash_tbl,
                &key, (void**)&hh) < 0) {
        return -1; /* ENOENT.*/
    }

    *entry = *hh;
    return 0;
}

int
ol_neigh_add_static(uint32_t vrfid, uint32_t gw,
                const struct rte_ether_addr *hw_addr)
{
    uint8_t hh_len;
    struct ol_neigh_entry *nhop;
    struct rte_ether_hdr *ethhdr;
    int ret;

    struct ol_neigh_lookup_key key = {
        .vrfid = vrfid,
        .nexthop = gw
    };

    ret = rte_hash_lookup_data(ol_neigh_table.hash_tbl, &key, ( void **)&nhop);
    if (ret < 0) {
        /* Not found, create a new entry.*/
        nhop = rte_zmalloc_socket(NULL, sizeof(*nhop), 0, SOCKET_ID_ANY);
        if (nhop == NULL)
            return NTB_COMM_NOMEM;

        if (rte_hash_add_key_data(ol_neigh_table.hash_tbl,
                                  &key, ( void **)nhop) < 0) {
            rte_free(nhop);
            return -EFAULT;
        }

        ol_neigh_table.count++;
    }

    if (!nhop->F_arp_static) {
        nhop->refcnt++;
    }
    
    nhop->F_arp_static = 1;
    nhop->F_entry_valid = 1;

    ethhdr = (struct rte_ether_hdr *)nhop->hh;
    hh_len = sizeof(*ethhdr);
    rte_ether_addr_copy(hw_addr, &ethhdr->d_addr);
    rte_ether_addr_copy(ntb_fake_mac_get(), &ethhdr->s_addr);
    ethhdr->ether_type = rte_cpu_to_be_16(RTE_ETHER_TYPE_IPV4);
    nhop->hh_len = hh_len;

    nhop->F_arp_valid = 1; /* ARP learned.*/
    return 0;
}

/*
static int
_ntb_hal_neigh_update(uint32_t neigh_oid, const uint8_t* dmac, int is_new)
{
    int ret;
    hal_inner_neighbor_key hal_neigh_key;
    hal_inner_neighbor_data hal_neigh_data; 

    if (is_new && !ntb_spec_obj_check(NTB_INNER_NEIGH_OBJ))
        return NTB_INNER_NEIGH_OBJ_EXCEED_SPEC; 

    hal_neigh_key.inner_neighbor_index = neigh_oid;
    memcpy(hal_neigh_data.dmac, dmac, RTE_ETHER_ADDR_LEN);
    hal_neigh_data.data_type = TYPE_SET_INNER_NEIGHBOR; 
    if (is_new)
        ret = hal_inner_neighbor_entry_add(&hal_neigh_key, &hal_neigh_data);
    else 
        ret = hal_inner_neighbor_entry_mod(&hal_neigh_key, &hal_neigh_data);
    if (ret != 0) {
        RTE_LOG(ERR, USER1, "_ntb_hal_neigh_update %s call error: %s", 
                                        is_new ? "add" : "update", 
                                        hal_status_to_string(ret)); 
        return NTB_LOAD_TO_P4_FAIL;
    }
    if (is_new)
        ntb_spec_obj_ent_num_inc(NTB_INNER_NEIGH_OBJ);
    return 0;
}

static int
_ntb_hal_neigh_del(uint32_t neigh_oid)
{
    int ret;

    hal_inner_neighbor_key hal_neigh_key;
    hal_neigh_key.inner_neighbor_index = neigh_oid;

    ret = hal_inner_neighbor_entry_del(&hal_neigh_key);
    if (ret != 0) {
        RTE_LOG(ERR, USER1, "_ntb_hal_neigh_del call error: %s",
                                hal_status_to_string(ret)); 
        return NTB_LOAD_TO_P4_FAIL;
    }

    ntb_spec_obj_ent_num_dec(NTB_INNER_NEIGH_OBJ);
    return 0;
}
*/

//neigh表添加，都是下路由时一并下发，不会单独创建neigh
int
ntb_neigh_add(vrf_dp_t *vrf_dp, uint32_t gw, uint32_t *neigh_oid_ret)
{
    struct ol_neigh_entry *nhop;
    int ret;
    uint8_t mac[RTE_ETHER_ADDR_LEN] = {0};

    struct ol_neigh_lookup_key key = {
        .vrfid = vrf_dp->vrf_id,
        .nexthop = gw
    };

    ret = rte_hash_lookup_data(ol_neigh_table.hash_tbl,
                &key, ( void **)&nhop);
    if (ret >= 0) {
        nhop->refcnt++;
        *neigh_oid_ret = nhop->neigh_oid;
        return 0;
    }

    /* Not found, create a new entry.*/
    nhop = rte_zmalloc_socket(NULL, sizeof(*nhop), 0, SOCKET_ID_ANY);
    if (nhop == NULL)
        return -ENOMEM;

    /* Initialize the nhop.*/
    nhop->refcnt = 1;
    nhop->F_fast_arp = 1;
    nhop->F_entry_valid = 1;

    if (rte_hash_add_key_data(ol_neigh_table.hash_tbl, &key, nhop) < 0) {
        rte_free(nhop);
        return -EFAULT;
    }

    /* alloc oid */
    //ret = hal_inner_neighbor_alloc_oid(&nhop->neigh_oid); //--------------修改成自己的函数
    if (ret < 0) {
        goto rollback2;
    }


    vrf_dp->neigh_cnt++;
    ol_neigh_table.count++;

    // if (neigh_ent_mod_notify_fn)
    //     neigh_ent_mod_notify_fn(NEIGH_NOTIFY_CREATE, &key, nhop);

    *neigh_oid_ret = nhop->neigh_oid;
    return 0;

rollback2:
    rte_hash_del_key(ol_neigh_table.hash_tbl, &key);
    rte_free(nhop);
    return ret;
}

int
ntb_neigh_del(vrf_dp_t *vrf_dp, uint32_t gw)
{
    struct ol_neigh_entry *nhop;
    int ret;

    struct ol_neigh_lookup_key key = {
        .vrfid = vrf_dp->vrf_id,
        .nexthop = gw
    };

    ret = rte_hash_lookup_data(ol_neigh_table.hash_tbl,
                &key, ( void **)&nhop);
    if (ret < 0) {
        return -1; /* Not found.*/
    }

    RTE_ASSERT(nhop->refcnt > 0);
    nhop->refcnt--;

    if (nhop->refcnt > 0)
        return 0;
    //if nhop->refcnt==0 shows no route use it, then delete it
    vrf_dp->neigh_cnt--;
    ol_neigh_table.count--;
    rte_hash_del_key(ol_neigh_table.hash_tbl, &key);
    rte_free(nhop);
    return 0;
}

int
ntb_neigh_lookup(uint32_t vrfid, uint32_t gw, struct rte_ether_addr *hw_addr)
{
    struct ol_neigh_entry *nhop;
    struct rte_ether_hdr *ethhdr;
    int ret;

    struct ol_neigh_lookup_key key = {
        .vrfid = vrfid,
        .nexthop = gw
    };

    ret = rte_hash_lookup_data(ol_neigh_table.hash_tbl,
                &key, ( void **)&nhop);
    if (ret < 0)
        return -ENOENT;


    if (!nhop->F_arp_valid)
        return -1; /* Incomplete arp.*/

    ethhdr = (struct rte_ether_hdr *)nhop->hh;
    rte_ether_addr_copy(&ethhdr->d_addr, hw_addr);
    return 0;
}

int
ntb_neigh_init(ntb_pmd_ctx_t *ctx)
{
    struct rte_hash_parameters params = {
        .name = RTE_STR(HASH_OVERLAY_GW),
        .entries = MAX_VRF_GW_HASH_ENT * 6,
        .hash_func = rte_hash_crc,
        .key_len = sizeof(struct ol_neigh_lookup_key),
    };

    ol_neigh_table.hash_tbl = rte_hash_create(&params);
    RTE_ASSERT(ol_neigh_table.hash_tbl != NULL);
    rte_hash_reset(ol_neigh_table.hash_tbl);

    return 0;
}

int
ntb_neigh_deinit(void)
{
    return 0;
}
