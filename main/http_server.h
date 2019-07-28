#ifndef _H_HTTP_SERVER_H_
#define _H_HTTP_SERVER_H_

#include "esp_http_server.h"

extern httpd_uri_t uri_post;
extern httpd_uri_t uri_get;

void start_webserver(void);
void stop_webserver(httpd_handle_t server);

#endif /* _H_HTTP_SERVER_H_ */
