#ifndef _fuzzy_h
#define _fuzzy_h

#include "headfile.h"

//ƫ���ģ���Ӽ�
#define NB1 -480
#define NM1 -320
#define NS1 -160
#define ZO1 0
#define PS1 160
#define PM1 320
#define PB1 480
//ƫ��仯����ģ���Ӽ�
#define NB2 -90
#define NM2 -60
#define NS2 -30
#define ZO2 0
#define PS2 30
#define PM2 60
#define PB2 90
//���ģ���Ӽ�
#define NB3 0
#define NM3 -6
#define NS3 -3
#define ZO3 0
#define PS3 3
#define PM3 6
#define PB3 9

extern float delt_p;                //��̬p�����
extern float delt_d;                //��̬d�����
extern float k1;                    //��̬p������ϵ��
extern float k2;                    //��̬d������ϵ��

void Fuzzy_pid(float error, float delt_error);

#endif
