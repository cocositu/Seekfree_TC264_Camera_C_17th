#include "fuzzy.h"
#pragma section all "cpu0_dsram"

//全局变量定义
float delt_p;                //动态p输出项
float delt_d;                //动态d输出项
float k1 = 1;                //动态p输出项的系数
float k2 = 0.01;             //动态d输出项的系数

/************************************************
函数名：Fuzzy_pid
功  能：模糊自适应pid
参  数：float error, float delt_error
返回值：void
************************************************/

void Fuzzy_pid(float error, float delt_error)
{
    //模糊子集
    float error_subset[7] = {NB1, NM1, NS1, ZO1, PS1, PM1, PB1};          //偏差的模糊子集
    float delt_error_subset[7] = {NB2, NM2, NS2, ZO2, PS2, PM2, PB2};     //偏差变化量的模糊子集
    float out_subset[7] = {NB3, NM3, NS3, ZO3, PS3, PM3, PB3};            //输出模糊子集
    float error_subjection[2] = {0};                                      //偏差的隶属度
    float delt_error_subjection[2] = {0};                                 //偏差变化量的隶属度
    int8 error_dir[2] = {0};                                              //偏差在规则表中的索引值
    int8 delt_error_dir[2] = {0};                                         //偏差变化量在规则表中的索引值
    int8 out_dir[4] = {0};                                                //规则表中值在输出子集中的索引值
    float temp1, temp2, temp3, temp4;                                     //每一项解模糊的值
    float out;                                                            //解模糊后输出值

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

    /**********模糊化**********/
    //error的隶属度
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

    //delt_error的隶属度
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

    /**********模糊推理**********/
    out_dir[0] = p_rule[error_dir[0]+3][delt_error_dir[0]+3];
    out_dir[1] = p_rule[error_dir[0]+3][delt_error_dir[1]+3];
    out_dir[2] = p_rule[error_dir[1]+3][delt_error_dir[0]+3];
    out_dir[3] = p_rule[error_dir[1]+3][delt_error_dir[1]+3];

    /**********解模糊**********/
    temp1 = out_subset[out_dir[0]+3]*error_subjection[0]*delt_error_subjection[0];
    temp2 = out_subset[out_dir[1]+3]*error_subjection[0]*delt_error_subjection[1];
    temp3 = out_subset[out_dir[2]+3]*error_subjection[1]*delt_error_subjection[0];
    temp4 = out_subset[out_dir[3]+3]*error_subjection[1]*delt_error_subjection[1];
    out = temp1+temp2+temp3+temp4;

    /**********参数变化量**********/
    delt_p = k1*out;
    delt_d = k2*out;
}

#pragma section all restore
