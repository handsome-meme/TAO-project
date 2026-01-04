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

    vrf_dp_t *vrf_dp;
    vrf_dp->vrf_id=5;
    struct fib_nhop *f_nhop;
    f_nhop->gateway=192168152;
    uint32_t neigh_oid_ret=0;
    struct rte_ether_addr *hw_addr;
    //添加表项
    if(ntb_neigh_add(vrf_dp,f_nhop->gateway,&neigh_oid_ret))
        printf("%s\n","neigh表添加成功！");
    else
        printf("%s\n","neigh表添加失败");
    vrf_dp->vrf_id=6;
    if(ntb_neigh_add(vrf_dp,f_nhop->gateway,&neigh_oid_ret))
        printf("%s\n","neigh表添加成功！");
    else
        printf("%s\n","neigh表添加失败");
    
    /*查询表项
        这个查询函数 如果没有查到 则返回错误
                    如果查询到 会返回0
    */
    if(!ntb_neigh_lookup(vrf_dp->vrf_id,f_nhop->gateway,hw_addr))
        printf("%s\n","查询的内容存在");
    else
        printf("%s\n","查询的内容不存在");
    
    //删除表项
    vrf_dp->vrf_id=5;
    if(!ntb_neigh_del(vrf_dp,f_nhop->gateway))
        printf("%s\n","neigh表项删除成功！")
    else
        printf("%s\n","neigh表项删除失败！");
    
    vrf_dp->vrf_id=4;
    if(!ntb_neigh_del(vrf_dp,f_nhop->gateway))
        printf("%s\n","neigh表项删除成功！")
    else
        printf("%s\n","neigh表项删除失败！");
    return 0;
}
