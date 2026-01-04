#include "hal_topk_api.hpp"
#include "hal_table_api.h"
#include "hal_utils.hpp"
#include "hal.hpp"


const char *hal_status_to_string(hal_status_t hal_status) {
  if (hal_status < 0)
    hal_status = ~hal_status;
  return hal_err_str(hal_status);
}

// -----------------------------
// tunnel_map_vrf table   shc
// -----------------------------
int hal_topk_dump(flow_table_t * &old_sets, flow_table_t * &new_sets){
  shc::space_saving->tableMutexLock();
  auto status = shc::space_saving->top_k_dump(old_table, new_table); 
  shc::space_saving->tableMutexUnlock();

  old_sets = &old_table;
  new_sets = &new_table;
  return status;
}
// hal_status_t topk_diff_cmp(){
//   shc::space_saving->tableMutexLock();
//   auto status = shc::space_saving->diff_cmp(); 
//   shc::space_saving->tableMutexUnlock();
//   return status;
// }

// -----------------------------
// tunnel_map_vrf table   shc
// -----------------------------
hal_status_t hal_tunnel_map_vrf_entry_add(const hal_tunnel_map_vrf_key *key,
                            const hal_tunnel_map_vrf_data *data){
  shc::tunnel_map_vrf_table->tableMutexLock();
  auto status = shc::tunnel_map_vrf_table->tableEntryAddModify(key, data, true); 
  shc::tunnel_map_vrf_table->tableMutexUnlock();
  return status;                           
}
hal_status_t hal_tunnel_map_vrf_entry_mod(const hal_tunnel_map_vrf_key *key,
                            const hal_tunnel_map_vrf_data *data){
  shc::tunnel_map_vrf_table->tableMutexLock();
  auto status = shc::tunnel_map_vrf_table->tableEntryAddModify(key, data, false);  
  shc::tunnel_map_vrf_table->tableMutexUnlock(); 
  return status;                                                     
}
hal_status_t hal_tunnel_map_vrf_entry_del(const hal_tunnel_map_vrf_key *key){
  shc::tunnel_map_vrf_table->tableMutexLock();
  auto status = shc::tunnel_map_vrf_table->tableEntryDel(key);
  shc::tunnel_map_vrf_table->tableMutexUnlock();
  return status;                           
}
hal_status_t hal_tunnel_map_vrf_entry_get(const hal_tunnel_map_vrf_key *key, hal_tunnel_map_vrf_data *data){
  shc::tunnel_map_vrf_table->tableMutexLock();
  auto status = shc::tunnel_map_vrf_table->tableEntryGet(key, data);
  shc::tunnel_map_vrf_table->tableMutexUnlock();
  return status;                           
}                                                  
hal_status_t hal_tunnel_map_vrf_table_usage_get(uint32_t *entry_count){
  shc::tunnel_map_vrf_table->tableMutexLock();
  auto status = shc::tunnel_map_vrf_table->tableUsageGet(entry_count);
  shc::tunnel_map_vrf_table->tableMutexUnlock();
  return status;                           
}
hal_status_t hal_tunnel_map_vrf_table_size_get(uint32_t *size){
  shc::tunnel_map_vrf_table->tableMutexLock();
  auto status = shc::tunnel_map_vrf_table->tableSizeGet(size);
  shc::tunnel_map_vrf_table->tableMutexUnlock();
  return status;                           
}

// -----------------------------
// route table   shc
// -----------------------------
hal_status_t hal_route_entry_add(const hal_route_key *key,
                            const hal_route_data *data){
  shc::route_table->tableMutexLock();
  auto status = shc::route_table->tableEntryAddModify(key, data, true); 
  shc::route_table->tableMutexUnlock();
  return status;                           
}
hal_status_t hal_route_entry_mod(const hal_route_key *key,
                            const hal_route_data *data){
  shc::route_table->tableMutexLock();
  auto status = shc::route_table->tableEntryAddModify(key, data, false);  
  shc::route_table->tableMutexUnlock(); 
  return status;                                                     
}
hal_status_t hal_route_entry_del(const hal_route_key *key){
  shc::route_table->tableMutexLock();
  auto status = shc::route_table->tableEntryDel(key);
  shc::route_table->tableMutexUnlock();
  return status;                           
}
hal_status_t hal_route_entry_get(const hal_route_key *key, hal_route_data *data){
  shc::route_table->tableMutexLock();
  auto status = shc::route_table->tableEntryGet(key, data);
  shc::route_table->tableMutexUnlock();
  return status;                           
}                                                  
hal_status_t hal_route_table_usage_get(uint32_t *entry_count){
  shc::route_table->tableMutexLock();
  auto status = shc::route_table->tableUsageGet(entry_count);
  shc::route_table->tableMutexUnlock();
  return status;                           
}
hal_status_t hal_route_table_size_get(uint32_t *size){
  shc::route_table->tableMutexLock();
  auto status = shc::route_table->tableSizeGet(size);
  shc::route_table->tableMutexUnlock();
  return status;                           
}


// -----------------------------
// route_sele table   shc
// -----------------------------
hal_status_t hal_route_sele_entry_add(const hal_route_sele_key *key,
                            const hal_route_data *data){
  shc::route_sele_table->tableMutexLock();
  auto status = shc::route_sele_table->tableEntryAddModify(key, data, true); 
  shc::route_sele_table->tableMutexUnlock();
  return status;                           
}
hal_status_t hal_route_sele_entry_mod(const hal_route_sele_key *key,
                            const hal_route_data *data){
  shc::route_sele_table->tableMutexLock();
  auto status = shc::route_sele_table->tableEntryAddModify(key, data, false);  
  shc::route_sele_table->tableMutexUnlock(); 
  return status;                                                     
}
hal_status_t hal_route_sele_entry_del(const hal_route_sele_key *key){
  shc::route_sele_table->tableMutexLock();
  auto status = shc::route_sele_table->tableEntryDel(key);
  shc::route_sele_table->tableMutexUnlock();
  return status;                           
}
hal_status_t hal_route_sele_entry_get(const hal_route_sele_key *key, hal_route_data *data){
  shc::route_sele_table->tableMutexLock();
  auto status = shc::route_sele_table->tableEntryGet(key, data);
  shc::route_sele_table->tableMutexUnlock();
  return status;                           
}                                                  
hal_status_t hal_route_sele_table_usage_get(uint32_t *entry_count){
  shc::route_sele_table->tableMutexLock();
  auto status = shc::route_sele_table->tableUsageGet(entry_count);
  shc::route_sele_table->tableMutexUnlock();
  return status;                           
}
hal_status_t hal_route_sele_table_size_get(uint32_t *size){
  shc::route_sele_table->tableMutexLock();
  auto status = shc::route_sele_table->tableSizeGet(size);
  shc::route_sele_table->tableMutexUnlock();
  return status;                           
}

// -----------------------------
// nhop table   shc
// -----------------------------
hal_status_t hal_nhop_entry_add(const hal_nhop_key *key,
                            const hal_nhop_data *data){
  shc::nhop_table->tableMutexLock();
  auto status = shc::nhop_table->tableEntryAddModify(key, data, true); 
  shc::nhop_table->tableMutexUnlock();
  return status;                           
}
hal_status_t hal_nhop_entry_mod(const hal_nhop_key *key,
                            const hal_nhop_data *data){
  shc::nhop_table->tableMutexLock();
  auto status = shc::nhop_table->tableEntryAddModify(key, data, false);  
  shc::nhop_table->tableMutexUnlock(); 
  return status;                                                     
}
hal_status_t hal_nhop_entry_del(const hal_nhop_key *key){
  shc::nhop_table->tableMutexLock();
  auto status = shc::nhop_table->tableEntryDel(key);
  shc::nhop_table->tableMutexUnlock();
  return status;                           
}
hal_status_t hal_nhop_entry_get(const hal_nhop_key *key, hal_nhop_data *data){
  shc::nhop_table->tableMutexLock();
  auto status = shc::nhop_table->tableEntryGet(key, data);
  shc::nhop_table->tableMutexUnlock();
  return status;                           
}                                                  
hal_status_t hal_nhop_table_usage_get(uint32_t *entry_count){
  shc::nhop_table->tableMutexLock();
  auto status = shc::nhop_table->tableUsageGet(entry_count);
  shc::nhop_table->tableMutexUnlock();
  return status;                           
}
hal_status_t hal_nhop_table_size_get(uint32_t *size){
  shc::nhop_table->tableMutexLock();
  auto status = shc::nhop_table->tableSizeGet(size);
  shc::nhop_table->tableMutexUnlock();
  return status;                           
}
hal_status_t hal_nhop_alloc_oid(uint32_t *oid){
  shc::nhop_table->tableMutexLock();
  auto status = shc::nhop_table->AllocOid(oid);
  shc::nhop_table->tableMutexUnlock();
  return status;   
}
hal_status_t hal_nhop_free_oid(uint32_t oid){
  shc::nhop_table->tableMutexLock();
  auto status = shc::nhop_table->FreeOid(oid);
  shc::nhop_table->tableMutexUnlock();
  return status;   
}


// -----------------------------
// neigh table   shc
// -----------------------------
hal_status_t hal_neigh_entry_add(const hal_neigh_key *key,
                            const hal_neigh_data *data){
  shc::neigh_table->tableMutexLock();
  auto status = shc::neigh_table->tableEntryAddModify(key, data, true); 
  shc::neigh_table->tableMutexUnlock();
  return status;                           
}
hal_status_t hal_neigh_entry_mod(const hal_neigh_key *key,
                            const hal_neigh_data *data){
  shc::neigh_table->tableMutexLock();
  auto status = shc::neigh_table->tableEntryAddModify(key, data, false);  
  shc::neigh_table->tableMutexUnlock(); 
  return status;                                                     
}
hal_status_t hal_neigh_entry_del(const hal_neigh_key *key){
  shc::neigh_table->tableMutexLock();
  auto status = shc::neigh_table->tableEntryDel(key);
  shc::neigh_table->tableMutexUnlock();
  return status;                           
}
hal_status_t hal_neigh_entry_get(const hal_neigh_key *key, hal_neigh_data *data){
  shc::neigh_table->tableMutexLock();
  auto status = shc::neigh_table->tableEntryGet(key, data);
  shc::neigh_table->tableMutexUnlock();
  return status;                           
}                                                  
hal_status_t hal_neigh_table_usage_get(uint32_t *entry_count){
  shc::neigh_table->tableMutexLock();
  auto status = shc::neigh_table->tableUsageGet(entry_count);
  shc::neigh_table->tableMutexUnlock();
  return status;                           
}
hal_status_t hal_neigh_table_size_get(uint32_t *size){
  shc::neigh_table->tableMutexLock();
  auto status = shc::neigh_table->tableSizeGet(size);
  shc::neigh_table->tableMutexUnlock();
  return status;                           
}
hal_status_t hal_neigh_alloc_oid(uint32_t *oid){
  shc::neigh_table->tableMutexLock();
  auto status = shc::neigh_table->AllocOid(oid);
  shc::neigh_table->tableMutexUnlock();
  return status;   
}
hal_status_t hal_neigh_free_oid(uint32_t oid){
  shc::neigh_table->tableMutexLock();
  auto status = shc::neigh_table->FreeOid(oid);
  shc::neigh_table->tableMutexUnlock();
  return status;   
}


// -----------------------------
// port_for_mac table   shc
// -----------------------------
hal_status_t hal_port_for_mac_entry_add(const hal_port_for_mac_key *key,
                            const hal_port_for_mac_data *data){
  shc::port_for_mac_table->tableMutexLock();
  auto status = shc::port_for_mac_table->tableEntryAddModify(key, data, true); 
  shc::port_for_mac_table->tableMutexUnlock();
  return status;                           
}
hal_status_t hal_port_for_mac_entry_mod(const hal_port_for_mac_key *key,
                            const hal_port_for_mac_data *data){
  shc::port_for_mac_table->tableMutexLock();
  auto status = shc::port_for_mac_table->tableEntryAddModify(key, data, false);  
  shc::port_for_mac_table->tableMutexUnlock(); 
  return status;                                                     
}
hal_status_t hal_port_for_mac_entry_del(const hal_port_for_mac_key *key){
  shc::port_for_mac_table->tableMutexLock();
  auto status = shc::port_for_mac_table->tableEntryDel(key);
  shc::port_for_mac_table->tableMutexUnlock();
  return status;                           
}
hal_status_t hal_port_for_mac_entry_get(const hal_port_for_mac_key *key, hal_port_for_mac_data *data){
  shc::port_for_mac_table->tableMutexLock();
  auto status = shc::port_for_mac_table->tableEntryGet(key, data);
  shc::port_for_mac_table->tableMutexUnlock();
  return status;                           
}                                                  
hal_status_t hal_port_for_mac_table_usage_get(uint32_t *entry_count){
  shc::port_for_mac_table->tableMutexLock();
  auto status = shc::port_for_mac_table->tableUsageGet(entry_count);
  shc::port_for_mac_table->tableMutexUnlock();
  return status;                           
}
hal_status_t hal_port_for_mac_table_size_get(uint32_t *size){
  shc::port_for_mac_table->tableMutexLock();
  auto status = shc::port_for_mac_table->tableSizeGet(size);
  shc::port_for_mac_table->tableMutexUnlock();
  return status;                           
}