//
// Created by zhaogang on 2025-03-19.
//
/*系统头文件*/
#include <stdint.h>
#include <signal.h>
#include <unistd.h>

/*dpdk头文件*/
#include <rte_log.h>
#include <rte_pdump.h>
#include <rte_latencystats.h>
#include <rte_errno.h>
#include <rte_ethdev.h>

/*本地头文件*/
#include "netdev/netdev.h"
#include "cvs/clog.h"
#include "cvsdb/cvsdb.h"
#include "netdev/netdev-dpdk.h"
#include "cvs/thread.h"

/* 程序退出标记 */
uint8_t f_quit = 0; // Global flag to indicate if we should quit
/* DPDK日志类型 */
int netdev_logtype;

/*
 * Display or mask ether events
 * Default to all events except VF_MBOX
 */
uint32_t event_print_mask = (UINT32_C(1) << RTE_ETH_EVENT_UNKNOWN) |
                            (UINT32_C(1) << RTE_ETH_EVENT_INTR_LSC) |
                            (UINT32_C(1) << RTE_ETH_EVENT_QUEUE_STATE) |
                            (UINT32_C(1) << RTE_ETH_EVENT_INTR_RESET) |
                            (UINT32_C(1) << RTE_ETH_EVENT_IPSEC) |
                            (UINT32_C(1) << RTE_ETH_EVENT_MACSEC) |
                            (UINT32_C(1) << RTE_ETH_EVENT_INTR_RMV) |
                            (UINT32_C(1) << RTE_ETH_EVENT_FLOW_AGED);

/* Pretty printing of ethdev events */
static const char * const eth_event_desc[] = {
        [RTE_ETH_EVENT_UNKNOWN] = "unknown",
        [RTE_ETH_EVENT_INTR_LSC] = "link state change",
        [RTE_ETH_EVENT_QUEUE_STATE] = "queue state",
        [RTE_ETH_EVENT_INTR_RESET] = "reset",
        [RTE_ETH_EVENT_VF_MBOX] = "VF mbox",
        [RTE_ETH_EVENT_IPSEC] = "IPsec",
        [RTE_ETH_EVENT_MACSEC] = "MACsec",
        [RTE_ETH_EVENT_INTR_RMV] = "device removal",
        [RTE_ETH_EVENT_NEW] = "device probed",
        [RTE_ETH_EVENT_DESTROY] = "device released",
        [RTE_ETH_EVENT_FLOW_AGED] = "flow aged",
        [RTE_ETH_EVENT_MAX] = NULL,
};

/* 定义了RTE_LIB_LATENCYSTATS
 * RTE_LIB_LATENCYSTATS 是一个专门设计的库，用于测量和分析网络数据包在应用程序处理过程中的
 * 延迟（latency）和抖动（jitter），帮助开发者优化高性能数据平面应用的性能。
 * */
#ifdef RTE_LIB_LATENCYSTATS

/*
 *用于确定 DPDK 的延迟统计功能（RTE_LIB_LATENCYSTATS）是否处于启用状态。
 */
uint8_t latencystats_enabled;

/*
 * Lcore ID to serive latency statistics.
 */
lcoreid_t latencystats_lcore_id = -1;

#endif


static void
force_quit(void)
{
    //pmd_test_exit();
    //prompt_exit();
}

/* * 处理以太网事件的回调函数
 * port_id: 端口ID
 * event: 事件类型
 * param: 事件参数
 * ret_param: 返回参数
 */
static int
eth_event_callback(portid_t port_id, enum rte_eth_event_type type,
                   void *param, void *ret_param)
{
    /*RTE_SET_USED宏用于标记未使用的参数，避免编译器警告*/
    /*
     * 是 DPDK 中的一个宏，作用是将参数或变量 param 标记为“已使用”，以抑制编译器的“未使用”警告。它通过 (void)(param) 实现，
     * 不影响运行时行为，常用于函数参数占位、调试或接口兼容性场景，是 DPDK 开发中一种常见的代码维护实践。
     * */
    RTE_SET_USED(param);
    RTE_SET_USED(ret_param);
    if (type >= RTE_ETH_EVENT_MAX) {
        LOG_ERROR("Unknown event type: %d", type);
        return -1; // 返回-1表示事件处理失败
    }else if (event_print_mask & (UINT32_C(1) << type)) {
        LOG_INFO("Event %s on port %u",
                 eth_event_desc[type], port_id);
        fflush(stdout);
    }
    char port_name[RTE_ETH_NAME_MAX_LEN] = {0};
    if (rte_eth_dev_get_name_by_port(port_id, port_name) == 0) {
        LOG_DEBUG("New device event: port %u, name: %s\n", port_id, port_name);
    } else {
        LOG_DEBUG("Failed to get name for new port %u\n", port_id);
    }
    struct CvsPort port = {
            .name = port_name,
            .ifindex = port_id,
            .is_up = false,
            .is_running = false
    };
    switch (type) {
        case RTE_ETH_EVENT_NEW:
            LOG_INFO("Device added: port %u", port_id);
            /* 处理设备添加事件 */
            port.is_up = true;
            cvsdb_update_port(&port);
            break;
        case RTE_ETH_EVENT_INTR_RMV:
            LOG_INFO("Device removed: port %u", port_id);
            /* 处理设备移除事件 */
            port.is_up = false;
            port.is_running = false;
            cvsdb_update_port(&port);
            break;
        case RTE_ETH_EVENT_INTR_LSC:
            LOG_INFO("Link state change on port %u", port_id);
            struct rte_eth_link link;
            rte_eth_link_get_nowait(port_id, &link);
            if (link.link_status) {
                LOG_INFO("Port %u: Link UP - speed %u Mbps - %s",
                         port_id,
                         link.link_speed,
                         link.link_duplex == ETH_LINK_FULL_DUPLEX ? "full-duplex" : "half-duplex");
            } else {
                LOG_INFO("Port %u: Link DOWN", port_id);
            }
            port.is_up = link.link_status;
            port.is_running = link.link_status;
            cvsdb_update_port(&port);
            break;
        default:
            break;
    }

    return 0; // 返回0表示事件处理成功
}

static int
register_eth_event_callback(void)
{
    /* 注册以太网事件回调函数 */
    int ret = 0;
    enum rte_eth_event_type event;
    for(event = RTE_ETH_EVENT_UNKNOWN;event < RTE_ETH_EVENT_MAX; event++) {
        ret = rte_eth_dev_callback_register(RTE_ETH_ALL, event, eth_event_callback, NULL);
        if (ret < 0) {
            LOG_ERROR("Failed to register callback for event %d: %s",
                      event, rte_strerror(rte_errno));
            return ret;
        }
    }
    return 0;
}


static void
signal_handler(int signum)
{
    if (signum == SIGINT || signum == SIGTERM) {
        LOG_INFO("\nSignal %d received, preparing to exit...\n",
               signum);

#ifdef RTE_LIB_PDUMP
        /* uninitialize packet capture framework */
		rte_pdump_uninit();
#endif
#ifdef RTE_LIB_LATENCYSTATS
        if (latencystats_enabled != 0)
            /* 清理和反初始化延迟统计功能*/
			rte_latencystats_uninit();
#endif
        force_quit();
        /* Set flag to indicate the force termination. */
        f_quit = 1;
        /* exit with the expected status */
        signal(signum, SIG_DFL);
        kill(getpid(), signum);
    }
}

void netdev_dpdk_init(const struct netdev_class *netdev)
{
    LOG_INFO("Init dpdk netdev class");
}

static void netdev_dpdk_run_(const struct netdev_class *netdev){
    /* Diagnostics “诊断”的缩写 */
    int diag;
    int ret;
    LOG_INFO("Running dpdk netdev class: %s", netdev->type);
    /* 注册信号处理函数，比如退出的*/
    /*中断信号*/
    signal(SIGINT, signal_handler);
    /*终止信号*/
    signal(SIGTERM, signal_handler);

    /* 创建日志打印对象 */
    netdev_logtype = rte_log_register("netdev");
    if (netdev_logtype < 0 ){
        LOG_ERROR("Failed to register netdev log type");
        rte_exit(EXIT_FAILURE, "Failed to register netdev log type\n");
    }
    rte_log_set_level(netdev_logtype, RTE_LOG_DEBUG);

    /* 初始化DPDK环境 */
    diag = rte_eal_init(0, NULL);
    if (diag < 0){
        LOG_ERROR("Failed to initialize DPDK EAL: %s", rte_strerror(rte_errno));
        rte_exit(EXIT_FAILURE, "Failed to initialize DPDK EAL\n");
    }

    if (rte_eal_process_type() == RTE_PROC_PRIMARY) {
        rte_exit(EXIT_FAILURE, "DPDK EAL is not running in primary process\n");
    }

    ret = register_eth_event_callback();
    if (ret < 0) {
        LOG_ERROR("Failed to register Ethernet event callback: %s", rte_strerror(rte_errno));
        rte_exit(EXIT_FAILURE, "Failed to register Ethernet event callback\n");
    }
#ifdef RTE_LIB_PDUMP
    /**
     *初始化 DPDK 的 pdump 框架，用于捕获以太网设备的数据包（接收或发送方向），
     * 并将其转储到文件或通过其他方式（如管道）传输，以便后续分析
     */
    rte_pdump_init();
#endif
}

void netdev_dpdk_run(const struct netdev_class *netdev){
    int pid;
    static struct cvsthread_once once = CVS_THREAD_ONCE_DPDK;
    if (cvs_thread_once_start(&once)) {
        pid = fork();
        if (pid < 0) {
            LOG_ERROR("Failed to fork process for DPDK netdev: %s", strerror(errno));
            rte_exit(EXIT_FAILURE, "Failed to fork process for DPDK netdev\n");
        } else if (pid == 0) {
            /* 子进程运行 DPDK */
            netdev_dpdk_run_(netdev);
            exit(0); // 子进程完成后退出
        }
        ovsthread_once_done(&once);
    }
}

const struct netdev_class netdev_dpdk_class = {
        .type = "dpdk",
        .init = netdev_dpdk_init,
        .run  = netdev_dpdk_run,
};
