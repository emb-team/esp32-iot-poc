/*
 * MQTT functions

*/

#include "freertos/FreeRTOS.h"

#include "mqtt.h"
#include "poc.h"

static const char *TAG = "MQTT: ";

extern const uint8_t iot_eclipse_org_pem_start[]   asm("_binary_iot_eclipse_org_pem_start");
extern const uint8_t iot_eclipse_org_pem_end[]   asm("_binary_iot_eclipse_org_pem_end");

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, "/topic/ota_update", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            ESP_LOGD(TAG, "TOPIC=%.*s\r\n", event->topic_len, event->topic);
            ESP_LOGD(TAG, "DATA=%.*s\r\n", event->data_len, event->data);
	    if (!strncmp("poc_v2.bin", event->data, strlen("poc_v2.bin"))) {
		ESP_LOGI(TAG, "New FOTA update available! Updating...");
		xEventGroupSetBits(g_data->wifi_event_group, OTA_AVAILABLE_BIT);
	    }
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_runner()
{
    /* Wait for the callback to set the STA_CONNECTED_BIT in the
       event group.
    */
    xEventGroupWaitBits(g_data->wifi_event_group, STA_CONNECTED_BIT,
                        false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "ESP connected to STA AP! Connecting to MQTT server....");

    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_BROKER_URI,
        .event_handle = mqtt_event_handler,
        .cert_pem = (const char *)iot_eclipse_org_pem_start,
    };

    g_data->mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(g_data->mqtt_client);

    vTaskDelete(NULL);
}

int mqtt_start(void)
{
    int ret;

    ret = xTaskCreate(mqtt_runner, "mqtt_runner", 4*1024, NULL, 5, NULL);
    if (ret != pdPASS)  {
        ESP_LOGI(TAG, "create thread mqtt_runner failed");
	return ESP_FAIL;
    }

    return ESP_OK;
}
