#pragma once
#include <bf_rt/bf_rt.hpp>
#include <stdint.h>
// #include "utils/log.h"
#include "base_table.hpp"

extern "C" {
#include "hal_table_type.h"
}

namespace shc {
  class PortForMac : public BaseTable {
    public:
      PortForMac(){}  
      PortForMac(const bfrt::BfRtInfo *bfrtInfo, bf_rt_target_t& dev_tgt, std::shared_ptr<bfrt::BfRtSession>& session);
      hal_status_t tableEntryAddModify(const hal_port_for_mac_key *key, const hal_port_for_mac_data *data, bool is_add);
      hal_status_t tableEntryGet(const hal_port_for_mac_key *key, hal_port_for_mac_data *data);
      hal_status_t tableEntryDel(const hal_port_for_mac_key *key);  
      hal_status_t tableUsageGet(uint32_t *entry_count);
      hal_status_t tableSizeGet(uint32_t *size);  

    private:
      void KeySetUp(const hal_port_for_mac_key &key,
                  bfrt::BfRtTableKey *table_key);
      void DataSetUp(const hal_port_for_mac_data &data,
                    bfrt::BfRtTableData *table_data);
      std::unique_ptr<bfrt::BfRtTableKey> bfrtTableKey;
      std::unique_ptr<bfrt::BfRtTableData> bfrtTableData;
      // Key field ids 
      bf_rt_id_t port_field_id_ = 0;

      // Action Ids
      bf_rt_id_t set_port_for_mac_action_id_ = 0;
      bf_rt_id_t set_fake_mac_for_outer_action_id_ = 0;
      // Data field Ids for action
      bf_rt_id_t set_port_for_mac_action_smac_id_ = 0;  
      bf_rt_id_t set_port_for_mac_action_dmac_id_ = 0;  
      // bfrt related
      bf_rt_target_t dev_tgt_;
      std::shared_ptr<bfrt::BfRtSession> session_;
      const bfrt::BfRtTable *table_ptr_ = nullptr;
      uint64_t size_;

      bool mac_array_to_uint64(uint64_t& dst, const uint8_t from[6]);
      bool uint64_to_mac_array(uint8_t* dst, uint64_t from);
  };

  extern PortForMac *port_for_mac_table;
  
  int port_for_mac_init(const bfrt::BfRtInfo *bfrtInfo, bf_rt_target_t& dev_tgt, std::shared_ptr<bfrt::BfRtSession>& session);
}

using namespace shc;

// std::ostream &operator<<(std::ostream &os, const hal_port_for_mac_key &key);
// std::ostream &operator<<(std::ostream &os, const hal_port_for_mac_data &data);      