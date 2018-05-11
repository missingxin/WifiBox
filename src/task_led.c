#include "tasks.h"
#include "gpio.h"

void LEDBlinkTask (void *pvParameters)
{
    // Delay and turn on
    printf("===========================led on===============================\n");
    GPIO_OUTPUT_SET (0, 1);

    // Delay and LED off
    vTaskDelay (500/portTICK_RATE_MS);
    printf("===============================led off===========================\n");
    GPIO_OUTPUT_SET (0, 0);
    vTaskDelete( pvParameters );
}
