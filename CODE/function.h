#ifndef _function_h
#define _function_h

#include "headfile.h"

//�� �ܣ��Բ��������޷�
//˵ �������꺯���Ĳ�������Ϊ���ʽ��ֻ��Ϊ������ֵ
#define Limit_Min_Max(data, min, max) (((data) > (max)) ? (max):(((data) < (min)) ? (min):(data)))

//�� �ܣ��򿪷�����
#define BUZZER_ON       gpio_set(BUZZER_PIN, 1);
//�� �ܣ��رշ�����
#define BUZZER_OFF      gpio_set(BUZZER_PIN, 0);

//��־λ�ṹ��
struct FLAG
{
        int8 init_ok;            //��ʼ���ɹ���־λ
        int8 start;              //�����ɹ���־λ
        int8 strategy1;          //�ٶȲ���1��־λ
        int8 strategy2;          //�ٶȲ���2��־λ
        int8 strategy3;          //�ٶȲ���3��־λ
        int8 strategy4;          //�ٶȲ���4��־λ
        int8 adjust_ok;          //������ɱ�־λ
        int8 t_ms;               //�жϻ�׼ʱ���־λ
        int8 msg_collect;        //��Ϣ�ɼ���־λ
        int8 speed;              //�ٶȻ���־λ
        int8 direction;          //���򻷱�־λ
        int8 open_check_ku;      //�����־λ
        int8 stop;               //ͣ����־λ
        int8 image_show;         //ͼ����ʾ��־λ
        int8 eeprom_check_ok;
};
extern struct FLAG flag;

//pid�ṹ��
struct PID
{
        float kp;
        float ki;
        float kd;
        float error;
        float last_error;
        float pre_error;
        int16 out;
        int16 last_out;
};

double Cha_bi_he0(int16 data1, int16 data2, uint8 x);
double Cha_bi_he1(int16 data1, int16 data2, uint8 x);
double Cha_bi_he2(int16 data1, int16 data2, int16 data3, int16 data4, uint8 x, uint8 y);
void Buzzer_di(uint16 ms);
int16 Filter_first(int16 data, int16 last_data, float k);
void Datasend(void);

#endif
