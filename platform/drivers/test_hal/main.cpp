#include <iostream>

extern "C" {
#include "common.h"
}

#include "test.hpp"
#include "hal.hpp"

// using ::smi::logging::logging_init;

void test_hal_status()
{
  for(int i = 0; i < HAL_STS_MAX; i++)
    printf("%s\n", hal_status_to_string(i));
}

int main(int argc, char **argv) {
  parse_opts_and_switchd_init(argc, argv);
  // Do initial set up
  hal_init(argc, argv);


  test_tunnel_map_vrf(shc::session);
  test_route_sele(shc::session);
  test_route(shc::session);
  test_nhop(shc::session);
  test_neigh(shc::session);
  test_port_for_mac(shc::session);
  
  printf("\n[[[ test finished. ]]]\n");


  run_cli_or_cleanup();
}
