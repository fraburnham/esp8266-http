#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"  // needed?
#include "os_type.h"
#include "user_interface.h"
#include "espconn.h"
#include "user_config.h"
#include "string.h"
#include "HTTPConfig.h"
#include "HTTPTypes.h"
#include "HTTPServer.h"
// do some cleanup on the above includes

// this is probably a waste of space, I wonder what kind of compression/hash I could use...
char *status_string[] = {
  [200] = "OK",
  [404] = "NOT FOUND"
};

static void ICACHE_FLASH_ATTR
recv(void *arg, char *pdata, unsigned short len) {
  RequestLine *request_line = extract_http_request_line(pdata);
  KeyValuePair *params = extract_query_params(request_line->path);
  struct espconn *conn = arg;

  Request request = {
    .request_line = request_line,
    .params = params,
    .path = strip_path(request_line->path)
  };

  Response *response = route_request(&request);

  static char status_line[32], content_type[64], head[256], response_data[512]; // probs needs protections, innit

  os_sprintf(content_type, CONTENT_TYPE_FMT,
	     response->content_type);
  os_sprintf(status_line, STATUS_FMT,
	     response->status, status_string[response->status]);
  os_sprintf(head, HEAD_FMT,
	     status_line, SERVER, content_type, os_strlen(response->body));
  os_sprintf(response_data, HEAD_BODY_FMT,
	     head, response->body);

  espconn_send(conn, response_data, os_strlen(response_data));
}

static void ICACHE_FLASH_ATTR
recon(void *arg, sint8 err) { }

static void ICACHE_FLASH_ATTR
discon(void *arg) { }

void ICACHE_FLASH_ATTR
webserver_listen(void *arg) {
  struct espconn *conn = arg;

  espconn_regist_recvcb(conn, &recv);
  espconn_regist_reconcb(conn, &recon);
  espconn_regist_disconcb(conn, &discon);
}
