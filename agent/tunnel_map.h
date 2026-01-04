#ifndef SHC_TUNNEL_MAP_H
#define SHC_TUNNEL_MAP_H

#include <sys/queue.h>

#include "shc_dp.h"
#include "vrf.h"

typedef enum {
    SHC_TUNNEL_INVALID = 0,
    SHC_TUNNEL_VXLAN,
    SHC_TUNNEL_TGRE,
    SHC_TUNNEL_MAX
} shc_tunnel_type_t;



#define TUNNEL_TYPE_VALID(type) \
    ((type) > SHC_TUNNEL_INVALID && (type) < SHC_TUNNEL_MAX)

typedef union {
    uint32_t vni;
    uint32_t vpcid;
    uint32_t common_id;
} shc_tunnel_id_t;

typedef struct {
    uint32_t urpf  :1;
    uint32_t htab16:1;
    uint32_t unused:30;
} shc_tunnel_attr_t;

#define TUNNEL_ATTR_EQUAL(t1, t2) \
    (((t1).urpf == (t2).urpf) && ((t1).htab16 == (t2).htab16))

/* VXLAN tunnel.*/
#define MIN_VNI_ID  (1)
#define MAX_VNI_ID  (1UL << 24) /* (1 << 24-1)UL */
#define VNI_ID_VALID(v) ((v) >= MIN_VNI_ID && (v) <= MAX_VNI_ID)

#define TUNNEL_BUN_ID_PER_TUN 4

struct shc_tunnel_info {
    SLIST_ENTRY(shc_tunnel_info) next;
    shc_tunnel_type_t type;
    shc_tunnel_id_t id;
    shc_tunnel_id_t bd_ids[TUNNEL_BUN_ID_PER_TUN];/* bundle ids */
    int32_t vrf_id;
    shc_tunnel_attr_t attribute;
    int8_t exit_flag;
};


// typedef int 
// (*tunmap_entry_cb_t)(struct shc_tunnel_info *entry,
//                 void *arg1, void *arg2);


extern int
shc_tunnel_map_vrf_init(void);

extern int
shc_tunnel_map_vrf_deinit(void);

extern int
shc_tunnel_map_vrf(const char *vrf_name, shc_tunnel_type_t type,
                shc_tunnel_id_t id, shc_tunnel_attr_t tunnel_attr);

extern int
shc_tunnel_unmap_vrf(shc_tunnel_type_t type, shc_tunnel_id_t id);

extern vrf_dp_t *
shc_tunnel_map_vrf_lookup(shc_tunnel_type_t type, shc_tunnel_id_t key,int flag_of_add_del);

extern int
shc_tunnel_map_cleanup(vrf_dp_t *vrf_dp);

#endif
