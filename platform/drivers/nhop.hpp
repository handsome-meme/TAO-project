#pragma once
#include <bf_rt/bf_rt.hpp>
#include <stdint.h>

#include "hal_utils.hpp"
// #include "utils/log.h"
#include "base_table.hpp"

extern "C" {
#include "hal_table_type.h"
#include <bfutils/id/id.h>
}

namespace shc {
  class NHop : public BaseTable {
    public:
      NHop(){}  
      NHop(const bfrt::BfRtInfo *bfrtInfo, bf_rt_target_t& dev_tgt, std::shared_ptr<bfrt::BfRtSession>& session);
      hal_status_t tableEntryAddModify(const hal_nhop_key *key, const hal_nhop_data *data, bool is_add);
      hal_status_t tableEntryGet(const hal_nhop_key *key, hal_nhop_data *data);
      hal_status_t tableEntryDel(const hal_nhop_key *key);  
      hal_status_t tableUsageGet(uint32_t *entry_count);
      hal_status_t tableSizeGet(uint32_t *size);  
      hal_status_t AllocOid(uint32_t *oid);
      hal_status_t FreeOid(uint32_t oid);

    private:
      void KeySetUp(const hal_nhop_key &key,
                  bfrt::BfRtTableKey *table_key);
      void DataSetUp(const hal_nhop_data &data,
                    bfrt::BfRtTableData *table_data);
      uint32_t OidHtoS(uint32_t hal_oid);
      uint32_t OidStoH(uint32_t sal_oid);
      bf_id_allocator *id_generator_;
      uint32_t oid_offset_;

      std::unique_ptr<bfrt::BfRtTableKey> bfrtTableKey;
      std::unique_ptr<bfrt::BfRtTableData> bfrtTableData;
      // Key field ids 
      bf_rt_id_t nhop_id_field_id_ = 0;
      // Action Ids
      bf_rt_id_t set_nhop_action_id_ = 0;
      // Data field Ids for fib hit action
      bf_rt_id_t set_nhop_action_vni_id_ = 0;  
      bf_rt_id_t set_nhop_action_local_id_ = 0;  
      bf_rt_id_t set_nhop_action_remote_id_ = 0;  
      bf_rt_id_t set_nhop_action_neigh_id_id_ = 0;  
      // bfrt related
      bf_rt_target_t dev_tgt_;
      std::shared_ptr<bfrt::BfRtSession> session_;
      const bfrt::BfRtTable *table_ptr_ = nullptr;
      uint64_t size_;
  };

  extern NHop *nhop_table;
  
  int nhop_init(const bfrt::BfRtInfo *bfrtInfo, bf_rt_target_t& dev_tgt, std::shared_ptr<bfrt::BfRtSession>& session);
}

using namespace shc;

// std::ostream &operator<<(std::ostream &os, const hal_nhop_key &key);
// std::ostream &operator<<(std::ostream &os, const hal_nhop_data &data);      