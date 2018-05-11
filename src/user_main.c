/*
* ESPRSSIF MIT License
*
* Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
*
* Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
* it is free of charge, to any person obtaining a copy of this software and associated
* documentation files (the "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the Software is furnished
* to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or
* substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*/


#include "tasks.h"


#include "uart.h"
#include "gpio.h"

#define GPIO_INPUT(gpio_bits)       ((gpio_input_get()&gpio_bits)?1:0)
#define GPIO_INTERRUPT_ENABLE   _xt_isr_unmask(1 << ETS_GPIO_INUM)

void gpio_config(GPIO_ConfigTypeDef *pGPIOConfig);

/******************************************************************************
 * FunctionName : gpio_intr_handler - generic handling of GPIO interrupts
*******************************************************************************/
void (*gpio_intr_callbacks[16])(void);
void gpio_intr_handler() {
    static uint32 prevTime = 0;
    int i;
    uint32 gpio_mask = _xt_read_ints();
    uint32_t gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    for (i=0 ; i<16 ; i++){
        if ( (0x1<<i) & gpio_status){
            printf("**\n");
            printf("interrupt@%dus: mask=0x%02x, status=0x%02x\n",system_get_time()-prevTime,gpio_mask,gpio_status);
            prevTime = system_get_time();
            if (gpio_intr_callbacks[i]!= NULL ){
                (*gpio_intr_callbacks[i])();
            }
        }
    }
            
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status );     //clear interrupt status
}

//****** THE ITEMS ABOVE SHOULD BE ADDED TO THE GPIO driver files ************//

/******************************************************************************
 * FunctionName : Pin0 specific interrupt handler
*******************************************************************************/
LOCAL void callback4() {
    static uint32  oldtime;
//  os_printf("oldtime: %d\n",oldtime);
    if ( (oldtime+200)<(oldtime=(system_get_time()/1000) ) ) {  //200ms debounce guard
        GPIO_OUTPUT(GPIO_Pin_2,GPIO_INPUT(GPIO_Pin_2)^1);       //toggle
        os_printf("Pin2-toggle\n");
    }
//  os_printf("newtime: %d\n",oldtime);
}





/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
    user_rf_cal_sector_set();
    // Initialize UART for debugging
    EEPROM_Init();
    uart_init_new();
    UART_SetBaudrate(0,115200);
    wifi_set_opmode(STATION_MODE);
    uint8_t wifiStatus = STATION_IDLE;
    printf("SDK version:%s\n\r", system_get_sdk_version());
    //wifi_station_scan(NULL,scan_done);
    xTaskCreate(wifi_task, "wifi_task", 256, NULL, 2, NULL);
    xTaskCreate(i2c_test_task, "user_start_tx_task", 200, NULL, 3, NULL);


    // GPIO_ConfigTypeDef EE_WC;                            //Define GPIO Init Structures
    // EE_WC.GPIO_IntrType = GPIO_PIN_INTR_DISABLE;         //Falling edge trigger
    // EE_WC.GPIO_Mode     = GPIO_Mode_Output;               //Input mode
    // EE_WC.GPIO_Pin      = GPIO_Pin_2;                    //Enable GPIO
    // gpio_config(&EE_WC);                                 //Initialization function


    GPIO_ConfigTypeDef gpio4_in_cfg;                            //Define GPIO Init Structures
    //GPIO_ConfigTypeDef gpio5_in_cfg;

    gpio4_in_cfg.GPIO_IntrType = GPIO_PIN_INTR_ANYEDGE;         //Falling edge trigger
    gpio4_in_cfg.GPIO_Mode     = GPIO_Mode_Input;               //Input mode
    gpio4_in_cfg.GPIO_Pin      = GPIO_Pin_4;                    //Enable GPIO
    gpio_config(&gpio4_in_cfg);                                 //Initialization function
    ////////////////////////////////////////////////////gpio_intr_handler_register(gpio_intr_handler,NULL);         //Register the interrupt function
    GPIO_INTERRUPT_ENABLE;

    gpio_intr_callbacks[4]=callback4;                           //define the Pin0 callback
/*
    gpio5_in_cfg.GPIO_IntrType = GPIO_PIN_INTR_DISABLE;         //no interrupt
    gpio5_in_cfg.GPIO_Mode     = GPIO_Mode_Output;              //Output mode
    gpio5_in_cfg.GPIO_Pullup   = GPIO_PullUp_EN;                //improves transitions
    gpio5_in_cfg.GPIO_Pin      = GPIO_Pin_5;                    //Enable GPIO
    gpio_config(&gpio5_in_cfg);                                 //Initialization function
*/

}

