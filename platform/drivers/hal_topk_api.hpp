#pragma once

#include <iostream>
#include "top_k/params.hpp"
#include "hal_topk_type.hpp"

// -----------------------------
// tunnel_map_vrf table   shc
// -----------------------------
int hal_topk_dump(flow_table_t * &old_sets, flow_table_t * &new_sets);
// hal_status_t topk_diff_cmp();