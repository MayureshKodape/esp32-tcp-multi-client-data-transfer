// TCP SERVER (Multiple Clients) - 05-09-2025
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"

#define WIFI_SSID       "Mayuresh"
#define WIFI_PASS       "12345678"
#define TCP_PORT        3333
#define TOTAL_BYTES     100000   // 1 lakh bytes per client
#define CHUNK_SIZE      1460     // MTU size

static const char *TAG = "TCP_SERVER";
static EventGroupHandle_t wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0

/* WiFi event handler */
static void wifi_event_handler(void *arg, esp_event_base_t base, int32_t id, void *data) {
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "WiFi disconnected, reconnecting...");
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/* WiFi STA init with static IP */
static void wifi_init_sta(void) {
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();

    // STATIC IP
    esp_netif_ip_info_t ip_info;
    IP4_ADDR(&ip_info.ip, 10, 180, 196, 150);  // ESP32 server IP
    IP4_ADDR(&ip_info.gw, 10, 180, 196, 1);    // Router IP
    IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
    ESP_ERROR_CHECK(esp_netif_dhcpc_stop(sta_netif));
    ESP_ERROR_CHECK(esp_netif_set_ip_info(sta_netif, &ip_info));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = { 0 };
    strncpy((char *)wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, WIFI_PASS, sizeof(wifi_config.sta.password));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Connecting to WiFi SSID:%s ...", WIFI_SSID);
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
    ESP_LOGI(TAG, "Server will use static IP: 10.180.196.150");
}

/* Handler for each client */
static void client_handler_task(void *pv) {
    int sock = (int)pv;
    uint8_t tx_buffer[CHUNK_SIZE];
    memset(tx_buffer, 'A', sizeof(tx_buffer));  // test data

    int total_sent = 0;
    while (total_sent < TOTAL_BYTES) {
        int to_send = (TOTAL_BYTES - total_sent > CHUNK_SIZE) ? CHUNK_SIZE : (TOTAL_BYTES - total_sent);
        int sent = send(sock, tx_buffer, to_send, 0);
        if (sent < 0) {
            ESP_LOGE(TAG, "send() failed: errno=%d", errno);
            break;
        }
        total_sent += sent;
        ESP_LOGI(TAG, "Client[%d] Sent %d/%d bytes", sock, total_sent, TOTAL_BYTES);
        vTaskDelay(pdMS_TO_TICKS(1));  // avoid Wi-Fi flooding
    }

    shutdown(sock, 0);
    close(sock);
    ESP_LOGI(TAG, "Client[%d] disconnected after %d bytes", sock, total_sent);
    vTaskDelete(NULL);  // end this task
}

/* TCP server task */
static void tcp_server_task(void *pv) {
    int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "socket() failed: errno=%d", errno);
        vTaskDelete(NULL);
        return;
    }

    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in bind_addr = {0};
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind_addr.sin_port = htons(TCP_PORT);

    if (bind(listen_sock, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) {
        ESP_LOGE(TAG, "bind() failed: errno=%d", errno);
        close(listen_sock);
        vTaskDelete(NULL);
        return;
    }

    if (listen(listen_sock, 5) < 0) {  // allow up to 5 pending clients
        ESP_LOGE(TAG, "listen() failed: errno=%d", errno);
        close(listen_sock);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "Server listening on port %d", TCP_PORT);

    while (1) {
        struct sockaddr_in source_addr;
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "accept() failed: errno=%d", errno);
            continue;
        }

        char addr_str[64];
        inet_ntoa_r(source_addr.sin_addr, addr_str, sizeof(addr_str) - 1);
        ESP_LOGI(TAG, "New client connected from %s:%d", addr_str, ntohs(source_addr.sin_port));

        // Spawn a new task for this client
        if (xTaskCreate(client_handler_task, "client_handler_task", 4096, (void *)sock, 5, NULL) != pdPASS) {
            ESP_LOGE(TAG, "Failed to create task for new client");
            close(sock);
        }
    }
}

extern "C" void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_sta();
    xTaskCreate(tcp_server_task, "tcp_server_task", 4096, NULL, 5, NULL);
}
