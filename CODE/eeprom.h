#ifndef _eeprom_h
#define _eeprom_h

#include "headfile.h"

#define SECTOR_NUM (3)        //д�����ݵ�������
#define PARAMETER_NUM (3)     //�ֶ����θ���

void Parameter_eeprom_init(void);
void Parameter_write_eeprom(void);
void Parameter_read_eeprom(void);

#endif
