/*
 * WiFI functions

*/

#include "freertos/FreeRTOS.h"
#include "esp_event_loop.h"

#include "http_server.h"
#include "poc.h"

static const char *TAG = "WIFI: ";

int wifi_scan_aps(wifi_ap_record_t *ap_records)
{
    uint16_t ap_num = POC_WIFI_MAX_APS;
    wifi_scan_config_t wifi_scan_config = {
            .ssid = 0,
            .bssid = 0, 
            .channel = 0,
	    .show_hidden = true
    };

    if (!ap_records) {
	return 0;
    }

    ESP_ERROR_CHECK(esp_wifi_scan_start(&wifi_scan_config, true));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_num, ap_records)); 

    return ap_num;
}


int wifi_connect_sta(char *ssid, char *pass)
{
    wifi_config_t *sta = &g_data->wifi_sta_config;

    memset(sta, 0x0, sizeof(wifi_config_t));

    sta->sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    sta->sta.bssid_set = 0;
    sta->sta.listen_interval = 1; // Without increased interval doesn't connect ???

    strncpy((char *)sta->sta.ssid, ssid, sizeof(sta->sta.ssid));
    strncpy((char *)sta->sta.password, pass, sizeof(sta->sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, sta) );

    // Restart Wifi (The Mode APSTA should work)
    ESP_ERROR_CHECK(esp_wifi_start());

    /* Connect to AP */
    if (esp_wifi_connect() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to connect WiFi");
        return ESP_FAIL;
    }

    g_data->wifi_sta_state = WIFI_STA_CONNECTING;
    ESP_LOGI(TAG, "Connecting to AP [%s] ...", ssid);
    return 0;
}

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_AP_STACONNECTED:
        ESP_LOGI(TAG, "station:"MACSTR" join, AID=%d",
                 MAC2STR(event->event_info.sta_connected.mac),
                 event->event_info.sta_connected.aid);
	
	// If user phone is connected start webserver
	start_webserver();
	break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        ESP_LOGI(TAG, "station:"MACSTR"leave, AID=%d",
                 MAC2STR(event->event_info.sta_disconnected.mac),
                 event->event_info.sta_disconnected.aid);

        // If the user phone is disconnected stop webserver	
	stop_webserver(g_data->server);
	break;
    case SYSTEM_EVENT_STA_START:
	ESP_LOGI(TAG,"STA_START event\n");
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
	ESP_LOGI(TAG,"Connect to the AP failed: station: %s reason: %d\n",
		(char*)event->event_info.disconnected.ssid,
		event->event_info.disconnected.reason);
	
	xEventGroupClearBits(g_data->wifi_event_group, STA_CONNECTED_BIT);

	/* Set code corresponding to the reason for disconnection */
        switch (event->event_info.disconnected.reason) {
        case WIFI_REASON_AUTH_EXPIRE:
        case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
        case WIFI_REASON_BEACON_TIMEOUT:
        case WIFI_REASON_AUTH_FAIL:
        case WIFI_REASON_ASSOC_FAIL:
        case WIFI_REASON_HANDSHAKE_TIMEOUT:
            ESP_LOGI(TAG, "STA Auth Error");
	    g_data->wifi_sta_state = WIFI_STA_DISCONNECTED;
            break;
        case WIFI_REASON_NO_AP_FOUND:
            ESP_LOGI(TAG, "STA AP Not found");
	    esp_wifi_disconnect();
	    g_data->wifi_sta_state = WIFI_STA_DISCONNECTED;
            break;
        default:
            ESP_LOGI(TAG, "STA connect again");
            esp_wifi_connect();
	    g_data->wifi_sta_state = WIFI_STA_CONNECTING;
        }
	break;
    case SYSTEM_EVENT_STA_GOT_IP:
	ESP_LOGI(TAG, "got ip:%s", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
	xEventGroupSetBits(g_data->wifi_event_group, STA_CONNECTED_BIT);
        break;	
    default:
        break;
    }
    return ESP_OK;
}

void wifi_init_ap_sta(void)
{
    g_data->wifi_event_group = xEventGroupCreate();
    wifi_config_t *ap = &g_data->wifi_ap_config;

    // Initialize TCP/IP stack
    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    // Initialize WiFi
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Config for SoftAP mode
    strncpy((char *) ap->ap.ssid, POC_ESP_WIFI_SSID, sizeof(ap->ap.ssid));
    ap->ap.ssid_len = strlen(POC_ESP_WIFI_SSID);
    strncpy((char *) ap->ap.password, POC_ESP_WIFI_PASS, sizeof(ap->ap.password));
    ap->ap.max_connection = POC_WIFI_MAX_STA_CONN;
    ap->ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

    // if there is no password make AP open
    if (strlen(POC_ESP_WIFI_PASS) == 0) {
        ap->ap.authmode = WIFI_AUTH_OPEN;
    }

    // Initialize WiFi in SoftAP and STA mode simultaniously
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, ap));

    // Start SoftAP
    ESP_ERROR_CHECK(esp_wifi_start());

    g_data->wifi_ap_state = WIFI_AP_STARTED;

    ESP_LOGI(TAG, "wifi_init_ap_sta finished. ssid:%s password:%s",
             POC_ESP_WIFI_SSID, POC_ESP_WIFI_PASS);
}
