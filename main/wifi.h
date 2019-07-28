#ifndef _H_POC_WIFI_H_
#define _H_POC_WIFI_H_

#include "esp_wifi.h"

extern const int CONNECTED_BIT;

int wifi_scan_aps(wifi_ap_record_t *ap_records);
int wifi_connect_sta(char *ssid, char *pass);
void wifi_init_ap_sta(void);

#endif /* _H_POC_WIFI_H_ */


