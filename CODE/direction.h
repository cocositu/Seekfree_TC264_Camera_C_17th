#ifndef _direction_h
#define _direction_h

#include "headfile.h"

#define STEER_MID   460         //�����ֵ
#define STEER_LIM   80          //�������

extern struct PID dpid;

void Set_Steer(int16 duty);
void Direction_pid(struct PID *sptr);

#endif
