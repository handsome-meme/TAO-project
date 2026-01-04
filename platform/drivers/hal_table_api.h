#pragma once

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#include <tofino/bf_pal/pltfm_intf.h>
#include "hal_table_type.h"

hal_status_t hal_transaction_start(void);
hal_status_t hal_transaction_commit(void);
hal_status_t hal_transaction_abort(void);
hal_status_t hal_session_complete(void);

const char *hal_status_to_string(hal_status_t hal_status);


// -----------------------------
// tunnel_map_vrf table   shc
// -----------------------------
hal_status_t hal_tunnel_map_vrf_entry_add(const hal_tunnel_map_vrf_key *key, const hal_tunnel_map_vrf_data *data);
hal_status_t hal_tunnel_map_vrf_entry_mod(const hal_tunnel_map_vrf_key *key, const hal_tunnel_map_vrf_data *data);
hal_status_t hal_tunnel_map_vrf_entry_del(const hal_tunnel_map_vrf_key *key);
hal_status_t hal_tunnel_map_vrf_entry_get(const hal_tunnel_map_vrf_key *key, hal_tunnel_map_vrf_data *data);                                                 
hal_status_t hal_tunnel_map_vrf_table_usage_get(uint32_t *entry_count);
hal_status_t hal_tunnel_map_vrf_table_size_get(uint32_t *size);



// -----------------------------
// route table   shc
// -----------------------------
hal_status_t hal_route_entry_add(const hal_route_key *key, const hal_route_data *data);
hal_status_t hal_route_entry_mod(const hal_route_key *key, const hal_route_data *data);
hal_status_t hal_route_entry_del(const hal_route_key *key);
hal_status_t hal_route_entry_get(const hal_route_key *key, hal_route_data *data);                                                
hal_status_t hal_route_table_usage_get(uint32_t *entry_count);
hal_status_t hal_route_table_size_get(uint32_t *size);



// -----------------------------
// route_sele table   shc
// -----------------------------
hal_status_t hal_route_sele_entry_add(const hal_route_sele_key *key, const hal_route_data *data);
hal_status_t hal_route_sele_entry_mod(const hal_route_sele_key *key, const hal_route_data *data);
hal_status_t hal_route_sele_entry_del(const hal_route_sele_key *key);
hal_status_t hal_route_sele_entry_get(const hal_route_sele_key *key, hal_route_data *data);                                                
hal_status_t hal_route_sele_table_usage_get(uint32_t *entry_count);
hal_status_t hal_route_sele_table_size_get(uint32_t *size);



// -----------------------------
// nhop table   shc
// -----------------------------
hal_status_t hal_nhop_entry_add(const hal_nhop_key *key, const hal_nhop_data *data);
hal_status_t hal_nhop_entry_mod(const hal_nhop_key *key, const hal_nhop_data *data);
hal_status_t hal_nhop_entry_del(const hal_nhop_key *key);
hal_status_t hal_nhop_entry_get(const hal_nhop_key *key, hal_nhop_data *data);                                                 
hal_status_t hal_nhop_table_usage_get(uint32_t *entry_count);
hal_status_t hal_nhop_table_size_get(uint32_t *size);
hal_status_t hal_nhop_alloc_oid(uint32_t *oid);
hal_status_t hal_nhop_free_oid(uint32_t oid);



// -----------------------------
// neigh table   shc
// -----------------------------
hal_status_t hal_neigh_entry_add(const hal_neigh_key *key, const hal_neigh_data *data);
hal_status_t hal_neigh_entry_mod(const hal_neigh_key *key, const hal_neigh_data *data);
hal_status_t hal_neigh_entry_del(const hal_neigh_key *key);
hal_status_t hal_neigh_entry_get(const hal_neigh_key *key, hal_neigh_data *data);                                                 
hal_status_t hal_neigh_table_usage_get(uint32_t *entry_count);
hal_status_t hal_neigh_table_size_get(uint32_t *size);
hal_status_t hal_neigh_alloc_oid(uint32_t *oid);
hal_status_t hal_neigh_free_oid(uint32_t oid);




// -----------------------------
// port_for_mac table   shc
// -----------------------------
hal_status_t hal_port_for_mac_entry_add(const hal_port_for_mac_key *key, const hal_port_for_mac_data *data);
hal_status_t hal_port_for_mac_entry_mod(const hal_port_for_mac_key *key, const hal_port_for_mac_data *data);
hal_status_t hal_port_for_mac_entry_del(const hal_port_for_mac_key *key);
hal_status_t hal_port_for_mac_entry_get(const hal_port_for_mac_key *key, hal_port_for_mac_data *data);                                                 
hal_status_t hal_port_for_mac_table_usage_get(uint32_t *entry_count);
hal_status_t hal_port_for_mac_table_size_get(uint32_t *size);


void hal_init(int argc, char **argv);

#ifdef __cplusplus
}
#endif