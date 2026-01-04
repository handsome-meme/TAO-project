#include "port_for_mac.hpp"

namespace shc
{

bool PortForMac::mac_array_to_uint64(uint64_t& dst, const uint8_t from[6]) {
    dst = 0;
    uint64_t tmp;
    for (uint32_t i = 6; i > 0; i--) {
        tmp = from[i - 1];
        dst += tmp << (6 - i) * 8;
    }
    return true;
}

bool PortForMac::uint64_to_mac_array(uint8_t* dst, uint64_t from) {
    uint8_t byte = 0;
    byte = (int8_t)(from >> 40);
    dst[0] = byte;
    byte = (int8_t)(from >> 32);
    dst[1] = byte;
    byte = (int8_t)(from >> 24);
    dst[2] = byte;
    byte = (int8_t)(from >> 16);
    dst[3] = byte;
    byte = (int8_t)(from >> 8);
    dst[4] = byte;
    byte = (int8_t)from;
    dst[5] = byte;
    return true;
}

void PortForMac::KeySetUp(const hal_port_for_mac_key &key,
                       bfrt::BfRtTableKey *table_key) { //将所有的key字段
  // Set value into the key object.
  bf_status_t bf_status = BF_SUCCESS;
  bf_status = table_key->setValue(port_field_id_,
                                  static_cast<uint64_t>(key.port));
  bf_sys_assert(bf_status == BF_SUCCESS);

  return;
}

void PortForMac::DataSetUp(const hal_port_for_mac_data &data,
                                  bfrt::BfRtTableData *table_data) { //将所有的data字段
  // Set value into the data object
  bf_status_t bf_status = BF_SUCCESS;
  uint64_t dmac;
  mac_array_to_uint64(dmac, data.dmac);
  bf_status = table_data->setValue(set_port_for_mac_action_dmac_id_, dmac);
  bf_sys_assert(bf_status == BF_SUCCESS);

  uint64_t smac;
  mac_array_to_uint64(smac, data.smac);
  bf_status = table_data->setValue(set_port_for_mac_action_smac_id_, smac);
  bf_sys_assert(bf_status == BF_SUCCESS);
  
  return;
}

PortForMac::PortForMac(const bfrt::BfRtInfo *bfrtInfo, bf_rt_target_t& d_tgt, std::shared_ptr<bfrt::BfRtSession>& common_session){
  // dev and session initialize
  dev_tgt_.dev_id = d_tgt.dev_id;
  dev_tgt_.pipe_id = d_tgt.pipe_id;
  session_ = common_session;

  // get table
  bf_status_t bf_status =
      bfrtInfo->bfrtTableFromNameGet("pipe.ShcIngress.port_for_mac_tbl", &table_ptr_); //将指针table_ptr_指向硬件表
  bf_sys_assert(bf_status == BF_SUCCESS);

  // Get field-ids for key field 
  bf_status = table_ptr_->keyFieldIdGet("ig_md.meta.port",
                                          &port_field_id_); //对应到硬件表key字段
  bf_sys_assert(bf_status == BF_SUCCESS);

  // Get action Ids
  bf_status = table_ptr_->actionIdGet("ShcIngress.port_for_mac",
                                        &set_port_for_mac_action_id_); //对应到硬件表action字段
  bf_sys_assert(bf_status == BF_SUCCESS);

  bf_status = table_ptr_->actionIdGet("ShcIngress.fake_mac_for_outer",
                                        &set_fake_mac_for_outer_action_id_); //对应到硬件表action字段
  bf_sys_assert(bf_status == BF_SUCCESS);

  // Get action data ids
  bf_status =
      table_ptr_->dataFieldIdGet("smac",
                                   set_port_for_mac_action_id_,
                                   &set_port_for_mac_action_smac_id_); //对应到硬件表相应action的data字段 vrf
  bf_sys_assert(bf_status == BF_SUCCESS);
  
  bf_status =
      table_ptr_->dataFieldIdGet("dmac",
                                   set_port_for_mac_action_id_,
                                   &set_port_for_mac_action_dmac_id_); //对应到硬件表相应action的data字段 vrf
  bf_sys_assert(bf_status == BF_SUCCESS);
  // get size of table
  size_t t_size;
  uint64_t flags = 0;
  BF_RT_FLAG_INIT(flags);
  BF_RT_FLAG_SET(flags, BF_RT_FROM_HW);
  bf_status = table_ptr_->tableSizeGet(*session_, dev_tgt_, flags, &t_size);
  bf_sys_assert(bf_status == BF_SUCCESS);
  size_ = static_cast<uint32_t>(t_size);

  // alloc space for key and data
  bf_status = table_ptr_->keyAllocate(&bfrtTableKey);
  bf_sys_assert(bf_status == BF_SUCCESS);

  bf_status = table_ptr_->dataAllocate(&bfrtTableData);
  bf_sys_assert(bf_status == BF_SUCCESS);
}

hal_status_t PortForMac::tableEntryAddModify(const hal_port_for_mac_key *key, const hal_port_for_mac_data *data, bool is_add){
//   hal_operation_t option = is_add ? HAL_ADD_OPERATION : HAL_MOD_OPERATION;
//   HAL_LOG_INFO(LOG_USER1, "recv PortForMac {}. {},{} ", option, *key , *data);

  table_ptr_->keyReset(bfrtTableKey.get());

  table_ptr_->dataReset(set_port_for_mac_action_id_, bfrtTableData.get());
  DataSetUp(*data, bfrtTableData.get()); //将data值填到bfrtTableData空间中对应的action

  // Fill in the Key and Data object
  KeySetUp(*key, bfrtTableKey.get());
  
  // Call table entry add API, if the request is for an add, else call modify
  bf_status_t status = BF_SUCCESS;
  uint64_t flags = 0;
  if (is_add) {
    status = table_ptr_->tableEntryAdd(
        *session_, dev_tgt_, flags, *bfrtTableKey, *bfrtTableData); //表项最终通过bfrtTableKey、bfrtTableData下发。
  } else {
    status = table_ptr_->tableEntryMod(
        *session_, dev_tgt_, flags, *bfrtTableKey, *bfrtTableData);
  }
//   CHECK_RET_AND_LOG_WARN(status);
  return status;
}

hal_status_t PortForMac::tableEntryGet(const hal_port_for_mac_key *key, hal_port_for_mac_data *data){
//   HAL_LOG_INFO(LOG_USER1, "recv PortForMac {}. {} ", HAL_GET_OPERATION, *key);
  table_ptr_->keyReset(bfrtTableKey.get());
  table_ptr_->dataReset(bfrtTableData.get());
  // Fill in the Key object  
  KeySetUp(*key, bfrtTableKey.get());
  
  // Call table entry add API, if the request is for an add, else call modify
  bf_status_t status = BF_SUCCESS;
  uint64_t flags = 0;

  BF_RT_FLAG_SET(flags, BF_RT_FROM_HW);
  status = table_ptr_->tableEntryGet(
      *session_, dev_tgt_, flags, *bfrtTableKey, bfrtTableData.get());
  if (status != BF_SUCCESS) {
    uint64_to_mac_array(data->dmac, 0);
    // CHECK_RET_AND_LOG_INFO(status);
    return status;
  }

  // First get actionId, then based on that, fill in appropriate fields
  bf_rt_id_t action_id;
  status = bfrtTableData->actionIdGet(&action_id);
  bf_sys_assert(status == BF_SUCCESS);

  uint64_t tmp;
  status = bfrtTableData->getValue(set_port_for_mac_action_smac_id_, &tmp);
  bf_sys_assert(status == BF_SUCCESS);
  uint64_to_mac_array(data->smac, tmp);

  tmp = 0;
  status = bfrtTableData->getValue(set_port_for_mac_action_dmac_id_, &tmp);
  bf_sys_assert(status == BF_SUCCESS);
  uint64_to_mac_array(data->dmac, tmp);

  return status;
}

hal_status_t PortForMac::tableEntryDel(const hal_port_for_mac_key *key){
  // HAL_LOG_INFO(LOG_USER1, "recv PortForMac {}. {} ", HAL_DEL_OPERATION, *key);
  table_ptr_->keyReset(bfrtTableKey.get());
  // Fill in the Key object
  KeySetUp(*key, bfrtTableKey.get());
  
  // Call table entry add API, if the request is for an add, else call modify
  bf_status_t status = BF_SUCCESS;
  uint64_t flags = 0;
  BF_RT_FLAG_INIT(flags);

  status = table_ptr_->tableEntryDel(*session_, dev_tgt_, flags, *bfrtTableKey);
//   CHECK_RET_AND_LOG_WARN(status);

  return status;
}

hal_status_t PortForMac::tableUsageGet(uint32_t *entry_count){
  bf_status_t status = BF_SUCCESS;
  uint64_t flags = 0;

  BF_RT_FLAG_INIT(flags);
  BF_RT_FLAG_SET(flags, BF_RT_FROM_HW);
  status =
      table_ptr_->tableUsageGet(*session_, dev_tgt_, flags, entry_count);
//   CHECK_RET_AND_LOG_WARN(status);

  return status;
}

hal_status_t PortForMac::tableSizeGet(uint32_t *size){
  bf_status_t status = BF_SUCCESS;
  *size = size_;
  return status;
}


PortForMac *port_for_mac_table = nullptr;

int port_for_mac_init(const bfrt::BfRtInfo *bfrtInfo, bf_rt_target_t& dev_tgt, std::shared_ptr<bfrt::BfRtSession>& session) {
  int status = 0;

  shc::port_for_mac_table = new shc::PortForMac(bfrtInfo, dev_tgt, session);

//   HAL_LOG_INFO(LOG_USER1, "vrfmapping_init finish.");
  return status;
}
} // namespace shc

using namespace shc;

// std::ostream &operator<<(std::ostream &os,
//                          const hal_port_for_mac_key &key) {
//   const char* key_type_string[] = {"none", "vxlan", "gre"};

//   if (key.tunnel_type >= (sizeof(key_type_string)/sizeof(char *))) {
//     os << " UNKNOWN type";
//     return os;
//   }
//   std::string str = "tunnel map vrf key: {tunnel_type:" + std::string(key_type_string[key.tunnel_type]);
//   str += fmt::format(", tunnel_key:{}", key.tunnel_key);
//   str += "}\n";

//   os << str.c_str();

//   return os;
// }

// std::ostream &operator<<(std::ostream &os,
//                          const hal_port_for_mac_data &data) {
//   const char *data_type_string[] = {"tunnel_to_vrf"};

//   os << "data: {type: " << data_type_string[0] 
//       << ", vrf: " << data.vrf << "}";

//   return os;
// }
