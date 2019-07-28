/*
 * OTA implementation

*/

#include "freertos/FreeRTOS.h"

#include "esp_http_client.h"
#include "esp_https_ota.h"

#include "ota.h"
#include "poc.h"

extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

static const char *TAG = "OTA: ";

static esp_err_t https_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}

static void ota_runner(void * p)
{
    ESP_LOGI(TAG, "Starting OTA Runner ...");

    /* Wait for the callback to set the CONNECTED_BIT in the
       event group.
    */
    xEventGroupWaitBits(g_data->wifi_event_group, CONNECTED_BIT,
                        false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "Connected to WiFi STA AP! Connecting to OTA server....");

    esp_http_client_config_t config = {
        .url = CONFIG_FIRMWARE_UPDATE_URL,
        .cert_pem = (char *)server_cert_pem_start,
        .event_handler = https_event_handler,
    };

    ESP_LOGI(TAG, "Free heap: %u\n", xPortGetFreeHeapSize());

    esp_err_t ret = esp_https_ota(&config);
    if (ret == ESP_OK) {
        esp_restart();
    } else {
        ESP_LOGE(TAG, "FOTA update failed");
    }

    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

int setup_ota(void)
{
    int ret;

    ret = xTaskCreate(ota_runner, "ota_runner", 8192, NULL, 5, NULL);
    if (ret != pdPASS)  {
        ESP_LOGI(TAG, "create thread ota_runner failed");
	return ESP_FAIL;
    }

    return ESP_OK;
}
