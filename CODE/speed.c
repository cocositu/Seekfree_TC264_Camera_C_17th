#include "speed.h"
#pragma section all "cpu0_dsram"

//�ṹ�嶨��
struct PID l_spid = {0};
struct PID r_spid = {0};
struct SPEED_TYPE speed_type = {
        .straight    = 0,
        .bend        = 0,
        .cross       = 0,
        .cirque      = 0,
        .fork        = 0,
        .lean_cross  = 0,
        .ramp        = 0,
        .stop        = 0,
        .barn        = 0
};

//ȫ�ֱ�������
int16 l_encoder_pulse = 0;       //�����������
int16 r_encoder_pulse = 0;       //�ұ���������
int16 l_speed_now = 0;           //���ֵ�ǰ�ٶ�
int16 r_speed_now = 0;           //���ֵ�ǰ�ٶ�
int16 l_speed_aim = 0;           //����Ŀ���ٶ�
int16 r_speed_aim = 0;           //����Ŀ���ٶ�
int16 differential = 0;          //���ֲ���ֵ

/************************************************
��������Set_Motor
��  �ܣ�����PWM����ֱ�����
��  ����int16 l_duty, int16 r_duty
����ֵ��void
 ************************************************/

void Set_Motor(int16 l_duty, int16 r_duty)
{
    //������ת
    if(l_duty > 0)
    {
        l_duty = Limit_Min_Max(l_duty, 0, 8000);
        pwm_duty(ATOM0_CH4_P02_4, 0);
        pwm_duty(ATOM0_CH5_P02_5, l_duty);
    }
    //������ת
    else
    {
        l_duty = Limit_Min_Max(-l_duty, 0, 8000);
        pwm_duty(ATOM0_CH4_P02_4, l_duty);
        pwm_duty(ATOM0_CH5_P02_5, 0);
    }

    //�ҵ����ת
    if(r_duty > 0)
    {
        r_duty = Limit_Min_Max(r_duty, 0, 8000);
        pwm_duty(ATOM0_CH6_P02_6, 0);
        pwm_duty(ATOM0_CH7_P02_7, r_duty);
    }
    //�ҵ����ת
    else
    {
        r_duty = Limit_Min_Max(-r_duty, 0, 8000);
        pwm_duty(ATOM0_CH6_P02_6, r_duty);
        pwm_duty(ATOM0_CH7_P02_7, 0);
    }
}

/************************************************
��������Speed_Measure
��  �ܣ���������������ص�������ΪС���ٶȣ������ٶȣ�
��  ����void
����ֵ��void
 ************************************************/

void Speed_Measure(void)
{
    //��ȡ�����������
    l_encoder_pulse = gpt12_get(GPT12_T2);//��ת����Ϊ��
    gpt12_clear(GPT12_T2);

    //��ȡ�ұ���������
    r_encoder_pulse = -gpt12_get(GPT12_T5);
    gpt12_clear(GPT12_T5);

    l_speed_now = l_encoder_pulse;
    r_speed_now = r_encoder_pulse;
}

/************************************************
��������Distance_Measure
��  �ܣ���С���������ٶȻ��ֽ��в��
��  ����void
����ֵ��int16
 ************************************************/

int16 Distance_Measure(void)
{
    int16 distance;

    distance = (l_speed_now+r_speed_now)/2;

    return distance;
}

/************************************************
��������Speed_difference_calculate
��  �ܣ����ݶ����Ǽ�����ֲ���
��  ����void
����ֵ��void
 ************************************************/

void Speed_difference_calculate(int16 diff_base)
{
    float k_diff = 5;//����ϵ��

    differential = k_diff*diff_base;
}

/************************************************
��������L_speed_error
��  �ܣ�������������ѡ��ͬ�������ٶȲ������ٶ�ƫ��
��  ����void
����ֵ��int16
 ************************************************/

int16 L_speed_error(void)
{
    int16 error = 0;

    //ֱ���ٶ�ƫ�����
    if(road_type.straight)
    {
        l_speed_aim = speed_type.straight;
        error = l_speed_aim-l_speed_now;
    }
    //����ٶ�ƫ�����
    else if(road_type.bend)
    {
        l_speed_aim = speed_type.bend;
        error = l_speed_aim-l_speed_now;
    }
    //ʮ���ٶ�ƫ�����
    else if(road_type.Cross)
    {
        l_speed_aim = speed_type.cross;
        error = l_speed_aim-l_speed_now;
    }
    //������ٶ�ƫ�����
    else if(road_type.Fork)
    {
        if(sancha_stage == 1 || (sancha_stage == 4))
        {
            l_speed_aim = speed_type.fork - 50;
            error = l_speed_aim-l_speed_now;
        }
        else if((sancha_stage == 2) || (sancha_stage == 5))
        {
            l_speed_aim = speed_type.fork;
            error = l_speed_aim-l_speed_now;
        }
        else if(sancha_stage == 3)
        {
            l_speed_aim = speed_type.fork + 30;
            error = l_speed_aim-l_speed_now;
        }
    }
    //�����ٶ�ƫ�����
    else if(road_type.LeftCirque || road_type.RightCirque)
    {
        if((huandao_stage == 1) || (huandao_stage == 2))
        {
            l_speed_aim = speed_type.cirque + 50;
            error = l_speed_aim-l_speed_now;
        }
        else if(huandao_stage == 3)
        {
            l_speed_aim = speed_type.cirque + 20;
            error = l_speed_aim-l_speed_now;
        }
        else if((huandao_stage == 4) || (huandao_stage == 5))
        {
            l_speed_aim = speed_type.cirque + 50;
            error = l_speed_aim-l_speed_now;
        }
        else if((huandao_stage == 6) || (huandao_stage == 7))
        {
            l_speed_aim = speed_type.cirque + 50;
            error = l_speed_aim-l_speed_now;
        }
    }
    //бʮ���ٶ�ƫ�����
    else if(road_type.L_Cross || road_type.R_Cross)
    {
        if(xieshizi_stage == 1)
        {
            l_speed_aim = speed_type.lean_cross;
            error = l_speed_aim-l_speed_now;
        }
        else if(xieshizi_stage == 2)
        {
            l_speed_aim = speed_type.lean_cross + 70;
            error = l_speed_aim-l_speed_now;
        }
        else if(xieshizi_stage == 3)
         {
             l_speed_aim = speed_type.lean_cross - 80;
             error = l_speed_aim-l_speed_now;
         }
    }
    //�¶��ٶ�ƫ�����
    else if(2 == pass_barn)
    {
        l_speed_aim = speed_type.barn;
        error = l_speed_aim-l_speed_now;
    }
    else if(flag.stop)
    {
        l_speed_aim = speed_type.stop;
        error = l_speed_aim-l_speed_now;
    }

    return error;
}

/************************************************
��������R_speed_error
��  �ܣ�������������ѡ��ͬ�������ٶȲ������ٶ�ƫ��
��  ����void
����ֵ��int16
 ************************************************/

int16 R_speed_error(void)
{
    int16 error = 0;

    //ֱ���ٶ�ƫ�����
    if(road_type.straight)
    {
        r_speed_aim = speed_type.straight;
        error = r_speed_aim-r_speed_now;
    }
    //����ٶ�ƫ�����
    else if(road_type.bend)
    {
        r_speed_aim = speed_type.bend;
        error = r_speed_aim-r_speed_now;
    }
    //ʮ���ٶ�ƫ�����
    else if(road_type.Cross)
    {
        r_speed_aim = speed_type.cross;
        error = r_speed_aim-r_speed_now;
    }
    //������ٶ�ƫ�����
    else if(road_type.Fork)
    {
        if(sancha_stage == 1 || (sancha_stage == 4))
        {
            r_speed_aim = speed_type.fork - 50;
            error = r_speed_aim-r_speed_now;
        }
        else if((sancha_stage == 2) || (sancha_stage == 5))
        {
            r_speed_aim = speed_type.fork;
            error = r_speed_aim-r_speed_now;
        }
        else if(sancha_stage == 3)
        {
            r_speed_aim = speed_type.fork + 30;
            error = r_speed_aim-r_speed_now;
        }
    }
    //�����ٶ�ƫ�����
    else if(road_type.LeftCirque || road_type.RightCirque)
    {
        if((huandao_stage == 1) || (huandao_stage == 2))
        {
            r_speed_aim = speed_type.cirque + 50;
            error = r_speed_aim-r_speed_now;
        }
        else if(huandao_stage == 3)
        {
            r_speed_aim = speed_type.cirque +20;
            error = r_speed_aim-r_speed_now;
        }
        else if((huandao_stage == 4) || (huandao_stage == 5))
        {
            r_speed_aim = speed_type.cirque + 50;
            error = r_speed_aim-r_speed_now;
        }
        else if((huandao_stage == 6) || (huandao_stage == 7))
        {
            r_speed_aim = speed_type.cirque + 50;
            error = r_speed_aim-r_speed_now;
        }
    }
    //бʮ���ٶ�ƫ�����
    else if(road_type.L_Cross || road_type.R_Cross)
    {
        if(xieshizi_stage == 1)
        {
            r_speed_aim = speed_type.lean_cross;
            error = r_speed_aim-r_speed_now;
        }
        else if(xieshizi_stage == 2)
        {
            r_speed_aim = speed_type.lean_cross + 70;
            error = r_speed_aim-r_speed_now;
        }
        else if(xieshizi_stage == 3)
         {
             r_speed_aim = speed_type.lean_cross - 80;
             error = r_speed_aim-r_speed_now;
         }
    }
    //�¶��ٶ�ƫ�����
    else if(2 == pass_barn)
    {
        r_speed_aim = speed_type.barn;
        error = r_speed_aim-r_speed_now;
    }
    else if(flag.stop)
    {
        r_speed_aim = speed_type.stop;
        error = r_speed_aim-r_speed_now;
    }

    return error;
}

/************************************************
��������L_speed_pid
��  �ܣ�����PID������ʽ����С�����ֵ��ٶ�
��  ����struct PID *sptr
����ֵ��void
 ************************************************/

void L_speed_pid(struct PID *sptr)
{
    sptr->error = L_speed_error();//�����ٶ�ƫ�����
    sptr->out += sptr->kp*(sptr->error-sptr->last_error)+sptr->ki*sptr->error;
    sptr->last_error = sptr->error;
}

/************************************************
��������R_speed_pid
��  �ܣ�����PID������ʽ����С�����ֵ��ٶ�
��  ����struct PID *sptr
����ֵ��void
 ************************************************/

void R_speed_pid(struct PID *sptr)
{
    sptr->error = R_speed_error();//�����ٶ�ƫ�����
    sptr->out += sptr->kp*(sptr->error-sptr->last_error)+sptr->ki*sptr->error;
    sptr->last_error = sptr->error;
}

#pragma section all restore
