#include "HTTPConfig.h"

#ifndef HTTP_TYPES
#define HTTP_TYPES

typedef enum HTTPMethod {
  GET,
  POST,
  NOT_ALLOWED
} HTTPMethod;

typedef struct RequestLine {
  HTTPMethod method;
  char *path;
  char *version;  // this could (should?) also be an enum
} RequestLine;

typedef struct KeyValuePair {
  char *key;
  char *value;
} KeyValuePair;

typedef struct Request {
  RequestLine *request_line;
  KeyValuePair *params;
  char *path;  // put the path on here w/o the query params
} Request;

typedef struct Response {
  int status;
  char *body;
  char *content_type;
} Response;

typedef struct Route {
  HTTPMethod method;
  char path[128];
  Response *(*route_fn)(Request *);  // a route function takes a Request and returns a Response
} Route;

#endif
