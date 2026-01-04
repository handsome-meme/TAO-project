#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_types.h"

//-------tunnel map vrf-------//
typedef enum {
  TYPE_TUNNEL_TYPE_NONE = 0,
  TYPE_TUNNEL_TYPE_VXLAN,
  TYPE_TUNNEL_TYPE_GRE,
} hal_tunnel_key_type;

typedef struct {
  hal_tunnel_key_type tunnel_type;
  uint32_t tunnel_key; 
} hal_tunnel_map_vrf_key;

typedef struct {
  uint16_t vrf;
} hal_tunnel_map_vrf_data;


//-------route and route_sele-------//
typedef struct {
  uint16_t vrf;
  uint32_t ip_src_addr;
  uint32_t ip_dst_addr;
} hal_route_key;

typedef struct {
  uint32_t vni;
  uint32_t ip_src_addr;
  uint32_t ip_dst_addr;
} hal_route_sele_key;

typedef struct {
  uint32_t nhop_id;
} hal_route_data;


//-------nhop-------//
typedef struct {
  uint32_t nhop_id;
} hal_nhop_key;

typedef struct {
  uint32_t tunnel_key;
  uint32_t local;
  uint32_t remote;
  uint32_t neigh_id;
} hal_nhop_data;


//-------neigh-------//
typedef struct {
  uint32_t neigh_id;
} hal_neigh_key;

typedef struct {
  uint8_t dmac[6];
} hal_neigh_data;


//-------port for mac-------//
typedef struct {
  uint16_t port;
} hal_port_for_mac_key;

typedef struct {
  uint8_t smac[6];
  uint8_t dmac[6];
} hal_port_for_mac_data;

#ifdef __cplusplus
}
#endif