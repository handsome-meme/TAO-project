#pragma once

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include <utility>

// extern "C" {
// #include "BOBHash32.h"
// }
// #include "BOBHash64.h"
#include "BOBHash32.hpp"
#include "params.hpp"

#include "base_table.hpp"
#include "../hal_topk_type.hpp"
// #include "../../agent/entry_api.h"
// using namespace std;

namespace shc{
    
    int flow_cmp(flow_table_element_t i, flow_table_element_t j);

    class ssummary
    {
        public:
            int tot;
            int sum[M+10],K,last[M+10],Next[M+10],ID[M+10]; // for elements
            int head[N+10],Left[N+10],Right[N+10],num; // for count buckets
            tuple_3_t tuple_3_data[M+10];
            int head2[len2+10],Next2[M+10]; // for hash to index i 
            BOBHash32 *bobhash;

            ssummary(){}
            // ssummary(int K):K(K);
            ssummary(int K);
            void clear();
            int getid();
            int location(tuple_3_t ST);
            void add2(int x,int y); //connect a hash value with a elem
            int find(tuple_3_t s);

            void linkhead(int i,int j); // create a new count bucket and link its neighbor
            void cuthead(int i); // delete a empty count bucket and link its neighbor
    
            int getmin();
            void link(int i,int ww);
            void cut(int i);
            void recycling(int i);
    };

    class spacesaving : public BaseTable
    {
        private:
            ssummary *ss;
            int M2,K;
            long long int total_pkt;
            bool be_able_to_insert;
        public:
            Node q[MAX_MEM+10];
            int CNT;
            

            spacesaving(){}
            // spacesaving(int M2,int K):M2(M2),K(K);
            spacesaving(int M2,int K); 
            int get_tot();
            void Insert(tuple_3_t x);
            static int cmp(Node i,Node j);
            void work();
            std::pair<tuple_3_t ,int> Query(int k);

            //for rules offload
            int top_k_dump(flow_table_t &old_table, flow_table_t &new_table);
            // void diff_cmp();
    };

    extern spacesaving *space_saving;
    int space_saving_init();

}

