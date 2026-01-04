#include "test.hpp"

void test_tunnel_map_vrf(std::shared_ptr<bfrt::BfRtSession>& session){
  printf("====== Test test_tunnelmapvrf begin ======\n");
  hal_tunnel_map_vrf_key key1 = {TYPE_TUNNEL_TYPE_VXLAN, 1};
  hal_tunnel_map_vrf_data data1 = {1};
  hal_tunnel_map_vrf_key key2 = {TYPE_TUNNEL_TYPE_VXLAN, 2};
  hal_tunnel_map_vrf_data data2 = {2};
  hal_tunnel_map_vrf_key key3 = {TYPE_TUNNEL_TYPE_VXLAN, 3};
  hal_tunnel_map_vrf_data data3 = {3};
  hal_tunnel_map_vrf_key key4 = {TYPE_TUNNEL_TYPE_VXLAN, 4};
  hal_tunnel_map_vrf_data data4 = {4};

  hal_tunnel_map_vrf_entry_add(&key1, &data1);
  hal_tunnel_map_vrf_entry_add(&key2, &data2);
  hal_tunnel_map_vrf_entry_add(&key3, &data3);
  hal_tunnel_map_vrf_entry_add(&key4, &data4);

  std::cout << "Add finished" << std::endl;
  session->sessionCompleteOperations();

  hal_tunnel_map_vrf_data get_data;
  hal_tunnel_map_vrf_entry_get(&key1, &get_data);
  printf("key: {%d, %d}, data: {vrf: %d}\n", 
      key1.tunnel_type, key1.tunnel_key, 
      get_data.vrf);
  hal_tunnel_map_vrf_entry_get(&key2, &get_data);
  printf("key: {%d, %d}, data: {vrf: %d}\n", 
      key2.tunnel_type, key2.tunnel_key, 
      get_data.vrf);
  hal_tunnel_map_vrf_entry_get(&key3, &get_data);
  printf("key: {%d, %d}, data: {vrf: %d}\n", 
      key3.tunnel_type, key3.tunnel_key, 
      get_data.vrf);
  hal_tunnel_map_vrf_entry_get(&key4, &get_data);
  printf("key: {%d, %d}, data: {vrf: %d}\n", 
      key4.tunnel_type, key4.tunnel_key, 
      get_data.vrf);
  
  hal_tunnel_map_vrf_entry_mod(&key1, &data2);
  hal_tunnel_map_vrf_entry_mod(&key2, &data3);
  hal_tunnel_map_vrf_entry_mod(&key3, &data4);
  hal_tunnel_map_vrf_entry_mod(&key4, &data1);

  std::cout << "Mod finished" << std::endl;
  session->sessionCompleteOperations();
  
  hal_tunnel_map_vrf_entry_get(&key1, &get_data);
  printf("key: {%d, %d}, data: {vrf: %d}\n", 
      key1.tunnel_type, key1.tunnel_key, 
      get_data.vrf);
  hal_tunnel_map_vrf_entry_get(&key2, &get_data);
  printf("key: {%d, %d}, data: {vrf: %d}\n", 
      key2.tunnel_type, key2.tunnel_key, 
      get_data.vrf);
  hal_tunnel_map_vrf_entry_get(&key3, &get_data);
  printf("key: {%d, %d}, data: {vrf: %d}\n", 
      key3.tunnel_type, key3.tunnel_key, 
      get_data.vrf);
  hal_tunnel_map_vrf_entry_get(&key4, &get_data);
  printf("key: {%d, %d}, data: {vrf: %d}\n", 
      key4.tunnel_type, key4.tunnel_key, 
      get_data.vrf);
  
  uint32_t entry_count;
  hal_tunnel_map_vrf_table_usage_get(&entry_count);
  printf("table usage = %d\n", entry_count);
  uint32_t size_;
  hal_tunnel_map_vrf_table_size_get(&size_);
  printf("table size = %d\n", size_);

  hal_tunnel_map_vrf_entry_del(&key1);
  hal_tunnel_map_vrf_entry_del(&key2);
  hal_tunnel_map_vrf_entry_del(&key3);
  hal_tunnel_map_vrf_entry_del(&key4);

  printf("entry delete finished\n");
  printf("====== Test tunnel_map_vrf end ======\n");
}