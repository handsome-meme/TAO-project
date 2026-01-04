#ifndef SHC_API_H
#define SHC_API_H

#ifdef __cplusplus  
extern "C" {
#endif

typedef struct {
    char vrf_name[64];
    uint8_t family;
    union {
        uint32_t ip4;
    };
    uint8_t mac[6];
} shc_api_neigh_ent_t;

typedef struct {
    uint32_t remote_ip;
    uint32_t nexthop;
    uint32_t vni;
    uint8_t dscp;
} shc_api_vxlan_encap_t;

typedef struct {
    /* Tunnel encap.*/
    uint32_t remote_ip;
    uint32_t vpcid;
    uint32_t vmip;

} shc_api_tgre_encap_t;

typedef struct {
    uint32_t intip;
} shc_api_vrf_conf_t;

/* API operation flags.*/
#define SHC_API_O_CREATE    (1ul << 0)


/* NTB datapath running context.*/
typedef int
(*pmd_run_cb_t)(void *arg);

typedef struct {
    int nd_notify_fd;
    pmd_run_cb_t run_prepare;
    pmd_run_cb_t run_post;
} shc_pmd_thread_args_t; 

typedef void (*timer_callback)(int a);
/*定时器参数*/
typedef struct
{
	uint32_t interval_time; 		/* 时间间隔，单位秒 */
	timer_callback func; 			/* 处理函数 */
}timer_para;


extern void *
shc_pmd_thread(void *arg);

extern int
shc_api_shc_pmd_thread_ready_get(void);

#ifdef __cplusplus  
}
#endif

#endif

