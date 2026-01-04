#ifndef SHC_ENTRY_API_H
#define SHC_ENTRY_API_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int 
shc_swTohw_rule(int32_t vni,uint32_t ip_src_addr,uint32_t ip_dst_addr,int flag_of_add_del);

#ifdef __cplusplus
}
#endif

#endif
