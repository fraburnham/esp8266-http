#include "osapi.h"
#include "string.h"

#include "HTTPTypes.h"
#include "HTTPConfig.h"

char * ICACHE_FLASH_ATTR
strip_path(char *path) {
  static char stripped_path[128];
  char *end = strchr(path, '?');
  if(!end) { return path; }
  int length = end - path;

  memcpy(stripped_path, path, (end - path));
  stripped_path[length] = '\0';

  return &stripped_path;
}

HTTPMethod ICACHE_FLASH_ATTR
parse_method(char *method) {
  if(strcmp(method, "GET") == 0) {
    return GET;
  } else if (strcmp(method, "POST") == 0) {
    return POST;
  }
}

KeyValuePair * ICACHE_FLASH_ATTR
extract_query_params(char *data) {
  static KeyValuePair params[MAX_PARAMS];
  static char raw_data[256*MAX_PARAMS];
  char *start = strchr(data, '?');

  if(!start) {
      return NULL;
  }

  memcpy(raw_data, start, os_strlen(start));
  start = &raw_data[0];

  char *end, *value;
  int i = 0;
  while(start) {
    start++;  // move past the last null that was inserted
    
    end = strchr(start, '&');
    // if there is an end plop a null there, else just keep moving (if end is null the loop will break next time)
    if(end) { *end = '\0'; }

    // needs protection against invalid input
    value = strchr(start, '=');
    *value = '\0';

    params[i].key = start;
    params[i].value = (value + 1);  // needs urldecode step

    start = end;
    i++;
  }
  
  return params;
}

RequestLine * ICACHE_FLASH_ATTR
extract_http_request_line(char *data) {
  static RequestLine request_line;
  char raw_request_line[512];  // N.B: this could cause issues w/ long urls or query params
  int length = (strchr(data, '\n') - data);

  memcpy(raw_request_line, data, length);
  raw_request_line[length] = '\0';

  static char method[8], path[256], version[10];

  length = strchr(raw_request_line, ' ') - raw_request_line;
  memcpy(method, raw_request_line, length);
  method[length] = '\0';

  char *offset = raw_request_line + (length + 1);  // also get the last separator behind us
  length = strchr(offset, ' ') - offset;
  memcpy(path, offset, length);
  path[length] = '\0';

  offset = offset + (length + 1);
  memcpy(version, offset, os_strlen(offset));

  request_line.method = parse_method(method);
  request_line.path = path;
  request_line.version = version;

  return &request_line;
}
