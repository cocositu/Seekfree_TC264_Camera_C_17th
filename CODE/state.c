#include "state.h"
#pragma section all "cpu0_dsram"

/************************************************
��������Speed_strategy_choose
��  �ܣ��ٶȲ���ѡ��
��  ����void
����ֵ��void
************************************************/

void Speed_strategy_choose(void)
{
    //�ٶȲ���1
    speed_type.straight    = 300;
    speed_type.bend        = 210;
    speed_type.cross       = 150;
    speed_type.fork        = 200;
    speed_type.cirque      = 200;
    speed_type.lean_cross  = 150;
    speed_type.ramp        = 180;
    speed_type.barn        = 160;
    speed_type.stop        = 0;
    //����pid
    dpid.kp = 5.0;
    dpid.ki = 35.0;
    dpid.kd = 27.0;
    //�ٶȻ�pid
    l_spid.kp = 31.0;//����
    l_spid.ki = 0.65;
    r_spid.kp = 28.0;//����
    r_spid.ki = 0.65;
}

/************************************************
��������Peripheral_init
��  �ܣ������ʼ��
��  ����void
����ֵ��void
************************************************/

void Peripheral_init(void)
{
    //�ϵ���ʱ
    systick_delay_ms(STM0, 500);

    //LCD��ʼ��
    lcd_init();

    //��ʼ������ͷ
    mt9v03x_init();

    //�����ʼ��
    gtm_pwm_init(STEER_PIN, 50, STEER_MID);

    //������ʼ��
    gtm_pwm_init(ATOM0_CH4_P02_4, 20000, 0);
    gtm_pwm_init(ATOM0_CH5_P02_5, 20000, 0);

    //�ҵ����ʼ��
    gtm_pwm_init(ATOM0_CH6_P02_6, 20000, 0);
    gtm_pwm_init(ATOM0_CH7_P02_7, 20000, 0);

    //���������ʼ��
    gpt12_init(GPT12_T2, GPT12_T2INB_P33_7, GPT12_T2EUDB_P33_6);

    //�ұ�������ʼ��
    gpt12_init(GPT12_T5, GPT12_T5INB_P10_3, GPT12_T5EUDB_P10_1);

    //��������ʼ��
    gpio_init(BUZZER_PIN, GPO, 0, PUSHPULL);

    //�������ڳ�ʼ��
    seekfree_wireless_init();

    //���뿪�س�ʼ��
    gpio_init(SWITCH_KEY1_PIN, GPI, 1, PULLUP);//ǰ��
    gpio_init(SWITCH_KEY2_PIN, GPI, 1, PULLUP);
    gpio_init(SWITCH_KEY3_PIN, GPI, 1, PULLUP);
    gpio_init(SWITCH_KEY4_PIN, GPI, 1, PULLUP);//����
    gpio_init(SWITCH_KEY5_PIN, GPI, 1, PULLUP);
    gpio_init(SWITCH_KEY6_PIN, GPI, 1, PULLUP);

    //������ʼ��
    gpio_init(KEY1_PIN, GPI, 1, PULLUP);
    gpio_init(KEY2_PIN, GPI, 1, PULLUP);
    gpio_init(KEY3_PIN, GPI, 1, PULLUP);
    gpio_init(KEY4_PIN, GPI, 1, PULLUP);
    gpio_init(KEY5_PIN, GPI, 1, PULLUP);

    //���ö�ʱ�ж�
    pit_interrupt_ms(CCU6_0, PIT_CH0, 5);
}

/************************************************
��������Parameter_init
��  �ܣ�������ʼ��
��  ����void
����ֵ��void
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
��������State_Adjust
��  �ܣ�״̬����
��  ����void
����ֵ��void
************************************************/

void State_Adjust(void)
{
    //���ڱ�־λ
    if(flag.t_ms == 5)
    {
        flag.msg_collect += 5;
        flag.speed += 5;
        flag.direction += 5;

        flag.t_ms = 0;
    }

    //��Ϣ�ɼ���10ms
    if(flag.msg_collect == 10)
    {
        Speed_Measure();//�ٶ�ֵ����

        flag.msg_collect = 0;
    }

    //�������:10ms
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

    //�ٶȵ��ڣ�20ms
    if(flag.speed == 20)
    {
        L_speed_pid(&l_spid);//����pid����
        R_speed_pid(&r_spid);//����pid����
        Speed_difference_calculate(dpid.out);//���ټ���
        Set_Motor(l_spid.out-differential, r_spid.out+differential);
//        Set_Motor(2000,2000);

        flag.speed = 0;
    }

    //ͣ������
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
        disableInterrupts();//���ж�
        BUZZER_OFF;

        while(TRUE)
        {
//            Speed_Measure();//�ٶ�ֵ����
//            L_speed_pid(&l_spid);//����pid����
//            R_speed_pid(&r_spid);//����pid����
              Set_Motor(0,0);//�ص��

//            systick_delay_ms(STM0, 20);
        }
    }
}

#pragma section all restore
