#include "fuzzy.h"
#pragma section all "cpu0_dsram"

//ȫ�ֱ�������
float delt_p;                //��̬p�����
float delt_d;                //��̬d�����
float k1 = 1;                //��̬p������ϵ��
float k2 = 0.01;             //��̬d������ϵ��

/************************************************
��������Fuzzy_pid
��  �ܣ�ģ������Ӧpid
��  ����float error, float delt_error
����ֵ��void
************************************************/

void Fuzzy_pid(float error, float delt_error)
{
    //ģ���Ӽ�
    float error_subset[7] = {NB1, NM1, NS1, ZO1, PS1, PM1, PB1};          //ƫ���ģ���Ӽ�
    float delt_error_subset[7] = {NB2, NM2, NS2, ZO2, PS2, PM2, PB2};     //ƫ��仯����ģ���Ӽ�
    float out_subset[7] = {NB3, NM3, NS3, ZO3, PS3, PM3, PB3};            //���ģ���Ӽ�
    float error_subjection[2] = {0};                                      //ƫ���������
    float delt_error_subjection[2] = {0};                                 //ƫ��仯����������
    int8 error_dir[2] = {0};                                              //ƫ���ڹ�����е�����ֵ
    int8 delt_error_dir[2] = {0};                                         //ƫ��仯���ڹ�����е�����ֵ
    int8 out_dir[4] = {0};                                                //�������ֵ������Ӽ��е�����ֵ
    float temp1, temp2, temp3, temp4;                                     //ÿһ���ģ����ֵ
    float out;                                                            //��ģ�������ֵ

    int8 p_rule[7][7] = {
   //ec_dir -3  -2  -1   0   1   2   3        e_dir
            {3,  3,  2,  2,  1,  0,  0},  // -3
            {3,  3,  2,  1,  1,  0, -1},  // -2
            {2,  2,  2,  1,  0, -1, -1},  // -1
            {2,  2,  1,  0, -1, -2, -2},  //  0
            {1,  1,  0, -1, -1, -2, -2},  //  1
            {1,  0, -1, -2, -2, -2, -3},  //  2
            {0,  0, -2, -2, -2, -3, -3}   //  3
    };

    /**********ģ����**********/
    //error��������
    if(error <= error_subset[0])
    {
        error_dir[0] = -3;
        error_dir[1] = -2;
        error_subjection[0] = 1;
        error_subjection[1] = 1-error_subjection[0];
    }
    else if(error >= error_subset[6])
    {
        error_dir[0] = 2;
        error_dir[1] = 3;
        error_subjection[0] = 0;
        error_subjection[1] = 1-error_subjection[0];
    }
    else
    {
        if(error <= error_subset[1])
        {
            error_dir[0] = -3;
            error_dir[1] = -2;
            error_subjection[0] = (error_subset[1]-error)/(error_subset[1]-error_subset[0]);
            error_subjection[1] = 1-error_subjection[0];
        }
        else if(error <= error_subset[2])
        {
            error_dir[0] = -2;
            error_dir[1] = -1;
            error_subjection[0] = (error_subset[2]-error)/(error_subset[2]-error_subset[1]);
            error_subjection[1] = 1-error_subjection[0];
        }
        else if(error <= error_subset[3])
        {
            error_dir[0] = -1;
            error_dir[1] = 0;
            error_subjection[0] = (error_subset[3]-error)/(error_subset[3]-error_subset[2]);
            error_subjection[1] = 1-error_subjection[0];
        }
        else if(error <= error_subset[4])
        {
            error_dir[0] = 0;
            error_dir[1] = 1;
            error_subjection[0] = (error_subset[4]-error)/(error_subset[4]-error_subset[3]);
            error_subjection[1] = 1-error_subjection[0];
        }
        else if(error <= error_subset[5])
        {
            error_dir[0] = 1;
            error_dir[1] = 2;
            error_subjection[0] = (error_subset[5]-error)/(error_subset[5]-error_subset[4]);
            error_subjection[1] = 1-error_subjection[0];
        }
        else
        {
            error_dir[0] = 2;
            error_dir[1] = 3;
            error_subjection[0] = (error_subset[6]-error)/(error_subset[6]-error_subset[5]);
            error_subjection[1] = 1-error_subjection[0];
        }
    }

    //delt_error��������
    if(delt_error <= delt_error_subset[0])
    {
        delt_error_dir[0] = -3;
        delt_error_dir[1] = -2;
        delt_error_subjection[0] = 1;
        delt_error_subjection[1] = 1-delt_error_subjection[0];
    }
    else if(delt_error >= delt_error_subset[6])
    {
        delt_error_dir[0] = 2;
        delt_error_dir[1] = 3;
        delt_error_subjection[0] = 0;
        delt_error_subjection[1] = 1-delt_error_subjection[0];
    }
    else
    {
        if(delt_error <= delt_error_subset[1])
        {
            delt_error_dir[0] = -3;
            delt_error_dir[1] = -2;
            delt_error_subjection[0] = (delt_error_subset[1]-delt_error)/(delt_error_subset[1]-delt_error_subset[0]);
            delt_error_subjection[1] = 1-delt_error_subjection[0];
        }
        else if(delt_error <= delt_error_subset[2])
        {
            delt_error_dir[0] = -2;
            delt_error_dir[1] = -1;
            delt_error_subjection[0] = (delt_error_subset[2]-delt_error)/(delt_error_subset[2]-delt_error_subset[1]);
            delt_error_subjection[1] = 1-delt_error_subjection[0];
        }
        else if(delt_error <= delt_error_subset[3])
        {
            delt_error_dir[0] = -1;
            delt_error_dir[1] = 0;
            delt_error_subjection[0] = (delt_error_subset[3]-delt_error)/(delt_error_subset[3]-delt_error_subset[2]);
            delt_error_subjection[1] = 1-delt_error_subjection[0];
        }
        else if(delt_error <= delt_error_subset[4])
        {
            delt_error_dir[0] = 0;
            delt_error_dir[1] = 1;
            delt_error_subjection[0] = (delt_error_subset[4]-delt_error)/(delt_error_subset[4]-delt_error_subset[3]);
            delt_error_subjection[1] = 1-delt_error_subjection[0];
        }
        else if(delt_error <= delt_error_subset[5])
        {
            delt_error_dir[0] = 1;
            delt_error_dir[1] = 2;
            delt_error_subjection[0] = (delt_error_subset[5]-delt_error)/(delt_error_subset[5]-delt_error_subset[4]);
            delt_error_subjection[1] = 1-delt_error_subjection[0];
        }
        else
        {
            delt_error_dir[0] = 2;
            delt_error_dir[1] = 3;
            delt_error_subjection[0] = (delt_error_subset[6]-error)/(delt_error_subset[6]-delt_error_subset[5]);
            delt_error_subjection[1] = 1-delt_error_subjection[0];
        }
    }

    /**********ģ������**********/
    out_dir[0] = p_rule[error_dir[0]+3][delt_error_dir[0]+3];
    out_dir[1] = p_rule[error_dir[0]+3][delt_error_dir[1]+3];
    out_dir[2] = p_rule[error_dir[1]+3][delt_error_dir[0]+3];
    out_dir[3] = p_rule[error_dir[1]+3][delt_error_dir[1]+3];

    /**********��ģ��**********/
    temp1 = out_subset[out_dir[0]+3]*error_subjection[0]*delt_error_subjection[0];
    temp2 = out_subset[out_dir[1]+3]*error_subjection[0]*delt_error_subjection[1];
    temp3 = out_subset[out_dir[2]+3]*error_subjection[1]*delt_error_subjection[0];
    temp4 = out_subset[out_dir[3]+3]*error_subjection[1]*delt_error_subjection[1];
    out = temp1+temp2+temp3+temp4;

    /**********�����仯��**********/
    delt_p = k1*out;
    delt_d = k2*out;
}

#pragma section all restore
