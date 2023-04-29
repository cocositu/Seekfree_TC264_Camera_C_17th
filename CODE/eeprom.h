#ifndef _eeprom_h
#define _eeprom_h

#include "headfile.h"

#define SECTOR_NUM (3)        //写入数据的扇区号
#define PARAMETER_NUM (3)     //手动调参个数

void Parameter_eeprom_init(void);
void Parameter_write_eeprom(void);
void Parameter_read_eeprom(void);

#endif
