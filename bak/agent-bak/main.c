#include <bfsys/bf_sal/bf_sys_intf.h>
#include <getopt.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <bf_rt/bf_rt.h>

#define __USE_GNU
#include <sched.h>
#include <pthread.h>

#include <bf_switchd/bf_switchd.h>





int main(int argc, char **argv) 
{
    int fds[2];
    int ret;
    ntb_pmd_thread_args_t pmd_args;
    pthread_t thread_grpc_config_id;
    pthread_t thread_grpc_report_id;
    pthread_t thread_arp_collect_id;
    pthread_t thread_port_monitor_id;
    pthread_t thread_ov_cfg_save_id;
    pthread_t thread_pltfm_monitor_id;
    pthread_t thread_tunnel_statis_id;
    pthread_t thread_dp_err_monitor_id;
    pthread_t thread_pmd_id;
    pthread_t thread_table_res_monitor_id;
    pthread_t thread_tm_monitor_id;

    parse_options(argc, argv);
    ntb_signal_init();

    bf_switchd_init(argc, argv);
    hal_init(argc, argv);
    ntb_obj_spec_init();

    /* pmd thread init */
    if (pipe(fds)) {
        rte_exit(EXIT_FAILURE, "Create pipe faild\n");
    }
    pmd_args.nd_notify_fd = fds[1];
    pmd_args.run_prepare = NULL;
    pmd_args.run_post = NULL;
    ret = pthread_create(&thread_pmd_id, NULL, ntb_pmd_thread, (void*)(&pmd_args));
    if (ret) {
        rte_exit(EXIT_FAILURE, "Failed to create ntb pmd thread\n");
    }
    while (!ntb_api_ntb_pmd_thread_ready_get()) {
        sleep(1);
    }