//
// Created by zhaogang on 2025-05-17.
//
#include <string.h>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/un.h>

#include "server/server-router.h"
#include "server/server-daemon.h"
#include "cvs/hmap.h"
#include "cvs/clog.h"


static struct event_base *base;

struct ServerConfig *server_config_init(enum ServerMode mode, const char *host, int port, const char *socket_path){
    struct ServerConfig *TCPServerConfig = malloc(sizeof(struct ServerConfig));
    TCPServerConfig->mode = mode;
    TCPServerConfig->host = strdup(host);
    TCPServerConfig->port = port;
    TCPServerConfig->socket_path = strdup(socket_path);
    return TCPServerConfig;
}

void server_config_free(struct ServerConfig *config) {
    if (config) {
        free(config->host);
        config->host = NULL;

        free(config->socket_path);
        config->socket_path = NULL;

        free(config);
        config = NULL;
    }
}


/* 回调处理函数*/
void read_cb(struct bufferevent *bev, void *ctx) {
    char buf[1024] = {0};
    bufferevent_read(bev, buf, sizeof(buf));

    // Simple format: "op:payload"
    // 在字符串 buf 中查找第一个冒号 : 的位置，并返回指向该位置的指针 本质是分割字符串
    char *sep = strchr(buf, ':');
    if (!sep) return;
    *sep = '\0';
    char *op = buf;
    char *payload = sep + 1;
    handler_fn *fn = NULL;
    if (!router_lookup(op,fn)){
        LOG_ERROR("Handler not found for operation: %s\n", op);
        return;
    };
    if (fn){
        struct ServerRequest *serverRequest = (struct ServerRequest*)payload;
        handler_fn handler_call = (handler_fn)fn;
        handler_call(serverRequest);
    }
    else bufferevent_write(bev, "unknown operation\n", 18);
}

/*事件回调函数*/
void event_cb(struct bufferevent *bev, short events, void *ctx) {
    if (events & (BEV_EVENT_ERROR | BEV_EVENT_EOF)) {
        bufferevent_free(bev);
    }
}

void accept_cb(struct evconnlistener *listener, evutil_socket_t fd,
               struct sockaddr *sa, int socklen, void *ctx) {
    // 处理新连接
    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    // 设置读写回调函数
    bufferevent_setcb(bev, read_cb, NULL, event_cb, NULL);
    bufferevent_enable(bev, EV_READ | EV_WRITE);
}
int cvs_server_start(struct ServerConfig *config){
    // 注册路由
    router_init();

    base = event_base_new();
    if (!base) {
        LOG_ERROR("Could not initialize libevent!\n");
        return -1;

    }
    struct evconnlistener *listener = NULL;
    // 创建一个新的事件监听器
    if (config->mode == MODE_TCP) {
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(config->port);
        server_addr.sin_addr.s_addr = inet_addr(config->host);
        // 设置accept_cb为回调函数
        listener = evconnlistener_new_bind(base, accept_cb, NULL,
                                           LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
                                           -1, (struct sockaddr *)&server_addr, sizeof(server_addr));

        LOG_INFO("TCP server started on %s:%d\n", config->host, config->port);
    } else {
        // Handle UNIX socket initialization
        struct sockaddr_un sun = {0};
        sun.sun_family = AF_UNIX;
        strncpy(sun.sun_path, config->socket_path, sizeof(sun.sun_path) - 1);
        listener = evconnlistener_new_bind(base, accept_cb, NULL,
                                           LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
                                           -1, (struct sockaddr *)&sun, sizeof(sun));
    }
    if (!listener) return -1;
    event_base_dispatch(base);
    evconnlistener_free(listener);
    event_base_free(base);
    server_config_free(config);


    pid_t pid = fork();
    // TODO: 还需优化这个地方，只是暂时使用一个线程
    if (pid == 0) {
        event_base_dispatch(base);
        evconnlistener_free(listener);
        event_base_free(base);
        server_config_free(config);
        return 0;
    } else if (pid > 0) {
        return 0;
    } else {
        LOG_ERROR("fork() failed\n");
    }

}