#include "osapi.h"
#include "string.h"

#include "HTTPTypes.h"
#include "HTTPConfig.h"
#include "HTTPServer.h"

bool ICACHE_FLASH_ATTR
route_match(Request *request, Route *route) {
  return (request->path
	  && route->path
	  && request->request_line->method == route->method
	  && (strcmp(request->path, route->path) == 0));
}

Response * ICACHE_FLASH_ATTR
route_request(Request *request) {
  for(int i = 0; (i < route_count && i < MAX_ROUTES); i++) {
    if(route_match(request, &routes[i])) {
      return (*routes[i].route_fn)(request);
    }
  }
  
  // return a 404 from here (eventually allow for custom 404)
  static Response not_found = {
    .status = 404,
    .body = "Not found",
    .content_type = "text/html"
  };
  
  return &not_found;
}
