#ifndef _speed_h
#define _speed_h

#include "headfile.h"

struct SPEED_TYPE
{
        int16 straight;
        int16 bend;
        int16 cross;
        int16 cirque;
        int16 fork;
        int16 lean_cross;
        int16 ramp;
        int16 stop;
        int16 barn;
};
extern struct SPEED_TYPE speed_type;
extern struct PID l_spid, r_spid;

extern int16 l_encoder_pulse;
extern int16 r_encoder_pulse;
extern int16 l_speed_now;
extern int16 r_speed_now;
extern int16 l_speed_aim;
extern int16 r_speed_aim;
extern int16 differential;

void Set_Motor(int16 l_duty, int16 r_duty);
void Speed_Measure(void);
int16 Distance_Measure(void);
void Speed_difference_calculate(int16 diff_base);
int16 L_speed_error(void);
int16 R_speed_error(void);
void L_speed_pid(struct PID *sptr);
void R_speed_pid(struct PID *sptr);

#endif
