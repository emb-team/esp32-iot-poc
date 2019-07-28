#ifndef _H_POC_H_
#define _H_POC_H_

#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "mqtt_client.h"

#include "wifi.h"

/* The examples use WiFi configuration that you can set via 'make menuconfig'.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define POC_WIFI_SSID "mywifissid"
*/
#define POC_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define POC_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define POC_WIFI_MAX_STA_CONN  CONFIG_WIFI_MAX_STA_CONN
#define POC_WIFI_MAX_APS       CONFIG_WIFI_MAX_APS_PER_SCAN

/**
 * @brief   WiFi STA status
 */
typedef enum {
    WIFI_AP_STOPPED,
    WIFI_AP_STARTED
} wifi_ap_state_t;

/**
 * @brief   WiFi STA status
 */
typedef enum {
    WIFI_STA_CONNECTING,
    WIFI_STA_CONNECTED,
    WIFI_STA_DISCONNECTED
} wifi_sta_state_t;

/**
 * @brief   POC data structure
 */
struct poc_data
{
    /* Http server handle */
    httpd_handle_t server;

    /* WiFi AP Config */
    wifi_config_t wifi_ap_config;

    /* WiFi Station Config */
    wifi_config_t wifi_sta_config;

    /* State of WiFi softAP */
    wifi_ap_state_t wifi_ap_state;

    /* State of WiFi Station */
    wifi_sta_state_t wifi_sta_state;

    /* Event group to signal */
    EventGroupHandle_t wifi_event_group;

    /* Mqtt client handle */
    esp_mqtt_client_handle_t mqtt_client;
};

/* Global POC data structure */
extern struct poc_data *g_data;

#define STA_CONNECTED_BIT BIT0
#define OTA_AVAILABLE_BIT BIT1

#endif /* _H_POC_H_ */
