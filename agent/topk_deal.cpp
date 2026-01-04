#include <sys/prctl.h>
#include <csignal> 
#include <unistd.h>

#include "entry_api.h"
#include "topk_deal.h"
#include "../platform/drivers/hal_topk_type.hpp"
#include "../platform/drivers/hal_topk_api.hpp"

extern "C" {
#include "shc_api.h"
}


using namespace std;


void diff_cmp(int a){
    printf("1--------------\n");
    flow_table_t *old_s, *new_s;
    
    hal_topk_dump(old_s, new_s);

    // int re_sh = 0;
    int old_index, new_index;
    old_index = 0; new_index = 0;
    while(old_index < old_s->used_bucket || new_index < new_s->used_bucket)
    // while(1)
    {
        if(old_index == old_s->used_bucket && new_index == new_s->used_bucket) break;
        if(old_index == old_s->used_bucket)
        {
            // add new rule with zdw, new_s->data[new_index].flow_id.(src_ip/dst_ip/port)
            shc_swTohw_rule(static_cast<int32_t>(new_s->data[new_index].flow_id.port),static_cast<uint32_t>(new_s->data[new_index].flow_id.src_ip),static_cast<uint32_t>(new_s->data[new_index].flow_id.dst_ip),0);
            printf("add %lu %lu %lu\n",new_s->data[new_index].flow_id.src_ip,new_s->data[new_index].flow_id.dst_ip,new_s->data[new_index].flow_id.port);
            new_index++;
            continue;
        }
        else if(new_index == new_s->used_bucket)
        {
            // delete old rule with zdw, old_s->data[old_index].flow_id.(src_ip/dst_ip/port)
            shc_swTohw_rule(static_cast<int32_t>(old_s->data[old_index].flow_id.port),static_cast<uint32_t>(old_s->data[old_index].flow_id.src_ip),static_cast<uint32_t>(old_s->data[old_index].flow_id.dst_ip),1);
            printf("delete %lu %lu %lu\n",old_s->data[old_index].flow_id.src_ip,old_s->data[old_index].flow_id.dst_ip,old_s->data[old_index].flow_id.port);
            old_index++;
            continue;
        }
        else if(old_s->data[old_index].flow_id == new_s->data[new_index].flow_id)
        {
            new_index++;
            old_index++;
            continue;
        }
        else if(old_s->data[old_index].flow_id < new_s->data[new_index].flow_id)
        {   // 老的比新的多出条目，需要删
            // delete old rule with zdw, old_s->data[old_index].flow_id.(src_ip/dst_ip/port)
            shc_swTohw_rule(static_cast<int32_t>(old_s->data[old_index].flow_id.port),static_cast<uint32_t>(old_s->data[old_index].flow_id.src_ip),static_cast<uint32_t>(old_s->data[old_index].flow_id.dst_ip),1);
            printf("delete %lu %lu %lu\n",old_s->data[old_index].flow_id.src_ip,old_s->data[old_index].flow_id.dst_ip,old_s->data[old_index].flow_id.port);
            old_index++;
            continue;
        }
        else
        {   // 新的比老的多出条目，需要加
            // add new rule with zdw, new_s->data[new_index].flow_id.(src_ip/dst_ip/port)
            shc_swTohw_rule(static_cast<int32_t>(new_s->data[new_index].flow_id.port),static_cast<uint32_t>(new_s->data[new_index].flow_id.src_ip),static_cast<uint32_t>(new_s->data[new_index].flow_id.dst_ip),0);
            printf("add %lu %lu %lu\n",new_s->data[new_index].flow_id.src_ip,new_s->data[new_index].flow_id.dst_ip,new_s->data[new_index].flow_id.port);
            new_index++;
            continue;
        }
        // be_able_to_insert = true;
    }
}

void *shc_topk_thread(void * argv){
    prctl(PR_SET_NAME, "shc_topk");

    struct itimerval tick;
	timer_para * time_val;

    time_val = (timer_para *)argv;
    // pthread_detach(pthread_self());

    printf("StartTimer start!\n");
    signal(SIGALRM, time_val->func);
    // signal(SIGALRM, diff_cmp);
    memset(&tick, 0, sizeof(tick));

    //Timeout to run first time
	tick.it_value.tv_sec = time_val->interval_time;
    // tick.it_value.tv_sec = 5;
	tick.it_value.tv_usec = 0;

    //After first, the Interval time for clock
	tick.it_interval.tv_sec = time_val->interval_time;
    // tick.it_interval.tv_sec = 5;
	tick.it_interval.tv_usec = 0;

    if(setitimer(ITIMER_REAL, &tick, NULL) < 0)
	{
		printf("Set timer failed!\n");
	}

    while(1)
	{
		pause();
	}
	printf("AppStartTimer exit!\n");

    return NULL;
}
