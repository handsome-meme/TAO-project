#include "test.hpp"

void test_neigh(std::shared_ptr<bfrt::BfRtSession>& session){
  printf("====== Test test_neigh begin ======\n");
  uint32_t oid1;
  uint32_t oid2;
  uint32_t oid3;
  uint32_t oid4;
  hal_neigh_alloc_oid(&oid1);
  hal_neigh_alloc_oid(&oid2);
  hal_neigh_alloc_oid(&oid3);
  hal_neigh_alloc_oid(&oid4);

  printf("oid get: %d\n", oid1);
  printf("oid get: %d\n", oid2);
  printf("oid get: %d\n", oid3);
  printf("oid get: %d\n", oid4);

  hal_neigh_key key1 = {.neigh_id=oid1};
  hal_neigh_data data1 = {.dmac={1,1,1,1,1,11}};
  hal_neigh_key key2 = {.neigh_id=oid2};
  hal_neigh_data data2 = {.dmac={2,2,2,2,2,22}};
  hal_neigh_key key3 = {.neigh_id=oid3};
  hal_neigh_data data3 = {.dmac={3,3,3,3,3,33}};
  hal_neigh_key key4 = {.neigh_id=oid4};
  hal_neigh_data data4 = {.dmac={4,4,4,4,4,44}};

  hal_neigh_entry_add(&key1, &data1);
  hal_neigh_entry_add(&key2, &data2);
  hal_neigh_entry_add(&key3, &data3);
  hal_neigh_entry_add(&key4, &data4);

  std::cout << "Add finished" << std::endl;
  session->sessionCompleteOperations();

  hal_neigh_data get_data;
  hal_neigh_entry_get(&key1, &get_data);
  printf("key: {%d}, data: {dmac: %d:%d:%d:%d:%d:%d}\n", 
      key1.neigh_id,
      get_data.dmac[0], get_data.dmac[1],
      get_data.dmac[2], get_data.dmac[3], get_data.dmac[4], get_data.dmac[5]);
  hal_neigh_entry_get(&key2, &get_data);
  printf("key: {%d}, data: {dmac: %d:%d:%d:%d:%d:%d}\n", 
      key2.neigh_id,
      get_data.dmac[0], get_data.dmac[1],
      get_data.dmac[2], get_data.dmac[3], get_data.dmac[4], get_data.dmac[5]);
  hal_neigh_entry_get(&key3, &get_data);
  printf("key: {%d}, data: {dmac: %d:%d:%d:%d:%d:%d}\n", 
      key3.neigh_id,
      get_data.dmac[0], get_data.dmac[1],
      get_data.dmac[2], get_data.dmac[3], get_data.dmac[4], get_data.dmac[5]);
  hal_neigh_entry_get(&key4, &get_data);
  printf("key: {%d}, data: {dmac: %d:%d:%d:%d:%d:%d}\n", 
      key4.neigh_id,
      get_data.dmac[0], get_data.dmac[1],
      get_data.dmac[2], get_data.dmac[3], get_data.dmac[4], get_data.dmac[5]);
  
  hal_neigh_entry_mod(&key1, &data2);
  hal_neigh_entry_mod(&key2, &data3);
  hal_neigh_entry_mod(&key3, &data4);
  hal_neigh_entry_mod(&key4, &data1);

  std::cout << "Mod finished" << std::endl;
  session->sessionCompleteOperations();
  
  hal_neigh_entry_get(&key1, &get_data);
  printf("key: {%d}, data: {dmac: %d:%d:%d:%d:%d:%d}\n", 
      key1.neigh_id,
      get_data.dmac[0], get_data.dmac[1],
      get_data.dmac[2], get_data.dmac[3], get_data.dmac[4], get_data.dmac[5]);
  hal_neigh_entry_get(&key2, &get_data);
  printf("key: {%d}, data: {dmac: %d:%d:%d:%d:%d:%d}\n", 
      key2.neigh_id,
      get_data.dmac[0], get_data.dmac[1],
      get_data.dmac[2], get_data.dmac[3], get_data.dmac[4], get_data.dmac[5]);
  hal_neigh_entry_get(&key3, &get_data);
  printf("key: {%d}, data: {dmac: %d:%d:%d:%d:%d:%d}\n", 
      key3.neigh_id,
      get_data.dmac[0], get_data.dmac[1],
      get_data.dmac[2], get_data.dmac[3], get_data.dmac[4], get_data.dmac[5]);
  hal_neigh_entry_get(&key4, &get_data);
  printf("key: {%d}, data: {dmac: %d:%d:%d:%d:%d:%d}\n", 
      key4.neigh_id,
      get_data.dmac[0], get_data.dmac[1],
      get_data.dmac[2], get_data.dmac[3], get_data.dmac[4], get_data.dmac[5]);
  
  uint32_t entry_count;
  hal_neigh_table_usage_get(&entry_count);
  printf("table usage = %d\n", entry_count);
  uint32_t size_;
  hal_neigh_table_size_get(&size_);
  printf("table size = %d\n", size_);

  hal_neigh_entry_del(&key1);
  hal_neigh_entry_del(&key2);
  hal_neigh_entry_del(&key3);
  hal_neigh_entry_del(&key4);

  printf("entry delete finished\n");
  printf("====== Test neigh end ======\n");
}