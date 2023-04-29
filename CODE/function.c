#include "function.h"
#pragma section all "cpu0_dsram"

//�ṹ�嶨��
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
��������Cha_bi_he0
��  �ܣ���Ⱥ͡������Һ�1.5�η���
��  ����int16 data1, int16 data2, uint8 x
����ֵ��double
˵  ����data1��� data2�Һ�
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
��������Cha_bi_he1
��  �ܣ���Ⱥ͡�����������
��  ����int16 data1, int16 data2, uint8 x
����ֵ��double
˵  ����data1���� data2����
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
��������Cha_bi_he2
��  �ܣ���Ⱥ͡������Һ�1.5�η�+��������
��  ����int16 data1, int16 data2, int16 data3, int16 data4
����ֵ��double
˵  ����data1��� data2�Һ� data3���� data4����
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
��������Buzzer_di
��  �ܣ�����������
��  ����uint16 ms
����ֵ��void
************************************************/

void Buzzer_di(uint16 ms)
{
    BUZZER_ON;
    systick_delay_ms(STM1, ms);
    BUZZER_OFF;
    systick_delay_ms(STM1, ms);
}

/************************************************
��������Filter_first
��  �ܣ�һ�׵�ͨ�˲�
��  ����int32 data, int32 last_data, float k
����ֵ��int32
˵  ����dataδ�������ݣ�last_dataΪ�ϴ�����,kΪ�˲�ϵ��
************************************************/

int16 Filter_first(int16 data, int16 last_data, float k)
{
    int16 result;

    result = (int16)((1-k)*data+k*last_data);

    return result;
}



/************************************************
��������Datasend
��  �ܣ�����͸��ģ�鷢�����ݵ���λ��
��    ������
����ֵ����
˵  ������ɿ�ֻ�ܷ�һ���ֽ���ֲ����Է������ֽ�
************************************************/

void Datasend(void)
{
    //int16 send_data[8];

    //�ٶȻ�speed_type
//    send_data[0]= speed_type.;
//    send_data[1]= speed_type;
//    send_data[2]= speed_type;
//    send_data[3]= r_speed_aim;
//    send_data[0] = road_type.Fork;
//    send_data[5] = road_type.Barn_r_in;

    //����
//    send_data[0]= dpid.out;
//    send_data[1]= offset;

//    //��������
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
