/* toralize.c - Transparent SOCKS4 Proxy via LD_PRELOAD */
#include "toralize.h"
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>

/* Create SOCKS4 request from destination address */
static Req *create_socks_request(const struct sockaddr_in *dest) {
    Req *req = malloc(reqsize);
    if (!req) return NULL;
    
    req->vn = 4;
    req->cd = 1;
    req->dstport = dest->sin_port;
    req->dstip = dest->sin_addr.s_addr;
    strncpy((char *)req->userid, USERNAME, 7);
    req->userid[7] = '\0';
    
    return req;
}

/* Main connect() interceptor */
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    static int (*orig_connect)(int, const struct sockaddr*, socklen_t) = NULL;
    int proxy_sock = -1;
    Req *req = NULL;
    char response_buf[8];
    
    /* Load original connect() on first call */
    if (!orig_connect) {
        orig_connect = dlsym(RTLD_NEXT, "connect");
        if (!orig_connect) {
            fprintf(stderr, "toralize: dlsym(connect) failed: %s\n", dlerror());
            errno = ENOSYS;
            return -1;
        }
    }
    
    /* Only intercept IPv4 TCP connections */
    if (!addr || addr->sa_family != AF_INET) {
        return orig_connect(sockfd, addr, addrlen);
    }
    
    /* Get proxy configuration */
    const char *proxy_addr = getenv("TORALIZE_PROXY");
    const char *proxy_port_str = getenv("TORALIZE_PORT");
    const char *target_proxy = proxy_addr ? proxy_addr : PROXY;
    int target_port = proxy_port_str ? atoi(proxy_port_str) : PROXYPORT;
    
    /* Create socket for proxy connection */
    proxy_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (proxy_sock < 0) {
        perror("toralize: socket");
        return -1;
    }
    
    /* Preserve socket flags */
    
    /* Connect to SOCKS4 proxy */
    struct sockaddr_in proxy = {0};
    proxy.sin_family = AF_INET;
    proxy.sin_port = htons(target_port);
    
    /* Resolve proxy hostname */
    struct hostent *he = gethostbyname(target_proxy);
    if (he) {
        memcpy(&proxy.sin_addr, he->h_addr_list[0], he->h_length);
    } else {
        proxy.sin_addr.s_addr = inet_addr(target_proxy);
        if (proxy.sin_addr.s_addr == INADDR_NONE) {
            fprintf(stderr, "toralize: Cannot resolve proxy '%s'\n", target_proxy);
            close(proxy_sock);
            errno = EINVAL;
            return -1;
        }
    }
    
    if (orig_connect(proxy_sock, (struct sockaddr *)&proxy, sizeof(proxy)) < 0) {
        perror("toralize: connect to proxy");
        close(proxy_sock);
        return -1;
    }
    
    /* Create and send SOCKS4 request */
    const struct sockaddr_in *dest = (const struct sockaddr_in *)addr;
    req = create_socks_request(dest);
    if (!req) {
        fprintf(stderr, "toralize: Failed to create SOCKS4 request\n");
        close(proxy_sock);
        errno = ENOMEM;
        return -1;
    }
    
    if (write(proxy_sock, req, reqsize) != reqsize) {
        perror("toralize: write SOCKS4 request");
        free(req);
        close(proxy_sock);
        return -1;
    }
    free(req);
    
    /* Read SOCKS4 response */
    memset(response_buf, 0, sizeof(response_buf));
    if (read(proxy_sock, response_buf, sizeof(response_buf)) != sizeof(response_buf)) {
        perror("toralize: read SOCKS4 response");
        close(proxy_sock);
        return -1;
    }
    
    /* Check response code (90 = success) */
    Res *res = (Res *)response_buf;
    if (res->cd != 90) {
        fprintf(stderr, "toralize: SOCKS4 error %d\n", res->cd);
        close(proxy_sock);
        errno = ECONNREFUSED;
        return -1;
    }
    
    /* Redirect proxy socket to original file descriptor */
    if (dup2(proxy_sock, sockfd) < 0) {
        perror("toralize: dup2");
        close(proxy_sock);
        return -1;
    }
    
    close(proxy_sock);
    return 0;
}
