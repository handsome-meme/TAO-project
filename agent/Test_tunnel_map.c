#include <stdio.h>
#include <stdlib.h>

#include "tunnel_map.h"
#include "neigh.h"
#include "route.h"
#include "vrf.h"
#include "netio.h"
#include "stat.h"
#include "shc_dp.h"
#include "shc_errno.h"


int main(){
    shc_tunnel_attr_t tunnel_attrl={
        .urpf=1,
        .htab16=1,
        .unused=45,
    };

    char *vrf_name1="vrf1";
    char *vrf_name2="vrf2";
    /*创建map表
    vrf_name  type   id  tunnel_attr
    vrf1     VXLAN   1      urpf
    vrf1     VXLAN   2      urpf
    vrf1     VXLAN   3      urpf
    vrf1     VXLAN   4      urpf
    vrf1     VXLAN   5      urpf
    vrf2     VXLAN   6      urpf
    vrf2     VXLAN   7      urpf
    vrf2     VXLAN   8      urpf
    vrf2     VXLAN   9      urpf
    vrf2     VXLAN   10     urpf
    */
    printf("-------表的初始化-------");
    if(!shc_tunnel_map_vrf_init())
        return 0;
    printf("-------往表中添加表项-------");
    shc_tunnel_id_t id={
        .vni=1,
        .vpcid=1,
        .common_id=5,
    };
    shc_tunnel_map_vrf(vrf_name1,SHC_TUNNEL_VXLAN,id,tunnel_attrl);
    id.vni=2;
    id.vpcid=2;
    id.common_id=6;
    shc_tunnel_map_vrf(vrf_name2,SHC_TUNNEL_VXLAN,id,tunnel_attrl);

    typedef struct vrf_dp_i vrf_dp_t;
    vrf_dp_t *vrf_dp;
    //查表
    id.vni=1;
    id.vpcid=1;
    id.common_id=5;
    printf("-------在表中查询相关表项id=1-------");
    vrf_dp = shc_tunnel_map_vrf_lookup(SHC_TUNNEL_VXLAN,id);
    if(vrf_dp) 
        printf("-------exist-------");
    else
        printf("-------no exist-------");
    
    id.vni=2;
    id.vpcid=2;
    id.common_id=6;
    printf("-------在表中查询相关表项id=2-------");
    vrf_dp = shc_tunnel_map_vrf_lookup(SHC_TUNNEL_VXLAN,id);
    if(vrf_dp) 
        printf("-------exist-------");
    else
        printf("-------no exist-------");
    return 0;
}
