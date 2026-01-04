#include "hal_common_api.h"
#include "hal_utils.hpp"
#include "hal.hpp"
// #include <tm.hpp>

hal_status_t hal_bf_sys_log_set(int module, int level) {
    bf_sys_trace_level_set(module, level);
    bf_sys_log_level_set(module, BF_LOG_DEST_STDOUT, level);
    return HAL_STATUS_SUCCESS;
}

void hal_sys_log_set(hal_verbosity_t level) {
    set_log_level(level);
    return ;
}

void hal_sys_mac_addr_get(uint8_t mac_addr[6]) {
    hal_pltfm_sys_mac_addr_get(mac_addr);
    
}

hal_status_t hal_get_first_pipe(uint32_t *first_l_pipe) {
    return bf_tm_pipe_get_first(dev_tgt.dev_id, first_l_pipe);
}

hal_status_t hal_get_next_pipe(uint32_t current_l_pipe, uint32_t *next_l_pipe) {
    return bf_tm_pipe_get_next(dev_tgt.dev_id, current_l_pipe, next_l_pipe);
}

// hal_status_t hal_tm_blklvl_drop_get_api(uint32_t pipe, hal_tm_blklvl_cntrs_t *blk_cntrs) {
//     return hal_tm_blklvl_drop_get(dev_tgt.dev_id, pipe, blk_cntrs);
// }

// hal_status_t hal_tm_blklvl_drop_clear_api(uint32_t pipe) {
//     return hal_tm_blklvl_drop_clear(dev_tgt.dev_id, pipe);
// }
