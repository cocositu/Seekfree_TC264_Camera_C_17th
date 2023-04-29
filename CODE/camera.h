#ifndef _camera_h
#define _camera_h

#include "headfile.h"

extern int16 X1,X2,Y1,Y2;  //�����ڵ��Թ�����ʹ�õ�ȫ�ֱ���

extern uint8 search_line_end;       //������ֹ��,û�в���(Ҫ�úÿ���,�ﵽ�Ȳ�Ӱ����Ҫ��õ���Ϣ,�ּ��ٲ���Ҫ������,����ɼ���û�е���Ϣ)
extern uint8 image_01[MT9V03X_H][MT9V03X_W];   //�����ֵ����ͼ������
extern uint8 image_yuanshi[MT9V03X_H][MT9V03X_W];//����ԭʼͼ�������ֵ
extern uint8 kuandu_saidao[MT9V03X_H-1];   //����������ȵ�����



//�������ͽṹ��
struct ROAD_TYPE
{
     int8 straight;         //ֱ��
     int8 bend;             //���
     int8 Ramp;             //�µ�
     int8 Cross;            //ʮ��
     int8 L_Cross;          //����ʮ��
     int8 R_Cross;          //����ʮ��
     int8 LeftCirque;       //�󻷵�
     int8 RightCirque;      //�һ���
     int8 Fork;             //�����
     int8 Barn_l_out;       //�����
     int8 Barn_r_out;       //���ҿ�
     int8 Barn_l_in;        //�����
     int8 Barn_r_in;        //���ҿ�
};
extern struct ROAD_TYPE road_type;

void Camera_Display(void);          //����ͷ����ȫ����

/**************************Ԫ�ش����ֵĺ���********************************************************************************************/
void Element_Test(void);            //Ԫ��ʶ��

void Element_Handle(void);         //Ԫ�ش���
extern uint16 distance_L_Cross_out;
extern uint16 distance_R_Cross_out;
extern uint8 huandao_stage;
extern uint8 xieshizi_stage;
extern int16 annulus_s1;
extern int16 jiaodu_jifen;
extern int16 annulus_jinku_s;

void Handle_Left_Cirque(void);  //�����󻷵�

void Handle_Right_Cirque(void);  //�����һ���

void Diuxian_weizhi_test(uint8 type, uint8 startline, uint8 endline);  //�ж�ĳ�е�ĳ�еĶ������Ͳ�������(�����жϻ����Ľ׶�,����ȽϺ���)
extern uint8 diuxian_hang, budiuxian_hang;

void Handle_Fork(void);   //��������
void Sancha_didian(void);  //��V����͵�(����������)(�ȶ�)
extern int16 fork_s1;
extern int16 fork_s2;
extern uint8 sancha_stage;
extern uint8 sancha_x, sancha_x_zhengque;
extern uint8 sancha_y, sancha_y_zhengque;

void Handle_Cross(void);  //����ʮ��
void Check_Cross_Guaidian(uint8 type);  //��ʮ�ֹյ�
extern int16 cross_s;
extern uint8 cross_left[2];  //��һ���±��ͼ���·��ĵ��yֵ
extern uint8 cross_right[2];

void Handle_L_Cross(void);  //������бʮ��

void Handle_R_Cross(void);   //������бʮ��

void Handle_Barn_Out(uint8 type);   //�������

void Handle_Barn_in(uint8 type);   //�������
extern int8 pass_barn;
extern int16 close_check_ku_s;
extern int16 jinku_s;
/**************************Ԫ�ش����ֽ���**********************************************************************************************/


/**************************Ԫ��ʶ�𲿷ֵĺ���********************************************************************************************/
void check_cheku(uint8 start_point, uint8 end_point, uint8 qiangdu);   //�жϰ����ߺͳ���ķ���(��ʰ�����ߵĻ����ϸ�����)(�ȶ�)
extern uint8 garage_direction;      //1Ϊ��,2λ��
extern uint8 flag_starting_line;

uint8 RoundaboutGetArc(uint8 type, uint8 num); //�ж�Բ��(�������)(�ȶ�)
extern uint8 bianxian_guaidian_l;
extern uint8 bianxian_guaidian_r;
/**************************Ԫ��ʶ�𲿷ֽ���********************************************************************************************/


/**************************����ѭ���ĺ���************************************************************************************************/
void Transfer_Camera(void);          //ͼ��ת��

uint8 Threshold_Deal(uint8* image, uint16 col, uint16 row, uint32 pixel_threshold);   //�Ż���Ĵ��

void Get01change_Dajin(void);    //�ڰ׻�����

void Pixle_Filter(void);   //��ʴ(ȥ�����)

void Search_Line(uint8 type);     //0Ϊ��ͨģʽ��1Ϊ������ģʽ��ֻ��������ʼ�����겻ͬ��
extern uint8 m_line_x[MT9V03X_H];                                     //�����������ߵ���
extern uint8 m_second_line_x[MT9V03X_H];                              //���油�ߺ������
extern uint8 r_line_x[MT9V03X_H], l_line_x[MT9V03X_H];               //�������ұ߽��������ÿһ��ֻ��һ����߽��һ���ұ߽�
extern uint8 r_second_line_x[MT9V03X_H], l_second_line_x[MT9V03X_H]; //�������ұ߽��������ÿһ��ֻ��һ����߽��һ���ұ߽�
extern uint8 r_lose_value, l_lose_value;                         //���Ҷ�����������ע��Ҫ��һ������
extern int16 l_line_qulv,r_line_qulv;
extern uint8 r_losemax,l_losemax;

void Calculate_Offset(void);

void Calculate_Offset_1(void);

extern int16 offset;                    //����ͷ����õ���ƫ��

void Blacking(void);        //���ߺ���(��ʾ�������ߺ��������ߺ�һЩ�������)
/**************************����ѭ������**************************************************************************************************/


/**************************ʵ�����߹��ܵĺ���*******************************************************************************************/
void Regression(uint8 type, uint8 startline, uint8 endline);       //�������ߵ�б�ʺͽؾ�(������һ��������Ͽ���ʵ��,��һ�����������һ��������)
extern float parameterA, parameterB;

void Hua_Xian(uint8 type, uint8 startline, uint8 endline, float parameterB, float parameterA);  //��С���˷�����(��Regression���ʹ��,���ʰ������)

void La_zhixian(uint8 x_down, uint8 y_down, uint8 x_up, uint8 y_up, uint8 *array);    //������ֱ��(���ñ߳�������ϵд��)(�ȶ�)

/**************************ʵ�����߹��ܵĺ�������****************************************************************************************/


/**************************һЩ���߰���ĺ����д����Ժ�ɾ��******************************************************************************/
float Process_Curvity(uint8 x1, uint8 y1, uint8 x2, uint8 y2, uint8 x3, uint8 y3);   //���㷨������������(Ч������,������ֱ������)

uint8 My_Sqrt(int16 x);    //�Լ�д�Ŀ�������(�ȶ�,�������ʵĺ������е���)

void Bianxian_guaidian(uint8 type, uint8 startline, uint8 endline );//��һ���յ㣬û�иĹ�
extern uint8 l_guaidain_x, l_guaidain_y;
extern uint8 r_guaidain_x, r_guaidain_y;

uint8 Bianxian_guaidian_num(uint8 type, uint8 startline, uint8 endline );//�������յ�
extern uint8 l_guaidain_x1, l_guaidain_y1;//һ�յ�
extern uint8 r_guaidain_x1, r_guaidain_y1;
extern uint8 l_guaidain_x2, l_guaidain_y2;//���յ�
extern uint8 r_guaidain_x2, r_guaidain_y2;

void Qianzhang(void);    //δ���Ժ�
extern uint8 qianzhang;

uint8 Cirque_or_Cross(uint8 type, uint8 startline);   //���ȶ�

extern uint8 Num;

void Mid_Col(void);
extern uint8 length;

void Outside_protect(void);

void HDPJ_lvbo(uint8 data[], uint8 N, uint8 size);

void Check_Zhidao(void);
extern uint8 sudu_yingzi;

uint8 Tututu(uint8 type);

#endif
