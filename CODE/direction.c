#include "direction.h"
#pragma section all "cpu0_dsram"

//ȫ�ֱ�������
struct PID dpid = {0};

/************************************************
��������Set_Steer
��  �ܣ�����PWM�������
��  ����int16 duty
����ֵ��void
************************************************/

void Set_Steer(int16 duty)
{
    duty = Limit_Min_Max(duty, -STEER_LIM, STEER_LIM);
    pwm_duty(STEER_PIN, STEER_MID+duty);
}

/************************************************
��������Direction_pid
��  �ܣ����ö��ζ�̬λ��ʽ����С���ķ���
��  ����struct PID *sptr
����ֵ��void
************************************************/

void Direction_pid(struct PID *sptr)
{
    float real_kp;

    sptr->error = -(1.0*offset)/12;      //����������ƫ��ֵ
    real_kp = 1.0*(sptr->error*sptr->error)/sptr->ki+sptr->kp;
    sptr->out = (int16)(real_kp*sptr->error+sptr->kd*(sptr->error-sptr->last_error));
    sptr->out = Filter_first(sptr->out, sptr->last_out, 0.2);//һ�׵�ͨ�˲�
    sptr->last_error = sptr->error;
    sptr->last_out = sptr->out;
}

#pragma section all restore
