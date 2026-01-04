#pragma once

#include <iostream>

#include "top_k/params.hpp"

using namespace std;
//-------top_k struct-------//
struct tuple_3_t{
        uint64_t src_ip;
        uint64_t dst_ip;
        uint64_t port;
        bool operator< (const tuple_3_t &rhs) const{
            if(src_ip == rhs.src_ip)
            {
                if(dst_ip == rhs.dst_ip) return (port < rhs.port);
                else return (dst_ip < rhs.dst_ip);
            }
            else return (src_ip < rhs.src_ip);
        }
        bool operator== (const tuple_3_t &rhs) const{
            if(src_ip == rhs.src_ip && dst_ip == rhs.dst_ip && port == rhs.port) return true;
            else return false;
        }
        string to_str()
        {
            string ret = "";
            char tmp_char;
            uint64_t tmp_num;
            tmp_num = src_ip;
            while(tmp_num > 0)
            {
                tmp_char = '0'+(tmp_num %10);
                ret += tmp_char;
                tmp_num /= 10;
            }
            tmp_num = dst_ip;
            while(tmp_num > 0)
            {
                tmp_char = '0'+(tmp_num %10);
                ret += tmp_char;
                tmp_num /= 10;
            }
            tmp_num = port;
            while(tmp_num > 0)
            {
                tmp_char = '0'+(tmp_num %10);
                ret += tmp_char;
                tmp_num /= 10;
            }
            return ret;
        }
    };

    struct Node{
        tuple_3_t x; 
        int y;
    };

    struct flow_table_element_t
    {
        tuple_3_t flow_id;
        uint32_t count;
        bool operator< (const flow_table_element_t &rhs) const{
            return flow_id < rhs.flow_id;
        }
    };

    struct flow_table_t
    {
        flow_table_element_t *data;
        int used_bucket;
        void init()
        {
            used_bucket = 0;
            data = new flow_table_element_t[shc::flow_table_size];
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
extern flow_table_t old_table;
extern flow_table_t new_table;