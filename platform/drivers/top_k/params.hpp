#pragma once

namespace shc{
    #define N 1000000  // maximum flow
    #define M 1000000  // maximum size of stream-summary or CSS
    #define MAX_MEM 200000 // maximum memory size
    #define HK_d 2 // maximum memory size
    const int flow_table_size = 100000;
    const int K = 100000; // Top K. take 1% of all flows
    const int ss_M = 199999; // spacesaving mem
    const int change_C = 20; // 交换常数
    #define len2 199999
    #define rep(i,a,n) for(int i=a;i<=n;i++)
}
