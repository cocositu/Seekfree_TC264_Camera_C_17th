/*********************************************************************************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2020,��ɿƼ�
 * All rights reserved.
 * ��������QQȺ����Ⱥ��824575535
 *
 * �����������ݰ�Ȩ������ɿƼ����У�δ��������������ҵ��;��
 * ��ӭ��λʹ�ò������������޸�����ʱ���뱣����ɿƼ��İ�Ȩ������
 *
 * @file            main
 * @company         �ɶ���ɿƼ����޹�˾
 * @author          ��ɿƼ�(QQ3184284598)
 * @version         �鿴doc��version�ļ� �汾˵��
 * @Software        ADS v1.2.2
 * @Target core     TC364DP
 * @Taobao          https://seekfree.taobao.com/
 * @date            2020-11-23
 ********************************************************************************************************************/

#include "Cpu0_Main.h"
#include "headfile.h"
#pragma section all "cpu0_dsram"
//���������#pragma section all restore���֮���ȫ�ֱ���������CPU0��RAM��

int core0_main(void)
{
    get_clk();//��ȡʱ��Ƶ��  ��ر���
    //�û��ڴ˴����ø��ֳ�ʼ��������
    Peripheral_init();
    Parameter_init();
    //�ȴ����к��ĳ�ʼ�����
    IfxCpu_emitEvent(&g_cpuSyncEvent);
    IfxCpu_waitEvent(&g_cpuSyncEvent, 0xFFFF);
    enableInterrupts();
    flag.init_ok = 1;//��ʼ���ɹ�

    Handle_Barn_Out(1);//�����
//    flag.start =1;

    while (TRUE)
    {
        if(flag.init_ok)
        {
            if(flag.start)
            {
//                Datasend();
                State_Adjust();
            }
        }
    }
}

#pragma section all restore
