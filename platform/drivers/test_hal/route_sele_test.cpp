#include "test.hpp"

void test_route_sele(std::shared_ptr<bfrt::BfRtSession>& session){
  printf("====== Test test_route_sele ======\n");
  hal_route_sele_key key1 = {.vni=1, .ip_src_addr=0xA000001, .ip_dst_addr=0xA0000A};
  hal_route_data data1 = {.nhop_id=1};
  hal_route_sele_key key2 = {.vni=2, .ip_src_addr=0xA000001, .ip_dst_addr=0xA0000A};
  hal_route_data data2 = {.nhop_id=2};
  hal_route_sele_key key3 = {.vni=3, .ip_src_addr=0xA000001, .ip_dst_addr=0xA0000A};
  hal_route_data data3 = {.nhop_id=3};
  hal_route_sele_key key4 = {.vni=4, .ip_src_addr=0xA000001, .ip_dst_addr=0xA0000A};
  hal_route_data data4 = {.nhop_id=4};

  hal_route_sele_entry_add(&key1, &data1);
  hal_route_sele_entry_add(&key2, &data2);
  hal_route_sele_entry_add(&key3, &data3);
  hal_route_sele_entry_add(&key4, &data4);

  std::cout << "Add finished" << std::endl;
  session->sessionCompleteOperations();

  hal_route_data get_data;
  hal_route_sele_entry_get(&key1, &get_data);
  printf("key: {%d, %d, %d}, data: {nhop_id: %d}\n", 
      key1.vni, key1.ip_src_addr, key1.ip_dst_addr,
      get_data.nhop_id);
  hal_route_sele_entry_get(&key2, &get_data);
  
  printf("key: {%d, %d, %d}, data: {nhop_id: %d}\n", 
      key2.vni, key2.ip_src_addr, key2.ip_dst_addr,
      get_data.nhop_id);
  hal_route_sele_entry_get(&key3, &get_data);
  printf("key: {%d, %d, %d}, data: {nhop_id: %d}\n", 
      key3.vni, key3.ip_src_addr, key3.ip_dst_addr,
      get_data.nhop_id);
  hal_route_sele_entry_get(&key4, &get_data);
  printf("key: {%d, %d, %d}, data: {nhop_id: %d}\n", 
      key4.vni, key4.ip_src_addr, key4.ip_dst_addr,
      get_data.nhop_id);
  
  hal_route_sele_entry_mod(&key1, &data2);
  hal_route_sele_entry_mod(&key2, &data3);
  hal_route_sele_entry_mod(&key3, &data4);
  hal_route_sele_entry_mod(&key4, &data1);

  std::cout << "Mod finished" << std::endl;
  session->sessionCompleteOperations();
  
  hal_route_sele_entry_get(&key1, &get_data);
  printf("key: {%d, %d, %d}, data: {nhop_id: %d}\n", 
      key1.vni, key1.ip_src_addr, key1.ip_dst_addr,
      get_data.nhop_id);
  hal_route_sele_entry_get(&key2, &get_data);
  printf("key: {%d, %d, %d}, data: {nhop_id: %d}\n", 
      key2.vni, key2.ip_src_addr, key2.ip_dst_addr,
      get_data.nhop_id);
  hal_route_sele_entry_get(&key3, &get_data);
  printf("key: {%d, %d, %d}, data: {nhop_id: %d}\n", 
      key3.vni, key3.ip_src_addr, key3.ip_dst_addr,
      get_data.nhop_id);
  hal_route_sele_entry_get(&key4, &get_data);
  printf("key: {%d, %d, %d}, data: {nhop_id: %d}\n", 
      key4.vni, key4.ip_src_addr, key4.ip_dst_addr,
      get_data.nhop_id);
  
  uint32_t entry_count;
  hal_route_sele_table_usage_get(&entry_count);
  printf("table usage = %d\n", entry_count);
  uint32_t size_;
  hal_route_sele_table_size_get(&size_);
  printf("table size = %d\n", size_);

  hal_route_sele_entry_del(&key1);
  hal_route_sele_entry_del(&key2);
  hal_route_sele_entry_del(&key3);
  hal_route_sele_entry_del(&key4);

  printf("entry delete finished\n");
  printf("====== Test route_sele end ======\n");
}