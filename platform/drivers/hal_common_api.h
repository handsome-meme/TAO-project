#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <bfsys/bf_sal/bf_sys_log.h>
#include "hal_types.h"
#include <traffic_mgr/traffic_mgr_counters.h>
#include <traffic_mgr/traffic_mgr_read_apis.h>
#include "hal_table_type.h"

hal_status_t hal_bf_sys_log_set(int module, int level);
void hal_sys_log_set(hal_verbosity_t level);
void hal_sys_mac_addr_get(uint8_t mac_addr[6]);

hal_status_t hal_get_first_pipe(uint32_t *first_l_pipe);
hal_status_t hal_get_next_pipe(uint32_t current_l_pipe, uint32_t *next_l_pipe);
hal_status_t hal_tm_blklvl_drop_get_api(uint32_t pipe, hal_tm_blklvl_cntrs_t *blk_cntrs);
hal_status_t hal_tm_blklvl_drop_clear_api(uint32_t pipe);
#ifdef __cplusplus
}
#endif
