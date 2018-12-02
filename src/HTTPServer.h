#ifndef HTTP_SERVER
#define HTTP_SERVER

#include "HTTPConfig.h"
#include "HTTPTypes.h"

#define STATUS_FMT "HTTP/1.1 %d %s"
#define HEAD_FMT "%s\r\n%s\r\n%s\r\nContent-Length: %d\r\n"
#define CONTENT_TYPE_FMT "Content-Type: %s"
#define HEAD_BODY_FMT "%s\r\n%s"
#define SERVER "Server: WIP"

extern Route routes[MAX_ROUTES];
extern int route_count;

extern void webserver_listen(void *);

#endif
