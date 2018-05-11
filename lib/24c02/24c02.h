
#ifndef _24C02_H_
#define _24C02_H_
unsigned char EEPROM_Init();
unsigned int EEFSR(unsigned char dataIndex);
void EEFSW(unsigned char dataIndex, unsigned int data);



#include "esp_common.h"
#include "espressif/espconn.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "uart.h"
#include "gpio.h"
#include "i2c_master.h"
#define ee24c02_W 0xA0
#define ee24c02_R 0xA1




#define EE_NUM_STORAGE_BYTES                   256
#define EE_NUM_SECIdx                          72
#define EE_NUM_SECTORS                         EE_NUM_SECIdx
#define EE_SECTOR_SIZE                         2
#define EE_SECIdx_SIZE                         1
#define EE_SECIdx_ADDRESS                      0
#define EE_SECTOR_BANK_ADDRESS                 (EE_NUM_SECTORS*EE_SECIdx_SIZE)
#define EE_MAX_PHYSICAL_SECTORS                ((EE_NUM_STORAGE_BYTES - EE_NUM_SECTORS)/EE_SECTOR_SIZE)
#define EE_FINAL_PHYSICAL_SECTOR               EE_MAX_PHYSICAL_SECTORS - 1

#define EE_index_content_to_physical_address(idx_content)    (EE_NUM_SECTORS+(idx_content*2))

#define EE_READ_ERROR_STATUS_BIT_MASK                       0b1
#define EE_WRITE_ERROR_STATUS_BIT_MASK                      0b10
#define EE_OUT_OF_SECTOR_ERROR_STATUS_BIT_MASK              0b100
#define EE_ACK_ERROR_STATUS_BIT_MASK                        0b1000

#define EEADDR_STANDARD_DA_STRENGTH                         0
#define EEADDR_STANDARD_DA_ON_HIGH_AD                       25
#define EEADDR_STANDARD_DA_ON_LOW_AD                        40
#define EEADDR_PIECE_SETTING                                55
#define EEADDR_CCTALK_ID_ADDR                               59
#define EEADDR_LOADER_STATUS                                60
#define EEADDR_CUSTOMIZED_PULSE                             61


unsigned char FS_autoformat();
unsigned char FS_is_formatted();
void FS_format();
unsigned char FS_find_number_of_used_sector();
unsigned char FS_read_physical(unsigned char eeAddress);
void FS_write_physical(unsigned char eeAddress, unsigned char data);
unsigned char EEPROM_validation();




#define EEPROM_S_ADDR_W     0xA0
#define EEPROM_S_ADDR_R     0xA1

unsigned char EEPROM_Read_Byte(unsigned char sub_address);
unsigned char EEPROM_Write(unsigned char sub_address, unsigned char value);

#endif 

