#include "test.hpp"

void test_port_for_mac(std::shared_ptr<bfrt::BfRtSession>& session){
  printf("====== Test test_portformac begin ======\n");
  hal_port_for_mac_key key1 = {.port=1};
  hal_port_for_mac_data data1 = {.smac={0,0,0,0,0,1}, .dmac={1,0,0,0,0,0}};
  hal_port_for_mac_key key2 = {.port=2};
  hal_port_for_mac_data data2 = {.smac={0,0,0,0,0,2}, .dmac={2,0,0,0,0,0}};
  hal_port_for_mac_key key3 = {.port=3};
  hal_port_for_mac_data data3 = {.smac={0,0,0,0,0,3}, .dmac={3,0,0,0,0,0}};
  hal_port_for_mac_key key4 = {.port=4};
  hal_port_for_mac_data data4 = {.smac={0,0,0,0,0,4}, .dmac={4,0,0,0,0,0}};

  hal_port_for_mac_entry_add(&key1, &data1);
  hal_port_for_mac_entry_add(&key2, &data2);
  hal_port_for_mac_entry_add(&key3, &data3);
  hal_port_for_mac_entry_add(&key4, &data4);

  std::cout << "Add finished" << std::endl;
  session->sessionCompleteOperations();

  hal_port_for_mac_data get_data;
  hal_port_for_mac_entry_get(&key1, &get_data);
  printf("key: {%d}, data: {smac: %d:%d:%d:%d:%d:%d , dmac: %d:%d:%d:%d:%d:%d}\n", 
      key1.port, 
      get_data.smac[0], get_data.smac[1],
      get_data.smac[2], get_data.smac[3], get_data.smac[4], get_data.smac[5],
      get_data.dmac[0], get_data.dmac[1],
      get_data.dmac[2], get_data.dmac[3], get_data.dmac[4], get_data.dmac[5]);
  hal_port_for_mac_entry_get(&key2, &get_data);
  printf("key: {%d}, data: {smac: %d:%d:%d:%d:%d:%d , dmac: %d:%d:%d:%d:%d:%d}\n", 
      key2.port, 
      get_data.smac[0], get_data.smac[1],
      get_data.smac[2], get_data.smac[3], get_data.smac[4], get_data.smac[5],
      get_data.dmac[0], get_data.dmac[1],
      get_data.dmac[2], get_data.dmac[3], get_data.dmac[4], get_data.dmac[5]);
  hal_port_for_mac_entry_get(&key3, &get_data);
  printf("key: {%d}, data: {smac: %d:%d:%d:%d:%d:%d , dmac: %d:%d:%d:%d:%d:%d}\n", 
      key3.port, 
      get_data.smac[0], get_data.smac[1],
      get_data.smac[2], get_data.smac[3], get_data.smac[4], get_data.smac[5],
      get_data.dmac[0], get_data.dmac[1],
      get_data.dmac[2], get_data.dmac[3], get_data.dmac[4], get_data.dmac[5]);
  hal_port_for_mac_entry_get(&key4, &get_data);
   printf("key: {%d}, data: {smac: %d:%d:%d:%d:%d:%d , dmac: %d:%d:%d:%d:%d:%d}\n", 
      key4.port, 
      get_data.smac[0], get_data.smac[1],
      get_data.smac[2], get_data.smac[3], get_data.smac[4], get_data.smac[5],
      get_data.dmac[0], get_data.dmac[1],
      get_data.dmac[2], get_data.dmac[3], get_data.dmac[4], get_data.dmac[5]);
  
  hal_port_for_mac_entry_mod(&key1, &data2);
  hal_port_for_mac_entry_mod(&key2, &data3);
  hal_port_for_mac_entry_mod(&key3, &data4);
  hal_port_for_mac_entry_mod(&key4, &data1);

  std::cout << "Mod finished" << std::endl;
  session->sessionCompleteOperations();
  
  hal_port_for_mac_entry_get(&key1, &get_data);
  printf("key: {%d}, data: {smac: %d:%d:%d:%d:%d:%d , dmac: %d:%d:%d:%d:%d:%d}\n", 
      key1.port, 
      get_data.smac[0], get_data.smac[1],
      get_data.smac[2], get_data.smac[3], get_data.smac[4], get_data.smac[5],
      get_data.dmac[0], get_data.dmac[1],
      get_data.dmac[2], get_data.dmac[3], get_data.dmac[4], get_data.dmac[5]);
  hal_port_for_mac_entry_get(&key2, &get_data);
  printf("key: {%d}, data: {smac: %d:%d:%d:%d:%d:%d , dmac: %d:%d:%d:%d:%d:%d}\n", 
      key2.port, 
      get_data.smac[0], get_data.smac[1],
      get_data.smac[2], get_data.smac[3], get_data.smac[4], get_data.smac[5],
      get_data.dmac[0], get_data.dmac[1],
      get_data.dmac[2], get_data.dmac[3], get_data.dmac[4], get_data.dmac[5]);
  hal_port_for_mac_entry_get(&key3, &get_data);
  printf("key: {%d}, data: {smac: %d:%d:%d:%d:%d:%d , dmac: %d:%d:%d:%d:%d:%d}\n", 
      key3.port, 
      get_data.smac[0], get_data.smac[1],
      get_data.smac[2], get_data.smac[3], get_data.smac[4], get_data.smac[5],
      get_data.dmac[0], get_data.dmac[1],
      get_data.dmac[2], get_data.dmac[3], get_data.dmac[4], get_data.dmac[5]);
  hal_port_for_mac_entry_get(&key4, &get_data);
   printf("key: {%d}, data: {smac: %d:%d:%d:%d:%d:%d , dmac: %d:%d:%d:%d:%d:%d}\n", 
      key4.port, 
      get_data.smac[0], get_data.smac[1],
      get_data.smac[2], get_data.smac[3], get_data.smac[4], get_data.smac[5],
      get_data.dmac[0], get_data.dmac[1],
      get_data.dmac[2], get_data.dmac[3], get_data.dmac[4], get_data.dmac[5]);
  
  uint32_t entry_count;
  hal_port_for_mac_table_usage_get(&entry_count);
  printf("table usage = %d\n", entry_count);
  uint32_t size_;
  hal_port_for_mac_table_size_get(&size_);
  printf("table size = %d\n", size_);

  hal_port_for_mac_entry_del(&key1);
  hal_port_for_mac_entry_del(&key2);
  hal_port_for_mac_entry_del(&key3);
  hal_port_for_mac_entry_del(&key4);

  printf("entry delete finished\n");
  printf("====== Test port_for_mac end ======\n");
}