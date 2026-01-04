#pragma once
#include <bf_rt/bf_rt.hpp>
#include <stdint.h>
// #include "utils/log.h"
#include "base_table.hpp"

extern "C" {
#include "hal_table_type.h"
}

namespace shc {
  class TunnelMapVrf : public BaseTable {
    public:
      TunnelMapVrf(){}  
      TunnelMapVrf(const bfrt::BfRtInfo *bfrtInfo, bf_rt_target_t& dev_tgt, std::shared_ptr<bfrt::BfRtSession>& session);
      hal_status_t tableEntryAddModify(const hal_tunnel_map_vrf_key *key, const hal_tunnel_map_vrf_data *data, bool is_add);
      hal_status_t tableEntryGet(const hal_tunnel_map_vrf_key *key, hal_tunnel_map_vrf_data *data);
      hal_status_t tableEntryDel(const hal_tunnel_map_vrf_key *key);  
      hal_status_t tableUsageGet(uint32_t *entry_count);
      hal_status_t tableSizeGet(uint32_t *size);  

    private:
      void KeySetUp(const hal_tunnel_map_vrf_key &key,
                  bfrt::BfRtTableKey *table_key);
      void DataSetUp(const hal_tunnel_map_vrf_data &data,
                    bfrt::BfRtTableData *table_data);
      std::unique_ptr<bfrt::BfRtTableKey> bfrtTableKey;
      std::unique_ptr<bfrt::BfRtTableData> bfrtTableData;
      // Key field ids 
      bf_rt_id_t tunnel_type_field_id_ = 0;
      bf_rt_id_t tunnel_vni_field_id_ = 0;
      // Action Ids
      bf_rt_id_t set_tunnel_map_vrf_action_id_ = 0;
      // Data field Ids for fib hit action
      bf_rt_id_t set_tunnel_map_vrf_action_vrf_id_ = 0;  
      // bfrt related
      bf_rt_target_t dev_tgt_;
      std::shared_ptr<bfrt::BfRtSession> session_;
      const bfrt::BfRtTable *table_ptr_ = nullptr;
      uint64_t size_;
  };

  extern TunnelMapVrf *tunnel_map_vrf_table;
  
  int tunnel_map_vrf_init(const bfrt::BfRtInfo *bfrtInfo, bf_rt_target_t& dev_tgt, std::shared_ptr<bfrt::BfRtSession>& session);
}

using namespace shc;

// std::ostream &operator<<(std::ostream &os, const hal_tunnel_map_vrf_key &key);
// std::ostream &operator<<(std::ostream &os, const hal_tunnel_map_vrf_data &data);      