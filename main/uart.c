/*
 * UART communication

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <sys/unistd.h>
#include <sys/select.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "esp_vfs_dev.h"
#include "driver/uart.h"

#include "uart.h"

const char *TAG = "UART: ";

static void uart_runner()
{
    int fd;
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);
    uart_driver_install(UART_NUM_0, 2*1024, 0, 0, NULL, 0);

    if ((fd = open("/dev/uart/0", O_RDWR)) == -1) {
        ESP_LOGE(TAG, "Cannot open UART");
	vTaskDelete(NULL);
	return;
    }

    // Set up the UART driver
    esp_vfs_dev_uart_use_driver(0);

    while  ( 1 )
    {
	int s;
	fd_set rfds;
	struct timeval tv = {
		.tv_sec = 5,
	.tv_usec = 0,
	};

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	s = select(fd + 1, &rfds, NULL, NULL, &tv);

	if (s < 0) {
		ESP_LOGE(TAG, "Select failed: errno %d", errno);
		break;
	} else if (s == 0) {
		ESP_LOGD(TAG, "Timeout has been reached and nothing has been received");
	} else {
		if (FD_ISSET(fd, &rfds)) {
                    char buf;
		    // Read 1 character a time
                    if (read(fd, &buf, 1) > 0) {
                        ESP_LOGI(TAG, "Received: %c", buf);
                    } else {
                        ESP_LOGE(TAG, "UART read error");
                        break;
                    }
                } else {
                    ESP_LOGE(TAG, "No FD has been set in select()");
                    break;
                }
	}
    }

    close(fd);

    vTaskDelete(NULL);
}

int setup_uart(void)
{
    int ret;

    ret = xTaskCreate(uart_runner, "uart_runner", 4*1024, NULL, 5, NULL);
    if (ret != pdPASS)  {
        ESP_LOGI(TAG, "create thread uart_runner failed");
	return ESP_FAIL;
    }

    return ESP_OK;
}
