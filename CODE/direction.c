#include "direction.h"
#pragma section all "cpu0_dsram"

//全局变量定义
struct PID dpid = {0};

/************************************************
函数名：Set_Steer
功  能：运用PWM驱动舵机
参  数：int16 duty
返回值：void
************************************************/

void Set_Steer(int16 duty)
{
    duty = Limit_Min_Max(duty, -STEER_LIM, STEER_LIM);
    pwm_duty(STEER_PIN, STEER_MID+duty);
}

/************************************************
函数名：Direction_pid
功  能：运用二次动态位置式控制小车的方向
参  数：struct PID *sptr
返回值：void
************************************************/

void Direction_pid(struct PID *sptr)
{
    float real_kp;

    sptr->error = -(1.0*offset)/12;      //面积法计算的偏差值
    real_kp = 1.0*(sptr->error*sptr->error)/sptr->ki+sptr->kp;
    sptr->out = (int16)(real_kp*sptr->error+sptr->kd*(sptr->error-sptr->last_error));
    sptr->out = Filter_first(sptr->out, sptr->last_out, 0.2);//一阶低通滤波
    sptr->last_error = sptr->error;
    sptr->last_out = sptr->out;
}

#pragma section all restore
