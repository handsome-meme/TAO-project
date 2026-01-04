#include <unistd.h>
#include <iostream>
#include <fstream>
#include <memory>

#include <sys/prctl.h>

#include "gflags/gflags.h"
#include "log.h"
#include "shc_pkt.h"

using namespace std;
using namespace spdlog;
using namespace shc;

bool g_runThread = true;

int main(int argc, char **argv) {
    prctl(PR_SET_NAME, "sfc");
    gflags :: SetVersionString( BUILD_VERSION );
    gflags :: SetUsageMessage( "Usage : ./sfc -loglevel=info " );
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    shc::log_init();
    std::thread sfcpktthread(sfc_pkt_thread);
    sfcpktthread.join();
    shc::SHC_LOG_INFO("sfc exit");
    return 0;
}
