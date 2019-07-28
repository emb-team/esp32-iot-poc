/*  POC Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_flash.h"
#include "freertos/portable.h"

#include "uart.h"
#include "http_server.h"
#include "wifi.h"
#include "ota.h"
#include "poc.h"

static const char *TAG = "POC";

/* Global POC data structure */
struct poc_data *g_data;

int app_main()
{
    ESP_LOGI(TAG, "POC version v0.1\n");

    ESP_LOGI(TAG, "Free heap: %u\n", xPortGetFreeHeapSize());

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize and setup UART interface
    ret = setup_uart();
    ESP_ERROR_CHECK(ret)

    /* Allocate memory for app data */
    g_data = (struct poc_data *) calloc(1, sizeof(struct poc_data));
    if (!g_data) {
        ESP_LOGI(TAG, "Unable to allocate application data structure");
        return ESP_ERR_NO_MEM;
    }

    memset(g_data, 0x0, sizeof(struct poc_data));

    g_data->wifi_ap_state = WIFI_AP_STOPPED;
    g_data->wifi_sta_state = WIFI_STA_DISCONNECTED;

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP_STA");
    // Initialize WiFi interface 
    wifi_init_ap_sta();

    // Setup OTA
    ESP_ERROR_CHECK(setup_ota());

    return 0;
}
