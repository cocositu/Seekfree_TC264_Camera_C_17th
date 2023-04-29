#include "function.h"
#pragma section all "cpu0_dsram"

//结构体定义
struct FLAG flag = {
        .init_ok         = 0,
        .start           = 0,
        .strategy1       = 0,
        .strategy2       = 0,
        .strategy3       = 0,
        .strategy4       = 0,
        .adjust_ok       = 0,
        .t_ms            = 0,
        .msg_collect     = 0,
        .speed           = 0,
        .direction       = 0,
        .open_check_ku   = 1,
        .stop            = 0,
        .image_show      = 0
};

/************************************************
函数名：Cha_bi_he0
功  能：差比和—（左右横1.5次方）
参  数：int16 data1, int16 data2, uint8 x
返回值：double
说  明：data1左横 data2右横
************************************************/

double Cha_bi_he0(int16 data1, int16 data2, uint8 x)
{
    int16 cha;
    int16 he;
    double result;

    cha = data1-data2;
    he = data1+data2;
    result = (x*cha)/(1.0*he*sqrt(he));

    return result;
}

/************************************************
函数名：Cha_bi_he1
功  能：差比和—（左右竖）
参  数：int16 data1, int16 data2, uint8 x
返回值：double
说  明：data1左竖 data2右竖
************************************************/

double Cha_bi_he1(int16 data1, int16 data2, uint8 x)
{
    int16 cha;
    int16 he;
    double result;

    cha = data1-data2;
    he = data1+data2;
    result = (x*cha)/(1.0*he);

    return result;
}

/************************************************
函数名：Cha_bi_he2
功  能：差比和—（左右横1.5次方+左右竖）
参  数：int16 data1, int16 data2, int16 data3, int16 data4
返回值：double
说  明：data1左横 data2右横 data3左竖 data4右竖
************************************************/

double Cha_bi_he2(int16 data1, int16 data2, int16 data3, int16 data4, uint8 x, uint8 y)
{
    int16 cha1, cha2;
    int16 he;
    double result;

    cha1 = data1-data2;
    cha2 = data3-data4;
    he = data1+data2;
    result = (x*cha1)/(1.0*he*sqrt(he))+cha2/y;

    return result;
}

/************************************************
函数名：Buzzer_di
功  能：蜂鸣器发声
参  数：uint16 ms
返回值：void
************************************************/

void Buzzer_di(uint16 ms)
{
    BUZZER_ON;
    systick_delay_ms(STM1, ms);
    BUZZER_OFF;
    systick_delay_ms(STM1, ms);
}

/************************************************
函数名：Filter_first
功  能：一阶低通滤波
参  数：int32 data, int32 last_data, float k
返回值：int32
说  明：data未本次数据，last_data为上次数据,k为滤波系数
************************************************/

int16 Filter_first(int16 data, int16 last_data, float k)
{
    int16 result;

    result = (int16)((1-k)*data+k*last_data);

    return result;
}



/************************************************
函数名：Datasend
功  能：无线透传模块发送数据到上位机
参    数：无
返回值：无
说  明：逐飞库只能发一个字节移植后可以发两个字节
************************************************/

void Datasend(void)
{
    //int16 send_data[8];

    //速度环speed_type
//    send_data[0]= speed_type.;
//    send_data[1]= speed_type;
//    send_data[2]= speed_type;
//    send_data[3]= r_speed_aim;
//    send_data[0] = road_type.Fork;
//    send_data[5] = road_type.Barn_r_in;

    //方向环
//    send_data[0]= dpid.out;
//    send_data[1]= offset;

//    //赛道类型
//    send_data[0] = road_type.straight;
//    send_data[1] = road_type.bend;
//    send_data[2] = road_type.Fork;
//    send_data[3] = road_type.L_Cross;
//    send_data[4] = road_type.R_Cross;
//    send_data[5] = road_type.LeftCirque;
//    send_data[6] = road_type.RightCirque;
//    send_data[7] = road_type.Ramp;

   // Data_Send(UART_2, send_data);
}


#pragma section all restore
