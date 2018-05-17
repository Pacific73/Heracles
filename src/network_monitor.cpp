#include "network_monitor.h"
#include "helpers.h"
#include <cassert>

void *run_monitoring(void *p) {
    NetworkMonitor *monitor = reinterpret_cast<NetworkMonitor *>(p);
    monitor->run();
}

NetworkMonitor::NetworkMonitor() {
    assert(init_ebpf() == true);

    assert(init_config() == true);

    int thread_res = pthread_create(&tid, NULL, run_monitoring, (void *)this);
    pthread_detach(tid);

    print_log("[NET_MONITOR] inited.");
}

bool NetworkMonitor::init_ebpf() {
    bpf = new ebpf::BPF();

    auto init_res = bpf->init(BPF_PROGRAM);
    if (init_res.code() != 0) {
        print_err("[NET_MONITOR] init_ebpf() init: %s", init_res.msg());
        return false;
    }

    auto attach_res =
        bpf->attach_tracepoint("net:net_dev_xmit", "on_net_dev_xmit");
    if (attach_res.code() != 0) {
        print_err("[NET_MONITOR] init_ebpf() attach: %s", attach_res.msg());
        return false;
    }
    return true;
}

bool NetworkMonitor::init_config() {
    device = get_opt<std::string>("NIC_NAME", "lo");
    LC_classid = get_opt<uint32_t>("NET_LC_CLASSID", 0x10003);
    BE_classid = get_opt<uint32_t>("NET_BE_CLASSID", 0x10004);
}

NetworkMonitor::~NetworkMonitor() { delete bpf; }

void NetworkMonitor::run() {

    while (true) {
        usleep(1000000);

        class_dev_bytes = bpf->get_hash_table<info_t, uint64_t>("info_set")
                              .get_table_offline();
        bpf->get_hash_table<info_t, uint64_t>("info_set")
            .clear_table_non_atomic();

        class_bytes.clear();
        for (auto it : class_dev_bytes) {
            if (std::string(it.first.name) == device) {
                class_bytes[it.first.classid] += it.second;
            }
        }
        print_log("[NET_MONITOR] updated.");
    }

    print_err("[NET_MONITOR] run() ends! error.");
}

uint64_t NetworkMonitor::LC_bytes() { return class_bytes[LC_classid] * 8; }

uint64_t NetworkMonitor::BE_bytes() { return class_bytes[BE_classid] * 8; }

const std::string NetworkMonitor::BPF_PROGRAM = R"(
#include <bcc/proto.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <net/cls_cgroup.h>
#include <uapi/linux/ptrace.h>


struct net_dev_xmit_args
{   uint64_t common__unused;
    void* skbaddr;
    unsigned int len;
    int rc;
    int name;
};

struct info_t {
    uint32_t classid;
    char name[16];
};

BPF_HASH(info_set, struct info_t);


int on_net_dev_xmit(struct net_dev_xmit_args* args)
{
    struct sk_buff* skb = NULL;
    struct net_device* dev = NULL;
    struct sock* sk = NULL;
    u8 skcd_is_data = 0;
    uint32_t skcd_classid = 0;
    int len = 0;
    struct info_t info = {};
    uint64_t *val, zero = 0;

    skb = args->skbaddr;
    len = skb->len;
    dev = skb->dev;
    bpf_probe_read_str((void*)info.name, sizeof(info.name), (void*)dev->name);

    sk = skb->sk;
    skcd_is_data = sk->sk_cgrp_data.is_data;
    skcd_classid = sk->sk_cgrp_data.classid;
    info.classid = (skcd_is_data & 1) ? skcd_classid : 0;

    val = info_set.lookup_or_init(&info, &zero);
    (*val) += len;
        
    return 0;
}
)";
