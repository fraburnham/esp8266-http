// Example usage of HTTPServer for esp8266

#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"  // needed?
#include "os_type.h"
#include "user_interface.h"
#include "espconn.h"
#include "user_config.h"
#include "string.h"
#include "HTTPServer.h"
#include "HTTPTypes.h"

uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set (void) {
  enum flash_size_map size_map = system_get_flash_size_map();
  uint32 rf_cal_sec = 0;

  switch (size_map) {
  case FLASH_SIZE_4M_MAP_256_256:
    rf_cal_sec = 128 - 5;
    break;
    
  case FLASH_SIZE_8M_MAP_512_512:
    rf_cal_sec = 256 - 5;
    break;
    
  case FLASH_SIZE_16M_MAP_512_512:
  case FLASH_SIZE_16M_MAP_1024_1024:
    rf_cal_sec = 512 - 5;
    break;
    
  case FLASH_SIZE_32M_MAP_512_512:
  case FLASH_SIZE_32M_MAP_1024_1024:
    rf_cal_sec = 512 - 5;
    break;
    
  case FLASH_SIZE_64M_MAP_1024_1024:
    rf_cal_sec = 2048 - 5;
    break;
    
  case FLASH_SIZE_128M_MAP_1024_1024:
    rf_cal_sec = 4096 - 5;
    break;
    
  default:
    rf_cal_sec = 0;
    break;
  }
  
  return	rf_cal_sec;
}

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void user_procTask(os_event_t *events);

static volatile os_timer_t some_timer;

Response * ICACHE_FLASH_ATTR
home_route(Request *request) {
  static Response response = {
    .status = 200,
    .body = "<html><h1>Hello world!</h1><br /><form action='/you'>Your name?:<input type='text' name='name' /><input type='submit' value='Submit' /></html>",
    .content_type = "text/html"
  };
  
  return &response;
}

Response * ICACHE_FLASH_ATTR
name_route(Request *request) {
  KeyValuePair *params = request->params;
  char *body_fmt = "<html><h1>Hello %s!</h1></html>";
  static char body[512];
  
  while(params && params->key) {
    if(strcmp(params->key, "name") == 0) {
      os_sprintf(body, body_fmt, params->value);
    }
    params++;
  }
  
  static Response response = {
    .status = 200,
    .body = body,  // needs to have the name injected
    .content_type = "text/html"
  };

  return &response;
}

// routes & route_count are `extern`d in `HTTPServer.h`
// I need to find a better way to pass the data in...
Route routes[] = {
  {
    .method = GET,
    .path = "/",
    .route_fn = home_route
  },
  {
    .method = GET,
    .path = "/you", // this isn't being matched due to query params, need to remove them before trying to match, innit
    .route_fn = name_route
  }
};
int route_count = 2;

void ICACHE_FLASH_ATTR
some_timerfunc(void *arg) {
  os_printf("Alive!\n");
}

void ICACHE_FLASH_ATTR
post_init_setup() {
  //enable softap
  if(!wifi_set_opmode(SOFTAP_MODE)) {
    os_printf("Failed to set wifi opmode!!\n");
  }
  
  // setup softap config
  struct softap_config config = {
    .authmode = AUTH_WPA2_PSK,
    .password = "youshouldchangethis",
    .ssid = "Skytemp 00",
    .ssid_len = 10,
    .channel = 2,
    .max_connection = 1
  };
  
  if(!wifi_softap_set_config(&config)) {
    os_printf("Failed to set softap config!\n");
  }

  // move some of this to a server config fn or something
  // maybe a `start_server` (it could take the routes and set them up)
  LOCAL esp_tcp http_connection = {
    .local_port = 80,
  };
  
  LOCAL struct espconn esp_connection = {
    .type = ESPCONN_TCP,
    .proto.tcp = &http_connection,
    .state = ESPCONN_NONE
  };

  if(espconn_regist_connectcb(&esp_connection, &webserver_listen) != 0) {
    os_printf("Failed to register connect callback!\n");
  }
  
  if(espconn_accept(&esp_connection) != 0) {
    os_printf("Failed to setup espconn!\n");
  }
}

void ICACHE_FLASH_ATTR
user_init() {
  // do some delay (maybe) and output the current station mode
  // then try to adjust it and what have you
  // eventually try to connect to the house AP
  uart_init(BIT_RATE, BIT_RATE);

  //Disarm timer
  os_timer_disarm(&some_timer);

  //Setup timer
  os_timer_setfn(&some_timer, (os_timer_func_t *)some_timerfunc, NULL);

  //Arm the timer
  //&some_timer is the pointer
  //1000 is the fire time in ms
  //0 for once and 1 for repeating
  os_timer_arm(&some_timer, 20000, 1);

  system_init_done_cb(&post_init_setup);
}
