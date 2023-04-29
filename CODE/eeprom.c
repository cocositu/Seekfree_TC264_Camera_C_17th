#include "eeprom.h"
#pragma section all "cpu0_dsram"

uint8 read_data0;
float read_data1;
uint8 read_data2;

void Parameter_eeprom_init(void)
{
    for(uint8 i = 0;i < PARAMETER_NUM;i++)
    {
        if(!flash_check(SECTOR_NUM, i))//����1�����ݣ�����0û������
        {
            Parameter_write_eeprom();
            break;
        }
    }
}

void Parameter_write_eeprom(void)
{
    uint32 write_parameter;

    //��������������
    eeprom_erase_sector(SECTOR_NUM);

    //����д������Ҷ
    //����pid����
    write_parameter = dpid.kp;
    eeprom_page_program(SECTOR_NUM, 0, &write_parameter);

    write_parameter = float_conversion_uint32(dpid.ki);
    eeprom_page_program(SECTOR_NUM, 1, &write_parameter);

    write_parameter = dpid.kd;
    eeprom_page_program(SECTOR_NUM, 2, &write_parameter);
}

void Parameter_read_eeprom(void)
{
    //��0����0-4ҳ��������----��0ҳ
    dpid.kp = flash_read(SECTOR_NUM, 0, uint8);
    dpid.ki = flash_read(SECTOR_NUM, 1, float);
    dpid.kd = flash_read(SECTOR_NUM, 2, uint8);
}

#pragma section all restore
