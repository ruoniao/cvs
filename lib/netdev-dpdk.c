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


/////////////////////////////////////////////定义port相关/////////////////////////////////////////////
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
/* 记录端口 dev start 的数组*/
static bool port_started[RTE_MAX_ETHPORTS] = {false};
static struct rte_eth_conf port_conf = {
        .rxmode = {
                .split_hdr_size = 0,
        },
        .txmode = {
                .mq_mode = ETH_MQ_TX_NONE,
        },
};


#define MAX_PKT_BURST 32
#define BURST_TX_DRAIN_US 100 /* TX drain every ~100us */
#define MEMPOOL_CACHE_SIZE 256

/*
 * Configurable number of RX/TX ring descriptors
 */
#define RTE_TEST_RX_DESC_DEFAULT 1024
#define RTE_TEST_TX_DESC_DEFAULT 1024
static uint16_t nb_rxd = RTE_TEST_RX_DESC_DEFAULT;
static uint16_t nb_txd = RTE_TEST_TX_DESC_DEFAULT;

struct rte_mempool * cvs_pktmbuf_pool = NULL;


/* A tsc-based timer responsible for triggering statistics printout */
/* 一个基于TSC的定时器，用于触发统计信息的打印 */
static uint64_t timer_period = 10; /* default period is 10 seconds */


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
        LOG_INFO("Registered callback for event %s",
                 eth_event_desc[event] ? eth_event_desc[event] : "unknown");
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

void netdev_dpdk_run(const struct netdev_class *netdev)
{
    LOG_INFO("Init dpdk netdev class");
    int ret;
    uint16_t nb_ports;
    uint16_t nb_ports_available = 0;
    uint16_t portid;

    nb_ports = rte_eth_dev_count_avail();
    if (nb_ports == 0) {
        rte_exit(EXIT_FAILURE, "No Ethernet ports - bye\n");
        return;
    }
    LOG_INFO("Number of available Ethernet ports: %d", nb_ports);


    /* 遍历所有可用的以太网端口 */
    struct CvsPort cvsdb_port = {
            .name = "",
            .ifindex = 0,
            .is_up = false,
            .is_running = false
    };
    RTE_ETH_FOREACH_DEV(portid) {
        struct rte_eth_rxconf rxq_conf;
        struct rte_eth_txconf txq_conf;
        struct rte_eth_dev_info dev_info;
        struct rte_eth_link link;
        struct rte_eth_conf local_port_conf = port_conf;
        /* 获取网卡 devinfo */
        ret = rte_eth_dev_info_get(portid, &dev_info);
        if (ret < 0) {
            LOG_ERROR("Failed to get device info for port %u: %s",
                      portid, rte_strerror(rte_errno));
            continue;
        }

        if (rte_eth_dev_is_valid_port(portid)){
            if (!port_started[portid]) {
                /* 1. 配置网卡configure */
                ret = rte_eth_dev_configure(portid, 1, 1, &local_port_conf);
                if (ret < 0) {
                    LOG_ERROR("rte_eth_dev_configure(%u) failed: %s", portid, rte_strerror(-ret));
                    continue;
                }
                /* 2. 配置rx tx 描述符数量*/
                ret = rte_eth_dev_adjust_nb_rx_tx_desc(portid, &nb_rxd,
                                                       &nb_txd);
                if (ret < 0) {
                    LOG_ERROR("rte_eth_dev_adjust_nb_rx_tx_desc(%u) failed: %s",
                              portid, rte_strerror(-ret));
                    continue;
                }
                /* 3. 配置rx 队列 */
                rxq_conf = dev_info.default_rxconf;
                rxq_conf.offloads = local_port_conf.rxmode.offloads;
                ret = rte_eth_rx_queue_setup(portid, 0, nb_rxd,
                                             rte_eth_dev_socket_id(portid),
                                             &rxq_conf,
                                             cvs_pktmbuf_pool);
                if (ret < 0)
                    rte_exit(EXIT_FAILURE, "rte_eth_rx_queue_setup:err=%d, port=%u\n",
                             ret, portid);

                /* 4. 配置tx 队列 */
                fflush(stdout);
                txq_conf = dev_info.default_txconf;
                txq_conf.offloads = local_port_conf.txmode.offloads;
                ret = rte_eth_tx_queue_setup(portid, 0, nb_txd,
                                             rte_eth_dev_socket_id(portid),
                                             &txq_conf);

                if (ret < 0) {
                    LOG_ERROR("rte_eth_tx_queue_setup(%u) failed: %s", portid, rte_strerror(-ret));
                    continue;
                }
                /* 5. 启动port  */
                ret = rte_eth_dev_start(portid);
                if (ret == 0) {
                    port_started[portid] = true;
                } else {
                    LOG_ERROR("rte_eth_dev_start(%u) failed: %s", portid, rte_strerror(-ret));
                }
            }
        }

        rte_eth_link_get(portid, &link);
        LOG_INFO("Port %u: %s", portid, dev_info.device->name);
        cvsdb_port.name = (char *)dev_info.device->name;
        cvsdb_port.ifindex = portid;
        cvsdb_port.is_up = link.link_status;
        cvsdb_port.is_running = link.link_status;
        if(link.link_status){
            LOG_INFO("Port %u: Link UP - speed %u Mbps - %s",
                     portid,
                     link.link_speed,
                     link.link_duplex == ETH_LINK_FULL_DUPLEX ? "full-duplex" : "half-duplex");
            // TODO: 1.这个link down目测不管用； 2.重新抽取配置网卡函数  3.mac地址获取
            // 4.mbuf的创建和新添加端口时的影响，以及cvs的队列绑定策略 5.CMocka单元测试使用

            rte_eth_dev_set_link_down(portid);
        } else {
            LOG_INFO("Port %u: Link DOWN", portid);
            rte_eth_dev_set_link_up(portid);
        }
        /* 更新端口信息到数据库 */
        cvsdb_update_port(&cvsdb_port);

        nb_ports_available++;
    }

}

static void netdev_dpdk_init_(const struct netdev_class *netdev) {
    /* Diagnostics “诊断”的缩写 */
    int diag;
    int ret;
    uint16_t nb_ports;
    uint16_t nb_ports_available = 0;
    uint16_t portid, last_port;
    unsigned int nb_mbufs;
    unsigned int nb_lcores = 2;

    LOG_INFO("Running dpdk netdev class: %s", netdev->type);
    /* 注册信号处理函数，比如退出的*/
    /*中断信号*/
    signal(SIGINT, signal_handler);
    /*终止信号*/
    signal(SIGTERM, signal_handler);

    /* 创建日志打印对象 */
    netdev_logtype = rte_log_register("netdev");
    if (netdev_logtype < 0) {
        LOG_ERROR("Failed to register netdev log type");
        rte_exit(EXIT_FAILURE, "Failed to register netdev log type\n");
    }
    rte_log_set_level(netdev_logtype, RTE_LOG_DEBUG);

    /* 初始化DPDK环境 */
    int argc = 1;
    char *argv[] = { "cvs_dpdk_app", NULL };
    diag = rte_eal_init(argc, argv);
    if (diag < 0) {
        LOG_ERROR("Failed to initialize DPDK EAL: %s", rte_strerror(rte_errno));
        rte_exit(EXIT_FAILURE, "Failed to initialize DPDK EAL\n");
    }

    if (rte_eal_process_type() == RTE_PROC_SECONDARY) {
        rte_exit(EXIT_FAILURE, "DPDK EAL is not running in primary process\n");
    }

    ret = register_eth_event_callback();
    if (ret < 0) {
        LOG_ERROR("Failed to register Ethernet event callback: %s", rte_strerror(rte_errno));
        rte_exit(EXIT_FAILURE, "Failed to register Ethernet event callback\n");
    }

    /*  *是乘 */
    timer_period *= rte_get_timer_hz();

    nb_mbufs = RTE_MAX(nb_ports * (nb_rxd + nb_txd + MAX_PKT_BURST +
                                   nb_lcores * MEMPOOL_CACHE_SIZE), 8192U);

    /* create the mbuf pool */
    cvs_pktmbuf_pool = rte_pktmbuf_pool_create("mbuf_pool", nb_mbufs,
                                                 MEMPOOL_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE,
                                                 rte_socket_id());
    if (cvs_pktmbuf_pool == NULL)
        rte_exit(EXIT_FAILURE, "Cannot init mbuf pool\n");

#ifdef RTE_LIB_PDUMP
    /**
     *初始化 DPDK 的 pdump 框架，用于捕获以太网设备的数据包（接收或发送方向），
     * 并将其转储到文件或通过其他方式（如管道）传输，以便后续分析
     */
    rte_pdump_init();
#endif
}


void netdev_dpdk_init(const struct netdev_class *netdev){
    static struct cvsthread_once once = CVS_THREAD_ONCE_DPDK;
    if (cvs_thread_once_start(&once)) {
        netdev_dpdk_init_(netdev);
        ovsthread_once_done(&once);
    }
}

const struct netdev_class netdev_dpdk_class = {
        .type = "dpdk",
        .init = netdev_dpdk_init,
        .run  = netdev_dpdk_run,
};
