/* toralize.h - SOCKS4 Proxy Library Header */
#ifndef TORALIZE_H
#define TORALIZE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <dlfcn.h>

/* Default Configurartion */
#define PROXY       "127.0.0.1"     /* Default Tor proxy address */
#define PROXYPORT   9050            /* Default Tor SOCKS port */
#define USERNAME    "toraliz"       /* SOCKS4 username (often ignored) */

/* Type definitions for SOCKS4 protocol */
typedef unsigned char int8;
typedef unsigned short int int16;
typedef unsigned int int32;

/* SOCKS4 Request Structure (RFC 1928)
		+----+----+----+----+----+----+----+----+----+----+....+----+
		| VN | CD | DSTPORT |      DSTIP        | USERID       |NULL|
		+----+----+----+----+----+----+----+----+----+----+....+----+
 #  	   1    1      2              4           variable       1
*/
struct proxy_request {
    int8 vn;
    int8 cd;
    int16 dstport;
    int32 dstip;
    unsigned char userid[8];
};
typedef struct proxy_request Req;
#define reqsize sizeof(Req)

/* SOCKS4 Response Structure
		+----+----+----+----+----+----+----+----+
		| VN | CD | DSTPORT |      DSTIP        |
		+----+----+----+----+----+----+----+----+
 #  	   1    1      2              4
*/
struct proxy_response {
    int8 cn;
    int8 cd;
    int16 dstport;
    int32 dstip;
};
typedef struct proxy_response Res;
#define ressize sizeof(Res)

#endif /* TORALIZE_H */
