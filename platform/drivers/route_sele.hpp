#pragma once
#include <bf_rt/bf_rt.hpp>
#include <stdint.h>
// #include "utils/log.h"
#include "base_table.hpp"

extern "C" {
#include "hal_table_type.h"
}

namespace shc {
  class RouteSele : public BaseTable {
    public:
      RouteSele(){}  
      RouteSele(const bfrt::BfRtInfo *bfrtInfo, bf_rt_target_t& dev_tgt, std::shared_ptr<bfrt::BfRtSession>& session);
      hal_status_t tableEntryAddModify(const hal_route_sele_key *key, const hal_route_data *data, bool is_add);
      hal_status_t tableEntryGet(const hal_route_sele_key *key, hal_route_data *data);
      hal_status_t tableEntryDel(const hal_route_sele_key *key);  
      hal_status_t tableUsageGet(uint32_t *entry_count);
      hal_status_t tableSizeGet(uint32_t *size);  

    private:
      void KeySetUp(const hal_route_sele_key &key,
                  bfrt::BfRtTableKey *table_key);
      void DataSetUp(const hal_route_data &data,
                    bfrt::BfRtTableData *table_data);
      std::unique_ptr<bfrt::BfRtTableKey> bfrtTableKey;
      std::unique_ptr<bfrt::BfRtTableData> bfrtTableData;
      // Key field ids 
      // bf_rt_id_t vrf_field_id_ = 0;
      bf_rt_id_t vni_field_id_ = 0;
      bf_rt_id_t ip_src_field_id_ = 0;
      bf_rt_id_t ip_dst_field_id_ = 0;
      // Action Ids
      bf_rt_id_t set_route_sele_for_nexthop_action_id_ = 0;
      bf_rt_id_t set_route_sele_for_miss_action_id_ = 0;
      // Data field Ids for action
      bf_rt_id_t set_route_sele_for_nexthop_action_nhop_id_id_ = 0;  

      bf_rt_id_t set_route_sele_for_nexthop_action_ttl_id_ = 0;
      // bfrt related
      bf_rt_target_t dev_tgt_;
      std::shared_ptr<bfrt::BfRtSession> session_;
      const bfrt::BfRtTable *table_ptr_ = nullptr;
      uint64_t size_;
      const uint64_t entry_ttl = 200; 
  };

  extern RouteSele *route_sele_table;
  
  int route_sele_init(const bfrt::BfRtInfo *bfrtInfo, bf_rt_target_t& dev_tgt, std::shared_ptr<bfrt::BfRtSession>& session);
}

using namespace shc;

// std::ostream &operator<<(std::ostream &os, const hal_route_sele_key &key);
// std::ostream &operator<<(std::ostream &os, const hal_route_data &data);      