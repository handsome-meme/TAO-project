#include "shc_digest.hpp"
#include "hal_table_api.h"
#include<iostream>
#include<fstream>
#include<stdlib.h>
#include<iomanip>
// #include "test_hal/test.hpp"

// #include "../../agent/entry_api.h"

#include "top_k/space_saving.hpp"
/***********************************************************************************
 * This sample cpp application code is based on the P4 program tna_digest.p4
 * Please refer to the P4 program and the generated bf-rt.json for information
 *on
 * the tables contained in the P4 program, and the associated key and data
 *fields.
 **********************************************************************************/

namespace shc {

  tuple_3_t tuple_3_tmp;


// int re_sh = 0;
uint32_t flow_count = 0;
uint32_t real_count = 0;
uint32_t fake_count = 0;
uint32_t error_count = 0;
//-------para for digest-------//
  // uint64_t port;
  uint64_t dtype;
  uint64_t vni;
  uint64_t ip_src_addr;
  uint64_t ip_dst_addr;
  // uint64_t time_stamp;
  uint64_t count;
// Learn field-ids
  // bf_rt_id_t learn_port_field_id = 0;
  bf_rt_id_t learn_type_field_id = 0;
  bf_rt_id_t learn_vni_digest_field_id = 0;
  bf_rt_id_t learn_src_ip_digest_field_id = 0;
  bf_rt_id_t learn_dst_ip_digest_field_id = 0;
  // bf_rt_id_t learn_time_stamp_field_id = 0;  
  bf_rt_id_t learn_count_field_id = 0; 
  // bfrt related
  const bfrt::BfRtLearn *learn_obj;
  // std::shared_ptr<bfrt::BfRtSession> session_;
  // bf_rt_target_t dev_tgt_;   
/**********************************************************************
 * CALLBACK funciton that gets invoked upon a learn event
 *  1. session : Session object that was used to register the callback. This is
 *               the session that has to be used to manipulate the table in
 *response to a learn
 *               event. Its always advisable to use a single session to
 *manipulate a single
 *               table.
 *  2. vec : Vector of learnData objects
 *  3. learn_msg_hdl : Pointer to the underlying learn message object, on which
 *                     an ack needs to be done in order for the hardware
 *resource to be freed up.
 *                     This is to be done once all the processing on the learn
 *update is done.
 *
 *********************************************************************/

bf_status_t learn_callback(const bf_rt_target_t &bf_rt_tgt,
                           std::shared_ptr<bfrt::BfRtSession> session,
                           std::vector<std::unique_ptr<bfrt::BfRtLearnData>> data,
                           bf_rt_learn_msg_hdl *const learn_msg_hdl,
                           const void *cookie) {
  /***********************************************************
   * INSERT CALLBACK IMPLEMENTATION HERE
   **********************************************************/
  std::ofstream mycout("/share/2a_eflow_p.txt",std::ios::app);
  // Extract learn data fields from Learn Data object and use it as needed.
    // std::vector<std::unique_ptr<bfrt::BfRtLearnData>> digest;
  
    for (auto const &digest : data){

      // port = 0;
      dtype = 0;
      vni = 0;
      ip_src_addr = 0;
      ip_dst_addr = 0;
      // time_stamp = 0;
      count = 0;

      // digest->getValue(learn_port_field_id, &port);
      digest->getValue(learn_type_field_id, &dtype);
      digest->getValue(learn_vni_digest_field_id, &vni);
      digest->getValue(learn_src_ip_digest_field_id, &ip_src_addr);
      digest->getValue(learn_dst_ip_digest_field_id, &ip_dst_addr);
      // digest->getValue(learn_time_stamp_field_id, &time_stamp);
      digest->getValue(learn_count_field_id, &count);

      mycout<<static_cast<uint32_t>(vni)<<","<<static_cast<uint32_t>(ip_src_addr)<<","<<static_cast<uint32_t>(ip_dst_addr)<<std::endl;

      /*
        using sapce-saving struct, processing top-k select
      */
      
      tuple_3_tmp.src_ip = ip_src_addr;
      tuple_3_tmp.dst_ip = ip_dst_addr;
      tuple_3_tmp.port = vni;
      shc::space_saving->Insert(tuple_3_tmp);

      if(dtype == 1){

        /****************************************/
        /*此处调用程泽群提供的接口，增加软件小表中的表项*/
        /****************************************/

        /***********************************************************************************/
        /*此处调用周大为提供的接口，查找每个hash表，以获取相应的转发规则，并下发到硬件，对应表项的count++*/
        /***********************************************************************************/

        // 考虑使用回调函数
        // re_sh = shc_swTohw_rule(static_cast<int32_t>(vni),static_cast<uint32_t>(ip_src_addr),static_cast<uint32_t>(ip_dst_addr),0);
        // hal_tunnel_map_vrf_key key1 = {TYPE_TUNNEL_TYPE_VXLAN, static_cast<uint16_t>(vni)};
        // hal_tunnel_map_vrf_data data1 = {static_cast<uint16_t>(vni)};
        // int stu_1 = hal_tunnel_map_vrf_entry_add(&key1, &data1);

        hal_route_sele_key key = {.vni=static_cast<uint32_t>(vni), .ip_src_addr=static_cast<uint32_t>(ip_src_addr), .ip_dst_addr=static_cast<uint32_t>(ip_dst_addr)};
        hal_route_data data = {.nhop_id=static_cast<uint32_t>(vni)};
        int stu_2 = hal_route_sele_entry_add(&key, &data);

        if(stu_2 == 0){
          flow_count++;
          if(vni <= 5000){
            real_count++;
          }

          if(15000 > vni && vni  > 5000){
            fake_count++;
          }
          if(15000 < vni){
            error_count++;
          }
        }

        std::cout << "flow_count = " << std::setbase(10) << static_cast<uint32_t>(flow_count)
                << " real_count = " << std::setbase(10) << static_cast<uint32_t>(real_count)
                << " fake_count = " << std::setbase(10) << static_cast<uint32_t>(fake_count)
                << " error_count = " << std::setbase(10) << static_cast<uint32_t>(error_count)
                // << "  port = " << static_cast<uint16_t>(port)  
                << "  vni = " <<static_cast<uint32_t>(vni) 
                // << "  ip_src_addr = 0x"<< std::setbase(16) << static_cast<uint32_t>(ip_src_addr) 
                // << "  ip_dst_addr = 0x"<< std::setbase(16) << static_cast<uint32_t>(ip_dst_addr) 
                // << "  entry insert-----"<< "  status_1 = " <<stu_1 << "  status_2 = " <<stu_2
                << "  entry insert-----"<< "  status_2 = " <<stu_2
                << std::endl;
        

        
      }
      else{
        /************************************************************/
        /*此处调用程泽群提供的接口，对软件大、小表中的表项的timestamp进行更新*/
        /************************************************************/
        
        // std::cout << "port = " << static_cast<uint16_t>(port) 
        //           << "  vni = " <<static_cast<uint16_t>(vni) 
        //           << "  ip_src_addr = 0x"<< std::setbase(16) << static_cast<uint32_t>(ip_src_addr) 
        //           << "  ip_dst_addr = 0x"<< std::setbase(16) << static_cast<uint32_t>(ip_dst_addr) 
        //           << "  time_stamp = "<<  std::setbase(10) << static_cast<uint64_t>(time_stamp) << std::endl;

        // test_neigh(session); //不用加命名空间可以执行成功
      }
    }

  /*********************************************************
  * WHEN DONE, ACK THE learn_msg_hdl
  ********************************************************/
  // (void)bf_rt_tgt; //避免了未使用的变量警告
  // (void)data;
  // (void)cookie;
  // printf("Learn callback invoked\n");
  auto bf_status = learn_obj->bfRtLearnNotifyAck(session, learn_msg_hdl);
  bf_sys_assert(bf_status == BF_SUCCESS);
  return BF_SUCCESS;
}

void ShcDigest(const bfrt::BfRtInfo *bfrtInfo, bf_rt_target_t& d_tgt, std::shared_ptr<bfrt::BfRtSession>& session){

  // dev_tgt_.dev_id = d_tgt.dev_id;
  // dev_tgt_.pipe_id = d_tgt.pipe_id; 

//----------------------------------------------------------------------------//
  /***********************************************************************
   * LEARN OBJECT GET FOR "digest" extern
   **********************************************************************/
  bf_status_t bf_status = bfrtInfo->bfrtLearnFromNameGet("ShcIngressDeparser.flowkey_digest",
                                             &learn_obj);
  bf_sys_assert(bf_status == BF_SUCCESS);

  /***********************************************************************
   * LEARN FIELD ID GET FROM LEARN OBJECT
   **********************************************************************/
  // bf_status =
  //     learn_obj->learnFieldIdGet("port", &learn_port_field_id);
  // bf_sys_assert(bf_status == BF_SUCCESS);

  bf_status = learn_obj->learnFieldIdGet("type", &learn_type_field_id);
  bf_sys_assert(bf_status == BF_SUCCESS);

  bf_status =
      learn_obj->learnFieldIdGet("vni_digest", &learn_vni_digest_field_id);
  bf_sys_assert(bf_status == BF_SUCCESS);

  bf_status =
      learn_obj->learnFieldIdGet("src_ip_digest", &learn_src_ip_digest_field_id);
  bf_sys_assert(bf_status == BF_SUCCESS);

  bf_status = learn_obj->learnFieldIdGet("dst_ip_digest", &learn_dst_ip_digest_field_id);
  bf_sys_assert(bf_status == BF_SUCCESS);

  // bf_status =
  //     learn_obj->learnFieldIdGet("time_stamp", &learn_time_stamp_field_id);
  // bf_sys_assert(bf_status == BF_SUCCESS);

  bf_status =
      learn_obj->learnFieldIdGet("count", &learn_count_field_id);
  bf_sys_assert(bf_status == BF_SUCCESS);

  /***********************************************************************
   * LEARN callback registration
   **********************************************************************/
  bf_status = learn_obj->bfRtLearnCallbackRegister(
      session, d_tgt, learn_callback, NULL);
  bf_sys_assert(bf_status == BF_SUCCESS);
  printf("bfRtLearnCallbackRegister---------\n");
//----------------------------------------------------------------------------//

}
// This function does the initial set up of getting key field-ids, action-ids
// and data field ids associated with the smac table. This is done once
// during init time.

int shc_digest_init(const bfrt::BfRtInfo *bfrtInfo, bf_rt_target_t& dev_tgt, std::shared_ptr<bfrt::BfRtSession>& session) {
  int status = 0;

  shc::ShcDigest(bfrtInfo, dev_tgt, session);
  printf("shc_digest_init---------\n");
  printf("shc_digest_init---------\n");
  printf("shc_digest_init---------\n");
  printf("shc_digest_init---------\n");
  printf("shc_digest_init---------\n");


  // HAL_LOG_INFO(LOG_USER1, "vrfmapping_init finish.");
  return status;
}

}  // shc_digest
// int main(int argc, char **argv) {
//   parse_opts_and_switchd_init(argc, argv);

//   // Do initial set up
//   bfrt::examples::shc_digest::setUp();
//   // Do table level set up
//   bfrt::examples::shc_digest::tableSetUp();

//   run_cli_or_cleanup();
//   return 0;
// }