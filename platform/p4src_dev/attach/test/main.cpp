#include <iostream>
#include <fstream>
#include <ctime>
#include "route.hpp"

extern "C" {
#include "common.h"
}

const bfrt::BfRtInfo *bfrtInfo = nullptr;
std::shared_ptr<bfrt::BfRtSession> session;

#define ALL_PIPES 0xffff
bf_rt_target_t dev_tgt;

void setUp() {
  dev_tgt.dev_id = 0;
  dev_tgt.pipe_id = ALL_PIPES;
  dev_tgt.direction = BF_DEV_DIR_ALL;
  dev_tgt.prsr_id = 0xff;
  // Get devMgr singleton instance
  auto &devMgr = bfrt::BfRtDevMgr::getInstance();

  // Get bfrtInfo object from dev_id and p4 program name
  auto bf_status =
      devMgr.bfRtInfoGet(dev_tgt.dev_id, "attach_insert_test", &bfrtInfo);
  // Check for status
  bf_sys_assert(bf_status == BF_SUCCESS);

  // Create a session object
  session = bfrt::BfRtSession::sessionCreate();
}
using namespace std;

int main(int argc, char **argv){
    parse_opts_and_switchd_init(argc, argv);

    setUp();

    route_init(bfrtInfo, dev_tgt, session);

    clock_t startTime,endTime;
    hal_route_key key;
    hal_route_data data;
    uint32_t src,dst;
    int i,pos,k;
    double t[500000];
    string A;

    ifstream file1;
    file1.open("/share/platform/p4src_dev/attach/test/202207251400_re.txt");
    if(!file1){
        cout<<"file open failed!"<<endl;
    }
    i = 0; k = 0;
    // while(1) cout<<"1"<<endl;
    while(getline(file1,A)){
        i++;
        if(i<3) continue;

        pos = A.find_first_of(" ");
        A = A.substr(0,pos);

        pos = A.find_first_of(",");
        // src = stoi(A.substr(0,pos));
        // dst = stoi(A.substr(pos+1));

        src = stoul(A.substr(0,pos));
        dst = stoul(A.substr(pos+1));


        // cout<<src<<" "<<dst<<endl;

        

        key.ip_src_addr = src;
        key.ip_dst_addr = dst;
        // data.

        route_table->tableMutexLock();
            startTime = clock();
            auto status = route_table->tableEntryAddModify(&key, &data, true); 
            endTime = clock();
        route_table->tableMutexUnlock();

        if(status == 0){
            t[k] = (double)(endTime - startTime);
            k++;
        }
            
        // cout<<status<<endl;

        if(k == 65536 || i == 65538) break;
    }
    
    cout<<"done"<<endl;

    freopen("/share/platform/p4src_dev/attach/test/insert_time_result.txt","w",stdout);
    for(int j=0;j<k;j++){
        // cout<<j;
        cout<<t[j]<<endl;
    }
    freopen("/dev/tty", "w", stdout);

    cout<<"write to file done"<<endl;
    run_cli_or_cleanup();

}