#include "tasks.h"
#include "24c02.h"

void i2c_test_task(void *pvParameters){
    unsigned int i;
    for(i=0;i<0x48;i++){
        unsigned short write = ((i<<8) + i);
        EEFSW(i,write);
        unsigned short read = EEFSR(i);
        printf("%02d write : read  = %04x : %04x\n",i,write,read);
    }
    vTaskDelete(NULL);
}


