

#include "./24c02.h"



unsigned char EEPROM_RW_ERROR_STATUS = 0;
unsigned char EEPROM_MOST_PHYSICAL_SECTOR = 0;
unsigned char EEPROM_ERROR_COUNT = 0;
unsigned char EEPROM_LATEST_READ_ERROR_ADDRESS = 0;
unsigned char EEPROM_LATEST_WRITE_ERROR_ADDRESS = 0;



unsigned char EEPROM_Init(){
    i2c_master_gpio_init();
    i2c_master_stop();
    vTaskDelay(1);
    GPIO_OUTPUT_SET (2, 0);
    return FS_autoformat();
}


unsigned int EEFSR(unsigned char dataIndex){
    EEPROM_RW_ERROR_STATUS = 0;
    unsigned char phyIdx = FS_read_physical(EE_SECIdx_ADDRESS+dataIndex*EE_SECIdx_SIZE);
    if(EEPROM_RW_ERROR_STATUS) return 0;
    unsigned char realAddr = EE_index_content_to_physical_address(phyIdx);
    unsigned int res = (((unsigned int)FS_read_physical(realAddr+1))<<8) + FS_read_physical(realAddr);
    if(EEPROM_RW_ERROR_STATUS) return 0;
    return res;
}

void EEFSW(unsigned char dataIndex, unsigned int data){
    EEPROM_RW_ERROR_STATUS = 0;
    unsigned char phyIdx = FS_read_physical(EE_SECIdx_ADDRESS+dataIndex*EE_SECIdx_SIZE);
    if(EEPROM_RW_ERROR_STATUS) return;


    unsigned char realAddr = EE_index_content_to_physical_address(phyIdx);
    FS_write_physical(realAddr,data&0xFF);
    FS_write_physical(realAddr+1,data>>8);
    if(EEPROM_RW_ERROR_STATUS){
        unsigned char current_testing_sector = FS_find_number_of_used_sector()+1;
        EEPROM_MOST_PHYSICAL_SECTOR = current_testing_sector;
        do{
            EEPROM_RW_ERROR_STATUS = 0;
            realAddr = EE_index_content_to_physical_address(current_testing_sector);
            FS_write_physical(realAddr,data&0xFF);
            FS_write_physical(realAddr+1,data>>8);
            if(EEPROM_RW_ERROR_STATUS){
                current_testing_sector++;
                EEPROM_MOST_PHYSICAL_SECTOR = current_testing_sector;
                if(current_testing_sector>=EE_MAX_PHYSICAL_SECTORS){
                    return;
                }
            }else{
                break;
            }
        }while(1);
    }
}




unsigned char EEPROM_Read_Byte(unsigned char sub_address){
    i2c_master_start();
    i2c_master_writeByte(ee24c02_W);
    i2c_master_getAck();
    i2c_master_writeByte(sub_address);
    i2c_master_getAck();
    i2c_master_start();
    i2c_master_writeByte(ee24c02_R);
    i2c_master_getAck();
    unsigned char read = i2c_master_readByte();
    i2c_master_send_nack();
    i2c_master_stop();
    return read;
}

unsigned char EEPROM_Write(unsigned char sub_address, unsigned char value){
    i2c_master_start();
    i2c_master_writeByte(ee24c02_W);
    i2c_master_getAck();
    i2c_master_writeByte(sub_address);
    i2c_master_getAck();
    i2c_master_writeByte(value);
    i2c_master_getAck();
    i2c_master_stop();
    vTaskDelay(5);
}





unsigned char EEPROM_validation(){
    unsigned int addr;
    unsigned char buff[256] = {0};
    unsigned char faultCount = 0;
    for(addr = 0;addr<256;addr++){
        buff[addr] = EEPROM_Read_Byte(addr);
        ClrCWdt();
    }

    for(addr = 0;addr<256;addr++){
        EEPROM_Write(addr, ~(buff[addr]));
        ClrCWdt();
    }
    for(addr = 0;addr<256;addr++){
        unsigned char read = EEPROM_Read_Byte(addr);
        if ((unsigned char)read != (unsigned char)~(buff[addr])){
            if (faultCount<255){
                faultCount++;
            }
        }
        ClrCWdt();
    }
    for(addr = 0;addr<256;addr++){
        EEPROM_Write(addr, buff[addr]);
        ClrCWdt();
    }
    for(addr = 0;addr<256;addr++){
        unsigned read = EEPROM_Read_Byte(addr);
        if ((unsigned char)read != (unsigned char)buff[addr]){
            if (faultCount<255){
                faultCount++;
            }
        }
        ClrCWdt();
    }
    return faultCount;
}

unsigned char FS_autoformat(){
    unsigned char isEEFormated = FS_is_formatted();
    if(EEPROM_RW_ERROR_STATUS){
        return 2;
    }else if(!isEEFormated){
        FS_format();
    }
    return isEEFormated;
}
unsigned char FS_is_formatted(){
    unsigned char idxVal[EE_NUM_SECIdx];
    unsigned char i,j;
    for(i=0;i<EE_NUM_SECIdx;i++){
        EEPROM_RW_ERROR_STATUS = 0;
        idxVal[i] = FS_read_physical(i+EE_SECIdx_ADDRESS);
        if((EEPROM_RW_ERROR_STATUS & EE_READ_ERROR_STATUS_BIT_MASK)) return 0;
        if(EEPROM_MOST_PHYSICAL_SECTOR < idxVal[i]){EEPROM_MOST_PHYSICAL_SECTOR = idxVal[i];}
        if(idxVal[i]>EE_FINAL_PHYSICAL_SECTOR) return 0;

        for(j=0;j<i;j++){
            if (i == j) continue;
            if (idxVal[i] == idxVal[j]) return 0;
        }
    }
    return 1;
}
void FS_format(){
    unsigned char idxList[EE_NUM_SECIdx];
    unsigned char randPos;
    unsigned char tmpidx;
    unsigned char i,j,failCnt;
    unsigned char goodSector;
    for(i=0;i<EE_NUM_SECIdx;i++)
        idxList[i] = i;
    unsigned char current_sector_to_try = 0;
    EEPROM_MOST_PHYSICAL_SECTOR = 0;

    for(i=0;i<EE_NUM_SECIdx;i++){


        EEPROM_RW_ERROR_STATUS = 0;
        do{
            goodSector = 0;
            for(failCnt=0;failCnt<3;failCnt++){
                for(j=0;j<EE_SECTOR_SIZE;j++){
                    FS_write_physical( EE_SECTOR_BANK_ADDRESS + current_sector_to_try*EE_SECTOR_SIZE + j ,0xFF);
                    FS_write_physical( EE_SECTOR_BANK_ADDRESS + current_sector_to_try*EE_SECTOR_SIZE + j ,0x00);
                    if(!EEPROM_RW_ERROR_STATUS)
                        goodSector = 1;
                }
                if(goodSector == 1) { break; }
            }
            if(goodSector == 1){
                break;
            }else{
                current_sector_to_try++;
                EEPROM_MOST_PHYSICAL_SECTOR++;
                if (current_sector_to_try >= EE_MAX_PHYSICAL_SECTORS){
                    EEPROM_RW_ERROR_STATUS |= EE_OUT_OF_SECTOR_ERROR_STATUS_BIT_MASK;
                    EEPROM_ERROR_COUNT++;
                    return;
                }
            }

        }while(1);
        for(j=0;j<EE_SECIdx_SIZE;j++){
            FS_write_physical( EE_SECIdx_ADDRESS      + idxList[i]           *EE_SECIdx_SIZE +j   , (~current_sector_to_try));
            FS_write_physical( EE_SECIdx_ADDRESS      + idxList[i]           *EE_SECIdx_SIZE +j   , (current_sector_to_try));
            if(EEPROM_RW_ERROR_STATUS) return;
        }
        EEPROM_MOST_PHYSICAL_SECTOR++;
        current_sector_to_try++;
    }
}

unsigned char FS_find_number_of_used_sector(){
    unsigned char maxSectorUsed = 0;
    unsigned char i;
    for(i=0;i<EE_NUM_SECIdx;i++){
        EEPROM_RW_ERROR_STATUS = 0;
        unsigned char currSector = FS_read_physical(EE_SECIdx_ADDRESS+i*EE_SECIdx_SIZE);
        if(EEPROM_RW_ERROR_STATUS)return 0;
        if(maxSectorUsed<currSector){
            maxSectorUsed = currSector;
        }
    }
    EEPROM_MOST_PHYSICAL_SECTOR = maxSectorUsed;
    return maxSectorUsed;
}

unsigned char FS_read_physical(unsigned char eeAddress){
    unsigned char res0 = EEPROM_Read_Byte(eeAddress);
    unsigned char res1 = EEPROM_Read_Byte(eeAddress);
    if(res0 == res1){
        return res0;
    }else{
        unsigned char res2 = EEPROM_Read_Byte(eeAddress);
        if(res2==res0 || res2 == res1){
            return res2;
        }
        EEPROM_RW_ERROR_STATUS |= EE_READ_ERROR_STATUS_BIT_MASK;
        EEPROM_ERROR_COUNT++;
        EEPROM_LATEST_READ_ERROR_ADDRESS = eeAddress;
        return 0;
    }
}

void FS_write_physical(unsigned char eeAddress, unsigned char data){

    unsigned readValue = FS_read_physical(eeAddress);
    if(EEPROM_RW_ERROR_STATUS & EE_READ_ERROR_STATUS_BIT_MASK) return;
    if(readValue == data) return;

    EEPROM_Write(eeAddress,data);

    readValue = FS_read_physical(eeAddress);
    if(EEPROM_RW_ERROR_STATUS & EE_READ_ERROR_STATUS_BIT_MASK){
        EEPROM_RW_ERROR_STATUS |= EE_WRITE_ERROR_STATUS_BIT_MASK;
        EEPROM_ERROR_COUNT++;
        EEPROM_LATEST_WRITE_ERROR_ADDRESS = eeAddress;
        return;
    }
}
