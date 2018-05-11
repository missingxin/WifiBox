#ifndef _TASKS_DEF_H_
#define _TASKS_DEF_H_

#include "esp_common.h"
#include "espconn.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"


void prvMQTTEchoTask(void *pvParameters);
void LEDBlinkTask (void *pvParameters);
void wifi_task(void *pvParameters);
void i2c_test_task(void *pvParameters);
void LEDBlinkTask (void *pvParameters);

#endif //_TASKS_DEF_H_