#pragma once

#include "hal_table_api.h"
#include "hal.hpp"

#include <iostream>

void test_tunnel_map_vrf(std::shared_ptr<bfrt::BfRtSession>& session);
void test_route(std::shared_ptr<bfrt::BfRtSession>& session);
void test_route_sele(std::shared_ptr<bfrt::BfRtSession>& session);
void test_nhop(std::shared_ptr<bfrt::BfRtSession>& session);
void test_neigh(std::shared_ptr<bfrt::BfRtSession>& session);
void test_port_for_mac(std::shared_ptr<bfrt::BfRtSession>& session);
