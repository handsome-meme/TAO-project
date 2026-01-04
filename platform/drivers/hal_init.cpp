#include "hal.hpp"
#include "hal_table_api.h"

namespace shc {
  
const bfrt::BfRtInfo *bfrtInfo = nullptr;
std::shared_ptr<bfrt::BfRtSession> session;

#define ALL_PIPES 0xffff
bf_rt_target_t dev_tgt;

void setUp() {
  dev_tgt.dev_id = 0;
  dev_tgt.pipe_id = ALL_PIPES;
  dev_tgt.direction = BF_DEV_DIR_ALL;
  dev_tgt.prsr_id = 0xff;
  // Get devMgr singleton instance
  auto &devMgr = bfrt::BfRtDevMgr::getInstance();

  // Get bfrtInfo object from dev_id and p4 program name
  auto bf_status =
      devMgr.bfRtInfoGet(dev_tgt.dev_id, "shc_real_time_sketch", &bfrtInfo);
  // Check for status
  bf_sys_assert(bf_status == BF_SUCCESS);

  // Create a session object
  session = bfrt::BfRtSession::sessionCreate();
}

}// end namespace shc 

using namespace shc;

void hal_init(int argc, char **argv) {
  // Do initial set up
  shc::setUp();
  
  // TODO:先设置为DEBUG,之后修改为INFO
  // auto  status = logging_init(HAL_API_LEVEL_DEBUG);
  // if (status != HAL_STATUS_SUCCESS) {
  //     printf("init log failed!!!");
  // }

  // HAL_LOG_INFO(LOG_USER1, "HAL SetUp Finish. Session Handle: {}, PRE Session Handle: {}", 
  //                           session->sessHandleGet(), session->preSessHandleGet());

  shc::space_saving_init();

  shc::tunnel_map_vrf_init(bfrtInfo, dev_tgt, session);
  shc::route_init(bfrtInfo, dev_tgt, session);
  shc::route_sele_init(bfrtInfo, dev_tgt, session);
  shc::nhop_init(bfrtInfo, dev_tgt, session);
  shc::neigh_init(bfrtInfo, dev_tgt, session);
  shc::port_for_mac_init(bfrtInfo, dev_tgt, session);

  shc::shc_digest_init(bfrtInfo, dev_tgt, session);
  shc::shc_aging_init(bfrtInfo, dev_tgt, session);
  

}



