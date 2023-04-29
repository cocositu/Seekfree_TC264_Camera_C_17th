#ifndef _camera_h
#define _camera_h

#include "headfile.h"

extern int16 X1,X2,Y1,Y2;  //几个在调试过程中使用的全局变量

extern uint8 search_line_end;       //搜线终止行,没有采用(要好好考虑,达到既不影响所要获得的信息,又减少不必要的搜线,避免采集到没有的信息)
extern uint8 image_01[MT9V03X_H][MT9V03X_W];   //储存二值化的图像数组
extern uint8 image_yuanshi[MT9V03X_H][MT9V03X_W];//储存原始图像计算阈值
extern uint8 kuandu_saidao[MT9V03X_H-1];   //储存赛道宽度的数组



//赛道类型结构体
struct ROAD_TYPE
{
     int8 straight;         //直道
     int8 bend;             //弯道
     int8 Ramp;             //坡道
     int8 Cross;            //十字
     int8 L_Cross;          //入左十字
     int8 R_Cross;          //入右十字
     int8 LeftCirque;       //左环岛
     int8 RightCirque;      //右环岛
     int8 Fork;             //三岔口
     int8 Barn_l_out;       //出左库
     int8 Barn_r_out;       //出右库
     int8 Barn_l_in;        //入左库
     int8 Barn_r_in;        //入右库
};
extern struct ROAD_TYPE road_type;

void Camera_Display(void);          //摄像头处理全流程

/**************************元素处理部分的函数********************************************************************************************/
void Element_Test(void);            //元素识别

void Element_Handle(void);         //元素处理
extern uint16 distance_L_Cross_out;
extern uint16 distance_R_Cross_out;
extern uint8 huandao_stage;
extern uint8 xieshizi_stage;
extern int16 annulus_s1;
extern int16 jiaodu_jifen;
extern int16 annulus_jinku_s;

void Handle_Left_Cirque(void);  //处理左环岛

void Handle_Right_Cirque(void);  //处理右环岛

void Diuxian_weizhi_test(uint8 type, uint8 startline, uint8 endline);  //判断某行到某行的丢线数和不丢线数(用于判断环岛的阶段,还算比较好用)
extern uint8 diuxian_hang, budiuxian_hang;

void Handle_Fork(void);   //处理三岔
void Sancha_didian(void);  //找V字最低点(处理三岔用)(稳定)
extern int16 fork_s1;
extern int16 fork_s2;
extern uint8 sancha_stage;
extern uint8 sancha_x, sancha_x_zhengque;
extern uint8 sancha_y, sancha_y_zhengque;

void Handle_Cross(void);  //处理十字
void Check_Cross_Guaidian(uint8 type);  //找十字拐点
extern int16 cross_s;
extern uint8 cross_left[2];  //第一个下标存图像下方的点的y值
extern uint8 cross_right[2];

void Handle_L_Cross(void);  //处理左斜十字

void Handle_R_Cross(void);   //处理右斜十字

void Handle_Barn_Out(uint8 type);   //处理出库

void Handle_Barn_in(uint8 type);   //处理入库
extern int8 pass_barn;
extern int16 close_check_ku_s;
extern int16 jinku_s;
/**************************元素处理部分结束**********************************************************************************************/


/**************************元素识别部分的函数********************************************************************************************/
void check_cheku(uint8 start_point, uint8 end_point, uint8 qiangdu);   //判断斑马线和车库的方向(在拾牙慧者的基础上改良了)(稳定)
extern uint8 garage_direction;      //1为左,2位右
extern uint8 flag_starting_line;

uint8 RoundaboutGetArc(uint8 type, uint8 num); //判断圆弧(借鉴龙邱)(稳定)
extern uint8 bianxian_guaidian_l;
extern uint8 bianxian_guaidian_r;
/**************************元素识别部分结束********************************************************************************************/


/**************************基本循迹的函数************************************************************************************************/
void Transfer_Camera(void);          //图像转存

uint8 Threshold_Deal(uint8* image, uint16 col, uint16 row, uint32 pixel_threshold);   //优化后的大津法

void Get01change_Dajin(void);    //黑白化函数

void Pixle_Filter(void);   //腐蚀(去除噪点)

void Search_Line(uint8 type);     //0为普通模式，1为斑马线模式（只有搜线起始横坐标不同）
extern uint8 m_line_x[MT9V03X_H];                                     //储存赛道中线的列
extern uint8 m_second_line_x[MT9V03X_H];                              //储存补线后的中线
extern uint8 r_line_x[MT9V03X_H], l_line_x[MT9V03X_H];               //储存左右边界的列数，每一行只有一个左边界和一个右边界
extern uint8 r_second_line_x[MT9V03X_H], l_second_line_x[MT9V03X_H]; //储存左右边界的行数，每一行只有一个左边界和一个右边界
extern uint8 r_lose_value, l_lose_value;                         //左右丢线数，后面注意要走一次清零
extern int16 l_line_qulv,r_line_qulv;
extern uint8 r_losemax,l_losemax;

void Calculate_Offset(void);

void Calculate_Offset_1(void);

extern int16 offset;                    //摄像头处理得到的偏差

void Blacking(void);        //划线函数(显示赛道边线和赛道中线和一些特殊的行)
/**************************基本循迹结束**************************************************************************************************/


/**************************实现拉线功能的函数*******************************************************************************************/
void Regression(uint8 type, uint8 startline, uint8 endline);       //计算拉线的斜率和截距(与下面一个函数结合可以实现,用一个边线数组的一段来拉线)
extern float parameterA, parameterB;

void Hua_Xian(uint8 type, uint8 startline, uint8 endline, float parameterB, float parameterA);  //最小二乘法划线(与Regression结合使用,借鉴拾牙慧者)

void La_zhixian(uint8 x_down, uint8 y_down, uint8 x_up, uint8 y_up, uint8 *array);    //两点拉直线(运用边长比例关系写的)(稳定)

/**************************实现拉线功能的函数结束****************************************************************************************/


/**************************一些乱七八糟的函数有待调试和删减******************************************************************************/
float Process_Curvity(uint8 x1, uint8 y1, uint8 x2, uint8 y2, uint8 x3, uint8 y3);   //三点法计算赛道曲率(效果还行,可用于直道加速)

uint8 My_Sqrt(int16 x);    //自己写的开方函数(稳定,计算曲率的函数中有调用)

void Bianxian_guaidian(uint8 type, uint8 startline, uint8 endline );//找一个拐点，没有改过
extern uint8 l_guaidain_x, l_guaidain_y;
extern uint8 r_guaidain_x, r_guaidain_y;

uint8 Bianxian_guaidian_num(uint8 type, uint8 startline, uint8 endline );//找两个拐点
extern uint8 l_guaidain_x1, l_guaidain_y1;//一拐点
extern uint8 r_guaidain_x1, r_guaidain_y1;
extern uint8 l_guaidain_x2, l_guaidain_y2;//二拐点
extern uint8 r_guaidain_x2, r_guaidain_y2;

void Qianzhang(void);    //未调试好
extern uint8 qianzhang;

uint8 Cirque_or_Cross(uint8 type, uint8 startline);   //较稳定

extern uint8 Num;

void Mid_Col(void);
extern uint8 length;

void Outside_protect(void);

void HDPJ_lvbo(uint8 data[], uint8 N, uint8 size);

void Check_Zhidao(void);
extern uint8 sudu_yingzi;

uint8 Tututu(uint8 type);

#endif
