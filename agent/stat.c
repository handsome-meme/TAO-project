#include <rte_common.h>
#include <rte_byteorder.h>
#include <rte_log.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_launch.h>
#include <rte_atomic.h>
#include <rte_ether.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_debug.h>
#include <rte_malloc.h>

#include "shc_errno.h"
#include "vrf.h"
#include "neigh.h"
#include "route.h"
#include "stat.h"


__thread shc_swf_evt_stat_t *cur_thr_stats;


static const char *_shc_swf_evt_strs[] = {
#define _(f, e) RTE_STR(f),
    foreach_swf_evt_type 
#undef _
};

const char *
shc_swf_evt_str_get(int type)
{
    if (SHC_SWF_EVT_TYPE_VALID(type))
        return _shc_swf_evt_strs[type];

    return "SWF event type is not defined.";
}

static const char _shc_swf_evt_is_err[] = {
#define _(f, e) e,
    foreach_swf_evt_type 
#undef _
};

const char
shc_swf_evt_is_err(int type)
{
    if (SHC_SWF_EVT_TYPE_VALID(type))
        return _shc_swf_evt_is_err[type];

    return 0;
}

static int 
pmd_thr_stat_cb(shc_pmd_ctx_t *pmd_thr_ctx, void *arg1, void *arg2)
{
    int i;
    shc_swf_evt_stat_t *total = arg1;

    for (i = 0; i < RTE_DIM(total->stat); i++)
        total->stat[i] += pmd_thr_ctx->evt_stat.stat[i];

    return 0;
}

int
shc_swf_evt_stat_total_get(shc_swf_evt_stat_t *total)
{
    memset(total, 0, sizeof(*total));
    shc_pmd_thread_walk(pmd_thr_stat_cb, total, NULL);
    return 0;
}

static int
pmd_thr_stat_clr_cb(shc_pmd_ctx_t *pmd_thr_ctx, void *arg1, void *arg2)
{
    int i;

    for (i = 0; i < RTE_DIM(pmd_thr_ctx->evt_stat.stat); i++)
        pmd_thr_ctx->evt_stat.stat[i] = 0;

    return 0;
}

int
shc_swf_evt_stat_clear(void)
{
    shc_pmd_thread_walk(pmd_thr_stat_clr_cb, NULL, NULL);
    return 0;
}

int
shc_mempool_stat_get(mempool_stat_t *mp_stat)
{
    struct rte_mempool *mbuf_pool = rte_mempool_lookup("MBUF_POOL");
    if (NULL == mbuf_pool)
        return -1;

    mp_stat->mp_name = "MBUF_POOL";
    mp_stat->count_all = mbuf_pool->size;
    mp_stat->count_avail = rte_mempool_avail_count(mbuf_pool);
    mp_stat->count_inuse = mp_stat->count_all - mp_stat->count_avail;
    return 0;
}

static void
mpool_cnt_cb(struct rte_mempool *mp, void *arg)
{
    uint32_t *mpcntptr = arg;

    RTE_SET_USED(mp);

    (*mpcntptr)++;
}

uint32_t
shc_mempool_cnt_get(void)
{
    uint32_t mpcnt = 0;

    rte_mempool_walk(mpool_cnt_cb, &mpcnt);
    return mpcnt;
}

struct mpool_walk_cookie {
    mempool_walk_cb_t cb;
    void *arg;
};

static void
mpool_stat_priv_cb(struct rte_mempool *mp, void *arg)
{
    struct mpool_walk_cookie *mw_cookie = arg;
    mempool_stat_t mp_stat = {
        .mp_name = mp->name,
        .count_all = mp->size,
        .count_avail = rte_mempool_avail_count(mp)
    };
    mp_stat.count_inuse = mp_stat.count_all
                        - mp_stat.count_avail;
    mw_cookie->cb(&mp_stat, mw_cookie->arg);
}

void
shc_mempool_stat_walk(mempool_walk_cb_t cb, void *arg)
{
    struct mpool_walk_cookie mw_cookie = {
        .cb = cb,
        .arg = arg,
    };
    rte_mempool_walk(mpool_stat_priv_cb, &mw_cookie);
}

int
shc_socket_mem_stat_get(int node_n, shc_socket_mem_stat_t *node_mem_stats)
{
    struct rte_malloc_socket_stats sock_stats;
    int i, ret;
    int nid = 0;

    for (i = 0; i < node_n; i++) {
        ret = rte_malloc_get_socket_stats(i, &sock_stats);
        if (ret < 0) {
             /* This node is offline or unavailable.*/
            continue;
        }

        node_mem_stats[nid].node_id = i;
        node_mem_stats[nid].total_bytes = sock_stats.heap_totalsz_bytes;
        node_mem_stats[nid].free_bytes = sock_stats.heap_freesz_bytes;
        node_mem_stats[nid].alloc_bytes = sock_stats.heap_allocsz_bytes;

        nid++;
    }

    return nid;
}
