#include "state.h"
#pragma section all "cpu0_dsram"

/************************************************
函数名：Speed_strategy_choose
功  能：速度策略选择
参  数：void
返回值：void
************************************************/

void Speed_strategy_choose(void)
{
    //速度策略1
    speed_type.straight    = 300;
    speed_type.bend        = 210;
    speed_type.cross       = 150;
    speed_type.fork        = 200;
    speed_type.cirque      = 200;
    speed_type.lean_cross  = 150;
    speed_type.ramp        = 180;
    speed_type.barn        = 160;
    speed_type.stop        = 0;
    //方向环pid
    dpid.kp = 5.0;
    dpid.ki = 35.0;
    dpid.kd = 27.0;
    //速度环pid
    l_spid.kp = 31.0;//左轮
    l_spid.ki = 0.65;
    r_spid.kp = 28.0;//右轮
    r_spid.ki = 0.65;
}

/************************************************
函数名：Peripheral_init
功  能：外设初始化
参  数：void
返回值：void
************************************************/

void Peripheral_init(void)
{
    //上电延时
    systick_delay_ms(STM0, 500);

    //LCD初始化
    lcd_init();

    //初始化摄像头
    mt9v03x_init();

    //舵机初始化
    gtm_pwm_init(STEER_PIN, 50, STEER_MID);

    //左电机初始化
    gtm_pwm_init(ATOM0_CH4_P02_4, 20000, 0);
    gtm_pwm_init(ATOM0_CH5_P02_5, 20000, 0);

    //右电机初始化
    gtm_pwm_init(ATOM0_CH6_P02_6, 20000, 0);
    gtm_pwm_init(ATOM0_CH7_P02_7, 20000, 0);

    //左编码器初始化
    gpt12_init(GPT12_T2, GPT12_T2INB_P33_7, GPT12_T2EUDB_P33_6);

    //右编码器初始化
    gpt12_init(GPT12_T5, GPT12_T5INB_P10_3, GPT12_T5EUDB_P10_1);

    //蜂鸣器初始化
    gpio_init(BUZZER_PIN, GPO, 0, PUSHPULL);

    //蓝牙串口初始化
    seekfree_wireless_init();

    //拨码开关初始化
    gpio_init(SWITCH_KEY1_PIN, GPI, 1, PULLUP);//前三
    gpio_init(SWITCH_KEY2_PIN, GPI, 1, PULLUP);
    gpio_init(SWITCH_KEY3_PIN, GPI, 1, PULLUP);
    gpio_init(SWITCH_KEY4_PIN, GPI, 1, PULLUP);//后三
    gpio_init(SWITCH_KEY5_PIN, GPI, 1, PULLUP);
    gpio_init(SWITCH_KEY6_PIN, GPI, 1, PULLUP);

    //按键初始化
    gpio_init(KEY1_PIN, GPI, 1, PULLUP);
    gpio_init(KEY2_PIN, GPI, 1, PULLUP);
    gpio_init(KEY3_PIN, GPI, 1, PULLUP);
    gpio_init(KEY4_PIN, GPI, 1, PULLUP);
    gpio_init(KEY5_PIN, GPI, 1, PULLUP);

    //设置定时中断
    pit_interrupt_ms(CCU6_0, PIT_CH0, 5);
}

/************************************************
函数名：Parameter_init
功  能：参数初始化
参  数：void
返回值：void
************************************************/

void Parameter_init(void)
{
    Speed_strategy_choose();
//    Parameter_eeprom_init();
//    Parameter_read_eeprom();
//    Key_scan();
//    Parameter_write_eeprom();
}

/************************************************
函数名：State_Adjust
功  能：状态调节
参  数：void
返回值：void
************************************************/

void State_Adjust(void)
{
    //调节标志位
    if(flag.t_ms == 5)
    {
        flag.msg_collect += 5;
        flag.speed += 5;
        flag.direction += 5;

        flag.t_ms = 0;
    }

    //信息采集：10ms
    if(flag.msg_collect == 10)
    {
        Speed_Measure();//速度值测量

        flag.msg_collect = 0;
    }

    //方向调节:10ms
    if(flag.direction == 10)
    {
        Direction_pid(&dpid);
        if(road_type.Cross || road_type.Barn_l_in || road_type.Barn_r_in)
        {
            Limit_Min_Max(dpid.out, dpid.last_out-2, dpid.last_out+2);
        }
        Set_Steer(dpid.out);

        flag.direction = 0;
    }

    //速度调节：20ms
    if(flag.speed == 20)
    {
        L_speed_pid(&l_spid);//左轮pid调节
        R_speed_pid(&r_spid);//右轮pid调节
        Speed_difference_calculate(dpid.out);//差速计算
        Set_Motor(l_spid.out-differential, r_spid.out+differential);
//        Set_Motor(2000,2000);

        flag.speed = 0;
    }

    //停车处理
    if(flag.stop)
    {
        BUZZER_ON;
        Set_Motor(-4000,-4000);
//        systick_delay_ms(STM0, 200);
        Set_Steer(0);
        systick_delay_ms(STM0, 350);
        lcd_showstr(0,0,"straight");lcd_showint8(100,0,road_type.straight);
        lcd_showstr(0,1,"bend");lcd_showint8(100,1,road_type.bend);
        lcd_showstr(0,2,"L_Cross");lcd_showint8(100,2,road_type.L_Cross);
        lcd_showstr(0,3,"R_Cross");lcd_showint8(100,3,road_type.R_Cross);
        lcd_showstr(0,4,"LeftCirque");lcd_showuint8(100,4,road_type.LeftCirque);
        lcd_showstr(0,5,"RightCirque");lcd_showuint8(100,5,road_type.RightCirque);
        lcd_showstr(0,6,"Fork");lcd_showuint8(100,6,road_type.Fork);
        disableInterrupts();//关中断
        BUZZER_OFF;

        while(TRUE)
        {
//            Speed_Measure();//速度值测量
//            L_speed_pid(&l_spid);//左轮pid调节
//            R_speed_pid(&r_spid);//右轮pid调节
              Set_Motor(0,0);//关电机

//            systick_delay_ms(STM0, 20);
        }
    }
}

#pragma section all restore
