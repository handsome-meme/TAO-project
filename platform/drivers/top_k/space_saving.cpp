#include "space_saving.hpp"

// using namespace std;

flow_table_t old_table;
flow_table_t new_table;

namespace shc{

    int flow_cmp(flow_table_element_t i, flow_table_element_t j)
    {
        return i.count > j.count;
    }


    ssummary::ssummary(int K):K(K) {
        bobhash=new BOBHash32(1000);
    }

    void ssummary::clear()
    {
        std::memset(sum,0,sizeof(sum));
        std::memset(last,0,sizeof(Next));
        std::memset(Next2,0,sizeof(Next2));
        rep(i,0,N)head[i]=Left[i]=Right[i]=0;
        rep(i,0,len2-1)head2[0]=0;
        tot=0;
        rep(i,1,M+2)ID[i]=i;
        num=M+2;
        Right[0]=N;
        Left[N]=0;
    }
    int ssummary::getid()
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
    int ssummary::location(tuple_3_t ST)
    {
        std::string ST_str = ST.to_str();
        return (bobhash->run(ST_str.c_str(),ST_str.size()))%len2;
    }
    void ssummary::add2(int x,int y) //connect a hash value with a elem
    {
        Next2[y]=head2[x];
        head2[x]=y;
    }
    int ssummary::find(tuple_3_t s)
    {
        for(int i=head2[location(s)];i;i=Next2[i])
            if(tuple_3_data[i]==s)return i;
        return 0;
    }
    void ssummary::linkhead(int i,int j) // create a new count bucket and link its neighbor
    {
        Left[i]=j;
        Right[i]=Right[j];
        Right[j]=i;
        Left[Right[i]]=i;
    }
    void ssummary::cuthead(int i) // delete a empty count bucket and link its neighbor
    {
        int t1=Left[i],t2=Right[i];
        Right[t1]=t2;
        Left[t2]=t1;
    }
    int ssummary::getmin()
    {
        if (tot<K) return 0;
        if(Right[0]==N)return 1;
        return Right[0]; //right of 0 is the min
    }
    void ssummary::link(int i,int ww)
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
    void ssummary::cut(int i)
    {
        --tot;
        if(head[sum[i]]==i)head[sum[i]]=Next[i];
        if(head[sum[i]]==0)cuthead(sum[i]);
        int t1=last[i],t2=Next[i];
        if(t1)Next[t1]=t2;
        if(t2)last[t2]=t1;
    }
    void ssummary::recycling(int i)
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


    spacesaving::spacesaving(int M2,int K):M2(M2),K(K){
        be_able_to_insert = true;
        total_pkt = 0;
        ss=new ssummary(M2); 
        ss->clear();
        old_table.init();
        new_table.init();
    }

    int spacesaving::get_tot(){
        return ss->tot;
    }
    void spacesaving::Insert(tuple_3_t x){
        if(!be_able_to_insert)return;
        total_pkt++;
        bool mon=false;
        int p=ss->find(x);
        if (p) mon=true;
        if (!mon) //found
        {
            int q;
            if (ss->tot<M2) q=1; else
            q=ss->getmin()+1;
            int i=ss->getid();
            ss->add2(ss->location(x),i);
            ss->tuple_3_data[i]=x;
            ss->sum[i]=q;
            ss->link(i,0);
            while(ss->tot>M2)
            {
                int t=ss->Right[0];
                int tmp=ss->head[t];
                ss->cut(ss->head[t]);
                ss->recycling(tmp);
            }
        } else
        {
            int tmp=ss->Left[ss->sum[p]];
            ss->cut(p);
            if(ss->head[ss->sum[p]]) tmp=ss->sum[p];
            ss->sum[p]++;
            ss->link(p,tmp);
        }

        // if(get_tot() * change_C < total_pkt) // flag TIME
        // {
        //     be_able_to_insert = false;
        //     top_k_dump();
        //     diff_cmp();
        // }
    }

    int spacesaving::cmp(Node i,Node j){
        return i.y>j.y;
    }
    void spacesaving::work()
    {
        CNT=0;
        for(int i=N;i;i=ss->Left[i])
            for(int j=ss->head[i];j;j=ss->Next[j]) {q[CNT].x=ss->tuple_3_data[j]; q[CNT].y=ss->sum[j]; CNT++; }
        std::sort(q,q+CNT,cmp);
    }
    pair<tuple_3_t ,int> spacesaving::Query(int k)
    {
        return make_pair(q[k].x,q[k].y);
    }

    int spacesaving::top_k_dump(flow_table_t &old_table, flow_table_t &new_table){
        total_pkt = 0;
        old_table.clear();
        std::swap(old_table, new_table);
        work();

        new_table.used_bucket = CNT;
        if(new_table.used_bucket > K) new_table.used_bucket = K;
        for(int i = 0; i < new_table.used_bucket; i++)
        {
            new_table.data[i].flow_id = q[i].x;
            new_table.data[i].count = q[i].y;
        }
        std::sort(new_table.data, new_table.data+(new_table.used_bucket));

        ss->clear();

        return 0;
    }
    
    // void spacesaving::diff_cmp(){
    //     // int re_sh = 0;
    //     int old_index, new_index;
    //     old_index = 0; new_index = 0;
    //     // while(old_index < old_table.used_bucket || new_index < new_table.used_bucket)
    //     while(1)
    //     {
    //         if(old_index == old_table.used_bucket && new_index == new_table.used_bucket) break;
    //         if(old_index == old_table.used_bucket)
    //         {
    //             // add new rule with zdw, new_table.data[new_index].flow_id.(src_ip/dst_ip/port)
    //             // shc_swTohw_rule(static_cast<int32_t>(new_table.data[new_index].flow_id.port),static_cast<uint32_t>(new_table.data[new_index].flow_id.src_ip),static_cast<uint32_t>(new_table.data[new_index].flow_id.dst_ip),0);
    //             printf("add %lu %lu %lu\n",new_table.data[new_index].flow_id.src_ip,new_table.data[new_index].flow_id.dst_ip,new_table.data[new_index].flow_id.port);
    //             new_index++;
    //             continue;
    //         }
    //         else if(new_index == new_table.used_bucket)
    //         {
    //             // delete old rule with zdw, old_table.data[old_index].flow_id.(src_ip/dst_ip/port)
    //             // shc_swTohw_rule(static_cast<int32_t>(old_table.data[old_index].flow_id.port),static_cast<uint32_t>(old_table.data[old_index].flow_id.src_ip),static_cast<uint32_t>(old_table.data[old_index].flow_id.dst_ip),1);
    //             printf("delete %lu %lu %lu\n",old_table.data[old_index].flow_id.src_ip,old_table.data[old_index].flow_id.dst_ip,old_table.data[old_index].flow_id.port);
    //             old_index++;
    //             continue;
    //         }
    //         else if(old_table.data[old_index].flow_id == new_table.data[new_index].flow_id)
    //         {
    //             new_index++;
    //             old_index++;
    //             continue;
    //         }
    //         else if(old_table.data[old_index].flow_id < new_table.data[new_index].flow_id)
    //         {   // 老的比新的多出条目，需要删
    //             // delete old rule with zdw, old_table.data[old_index].flow_id.(src_ip/dst_ip/port)
    //             // shc_swTohw_rule(static_cast<int32_t>(old_table.data[old_index].flow_id.port),static_cast<uint32_t>(old_table.data[old_index].flow_id.src_ip),static_cast<uint32_t>(old_table.data[old_index].flow_id.dst_ip),1);
    //             printf("delete %lu %lu %lu\n",old_table.data[old_index].flow_id.src_ip,old_table.data[old_index].flow_id.dst_ip,old_table.data[old_index].flow_id.port);
    //             old_index++;
    //             continue;
    //         }
    //         else
    //         {   // 新的比老的多出条目，需要加
    //             // add new rule with zdw, new_table.data[new_index].flow_id.(src_ip/dst_ip/port)
    //             // shc_swTohw_rule(static_cast<int32_t>(new_table.data[new_index].flow_id.port),static_cast<uint32_t>(new_table.data[new_index].flow_id.src_ip),static_cast<uint32_t>(new_table.data[new_index].flow_id.dst_ip),0);
    //             printf("add %lu %lu %lu\n",new_table.data[new_index].flow_id.src_ip,new_table.data[new_index].flow_id.dst_ip,new_table.data[new_index].flow_id.port);
    //             new_index++;
    //             continue;
    //         }

    //         be_able_to_insert = true;
    //     }
    // }


    spacesaving *space_saving = nullptr;
    //
    int space_saving_init(){
        shc::space_saving = new shc::spacesaving(ss_M,K);
        return 0;
    }

}

