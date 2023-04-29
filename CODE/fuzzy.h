#ifndef _fuzzy_h
#define _fuzzy_h

#include "headfile.h"

//偏差的模糊子集
#define NB1 -480
#define NM1 -320
#define NS1 -160
#define ZO1 0
#define PS1 160
#define PM1 320
#define PB1 480
//偏差变化量的模糊子集
#define NB2 -90
#define NM2 -60
#define NS2 -30
#define ZO2 0
#define PS2 30
#define PM2 60
#define PB2 90
//输出模糊子集
#define NB3 0
#define NM3 -6
#define NS3 -3
#define ZO3 0
#define PS3 3
#define PM3 6
#define PB3 9

extern float delt_p;                //动态p输出项
extern float delt_d;                //动态d输出项
extern float k1;                    //动态p输出项的系数
extern float k2;                    //动态d输出项的系数

void Fuzzy_pid(float error, float delt_error);

#endif
