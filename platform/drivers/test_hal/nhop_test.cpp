#include "test.hpp"

void test_nhop(std::shared_ptr<bfrt::BfRtSession>& session){
  printf("====== Test test_nhop begin ======\n");
  uint32_t oid1;
  uint32_t oid2;
  uint32_t oid3;
  uint32_t oid4;
  hal_nhop_alloc_oid(&oid1);
  hal_nhop_alloc_oid(&oid2);
  hal_nhop_alloc_oid(&oid3);
  hal_nhop_alloc_oid(&oid4);

  printf("oid get: %d\n", oid1);
  printf("oid get: %d\n", oid2);
  printf("oid get: %d\n", oid3);
  printf("oid get: %d\n", oid4);

  hal_nhop_key key1 = {.nhop_id=oid1};
  hal_nhop_data data1 = {.tunnel_key=11, .local=0xD00001, .remote=0xD0000B, .neigh_id=1};
  hal_nhop_key key2 = {.nhop_id=oid2};
  hal_nhop_data data2 = {.tunnel_key=22, .local=0xD00001, .remote=0xD0000B, .neigh_id=2};
  hal_nhop_key key3 = {.nhop_id=oid3};
  hal_nhop_data data3 = {.tunnel_key=33, .local=0xD00001, .remote=0xD0000B, .neigh_id=3};
  hal_nhop_key key4 = {.nhop_id=oid4};
  hal_nhop_data data4 = {.tunnel_key=44, .local=0xD00001, .remote=0xD0000B, .neigh_id=4};

  hal_nhop_entry_add(&key1, &data1);
  hal_nhop_entry_add(&key2, &data2);
  hal_nhop_entry_add(&key3, &data3);
  hal_nhop_entry_add(&key4, &data4);

  std::cout << "Add finished" << std::endl;
  session->sessionCompleteOperations();

  hal_nhop_data get_data;
  hal_nhop_entry_get(&key1, &get_data);
  printf("key: {%d}, data: {tunel_key: %d, local: %d, remote: %d, neigh_id: %d}\n", 
      key1.nhop_id,
      get_data.tunnel_key, get_data.local, get_data.remote, get_data.neigh_id);
  hal_nhop_entry_get(&key2, &get_data);
  printf("key: {%d}, data: {tunel_key: %d, local: %d, remote: %d, neigh_id: %d}\n", 
      key2.nhop_id, 
      get_data.tunnel_key, get_data.local, get_data.remote, get_data.neigh_id);
  hal_nhop_entry_get(&key3, &get_data);
  printf("key: {%d}, data: {tunel_key: %d, local: %d, remote: %d, neigh_id: %d}\n", 
      key3.nhop_id, 
      get_data.tunnel_key, get_data.local, get_data.remote, get_data.neigh_id);
  hal_nhop_entry_get(&key4, &get_data);
  printf("key: {%d}, data: {tunel_key: %d, local: %d, remote: %d, neigh_id: %d}\n", 
      key4.nhop_id, 
      get_data.tunnel_key, get_data.local, get_data.remote, get_data.neigh_id);
  
  hal_nhop_entry_mod(&key1, &data2);
  hal_nhop_entry_mod(&key2, &data3);
  hal_nhop_entry_mod(&key3, &data4);
  hal_nhop_entry_mod(&key4, &data1);

  std::cout << "Mod finished" << std::endl;
  session->sessionCompleteOperations();
  
  hal_nhop_entry_get(&key1, &get_data);
  printf("key: {%d}, data: {tunel_key: %d, local: %d, remote: %d, neigh_id: %d}\n", 
      key1.nhop_id,
      get_data.tunnel_key, get_data.local, get_data.remote, get_data.neigh_id);
  hal_nhop_entry_get(&key2, &get_data);
  printf("key: {%d}, data: {tunel_key: %d, local: %d, remote: %d, neigh_id: %d}\n", 
      key2.nhop_id, 
      get_data.tunnel_key, get_data.local, get_data.remote, get_data.neigh_id);
  hal_nhop_entry_get(&key3, &get_data);
  printf("key: {%d}, data: {tunel_key: %d, local: %d, remote: %d, neigh_id: %d}\n", 
      key3.nhop_id, 
      get_data.tunnel_key, get_data.local, get_data.remote, get_data.neigh_id);
  hal_nhop_entry_get(&key4, &get_data);
  printf("key: {%d}, data: {tunel_key: %d, local: %d, remote: %d, neigh_id: %d}\n", 
      key4.nhop_id, 
      get_data.tunnel_key, get_data.local, get_data.remote, get_data.neigh_id);
  
  uint32_t entry_count;
  hal_nhop_table_usage_get(&entry_count);
  printf("table usage = %d\n", entry_count);
  uint32_t size_;
  hal_nhop_table_size_get(&size_);
  printf("table size = %d\n", size_);

  hal_nhop_entry_del(&key1);
  hal_nhop_entry_del(&key2);
  hal_nhop_entry_del(&key3);
  hal_nhop_entry_del(&key4);

  printf("entry delete finished\n");
  printf("====== Test nhop end ======\n");
}