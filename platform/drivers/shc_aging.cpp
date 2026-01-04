#include "shc_aging.hpp"
#include "hal_table_api.h"
#include<iostream>
#include<iomanip>
// #include <bf_rt/bf_rt_table_attributes.hpp>
// #include "test_hal/test.hpp"



/************************************************************************************
 * This example demonstarates the following for a P4 table with idle_timeout set
 *to True
 * 1. Allocate an attribute object and enable idle time out in NOTIFY_MODE with
 *required parameters.
 * 2. Using the attirbute object to call the attribute set API on table object.
 * 3. A hook for idle time out callback implementation.
 ********************************************************************/

namespace shc {

  //-------para for aging-------//
  // uint64_t aging_vrf;
  uint64_t aging_vni;
  uint64_t aging_ip_src;
  uint64_t aging_ip_dst;
  // Aging field-ids
  // bf_rt_id_t aging_vrf_field_id = 0;
  bf_rt_id_t aging_vni_field_id = 0;
  bf_rt_id_t aging_ip_src_field_id = 0;
  bf_rt_id_t aging_ip_dst_field_id = 0;

// Key field ids, table data field ids, action ids, Table object required for
// interacting with the table
// const bfrt::BfRtLearn *learn_obj = nullptr;
const bfrt::BfRtTable *route_sele_ = nullptr;
// std::shared_ptr<bfrt::BfRtSession> session;

std::unique_ptr<bfrt::BfRtTableAttributes> attr;
bf_rt_target_t dev_tgt_;

/**********************************************************************
 * CALLBACK funciton that gets invoked upon a entry agin event. One per entry
 *  1. target : Device target from which the entry is aging out
 *  2. key : Pointer to the key object representing the entry which has aged out
 *  3. cookie : Pointer to the cookie which was given at the time of the
 *callback registration
 *
 *********************************************************************/
bf_status_t idletime_callback(const bf_rt_target_t &target,
                              const bfrt::BfRtTableKey *key,
                              const void *cookie) {
  (void)cookie;
  // printf("idletime_callback---------\n");                             
  /***********************************************************
   * INSERT CALLBACK IMPLEMENTATION HERE
   **********************************************************/


  // aging_vrf = 0;
  aging_vni = 0;
  aging_ip_src = 0;
  aging_ip_dst = 0;

  // key->getValue(aging_vrf_field_id, &aging_vrf);
  key->getValue(aging_vni_field_id, &aging_vni);
  key->getValue(aging_ip_src_field_id, &aging_ip_src);
  key->getValue(aging_ip_dst_field_id, &aging_ip_dst);

  /***********************************************/
  /*此处调用程泽群提供的接口，老化删除软件小表中相应的表项*/
  /***********************************************/

  /**********************************************************************************************/
  /*此处调用周大为提供的接口，对相应hash表中的count进行更新并根据count值对其他表（nexthop、neighbor）进行删除*/
  /**********************************************************************************************/


  // hal_tunnel_map_vrf_key key1 = {TYPE_TUNNEL_TYPE_VXLAN, static_cast<uint16_t>(aging_vrf)};
  // // hal_tunnel_map_vrf_data data1 = {static_cast<uint16_t>(vni)};
  // int stu_1 = hal_tunnel_map_vrf_entry_del(&key1);

  hal_route_sele_key key2 = {.vni=static_cast<uint16_t>(aging_vni), .ip_src_addr=static_cast<uint32_t>(aging_ip_src), .ip_dst_addr=static_cast<uint32_t>(aging_ip_dst)};
  // hal_route_data data = {.nhop_id=static_cast<uint32_t>(vni)};
  hal_route_sele_entry_del(&key2);

  std::cout //<< "  vrf= " << std::setbase(10) <<static_cast<uint16_t>(aging_vrf) 
            // << "  ip_src_addr = 0x"<< std::setbase(16) << static_cast<uint32_t>(aging_ip_src) 
            << "  ip_dst_addr = "<< std::setbase(10) << static_cast<uint32_t>(aging_ip_dst) 
            // << "  entry aging and delete-----"<< "  status_1 = " <<stu_1 << "  status_2 = " <<stu_2 << std::endl
            // << "  entry aging and delete-----"<< "  status_2 = " <<stu_2 << std::endl;
            // << "dev_tgt_.dev_id = " << target.dev_id << "dev_tgt_.pipe_id = " <<target.pipe_id
            << std::endl;

  return BF_SUCCESS;
}

// This function does the initial set up of getting key field-ids, action-ids
// and data field ids associated with the smac table. This is done once
// during init time.
void ShcAging(const bfrt::BfRtInfo *bfrtInfo, bf_rt_target_t& dev_tgt, std::shared_ptr<bfrt::BfRtSession> &session){

  dev_tgt_.dev_id = dev_tgt.dev_id;
  dev_tgt_.pipe_id = dev_tgt.pipe_id; 

  // Get table object from name
  bf_status_t bf_status = bfrtInfo->bfrtTableFromNameGet("pipe.ShcIngress.route_sele_tbl", &route_sele_);
  bf_sys_assert(bf_status == BF_SUCCESS);

  /***********************************************************************
   * LEARN FIELD ID GET FROM LEARN OBJECT
   **********************************************************************/
  // bf_status =
  //     route_sele_->keyFieldIdGet("ig_md.vrf", &aging_vrf_field_id);
  // bf_sys_assert(bf_status == BF_SUCCESS);

  bf_status =
      route_sele_->keyFieldIdGet("ig_md.vni", &aging_vni_field_id);
  bf_sys_assert(bf_status == BF_SUCCESS);

  bf_status = route_sele_->keyFieldIdGet("hdr.inner_ipv4.src_addr", &aging_ip_src_field_id);
  bf_sys_assert(bf_status == BF_SUCCESS);

  bf_status =
      route_sele_->keyFieldIdGet("hdr.inner_ipv4.dst_addr", &aging_ip_dst_field_id);
  bf_sys_assert(bf_status == BF_SUCCESS);

  /***********************************************************************
   * ALLOCATE TABLE ATTRIBUTE FOR ENABLING IDLE TIMEOUT AND REGISTER A CALLBACK
   **********************************************************************/
  bf_status =
      route_sele_->attributeAllocate(bfrt::TableAttributesType::IDLE_TABLE_RUNTIME,
                                   bfrt::TableAttributesIdleTableMode::NOTIFY_MODE,
                                   &attr);
  bf_sys_assert(bf_status == BF_SUCCESS);

  // Set min_ttl to 50 ms, max_ttl to 5000 ms and ttl_query intervale to 50 ms
  uint32_t min_ttl = 10;
  uint32_t max_ttl = 20000;
  uint32_t ttl_query_interval = 100;
  void *cookie = nullptr;

  bf_status = attr->idleTableNotifyModeSet(
      false, NULL, ttl_query_interval, max_ttl, min_ttl, NULL);

  bf_status = attr->idleTableNotifyModeSet(
      true, shc::idletime_callback, ttl_query_interval, max_ttl, min_ttl, cookie);
  bf_sys_assert(bf_status == BF_SUCCESS);
  printf("idleTableNotifyModeSet---------\n");
  /***********************************************************************
   * CALL ATTRIBUTE SET API ON TABLE OBJECT
   **********************************************************************/
  uint32_t flags = 0;
  bf_status =
      route_sele_->tableAttributesSet(*session, dev_tgt_, flags, *attr.get());
  bf_sys_assert(bf_status == BF_SUCCESS);
  printf("tableAttributesSet---------\n");

}

int shc_aging_init(const bfrt::BfRtInfo *bfrtInfo, bf_rt_target_t& dev_tgt, std::shared_ptr<bfrt::BfRtSession>& session) {
  int status = 0;

  shc::ShcAging(bfrtInfo, dev_tgt, session);
  printf("shc_aging_init---------\n");

  return status;
}

}  // shc_aging

// int main(int argc, char **argv) {
//   parse_opts_and_switchd_init(argc, argv);

//   // Do initial set up
//   bfrt::examples::tna_idletimeout::setUp();
//   // Do table level set up
//   bfrt::examples::tna_idletimeout::tableSetUp();

//   run_cli_or_cleanup();
//   return 0;
// }

