#include "nhop.hpp"

namespace shc
{
void NHop::KeySetUp(const hal_nhop_key &key,
                       bfrt::BfRtTableKey *table_key) { //将所有的key字段
  // Set value into the key object.
  bf_status_t bf_status = BF_SUCCESS;
  bf_status = table_key->setValue(nhop_id_field_id_,
                                  static_cast<uint64_t>(key.nhop_id));
  bf_sys_assert(bf_status == BF_SUCCESS);

  return;
}

void NHop::DataSetUp(const hal_nhop_data &data,
                                  bfrt::BfRtTableData *table_data) { //将所有的data字段
  // Set value into the data object
  bf_status_t bf_status = BF_SUCCESS;
  bf_status = table_data->setValue(set_nhop_action_vni_id_,
                                        static_cast<uint64_t>(data.tunnel_key));
  bf_sys_assert(bf_status == BF_SUCCESS);

  bf_status = table_data->setValue(set_nhop_action_local_id_,
                                        static_cast<uint64_t>(data.local));
  bf_sys_assert(bf_status == BF_SUCCESS);

  bf_status = table_data->setValue(set_nhop_action_remote_id_,
                                        static_cast<uint64_t>(data.remote));
  bf_sys_assert(bf_status == BF_SUCCESS);

  bf_status = table_data->setValue(set_nhop_action_neigh_id_id_,
                                        static_cast<uint64_t>(data.neigh_id));
  bf_sys_assert(bf_status == BF_SUCCESS);
  
  return;
}

NHop::NHop(const bfrt::BfRtInfo *bfrtInfo, bf_rt_target_t& d_tgt, std::shared_ptr<bfrt::BfRtSession>& common_session){
  // dev and session initialize
  dev_tgt_.dev_id = d_tgt.dev_id;
  dev_tgt_.pipe_id = d_tgt.pipe_id;
  session_ = common_session;

  // get table
  bf_status_t bf_status =
      bfrtInfo->bfrtTableFromNameGet("pipe.ShcIngress.nhop_tbl", &table_ptr_); //将指针table_ptr_指向硬件表
  bf_sys_assert(bf_status == BF_SUCCESS);

  // Get field-ids for key field 
  bf_status = table_ptr_->keyFieldIdGet("ig_md.nhop_id",
                                          &nhop_id_field_id_); //对应到硬件表key字段
  bf_sys_assert(bf_status == BF_SUCCESS);

  // Get action Ids
  bf_status = table_ptr_->actionIdGet("ShcIngress.nhop",
                                        &set_nhop_action_id_); //对应到硬件表action字段
  bf_sys_assert(bf_status == BF_SUCCESS);

  // Get action data ids
  bf_status =
      table_ptr_->dataFieldIdGet("vni",
                                   set_nhop_action_id_,
                                   &set_nhop_action_vni_id_); //对应到硬件表相应action的data字段 vrf
  bf_sys_assert(bf_status == BF_SUCCESS);

  bf_status =
      table_ptr_->dataFieldIdGet("local",
                                   set_nhop_action_id_,
                                   &set_nhop_action_local_id_); //对应到硬件表相应action的data字段 vrf
  bf_sys_assert(bf_status == BF_SUCCESS);

  bf_status =
      table_ptr_->dataFieldIdGet("remote",
                                   set_nhop_action_id_,
                                   &set_nhop_action_remote_id_); //对应到硬件表相应action的data字段 vrf
  bf_sys_assert(bf_status == BF_SUCCESS);

  bf_status =
      table_ptr_->dataFieldIdGet("neigh_id",
                                   set_nhop_action_id_,
                                   &set_nhop_action_neigh_id_id_); //对应到硬件表相应action的data字段 vrf
  bf_sys_assert(bf_status == BF_SUCCESS);
  
  // get size of table
  size_t t_size;
  uint64_t flags = 0;
  BF_RT_FLAG_INIT(flags);
  BF_RT_FLAG_SET(flags, BF_RT_FROM_HW);
  bf_status = table_ptr_->tableSizeGet(*session_, dev_tgt_, flags, &t_size);
  bf_sys_assert(bf_status == BF_SUCCESS);
  size_ = static_cast<uint32_t>(t_size);
  //   setScaleRatio(DEFAULT_SCALE_RATIO);

  // alloc space for key and data
  bf_status = table_ptr_->keyAllocate(&bfrtTableKey);
  bf_sys_assert(bf_status == BF_SUCCESS);

  bf_status = table_ptr_->dataAllocate(&bfrtTableData);
  bf_sys_assert(bf_status == BF_SUCCESS);

  id_generator_ = bf_id_allocator_new(size_, false); 
}

uint32_t NHop::OidHtoS(uint32_t hal_oid) {
//   return hal_oid + oid_offset_;
   return hal_oid + 0;
}

uint32_t NHop::OidStoH(uint32_t sal_oid){
//   return sal_oid - oid_offset_;
  return sal_oid - 0;
}

hal_status_t NHop::tableEntryAddModify(const hal_nhop_key *key, const hal_nhop_data *data, bool is_add){
//   hal_operation_t option = is_add ? HAL_ADD_OPERATION : HAL_MOD_OPERATION;
//   HAL_LOG_INFO(LOG_USER1, "recv NHop {}. {},{} ", option, *key , *data);

  table_ptr_->keyReset(bfrtTableKey.get());

  table_ptr_->dataReset(set_nhop_action_id_, bfrtTableData.get());
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
  // CHECK_RET_AND_LOG_WARN(status);
  return status;
}

hal_status_t NHop::tableEntryGet(const hal_nhop_key *key, hal_nhop_data *data){
  // HAL_LOG_INFO(LOG_USER1, "recv NHop {}. {} ", HAL_GET_OPERATION, *key);
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
    data->tunnel_key = -1;
    data->local = 0;
    data->remote = 0;
    data->neigh_id = -1;
    // CHECK_RET_AND_LOG_INFO(status);
    return status;
  }

  // First get actionId, then based on that, fill in appropriate fields
  bf_rt_id_t action_id;
  status = bfrtTableData->actionIdGet(&action_id);
  bf_sys_assert(status == BF_SUCCESS);

  uint64_t tmp;
  status = bfrtTableData->getValue(set_nhop_action_vni_id_, &tmp);
  bf_sys_assert(status == BF_SUCCESS);
  data->tunnel_key = static_cast<uint32_t>(tmp);

  tmp = 0;
  status = bfrtTableData->getValue(set_nhop_action_local_id_, &tmp);
  bf_sys_assert(status == BF_SUCCESS);
  data->local = static_cast<uint32_t>(tmp);

  tmp = 0;
  status = bfrtTableData->getValue(set_nhop_action_remote_id_, &tmp);
  bf_sys_assert(status == BF_SUCCESS);
  data->remote = static_cast<uint32_t>(tmp);

  tmp = 0;
  status = bfrtTableData->getValue(set_nhop_action_neigh_id_id_, &tmp);
  bf_sys_assert(status == BF_SUCCESS);
  data->neigh_id = static_cast<uint32_t>(tmp);

  return status;
}

hal_status_t NHop::tableEntryDel(const hal_nhop_key *key){
//   HAL_LOG_INFO(LOG_USER1, "recv NHop {}. {} ", HAL_DEL_OPERATION, *key);
  table_ptr_->keyReset(bfrtTableKey.get());
  // Fill in the Key object
  KeySetUp(*key, bfrtTableKey.get());
  
  // Call table entry add API, if the request is for an add, else call modify
  bf_status_t status = BF_SUCCESS;
  uint64_t flags = 0;
  BF_RT_FLAG_INIT(flags);

  status = table_ptr_->tableEntryDel(*session_, dev_tgt_, flags, *bfrtTableKey);
  // CHECK_RET_AND_LOG_WARN(status);

  return status;
}

hal_status_t NHop::tableUsageGet(uint32_t *entry_count){
  bf_status_t status = BF_SUCCESS;
  uint64_t flags = 0;

  BF_RT_FLAG_INIT(flags);
  BF_RT_FLAG_SET(flags, BF_RT_FROM_HW);
  status =
      table_ptr_->tableUsageGet(*session_, dev_tgt_, flags, entry_count);
  // CHECK_RET_AND_LOG_WARN(status);

  return status;
}

hal_status_t NHop::tableSizeGet(uint32_t *size){
  bf_status_t status = BF_SUCCESS;
  *size = size_;
  return status;
}

hal_status_t NHop::AllocOid(uint32_t *oid) {
  int64_t id = bf_id_allocator_allocate(id_generator_);
  if (id == -1 || (uint64_t)id > size_) {
    // HAL_LOG_WARN(LOG_USER1, "{}, total_size :{}", hal_err_str(HAL_NO_SPACE), total_size_);
    return HAL_NO_SPACE;
  }

  *oid = OidHtoS(id);
  return HAL_STATUS_SUCCESS;
}

hal_status_t NHop::FreeOid(uint32_t oid) {
//   bf_id_allocator_release(id_generator_, (oid - oid_offset_));
  bf_id_allocator_release(id_generator_, (oid - 0));
  return HAL_STATUS_SUCCESS;
}


NHop *nhop_table = nullptr;

int nhop_init(const bfrt::BfRtInfo *bfrtInfo, bf_rt_target_t& dev_tgt, std::shared_ptr<bfrt::BfRtSession>& session) {
  int status = 0;

  shc::nhop_table = new shc::NHop(bfrtInfo, dev_tgt, session);

//   HAL_LOG_INFO(LOG_USER1, "vrfmapping_init finish.");
  return status;
}
} // namespace shc

using namespace shc;

// std::ostream &operator<<(std::ostream &os,
//                          const hal_nhop_key &key) {
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
//                          const hal_nhop_data &data) {
//   const char *data_type_string[] = {"tunnel_to_vrf"};

//   os << "data: {type: " << data_type_string[0] 
//       << ", vrf: " << data.vrf << "}";

//   return os;
// }
