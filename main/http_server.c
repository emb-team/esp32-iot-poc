/*
 * HTTP Server implementation

*/

#include "freertos/FreeRTOS.h"

#include "http_server.h"
#include "poc.h"

static const char *TAG = "HTTP_SERVER: ";

// From auth_mode code to string
static char* getAuthModeName(wifi_auth_mode_t auth_mode) {

	char *names[] = { "OPEN", "WEP", "WPA PSK", "WPA2 PSK", "WPA WPA2 PSK", "WPA2 INTERPRISE", "MAX" };

	return names[auth_mode];
}

#define RESP_FORM "<form action=\"/\" method=\"post\">" \
        "<label for=\"ssid\">SSID: </label>" \
        "<input name=\"ssid\" id=\"ssid\" value=\"\"><br>" \
        "<label for=\"pass\">Password: </label>" \
        "<input name=\"pass\" id=\"pass\" value=\"\"><br>" \
        "<button>Send</button></body></html>"

static esp_err_t uri_get_handler(httpd_req_t *req)
{
    char buf[200];
    uint16_t ap_num;
    wifi_ap_record_t *ap_records;
    ap_records = calloc(POC_WIFI_MAX_APS, sizeof(wifi_ap_record_t));
    if (!ap_records) {
	ESP_LOGE(TAG, "Not enough memory to satisfy the request!\n");
	return ESP_FAIL;
    }

    // Get the list of WiFi APs
    ap_num = wifi_scan_aps(ap_records);
    if (ap_num > POC_WIFI_MAX_APS) {
	ESP_LOGE(TAG, "List of scanned WiFi APs is too big!");
	return ESP_FAIL;
    }

    snprintf(buf, sizeof(buf), "<html><body>Found %d access points:<br><br>", ap_num);
    httpd_resp_send_chunk(req, buf, strlen(buf));
    snprintf(buf, sizeof(buf), "<table><tr><th>SSID</th><th>Channel</th><th>RSSI</th><th>Auth Mode</th></tr>");
    httpd_resp_send_chunk(req, buf, strlen(buf));

    for(int i = 0; i < ap_num; i++) {
	ESP_LOGD(TAG, "SSID: %s auth %d",(char *)ap_records[i].ssid,ap_records[i].authmode);
	if (ap_records[i].authmode == WIFI_AUTH_MAX) {
		ESP_LOGE(TAG, "Wifi mode for SSID %s is unknown!\n", (char *)ap_records[i].ssid);
		return ESP_FAIL;
	}
	snprintf(buf, sizeof(buf), "<tr><td>%s</td><td>%d</td><td>%d</td><td>%s</td></tr>",
			(char *)ap_records[i].ssid, ap_records[i].primary, ap_records[i].rssi,
			getAuthModeName(ap_records[i].authmode));
	httpd_resp_send_chunk(req, buf, strlen(buf));
    }

    snprintf(buf, sizeof(buf), "</table><br><br>");
    httpd_resp_send_chunk(req, buf, strlen(buf));

    /* Respond with the accumulated value */
    httpd_resp_send_chunk(req, RESP_FORM, strlen(RESP_FORM));
    httpd_resp_send_chunk(req, NULL, 0);

    free(ap_records);

    return ESP_OK;
}

httpd_uri_t uri_get = {
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = uri_get_handler,
};

static esp_err_t uri_post_handler(httpd_req_t *req)
{
    char buf[100];
    char *ssid,*pass;
    int ret;

    memset(buf, 0x0, sizeof(buf));

    // Read client's socket
    ret = httpd_req_recv(req, buf, sizeof(buf));
    if (ret <= 0) {
	if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
		httpd_resp_send_408(req);
		return ESP_FAIL;
	}
    }

    ESP_LOGD(TAG,"Received data from user: %s", buf);

    ssid = strstr(buf, "ssid=");
    if (!ssid) {
	httpd_resp_send(req, RESP_FORM, strlen(RESP_FORM));
	return ESP_FAIL;
    }

    ssid = ssid + strlen("ssid=");
    for (pass = ssid; *pass != '\0' && *pass != '&'; pass++)
	    if (*pass == '+')
		    *pass = ' ';

    if (*pass != '&') {
	httpd_resp_send(req, RESP_FORM, strlen(RESP_FORM));
	return ESP_FAIL;
    }

    *pass = '\0'; pass++;

    if (g_data->wifi_sta_state == WIFI_STA_CONNECTING ||
		g_data->wifi_sta_state == WIFI_STA_CONNECTED) {
	wifi_config_t *sta = &g_data->wifi_sta_config;
	// Check if the user is trying to connect to the same AP
	if (!strncmp((char *) sta->sta.ssid, ssid, sizeof(sta->sta.ssid))) {
	    ESP_LOGI(TAG, "Skip POST request. Already connected to [%s]", sta->sta.ssid);
	    snprintf(buf, sizeof(buf),"NOT OK. Already connected to [%s]", sta->sta.ssid);
	    httpd_resp_send(req, buf, strlen(buf));
	    return ESP_FAIL;
	}
	ESP_LOGI(TAG, "POST request during existing STA connection. Disconnecting...");
	esp_wifi_disconnect();
	g_data->wifi_sta_state = WIFI_STA_DISCONNECTED;
    }

    if (strncmp(pass, "pass=", strlen("pass="))) {
	httpd_resp_send(req, RESP_FORM, strlen(RESP_FORM));
	return ESP_FAIL;
    }

    pass = pass + strlen("pass=");
    if (!*pass) {
	httpd_resp_send(req, RESP_FORM, strlen(RESP_FORM));
	return ESP_FAIL;
    }

    if (wifi_connect_sta(ssid, pass) < 0) {
	return ESP_FAIL;
    }

    snprintf(buf, sizeof(buf),"OK. Connecting to AP ...");
    httpd_resp_send(req, buf, strlen(buf));

    return ESP_OK;
}

httpd_uri_t uri_post = {
    .uri      = "/",
    .method   = HTTP_POST,
    .handler  = uri_post_handler,
};

void start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on address: port: '%d'", config.server_port);

    if (httpd_start(&g_data->server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(g_data->server, &uri_get);
        httpd_register_uri_handler(g_data->server, &uri_post);
    } else {
	ESP_LOGI(TAG, "Error starting server!");
    }
}

void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

