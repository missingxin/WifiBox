#include "tasks.h"

#define DEMO_AP_SSID "2F Alt 2.4G"
#define DEMO_AP_PASSWORD "54519993"

void wifi_task(void *pvParameters)
{
    struct station_config sta_config;
    struct ip_info ip_config;
    bzero(&sta_config, sizeof(struct station_config));

    sprintf(sta_config.ssid, DEMO_AP_SSID);
    sprintf(sta_config.password, DEMO_AP_PASSWORD);
    wifi_station_set_config(&sta_config);
    os_printf("%s\n", __func__);
    wifi_get_ip_info(STATION_IF, &ip_config);
    while(ip_config.ip.addr == 0){
        vTaskDelay(1000 / portTICK_RATE_MS);
        wifi_get_ip_info(STATION_IF, &ip_config);
    printf("Connecting...\n");
    }
    printf("Connected\n");
    xTaskCreate(prvMQTTEchoTask,	/* The function that implements the task. */
            "MQTTEcho0",			/* Just a text name for the task to aid debugging. */
            512,	/* The stack size is defined in FreeRTOSIPConfig.h. */
            NULL,		/* The task parameter, not used in this case. */
            3,		/* The priority assigned to the task is defined in FreeRTOSConfig.h. */
            NULL);				/* The task handle is not used. */
    vTaskDelete(NULL);
}
