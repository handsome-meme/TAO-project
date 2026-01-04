#ifndef _softwaresort_H
#define _softwaresort_H

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include "BOBHASH32.h"
#include "params.h"
#include "vx_ssummary.h"
#include "vx_spacesaving.h"
#include "BOBHASH64.h"

using namespace std;

struct flow_table_element_t
{
    tuple_3_t flow_id;
    uint32_t count;
    bool operator< (const flow_table_element_t &rhs) const{
        return flow_id < rhs.flow_id;
    }
};
int flow_cmp(flow_table_element_t i, flow_table_element_t j)
{
    return i.count > j.count;
}

const int flow_table_size = 100000;
const int K = 100000; // Top K. take 1% of all flows
const int ss_M = 199999; // spacesaving mem
const int change_C = 20; // 交换常数

struct flow_table_t
{
    flow_table_element_t *data;
    int used_bucket;
    void init()
    {
        used_bucket = 0;
        data = new flow_table_element_t[flow_table_size];
        return;
    }
    void clear()
    {
        used_bucket = 0;
        return;
    }
    void free()
    {
        used_bucket = 0;
        delete [] data;
        return;
    }
};

class software_sort
{
    public:
        flow_table_t old_table, new_table;
        spacesaving *top_k_buff;
        bool be_able_to_insert;
        uint64_t total_pkt;
        software_sort()
        {
            // printf("init!\n");
            top_k_buff = new spacesaving(ss_M,K);
            old_table.init();
            new_table.init();
            be_able_to_insert = true;
            total_pkt = 0;
        }
        void update_table()
        {
            total_pkt = 0;
            old_table.clear();
            swap(old_table, new_table);
            top_k_buff->work();

            // if(top_k_buff->CNT == top_k_buff->get_tot())
            // {
            //     printf("check pass\n");
            // }
            // else{
            //     printf("check wrong: CNT %d, TOT %d\n",top_k_buff->CNT, top_k_buff->get_tot());
            // }
            new_table.used_bucket = top_k_buff->CNT;
            if(new_table.used_bucket > K) new_table.used_bucket = K;
            for(int i = 0; i < new_table.used_bucket; i++)
            {
                new_table.data[i].flow_id = (top_k_buff->q[i]).x;
                new_table.data[i].count = (top_k_buff->q[i]).y;
            }
            sort(new_table.data, new_table.data+(new_table.used_bucket));

            
            int old_index, new_index;
            old_index = 0; new_index = 0;
            while(old_index < old_table.used_bucket || new_index < new_table.used_bucket)
            {
                if(old_index == old_table.used_bucket && new_index == new_table.used_bucket) break;
                if(old_index == old_table.used_bucket)
                {
                    // add new rule with zdw, new_table.data[new_index].flow_id.(src_ip/dst_ip/port)
                    printf("add %lu %lu %lu\n",new_table.data[new_index].flow_id.src_ip,new_table.data[new_index].flow_id.dst_ip,new_table.data[new_index].flow_id.port);
                    new_index++;
                    continue;
                }
                else if(new_index == new_table.used_bucket)
                {
                    // delete old rule with zdw, old_table.data[old_index].flow_id.(src_ip/dst_ip/port)
                    printf("delete %lu %lu %lu\n",old_table.data[old_index].flow_id.src_ip,old_table.data[old_index].flow_id.dst_ip,old_table.data[old_index].flow_id.port);
                    old_index++;
                    continue;
                }
                else if(old_table.data[old_index].flow_id == new_table.data[new_index].flow_id)
                {
                    new_index++;
                    old_index++;
                    continue;
                }
                else if(old_table.data[old_index].flow_id < new_table.data[new_index].flow_id)
                {   // 老的比新的多出条目，需要删
                    // delete old rule with zdw, old_table.data[old_index].flow_id.(src_ip/dst_ip/port)
                    printf("delete %lu %lu %lu\n",old_table.data[old_index].flow_id.src_ip,old_table.data[old_index].flow_id.dst_ip,old_table.data[old_index].flow_id.port);
                    old_index++;
                    continue;
                }
                else
                {   // 新的比老的多出条目，需要加
                    // add new rule with zdw, new_table.data[new_index].flow_id.(src_ip/dst_ip/port)
                    printf("add %lu %lu %lu\n",new_table.data[new_index].flow_id.src_ip,new_table.data[new_index].flow_id.dst_ip,new_table.data[new_index].flow_id.port);
                    new_index++;
                    continue;
                }
            }

            delete top_k_buff;
            top_k_buff = new spacesaving(ss_M,K);
            be_able_to_insert = true;
        }
        void insert(uint64_t src_ip, uint64_t dst_ip, uint64_t port)
        {
            if(!be_able_to_insert)return;
            total_pkt++;
            tuple_3_t tuple_3_tmp;
            tuple_3_tmp.src_ip = src_ip;
            tuple_3_tmp.dst_ip = dst_ip;
            tuple_3_tmp.port = port;
            top_k_buff->Insert(tuple_3_tmp);

            if(top_k_buff->get_tot() * change_C < total_pkt) // flag TIME
            {
                be_able_to_insert = false;
                update_table();
            }
        }
};

#endif