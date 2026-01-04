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
#include <rte_log.h>



static bf_switchd_context_t switchd_ctx;
static int bf_tmr_thr_core = 2; /* "bf_timer_src" runs on core 2 by default.*/

/* NTB datapath running context.*/
typedef int
(*pmd_run_cb_t)(void *arg);
typedef struct {
    int nd_notify_fd;
    pmd_run_cb_t run_prepare;
    pmd_run_cb_t run_post;
}ntb_pmd_thread_args_t; 

static void parse_options(int argc, char **argv) 
{
    int option_index = 0;
    enum opts {OPT_SDEINSTALLDIR = 1, OPT_P4CONFFILE, OPT_P4KERNELPKT, OPT_BFTMRCORE};
    static struct option options[] = {
        {"help", no_argument, 0, 'h'},
        {"sde-dir", required_argument, 0, OPT_SDEINSTALLDIR},
        {"p4-conf", required_argument, 0, OPT_P4CONFFILE},
        {"kernel-pkt", no_argument, 0, OPT_P4KERNELPKT},
        {"bf-tmr-cpu", required_argument, 0, OPT_BFTMRCORE},
        {0,0,0,0},
    };

    while (1) {
        int c = getopt_long(argc, argv, "h", options, &option_index);
        if (c == -1) {
            break;
        }
        switch (c) {
        case OPT_SDEINSTALLDIR:
            switchd_ctx.install_dir = optarg;
            break;
        case OPT_P4CONFFILE:
            switchd_ctx.conf_file = optarg;
            break;
        case OPT_P4KERNELPKT:
            /* this must be set in Asic mode, otherwise cause file system corruption */
            switchd_ctx.kernel_pkt = true;
            break;
        case OPT_BFTMRCORE:
            bf_tmr_thr_core = atoi(optarg);
            break;
        case 'h':
        case '?':
            printf("%s\n", argv[0]);
            printf(
                "Usage : %s --sde-dir=<SDE install path> --p4-conf=<path to "
                "ntb-p4 conf file> --kernel-pkt(ASIC mode need)\n",
                argv[0]);
            exit(c == 'h' ? 0 : 1);
            break;
        default:
            printf("Invalid option\n");
            exit(0);
            break;
        }
    }
    if (switchd_ctx.install_dir == NULL) {
        printf("ERROR : --sde-dir must be specified\n");
        exit(0);
    }

    if (switchd_ctx.conf_file == NULL) {
        printf("ERROR : --p4-conf must be specified\n");
        exit(0);
    }
}

static void bf_tmr_thr_cpu_affinity_set(void)
{
    bool is_sw_model = false;

    // if (hal_pal_pltfm_type_get(&is_sw_model) != HAL_STATUS_SUCCESS) {
    //     rte_exit(EXIT_FAILURE, "Failed to get platform type\n");
    // }

    if (!is_sw_model) {
        cpu_set_t affinity;
        int rc;

        CPU_ZERO(&affinity);
        CPU_SET(bf_tmr_thr_core, &affinity);
        rc = pthread_setaffinity_np(switchd_ctx.tmr_t_id, sizeof(affinity), &affinity);
        if (rc) {
            rte_exit(EXIT_FAILURE, "Failed to bind bf_timer_src to cpu%d (%s)\n",
                bf_tmr_thr_core, strerror(rc));
        }
    }
}

static void bf_switchd_lib_init_post(void)
{
    bf_tmr_thr_cpu_affinity_set();
}

static void bf_switchd_init() 
{
    if (geteuid() != 0) {
        rte_exit(EXIT_FAILURE, "Need to run as root user! Exiting !\n");
    }
    /* Always set "background" because we do not want bf_switchd_lib_init to start
    * a CLI session.  That can be done afterward by the caller if requested
    * through command line options. */
    switchd_ctx.running_in_background = true;
    /* Always set "skip port add" so that ports are not automatically created when
    * running on either model or HW. */
    /* for ptf test case get switchd status is start finished */
    switchd_ctx.dev_sts_thread = true;
    switchd_ctx.dev_sts_port = 7777;

    /* Initialize libbf_switchd. */
    bf_status_t status = bf_switchd_lib_init(&switchd_ctx);
    if (status != BF_SUCCESS) {
        rte_exit(EXIT_FAILURE, "Failed to initialize libbf_switchd (%s)\n", bf_err_str(status));
    }

    /* Post libbf_switchd initialization. */
    bf_switchd_lib_init_post();
}


int main(int argc, char **argv) 
{
    int fds[2];
    int ret;
    ntb_pmd_thread_args_t pmd_args;
    pthread_t thread_pmd_id;

    parse_options(argc, argv);
    // ntb_signal_init();

    bf_switchd_init(argc, argv);
    // hal_init(argc, argv);
    // ntb_obj_spec_init();

    /* pmd thread init */
    if (pipe(fds)) {
        rte_exit(EXIT_FAILURE, "Create pipe faild\n");
    }
    pmd_args.nd_notify_fd = fds[1];
    pmd_args.run_prepare = NULL;
    pmd_args.run_post = NULL;
    // ret = pthread_create(&thread_pmd_id, NULL, ntb_pmd_thread, (void*)(&pmd_args));
    // if (ret) {
    //     rte_exit(EXIT_FAILURE, "Failed to create ntb pmd thread\n");
    // }
    // while (!ntb_api_ntb_pmd_thread_ready_get()) {
    //     sleep(1);
    // }
}
