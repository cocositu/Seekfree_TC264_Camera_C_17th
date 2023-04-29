#ifndef _function_h
#define _function_h

#include "headfile.h"

//功 能：对参数进行限幅
//说 明：本宏函数的参数不能为表达式，只能为具体数值
#define Limit_Min_Max(data, min, max) (((data) > (max)) ? (max):(((data) < (min)) ? (min):(data)))

//功 能：打开蜂鸣器
#define BUZZER_ON       gpio_set(BUZZER_PIN, 1);
//功 能：关闭蜂鸣器
#define BUZZER_OFF      gpio_set(BUZZER_PIN, 0);

//标志位结构体
struct FLAG
{
        int8 init_ok;            //初始化成功标志位
        int8 start;              //发车成功标志位
        int8 strategy1;          //速度策略1标志位
        int8 strategy2;          //速度策略2标志位
        int8 strategy3;          //速度策略3标志位
        int8 strategy4;          //速度策略4标志位
        int8 adjust_ok;          //调参完成标志位
        int8 t_ms;               //中断基准时间标志位
        int8 msg_collect;        //信息采集标志位
        int8 speed;              //速度环标志位
        int8 direction;          //方向环标志位
        int8 open_check_ku;      //检测库标志位
        int8 stop;               //停车标志位
        int8 image_show;         //图像显示标志位
        int8 eeprom_check_ok;
};
extern struct FLAG flag;

//pid结构体
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
