#ifndef _H_POC_WIFI_H_
#define _H_POC_WIFI_H_

#include "esp_wifi.h"

extern const int CONNECTED_BIT;

int wifi_scan_aps(wifi_ap_record_t *ap_records);
struct sta_data
{
	uint8_t ssid[32];	/* SSID of target AP. Null terminated string. */
	uint8_t user[128];	/* User name for WPA2 enterprise */
	uint8_t pass[128];	/* Password */
};

int wifi_connect_sta(struct sta_data *);
void wifi_init_ap_sta(void);

#endif /* _H_POC_WIFI_H_ */


