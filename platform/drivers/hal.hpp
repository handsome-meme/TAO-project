#pragma once

#include "tunnel_map_vrf.hpp"
#include "route.hpp"
#include "route_sele.hpp"
#include "nhop.hpp"
#include "neigh.hpp"
#include "port_for_mac.hpp"
#include "shc_digest.hpp"
#include "shc_aging.hpp"
#include "top_k/space_saving.hpp"

// #include "utils/log.h"


namespace shc
{
  extern const bfrt::BfRtInfo *bfrtInfo;
  extern std::shared_ptr<bfrt::BfRtSession> session;

  extern bf_rt_target_t dev_tgt;
} // namespace shc
