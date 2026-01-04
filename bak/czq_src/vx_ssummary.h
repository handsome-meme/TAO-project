#ifndef _vxssummary_H
#define _vxssummary_H

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include "BOBHASH32.h"
#include "params.h"
#define len2 199999
#define rep(i,a,n) for(int i=a;i<=n;i++)
using namespace std;
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
class ssummary
{
    public:
        int tot;
        int sum[M+10],K,last[M+10],Next[M+10],ID[M+10]; // for elements
        int head[N+10],Left[N+10],Right[N+10],num; // for count buckets
        tuple_3_t tuple_3_data[M+10];
        int head2[len2+10],Next2[M+10]; // for hash to index i 
        BOBHash32 * bobhash;

        ssummary(int K):K(K) {bobhash=new BOBHash32(1000);}
        void clear()
        {
            memset(sum,0,sizeof(sum));
            memset(last,0,sizeof(Next));
            memset(Next2,0,sizeof(Next2));
            rep(i,0,N)head[i]=Left[i]=Right[i]=0;
            rep(i,0,len2-1)head2[0]=0;
            tot=0;
            rep(i,1,M+2)ID[i]=i;
            num=M+2;
            Right[0]=N;
            Left[N]=0;
        }
        int getid()
        {
            int i=ID[num--];
            last[i]=Next[i]=sum[i]=Next2[i]=0;
            /* 
                last: connect to neighbor element
                Next: point to its counter bucket
                sum:  the frequence of elem, same as count
                Next2: 
                head: counter bucket point to a element of its element chain
            */
            return i;
        }
        int location(tuple_3_t ST)
        {
            string ST_str = ST.to_str();
            return (bobhash->run(ST_str.c_str(),ST_str.size()))%len2;
        }
        void add2(int x,int y) //connect a hash value with a elem
        {
            Next2[y]=head2[x];
            head2[x]=y;
        }
        int find(tuple_3_t s)
        {
            for(int i=head2[location(s)];i;i=Next2[i])
              if(tuple_3_data[i]==s)return i;
            return 0;
        }
        void linkhead(int i,int j) // create a new count bucket and link its neighbor
        {
            Left[i]=j;
            Right[i]=Right[j];
            Right[j]=i;
            Left[Right[i]]=i;
        }
        void cuthead(int i) // delete a empty count bucket and link its neighbor
        {
            int t1=Left[i],t2=Right[i];
            Right[t1]=t2;
            Left[t2]=t1;
        }
        int getmin()
        {
            if (tot<K) return 0;
            if(Right[0]==N)return 1;
            return Right[0]; //right of 0 is the min
        }
        void link(int i,int ww)
        {
            ++tot;
            bool flag=(head[sum[i]]==0);
            Next[i]=head[sum[i]];
            if(Next[i])last[Next[i]]=i;
            last[i]=0;
            head[sum[i]]=i;
            if(flag) // add new count bucket
            {
                for(int j=sum[i]-1;j>0 && j>sum[i]-10;j--)
                if(head[j]){linkhead(sum[i],j);return;}
                linkhead(sum[i],ww);
            }
        }
        void cut(int i)
        {
            --tot;
            if(head[sum[i]]==i)head[sum[i]]=Next[i];
            if(head[sum[i]]==0)cuthead(sum[i]);
            int t1=last[i],t2=Next[i];
            if(t1)Next[t1]=t2;
            if(t2)last[t2]=t1;
        }
        void recycling(int i)
        {
            int w=location(tuple_3_data[i]);
            if (head2[w]==i)
              head2[w]=Next2[i];
              else
              {
                  for(int j=head2[w];j;j=Next2[j])
                  if(Next2[j]==i)
                  {
                        Next2[j]=Next2[i];
                        break;
                  }
              }
            ID[++num]=i;
        }
};
#endif
