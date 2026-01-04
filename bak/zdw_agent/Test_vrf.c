#include <stdio.h>
#include <stdlib.h>

#include "neigh.h"
#include "route.h"
#include "vrf.h"
#include "netio.h"
#include "stat.h"
#include "ntb_dp.h"
#include "ntb_errno.h"

int main(){

    char *vrf_name1="vrf1";
    char *vrf_name2="vrf2";

    ntb_vrf_t g_ntb_vrfs;
    vrf_dp_t dp;
    //添加表项
    if(!ntb_vrf_add(vrf_name1))
        printf("%s\n","vrf_name1插入成功或已有相同的vrf");
    else
        printf("%s\n","插入失败！");
    
    if(!ntb_vrf_add(vrf_name2))
        printf("%s\n","vrf_name2插入成功或已有相同的vrf");
    else
        printf("%s\n","插入失败！");

    //查询表项
    if(ntb_vrf_lookup(vrf_name1)!=NULL)
        printf("%s%d%d\n",g_ntb_vrfs.name,g_ntb_vrfs.valid,g_ntb_vrfs.dp.vrf_id);
    else   
        printf("%s\n","vrf_name1查询失败！");
    
    if(ntb_vrf_lookup(vrf_name3)!=NULL)
        printf("%s%d%d\n",g_ntb_vrfs.name,g_ntb_vrfs.valid,g_ntb_vrfs.dp.vrf_id);
    else   
        printf("%s\n","vrf_name3查询失败！");
    
    //删除表项
    if(!ntb_vrf_del(vrf_name1))
        printf("%s\n","vrf_name1删除成功！");
    else
        printf("%s\n","vrf_name1删除失败！");
    
    if(!ntb_vrf_del(vrf_name3))
        printf("%s\n","vrf_name3删除成功！");
    else
        printf("%s\n","vrf_name3删除失败！");
    return 0;
}
