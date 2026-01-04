#include <unistd.h>
#include <event.h>
#include <sys/prctl.h>
#include <linux/filter.h>

#include <iostream>
#include <fstream>
#include "gflags/gflags.h"

#include "log.h"

using namespace std;
using namespace spdlog;

extern bool g_runThread;

namespace shc {

void shc_process_pkt(evutil_socket_t, short, void *) {

}

static int
shc_pkt_vxlan_init(struct event_base *pev_base)
{
    const int on = 1;
    int raw_sock;
    int ret;

    static struct sock_filter insns[] = {
        BPF_STMT(BPF_LD|BPF_B|BPF_ABS, 9),                  /* load 9th byte (protocol) */
        BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, IPPROTO_UDP, 0, 6), /* jump to L1 if it is IPPROTO_UDP, else to L4 */
        BPF_STMT(BPF_LD|BPF_H|BPF_ABS, 6),                  /* L1: load halfword from offset 6 (flags and frag offset) */
        BPF_JUMP(BPF_JMP|BPF_JSET|BPF_K, 0x1fff, 4, 0),     /* jump to L4 if any bits in frag offset field are set, else to L2 */
        BPF_STMT(BPF_LDX|BPF_B|BPF_MSH, 0),                 /* L2: skip IP header (load index reg with header len) */
        BPF_STMT(BPF_LD|BPF_H|BPF_IND, 2),                  /* load udp destination port from halfword[header_len + 2] */
        BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, 4789, 0, 1),        /* jump to L3 if udp dport is vxlan 4789 , else to L4 */
        BPF_STMT(BPF_RET|BPF_K, 0x7fffffff),                /* L3: accept packet  */
        BPF_STMT(BPF_RET|BPF_K, 0)                          /* L4: discard packet ("accept zero bytes") */
    };

    static struct sock_fprog filter = {
        sizeof insns / sizeof(insns[0]),
        insns
    };

    assert(!!pev_base);
    /* Initialize inet raw socket for UDP proto.*/
    raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    assert(raw_sock >= 0);

    ret = setsockopt(raw_sock, SOL_IP, IP_HDRINCL, &on, sizeof(on));
    assert(ret == 0);

    ret = setsockopt(raw_sock, SOL_SOCKET, SO_ATTACH_FILTER, 
                &filter, sizeof(filter));
    assert(ret == 0);

    struct  event *event = event_new(pev_base,  raw_sock, EV_READ|EV_PERSIST, shc_process_pkt, NULL);
    assert(!!event);

    ret = event_add(event, NULL);
    assert(ret == 0);

    return ret;
}

static int
shc_pkt_gre_init(struct event_base *pev_base)
{
    const int on = 1;
    int raw_sock;
    int ret;

    assert(!!pev_base);
    /* Initialize inet raw socket for GRE proto.*/
    raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_GRE);
    assert(raw_sock >= 0);

    ret = setsockopt(raw_sock, SOL_IP, IP_HDRINCL, &on, sizeof(on));
    assert(ret == 0);

    struct  event *event = event_new(pev_base,  raw_sock, EV_READ|EV_PERSIST, shc_process_pkt, NULL);
    assert(!!event);

    ret = event_add(event, NULL);
    assert(ret == 0);

    return ret;
}

struct event_base *g_pev_base = nullptr;

int shc_pkt_init(void) {
    g_pev_base = event_base_new_with_config(NULL);
    assert(nullptr != g_pev_base);
    shc_pkt_vxlan_init(g_pev_base);
    shc_pkt_gre_init(g_pev_base);
    return 0;
}

void sfc_pkt_thread(void) {
    prctl(PR_SET_NAME, "sfc_pkt_thread");
    shc_pkt_init();
    event_base_loop(g_pev_base, 0);
    shc::SHC_LOG_INFO("sfc_pkt_thread exit");
}

} // namespace shc