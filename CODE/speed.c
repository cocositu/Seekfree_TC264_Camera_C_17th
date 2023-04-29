#include "speed.h"
#pragma section all "cpu0_dsram"

//结构体定义
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

//全局变量定义
int16 l_encoder_pulse = 0;       //左编码器脉冲
int16 r_encoder_pulse = 0;       //右编码器脉冲
int16 l_speed_now = 0;           //左轮当前速度
int16 r_speed_now = 0;           //右轮当前速度
int16 l_speed_aim = 0;           //左轮目标速度
int16 r_speed_aim = 0;           //右轮目标速度
int16 differential = 0;          //后轮差速值

/************************************************
函数名：Set_Motor
功  能：运用PWM驱动直流电机
参  数：int16 l_duty, int16 r_duty
返回值：void
 ************************************************/

void Set_Motor(int16 l_duty, int16 r_duty)
{
    //左电机正转
    if(l_duty > 0)
    {
        l_duty = Limit_Min_Max(l_duty, 0, 8000);
        pwm_duty(ATOM0_CH4_P02_4, 0);
        pwm_duty(ATOM0_CH5_P02_5, l_duty);
    }
    //左电机反转
    else
    {
        l_duty = Limit_Min_Max(-l_duty, 0, 8000);
        pwm_duty(ATOM0_CH4_P02_4, l_duty);
        pwm_duty(ATOM0_CH5_P02_5, 0);
    }

    //右电机正转
    if(r_duty > 0)
    {
        r_duty = Limit_Min_Max(r_duty, 0, 8000);
        pwm_duty(ATOM0_CH6_P02_6, 0);
        pwm_duty(ATOM0_CH7_P02_7, r_duty);
    }
    //右电机反转
    else
    {
        r_duty = Limit_Min_Max(-r_duty, 0, 8000);
        pwm_duty(ATOM0_CH6_P02_6, r_duty);
        pwm_duty(ATOM0_CH7_P02_7, 0);
    }
}

/************************************************
函数名：Speed_Measure
功  能：带方向编码器返回的脉冲数为小车速度（虚拟速度）
参  数：void
返回值：void
 ************************************************/

void Speed_Measure(void)
{
    //获取左编码器脉冲
    l_encoder_pulse = gpt12_get(GPT12_T2);//正转脉冲为正
    gpt12_clear(GPT12_T2);

    //获取右编码器脉冲
    r_encoder_pulse = -gpt12_get(GPT12_T5);
    gpt12_clear(GPT12_T5);

    l_speed_now = l_encoder_pulse;
    r_speed_now = r_encoder_pulse;
}

/************************************************
函数名：Distance_Measure
功  能：将小车的虚拟速度积分进行测距
参  数：void
返回值：int16
 ************************************************/

int16 Distance_Measure(void)
{
    int16 distance;

    distance = (l_speed_now+r_speed_now)/2;

    return distance;
}

/************************************************
函数名：Speed_difference_calculate
功  能：根据舵机打角计算后轮差速
参  数：void
返回值：void
 ************************************************/

void Speed_difference_calculate(int16 diff_base)
{
    float k_diff = 5;//差速系数

    differential = k_diff*diff_base;
}

/************************************************
函数名：L_speed_error
功  能：根据赛道类型选择不同的左轮速度并计算速度偏差
参  数：void
返回值：int16
 ************************************************/

int16 L_speed_error(void)
{
    int16 error = 0;

    //直道速度偏差计算
    if(road_type.straight)
    {
        l_speed_aim = speed_type.straight;
        error = l_speed_aim-l_speed_now;
    }
    //弯道速度偏差计算
    else if(road_type.bend)
    {
        l_speed_aim = speed_type.bend;
        error = l_speed_aim-l_speed_now;
    }
    //十字速度偏差计算
    else if(road_type.Cross)
    {
        l_speed_aim = speed_type.cross;
        error = l_speed_aim-l_speed_now;
    }
    //三岔口速度偏差计算
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
    //环岛速度偏差计算
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
    //斜十字速度偏差计算
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
    //坡度速度偏差计算
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
函数名：R_speed_error
功  能：根据赛道类型选择不同的右轮速度并计算速度偏差
参  数：void
返回值：int16
 ************************************************/

int16 R_speed_error(void)
{
    int16 error = 0;

    //直道速度偏差计算
    if(road_type.straight)
    {
        r_speed_aim = speed_type.straight;
        error = r_speed_aim-r_speed_now;
    }
    //弯道速度偏差计算
    else if(road_type.bend)
    {
        r_speed_aim = speed_type.bend;
        error = r_speed_aim-r_speed_now;
    }
    //十字速度偏差计算
    else if(road_type.Cross)
    {
        r_speed_aim = speed_type.cross;
        error = r_speed_aim-r_speed_now;
    }
    //三岔口速度偏差计算
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
    //环岛速度偏差计算
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
    //斜十字速度偏差计算
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
    //坡度速度偏差计算
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
函数名：L_speed_pid
功  能：运用PID的增量式控制小车左轮的速度
参  数：struct PID *sptr
返回值：void
 ************************************************/

void L_speed_pid(struct PID *sptr)
{
    sptr->error = L_speed_error();//左轮速度偏差计算
    sptr->out += sptr->kp*(sptr->error-sptr->last_error)+sptr->ki*sptr->error;
    sptr->last_error = sptr->error;
}

/************************************************
函数名：R_speed_pid
功  能：运用PID的增量式控制小车右轮的速度
参  数：struct PID *sptr
返回值：void
 ************************************************/

void R_speed_pid(struct PID *sptr)
{
    sptr->error = R_speed_error();//右轮速度偏差计算
    sptr->out += sptr->kp*(sptr->error-sptr->last_error)+sptr->ki*sptr->error;
    sptr->last_error = sptr->error;
}

#pragma section all restore
