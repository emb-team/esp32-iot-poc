#ifndef _H_POC_H_
#define _H_POC_H_

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
    /* WiFi AP Config */
    wifi_config_t wifi_ap_config;

    /* WiFi Station Config */
    wifi_config_t wifi_sta_config;

    /* State of WiFi softAP */
    wifi_ap_state_t wifi_ap_state;

    /* State of WiFi Station */
    wifi_sta_state_t wifi_sta_state;
};

#endif /* _H_POC_H_ */
