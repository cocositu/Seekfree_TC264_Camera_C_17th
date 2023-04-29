#include "camera.h"
#pragma section all "cpu1_dsram"

uint8 search_line_end = 10;     //������ֹ��

int16 X1=0,X2=0,Y1=0,Y2=0;   //��ʾһЩ����(��Ԫ�ص�ʱ��������һЩ������ֵ)
uint8 image_01[MT9V03X_H][MT9V03X_W];
uint8 image_yuanshi[MT9V03X_H][MT9V03X_W];


float offset_quanzhong [15] = {0.96, 0.92, 0.88, 0.83, 0.77,
                               0.71, 0.65, 0.59, 0.53, 0.47,
                               0.47, 0.47, 0.47, 0.47, 0.47,};  //ƫ��Ȩ��

uint8 kuandu_saidao [MT9V03X_H-1] = {
        106, 105, 104, 103, 102, 101, 100, 99, 98, 97,
        96,  95,  94,  93,  92,  91,  90,  89,  88,  87,
        86,  85,  84,  83,  82,  81,  80,  79,  78,  77,
        76,  75,  74,  73,  72,  71,  70,  69,  68,  67,
        66,  65,  64,  63,  62,  61,  60,  59,  58,  57,
        56,  55,  54,  53,  52,  51,  50,  49,  48,  47,
        46,  45,  44,  43,  42,  41,  40,  39,  38,  37,
        35,  33,  31,  29,  27,   25,  23,  21,  19,  17 }; //ǰ80�е��������

struct ROAD_TYPE road_type = {
        .straight      = 0,
        .bend          = 0,
        .Ramp          = 0,
        .Cross         = 0,
        .L_Cross       = 0,
        .R_Cross       = 0,
        .LeftCirque    = 0,
        .RightCirque   = 0,
        .Fork          = 0,
        .Barn_l_out    = 0,
        .Barn_r_out    = 0,
        .Barn_l_in     = 0,
        .Barn_r_in     = 0,
};

//����ͷ����ȫ����
void Camera_Display(void)
{
    if(mt9v03x_finish_flag==1)
    {
        Transfer_Camera();
        mt9v03x_finish_flag = 0;                   //��ͼ��ʹ����Ϻ�  ��������־λ�����򲻻Ὺʼ�ɼ���һ��ͼ��
        Get01change_Dajin();                       //��ֵ��
        Pixle_Filter();                            //��ʴ�������˲���
        Search_Line(0);                             //��������
        Element_Test();                            //Ԫ��ʶ��
        Element_Handle();                          //Ԫ�ش���
        Check_Zhidao();
//        HDPJ_lvbo(m_line_x, 20, MT9V03X_H -1);     //�����˲�
//        Calculate_Offset();                        //ƫ�����
        Calculate_Offset_1();                        //ƫ�����

        for(uint8 i=(MT9V03X_H -1); i>search_line_end; i--)
        {
            m_line_x[i] = (l_line_x[i] + r_line_x[i])/2;
        }
        if(flag.image_show)
        {            //��Ļ��ʾ
            Blacking();
        }
    }
}

/****************************END*****************************************/


//Ԫ�ش���
uint16 distance_L_Cross_out=0;
uint16 distance_R_Cross_out=0;
uint8 flag_L_shizi_R_turn=0;
uint8 flag_R_shizi_L_turn=0;

int16 annulus_s1=0;
int16 jiaodu_jifen =0;
int16 annulus_jinku_s =0;
void  Element_Handle()
{
    /**************���**********************/
    if(road_type.Barn_l_in)
    {
        Handle_Barn_in(1);
    }
    else if(road_type.Barn_r_in)
    {
        Handle_Barn_in(2);
    }
    /***************Բ��*********************/
    if(road_type.LeftCirque)
    {
        Handle_Left_Cirque();
    }
    else if(road_type.RightCirque)
    {
        Handle_Right_Cirque();
    }
    /****************бʮ��********************/
    if(road_type.L_Cross)
    {
        Handle_L_Cross();
    }
    else if(road_type.R_Cross)
    {
        Handle_R_Cross();
    }
    /****************����********************/
    if(road_type.Fork)
    {
        Handle_Fork();
    }
    /*************ʮ��***********************/
    if(road_type.Cross)
    {
        Handle_Cross();
    }
}
/****************************END*****************************************/


//Ԫ��ʶ��
int16 l_line_qulv=0, r_line_qulv=0;
void Element_Test(void)
{
    l_line_qulv = 1000*Process_Curvity(l_line_x[MT9V03X_H-10], MT9V03X_H-10, l_line_x[MT9V03X_H-25], MT9V03X_H-25, l_line_x[MT9V03X_H-40], MT9V03X_H-40);
    r_line_qulv = 1000*Process_Curvity(r_line_x[MT9V03X_H-10], MT9V03X_H-10, r_line_x[MT9V03X_H-25], MT9V03X_H-25, r_line_x[MT9V03X_H-40], MT9V03X_H-40);

    Mid_Col();
    Outside_protect();

    //�ж����ҳ���
    if(flag.open_check_ku)
    {
        check_cheku(45,40,2);//������
    }

    if(!road_type.Fork && !road_type.LeftCirque && !road_type.RightCirque && !road_type.L_Cross
            && !road_type.R_Cross && !road_type.Cross && !road_type.Barn_l_in && !road_type.Barn_r_in)
    {
        //�ж����ҳ���
        if(flag_starting_line)
        {
            if(garage_direction ==1)     //�󳵿�
            {
                road_type.Barn_l_in  =1;
                road_type.straight = 0;
                road_type.bend = 0;
                garage_direction =0;     //������ⷽ���־λ
            }
            else if(garage_direction ==2)     //�ҳ���
            {
                road_type.Barn_r_in  =1;
                road_type.straight = 0;
                road_type.bend = 0;
                garage_direction =0;     //������ⷽ���־λ
            }
        }
        //�ж���бʮ�ֺ���Բ��
        else if(length > 60 && l_lose_value >30 && r_lose_value <3 && fabs(r_line_qulv) <4 && !Tututu(2))//
        {
            BUZZER_ON;
            if(Cirque_or_Cross(1,l_losemax) >= 38 && road_type.LeftCirque ==0 && road_type.L_Cross ==0 && road_type.Fork==0)  //��ඪ��λ��y���·��н϶�׵�,�ж�ΪԲ��
            {
                road_type.LeftCirque =1;
                road_type.straight = 0;
                road_type.bend = 0;
            }
            else if(road_type.L_Cross ==0 && road_type.LeftCirque ==0 && road_type.Fork==0 && r_line_x[40] < 120 && r_line_x[43] < 120)//
            {
                //                BUZZER_ON;
                road_type.L_Cross =1;
                road_type.straight = 0;
                road_type.bend = 0;
            }
        }
        //��бʮ�ֺ���Բ��
        else if(length > 60 && r_lose_value >30 && l_lose_value <3 && fabs(l_line_qulv) <4 && !Tututu(1))//
        {
            BUZZER_ON;
            if(Cirque_or_Cross(2,r_losemax) > 40 && road_type.RightCirque ==0 && road_type.R_Cross ==0 && road_type.Fork==0)  //��ඪ��λ��y���·��н϶�׵�,�ж�ΪԲ��
            {
                road_type.RightCirque =1;
                road_type.straight = 0;
                road_type.bend = 0;
            }
            else if(road_type.R_Cross ==0 && road_type.RightCirque ==0 && road_type.Fork==0 && l_line_x[40] > 40 && l_line_x[43] > 40)//
            {
                road_type.R_Cross =1;
                road_type.straight = 0;
                road_type.bend = 0;
            }
        }
        //���������
        else if(RoundaboutGetArc(1, 15) && RoundaboutGetArc(2, 15))
        {
//            BUZZER_ON;
            road_type.Fork =1;
            road_type.straight = 0;
            road_type.bend = 0;
        }
        //���ʮ��
        else if(length > 60 && r_lose_value >20 && l_lose_value > 20)
        {
            //            BUZZER_ON;
            road_type.Cross = 1;
            road_type.straight = 0;
            road_type.bend = 0;
        }
        //�ų�����Ԫ�أ��ж�ֱ�������
        else if(sudu_yingzi <= 75)
        {
            road_type.straight =0;
            road_type.bend = 1;
        }
        else
        {
            road_type.straight =1;
            road_type.bend = 0;
        }
    }
}
/****************************END*****************************************/


//*******************ͼ������ת�溯��************************************//
void Transfer_Camera()
{
    for(uint8 y=0; y<MT9V03X_H; y++)       //�洢��һ�������飬��������HighΪ120��WidthΪ188��ˢ����Ϊ50��
    {
        for(uint8 x=0; x<MT9V03X_W; x++)
        {
            image_yuanshi[y][x] = mt9v03x_image[y][x];
        }
    }
}
/****************************END*****************************************/

uint8 kuan[MT9V03X_H];
//�������ߺ���
uint8 m_line_x[MT9V03X_H];                                    //�����������ߵ���
uint8 r_line_y[MT9V03X_H], l_line_y[MT9V03X_H];               //�������ұ߽������
uint8 r_line_x[MT9V03X_H], l_line_x[MT9V03X_H];               //����ԭʼͼ������ұ߽������
uint8 r_second_line_x[MT9V03X_H], l_second_line_x[MT9V03X_H]; //���油��֮������ұ߽�ֵ����δ�õ���
uint8 m_second_line_x[MT9V03X_H];                             //�����������ߵ���
uint8 r_lose_value=0, l_lose_value=0;                         //���Ҷ�����������ע��Ҫ��һ������
uint8 r_search_flag[MT9V03X_H], l_search_flag[MT9V03X_H];     //�Ƿ��ѵ��ߵı�־
uint8 height, r_width, l_width;                               //ѭ��������
uint8 r_losemax,l_losemax;
void Search_Line(uint8 type)     //0Ϊ��ͨģʽ��1Ϊ������ģʽ��ֻ��������ʼ�����겻ͬ��
{
    uint8 l_flag=0,r_flag=0;
    uint8 l_search_start, r_search_start;                   //������ʼ������
    uint8 r_searchget_flage, l_searchget_flage;             //�ѵ���ʱ�ı�־λ
    r_searchget_flage=1; l_searchget_flage=1;               //��ʼ������Ĭ��Ϊ�ϴ��ѵ���
    r_lose_value=0; l_lose_value=0;                         //���Ҷ�����������ע��Ҫ��һ������


    for(height=(MT9V03X_H -1); height>search_line_end; height--)
    {
        //ȷ��ÿ�е�������ʼ������
        if (type == 0)//��ͨģʽ
        {
            if( (height>MT9V03X_H-5) || ( (l_line_x[height+1] == 0) && (r_line_x[height+1] == MT9V03X_W -1) && (height <MT9V03X_H-4) )  )   //ǰ���У��������Ҷ����ߵ���
            {
                l_search_start = MT9V03X_W/2;
                r_search_start = MT9V03X_W/2;
            }
            else if((l_line_x[height+1] !=0) && (r_line_x[height+1] !=MT9V03X_W -1) && (height <MT9V03X_H-4))   //���Ҷ�������
            {
                l_search_start = l_line_x[height+1]+7;
                r_search_start = r_line_x[height+1]-7;
            }
            else if((l_line_x[height+1] != 0 && r_line_x[height+1] == MT9V03X_W -1) && (height <MT9V03X_H-4))   //�󲻶���,�Ҷ���
            {
                l_search_start = l_line_x[height+1]+7;
                r_search_start = MT9V03X_W/2;
            }
            else if((l_line_x[height+1] == 0 && r_line_x[height+1] != MT9V03X_W -1) && (height <MT9V03X_H-4))   //�Ҳ�����,����
            {
                l_search_start = MT9V03X_W/2;
                r_search_start = r_line_x[height+1]-7;
            }
        }

        if( (image_01[height][MT9V03X_W/2] ==0) && (image_01[height -1][MT9V03X_W/2] ==0) && (image_01[height -2][MT9V03X_W/2] ==0) && (height <MT9V03X_H-40)) //������ֹ����
        {
            search_line_end = height+1;
            break;
        }
        else
        {
            search_line_end = 10;
        }


        for(l_width=l_search_start; l_width>1; l_width--)      //�������
        {
            if(image_01[height][l_width -2]==0 && image_01[height][l_width -1]==0 && image_01[height][l_width] !=0 && l_width>2)
            {   //�ںڰ�
                l_line_x[height] = l_width-1;
                l_line_y[height] = height;
                l_search_flag[height] = 1;
                l_searchget_flage = 1;
                break;
            }
            else if(l_width==2)
            {
                if(l_flag==0)
                {
                    l_flag=1;
                    l_losemax=height;
                }
                l_line_x[height] = 0;
                l_line_y[height] = height;
                l_search_flag[height] = 0;
                l_searchget_flage = 0;
                l_lose_value++;
                break;
            }
        }

        for(r_width=r_search_start; r_width<(MT9V03X_W -2); r_width++)      //�ұ�����
        {
            if( image_01[height][r_width ] !=0 && image_01[height][r_width +1]==0 && image_01[height][r_width +2]==0 && r_width<MT9V03X_W-3)
            {   //�׺ں�
                r_line_x[height] = r_width+1;
                r_line_y[height] = height;
                r_search_flag[height] = 1;
                r_searchget_flage = 1;
                break;
            }
            else if(r_width==MT9V03X_W -3)
            {
                if(r_flag==0)
                {
                    r_flag=1;
                    r_losemax=height;
                }
                r_line_x[height] = MT9V03X_W -1;
                r_line_y[height] = height;
                r_search_flag[height] = 0;
                r_searchget_flage = 0;
                r_lose_value++;
                break;
            }
        }
        kuan[height]=r_line_x[height] - l_line_x[height];
    }
}
/****************************END*****************************************/


//��ֵ��
uint8 Threshold;  //��ֵ
uint8 Threshold_static = 160;   //��ֵ��̬����
uint16 Threshold_detach = 300;  //�����㷨�ָ���ֵ(��ǿԽǿ,��ֵԽ��)
void Get01change_Dajin(void)
{
    Threshold = Threshold_Deal(image_yuanshi[0], MT9V03X_W, MT9V03X_H, Threshold_detach);

    if (Threshold < Threshold_static)
    {
        Threshold = Threshold_static;
    }

    uint8 thre;
    for(uint8 y = 0; y < MT9V03X_H; y++)
    {
        for(uint8 x = 0; x < MT9V03X_W; x++)
        {
            if (x <= 15)
                thre = Threshold - 10;
            else if (x >= MT9V03X_W-15)
                thre = Threshold - 10;
            else
                thre = Threshold;

            if (image_yuanshi[y][x] >thre)         //��ֵԽ����ʾ������Խ�࣬��ǳ��ͼ��Ҳ����ʾ����
                image_01[y][x] = 255;  //��
            else
                image_01[y][x] = 0;  //��
        }
    }
}
/****************************END*****************************************/


///���ֵ����ֵ
//-------------------------------------------------------------------------------------------------------------------
//  @param      image  ͼ������
//  @param      clo    ��
//  @param      row    ��
//  @param      pixel_threshold ��ֵ����
//-------------------------------------------------------------------------------------------------------------------
uint8 Threshold_Deal(uint8* image, uint16 col, uint16 row, uint32 pixel_threshold)
{
#define GrayScale 256
    uint16 width = col;
    uint16 height = row;
    int pixelCount[GrayScale];
    float pixelPro[GrayScale];
    int i, j;
    int pixelSum = width * height;
    uint8 threshold = 0;
    uint8* data = image;  //ָ���������ݵ�ָ��
    for (i = 0; i < GrayScale; i++)
    {
        pixelCount[i] = 0;
        pixelPro[i] = 0;
    }

    uint32 gray_sum = 0;
    //ͳ�ƻҶȼ���ÿ������������ͼ���еĸ���
    for (i = 0; i < height; i += 1)
    {
        for (j = 0; j < width; j += 1)
        {
            // if((sun_mode&&data[i*width+j]<pixel_threshold)||(!sun_mode))
            //{
            pixelCount[(
                    int)data[i * width + j]]++;  //����ǰ�ĵ������ֵ��Ϊ����������±�
            gray_sum += (int)data[i * width + j];  //�Ҷ�ֵ�ܺ�
            //}
        }
    }

    //����ÿ������ֵ�ĵ�������ͼ���еı���
    for (i = 0; i < GrayScale; i++)
    {
        pixelPro[i] = (float)pixelCount[i] / pixelSum;
    }

    //�����Ҷȼ�[0,255]
    float w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;
    w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;
    for (j = 0; j < pixel_threshold; j++)
    {
        w0 +=
                pixelPro[j];  //��������ÿ���Ҷ�ֵ�����ص���ռ����֮�� ���������ֵı���
        u0tmp += j * pixelPro[j];  //�������� ÿ���Ҷ�ֵ�ĵ�ı��� *�Ҷ�ֵ

        w1 = 1 - w0;
        u1tmp = gray_sum / pixelSum - u0tmp;

        u0 = u0tmp / w0;    //����ƽ���Ҷ�
        u1 = u1tmp / w1;    //ǰ��ƽ���Ҷ�
        u = u0tmp + u1tmp;  //ȫ��ƽ���Ҷ�
        deltaTmp = w0 * pow((u0 - u), 2) + w1 * pow((u1 - u), 2);
        if (deltaTmp > deltaMax)
        {
            deltaMax = deltaTmp;
            threshold = (uint8)j;     //��������û��ǿ������ת����,���Լ��ӵ�
        }
        if (deltaTmp < deltaMax)
        {
            break;
        }
    }
    return threshold;
}
/****************************END*****************************************/


//�����������Ķ�ֵ��
uint8 my_adapt_threshold(uint8 *image, uint16 width, uint16 height)   //ע�������ֵ��һ��Ҫ��ԭͼ��
{
#define GrayScale 256
    int pixelCount[GrayScale];
    float pixelPro[GrayScale];
    int i, j, pixelSum = width * height/4;
    uint8 threshold = 0;
    uint8* data = image;  //ָ���������ݵ�ָ��
    for (i = 0; i < GrayScale; i++)
    {
        pixelCount[i] = 0;
        pixelPro[i] = 0;
    }

    uint32 gray_sum=0;
    //ͳ�ƻҶȼ���ÿ������������ͼ���еĸ���
    for (i = 0; i < height; i+=2)
    {
        for (j = 0; j < width; j+=2)
        {
            pixelCount[(int)data[i * width + j]]++;  //����ǰ�ĵ������ֵ��Ϊ����������±�
            gray_sum+=(int)data[i * width + j];       //�Ҷ�ֵ�ܺ�
        }
    }

    //����ÿ������ֵ�ĵ�������ͼ���еı���

    for (i = 0; i < GrayScale; i++)
    {
        pixelPro[i] = (float)pixelCount[i] / pixelSum;
    }

    //�����Ҷȼ�[0,255]
    float w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;


    w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;
    for (uint8 j = 0; j < GrayScale; j++)
    {

        w0 += pixelPro[j];  //��������ÿ���Ҷ�ֵ�����ص���ռ����֮��   ���������ֵı���
        u0tmp += j * pixelPro[j];  //�������� ÿ���Ҷ�ֵ�ĵ�ı��� *�Ҷ�ֵ

        w1=1-w0;
        u1tmp=gray_sum/pixelSum-u0tmp;

        u0 = u0tmp / w0;              //����ƽ���Ҷ�
        u1 = u1tmp / w1;              //ǰ��ƽ���Ҷ�
        u = u0tmp + u1tmp;            //ȫ��ƽ���Ҷ�
        deltaTmp = w0 * pow((u0 - u), 2) + w1 * pow((u1 - u), 2);
        if (deltaTmp > deltaMax)
        {
            deltaMax = deltaTmp;
            threshold = j;
        }
        if (deltaTmp < deltaMax)
        {
            break;
        }
    }
    return threshold;
}
/****************************END*****************************************/


//�����ߺ�����(Һ������,�����Ļ��ʾ��ͼ����������žͲ�������)
void Blacking()
{
    for(height=(MT9V03X_H-1); height>search_line_end; height--)
    {
        image_01[height][m_line_x[height]]=0x00;
        image_01[height][l_line_x[height]]=0x00;
        image_01[height][r_line_x[height]]=0x00;
        //        image_01[height][sancha_x_zhengque]=0x00;
        image_01[height][50]=0x00;
        image_01[height][120]=0x00;
    }
    for(uint8 i=1;i<MT9V03X_W-1;i++)
    {
        //        image_01[MT9V03X_H-20][i]=0x00;
        //        image_01[MT9V03X_H-40][i]=0x00;
        //        image_01[MT9V03X_H-60][i]=0x00;
        //        image_01[sancha_y_zhengque][i]=0x00;
        image_01[50][i]=0x00;
        image_01[45][i]=0x00;
    }

    lcd_showuint8(0,7,sancha_stage);

    //    lcd_showint8(50,7,road_type.Cross);
    lcd_showuint8(50,7,sancha_y_zhengque);
    //    lcd_showuint8(50,7,xieshizi_stage);

    //   lcd_showuint8(100,7,road_type.L_Cross);
    lcd_showint16(100,7,fork_s1);


    lcd_displayimage032(image_01[0],MT9V03X_W,MT9V03X_H);

    //    lcd_showint8(0,0,road_type.Fork_in);
    //    lcd_showuint8(0,1,road_type.Fork_out);
    //    lcd_showuint8(0,2,bianxian_guaidian_l);
    //    lcd_showuint8(0,3,bianxian_guaidian_r);
    //    lcd_showuint8(0,3,sancha_y_zhengque);

}

/****************************END*****************************************/


//****************************ƫ�����*******************************//
int16 offset;                    //����ͷ����õ���ƫ��
//int16 shizi_s1;
//void Calculate_Offset()
//{
//
//    uint16 total_midcourt_line =0;            //��������ֵ
//    uint16 mid_value = MT9V03X_W/2 -1 ;       //��Ļ�м�
//
//    for(uint8 y=74; y<78; y++)                //�ۼӿ����е�����ֵ
//    {
//        m_line_x[y] = (l_line_x[y] + r_line_x[y])/2;
//        total_midcourt_line = total_midcourt_line + m_line_x[y];
//    }
//    offset = (total_midcourt_line - 4*mid_value);
//    if((road_type.Barn_r_in == 1) && (pass_barn == 2))
//    {
//        offset =200;
//    }
//    if((road_type.Barn_l_in == 1) && (pass_barn == 2))
//    {
//        offset =-200;
//    }
//}
/****************************END*****************************************/

void Calculate_Offset_1()
{
      offset =0;
      for(uint8 y =MT9V03X_H -1; y >=10; y--)    //���ý���ԶС��Ȩ�ؼ���ƫ��(�������)
      {
          m_line_x[y] =1.0*(l_line_x[y] +r_line_x[y])/2;    //�����벢����û����,ǧ�򲻿���ɾ
      }

      HDPJ_lvbo(m_line_x, 20, MT9V03X_H -1);   //ƽ�������˲�

      for(uint8 y =MT9V03X_H -30; y >=MT9V03X_H -40; y--)    //���ý���ԶС��Ȩ�ؼ���ƫ��(�������)
      {
          offset +=offset_quanzhong[MT9V03X_H-30 -y] *(m_line_x[y] -MT9V03X_W/2);
      }
      offset =offset/2;

      if((road_type.Barn_r_in == 1) && (pass_barn == 2))
      {
          offset = 200;
      }
      if((road_type.Barn_l_in == 1) && (pass_barn == 2))
      {
          offset = -200;
      }
}



//���㷨������������
float Process_Curvity(uint8 x1, uint8 y1, uint8 x2, uint8 y2, uint8 x3, uint8 y3)
{
    float K;
    int S_of_ABC = ((x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1)) / 2;
    //����ķ��ű�ʾ����
    int16 q1 = (int16)((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
    uint8 AB = My_Sqrt(q1);
    q1 = (int16)((x3 - x2) * (x3 - x2) + (y3 - y2) * (y3 - y2));
    uint8 BC = My_Sqrt(q1);
    q1 = (int16)((x3 - x1) * (x3 - x1) + (y3 - y1) * (y3 - y1));
    uint8 AC = My_Sqrt(q1);
    if (AB * BC * AC == 0)
    {
        K = 0;
    }
    else
        K = (float)4 * S_of_ABC / (AB * BC * AC);
    return K;
}
/****************************END*****************************************/


//�Լ�д�Ŀ�������
uint8 My_Sqrt(int16 x)
{
    uint8 ans=0, p=0x80;
    while(p!=0)
    {
        ans += p;
        if(ans*ans>x)
        {
            ans -= p;
        }
        p = (uint8)(p/2);
    }
    return (ans);
}
/****************************END*****************************************/


//ʶ������ߣ��жϳ��ⷽ�򣨲����������Ұ����ߣ�(��ʰ�����ߵĻ����ϸĽ���һ��)
uint8 flag_starting_line =0;
uint8 garage_direction =0;
void check_cheku(uint8 start_point, uint8 end_point ,uint8 qiangdu)
{ //garage_direction=1Ϊ�󳵿⣬garage_direction=2Ϊ�ҳ���
    uint8 times = 0;
    uint8 baise_hang = 0;                              //��ɫ�������жϳ��ⷽ��ʹ��
    flag_starting_line =0;                             //��03.18�Ļ���������������
    garage_direction =0;

    for (uint8 height =start_point; height >=end_point; height--)  //�жϰ�����
    {
        uint8 black_blocks_l =0, black_blocks_r =0;
        uint8 cursor_l =0, cursor_r =0;

        for (uint8 width_l =MT9V03X_W/2, width_r =MT9V03X_W/2; width_l >=1 && width_r<MT9V03X_W-2; width_l--,width_r++)
        {
            if (image_01[height][width_l] ==0 )  //!=0 ��ִ��
            {
                if (cursor_l >20)
                {
                    break;    //����ɫԪ�س���ջ���ȵĲ���   //���break�������
                }
                else
                {
                    cursor_l++;
                    X1=cursor_l;
                }
            }
            else
            {
                if (cursor_l >= qiangdu && cursor_l <= qiangdu+4)     //�����Χ�ǲ��Ǹ�С��(4,8) > (4,12)
                {
                    black_blocks_l++;
                    X2=black_blocks_l;
                    cursor_l =0;
                }
                else
                {
                    cursor_l =0;
                }
            }

            if (image_01[height][width_r] ==0)
            {
                if (cursor_r >= 20)
                {
                    break;    //����ɫԪ�س���ջ���ȵĲ���
                }
                else
                {
                    cursor_r++;
                }
            }
            else
            {
                if (cursor_r >= qiangdu && cursor_r <= qiangdu+4)
                {
                    black_blocks_r++;
                    cursor_r = 0;
                }
                else
                {
                    cursor_r = 0;
                }
            }
        }

        if ((black_blocks_l+black_blocks_r) >= 4 && (black_blocks_l+black_blocks_r) <= 8)
        {
            times++;
        }

        if (times >= (start_point -end_point-3))
        {
            flag_starting_line = 1;
        }
        else
        {
            flag_starting_line = 0;
        }
    }

    if(flag_starting_line ==1)         //�ѵ����������ٽ��г��ⷽ����ж�
    {
        flag.open_check_ku = 0;       //�ѵ������߹رճ�����
        for (uint8 height =start_point; height >=end_point; height--)
        {
            if(image_01[height][0] !=0)
            {
                baise_hang++;
            }
        }
        if(baise_hang >(start_point -end_point-2))       //����ɫ�࣬�������󣬷�������(1��2��)
        {
            garage_direction = 1;
            baise_hang =0;
        }
        else
        {
            garage_direction = 2;
            baise_hang =0;
        }
    }
}
/****************************END*****************************************/


//��ʴ�������˲�,ȥ��ż�����ֵ����,Ч�����ޣ�(�������������,��Ϊ��ʱ��ֵ��ʱ��1��ʾ�İ�,������255��ʾ��,�ø�һ��)
void Pixle_Filter()
{
    for (uint8 height = 10; height < MT9V03X_H-10; height++)
    {
        for (uint8 width = 10; width < MT9V03X_W -10; width = width + 1)
        {
            if ((image_01[height][width] == 0) && (image_01[height - 1][width] + image_01[height + 1][width] +image_01[height][width + 1] + image_01[height][width - 1] >=3*255))
            { //һ���ڵ���������ҵİ׵���ڵ����������������Ϊ��
                image_01[height][width] = 1;
            }
            else if((image_01[height][width] !=0)&&(image_01[height-1][width]+image_01[height+1][width]+image_01[height][width+1]+image_01[height][width-1]<2*255))
            {
                image_01[height][width] =0;
            }
        }
    }
}
/****************************END*****************************************/


//�ҳ���յ�(�򵥴ֱ���)
//uint8 leftx_1[2],left_1[2];
uint8 rightx_1[2] ={MT9V03X_W-1, MT9V03X_W-1}, righty_1[2] ={MT9V03X_H-1, MT9V03X_H-1};
void Check_guaidian_cheku_1(uint8 type)
{
    for(uint8 y=MT9V03X_H -1; y>10; y--)
    {
        if(image_01[y][MT9V03X_W-3] ==0 && image_01[y-1][MT9V03X_W-3] ==0 && image_01[y-2][MT9V03X_W-3] ==0)
        {//y<MT9V03X_H-30 &&
            rightx_1[1] =MT9V03X_W-3;
            righty_1[1] =y;
            break;
        }
    }
}
/****************************END*****************************************/


//��������ͬһ�߽��ϵ���������
void La_zhixian(uint8 x_down, uint8 y_down, uint8 x_up, uint8 y_up, uint8 *array)    //����������ĺ�������
{
    for(uint8 i=y_down; i >(y_up-1); i--)         //���Կ��ܲ���д>=
    {
        int16 X;
        X = (y_down -i) *(x_up -x_down) /(y_down -y_up) ;
        array[i] = array[y_down] + X;
    }
}
/****************************END*****************************************/


//��С���˷�����б�ʺͽؾ�
float parameterA=0, parameterB=0;
void Regression(uint8 type, uint8 startline, uint8 endline)
{//һ�󣬶��ң�0�м�
    uint8 i = 0;
    uint8 sumlines = endline - startline;
    int16 sumX = 0;
    int16 sumY = 0;
    float averageX = 0;
    float averageY = 0;
    float sumUp = 0;
    float sumDown = 0;

    if (type == 0)               //�������
    {
        for (i = startline; i < endline; i++)
        {
            sumX += i;
            sumY += m_line_x[i];
        }
        if (sumlines != 0)
        {
            averageX = sumX*1.0 / sumlines;     //x��ƽ��ֵ
            averageY = sumY*1.0 / sumlines;     //y��ƽ��ֵ
        }
        for (i = startline; i < endline; i++)
        {
            sumUp += (m_line_x[i] - averageY) * (i - averageX);
            sumDown += (i - averageX) * (i - averageX);
        }
        parameterB = sumUp / sumDown;
        parameterA = averageY - parameterB * averageX;
    }
    else if (type == 1)         //�������
    {
        for (i = startline; i < endline; i++)
        {
            sumX += i;
            sumY += l_line_x[i];
        }
        averageX = sumX*1.0 / sumlines;     //x��ƽ��ֵ
        averageY = sumY*1.0 / sumlines;     //y��ƽ��ֵ
        for (i = startline; i < endline; i++)
        {
            sumUp += (l_line_x[i] - averageY) * (i - averageX);
            sumDown += (i - averageX) * (i - averageX);
        }
        parameterB = sumUp / sumDown;
        parameterA = averageY - parameterB * averageX;
    }
    else if (type == 2)             //�������
    {
        for (i = startline; i < endline; i++)
        {
            sumX += i;
            sumY += r_line_x[i];
        }
        averageX = sumX*1.0 / sumlines;     //x��ƽ��ֵ
        averageY = sumY*1.0 / sumlines;     //y��ƽ��ֵ
        for (i = startline; i < endline; i++)
        {
            sumUp += (r_line_x[i] - averageY) * (i - averageX);
            sumDown += (i - averageX) * (i - averageX);
        }
        parameterB = sumUp / sumDown;
        parameterA = averageY - parameterB * averageX;
    }
}
/****************************END*****************************************/


//��С���˷�����
void Hua_Xian(uint8 type, uint8 startline, uint8 endline, float parameterB, float parameterA)
{//һ�󣬶���
    if (type == 1) //��
    {
        for (uint8 i = startline; i < endline; i++)
        {
            l_line_x[i] = (uint8)(parameterB * i + parameterA);
            if (l_line_x[i] < 0)
            {
                l_line_x[i] = 0;
            }
        }
    }
    else if (type == 2)            //��
    {
        for (int i = startline; i < endline; i++)
        {
            r_line_x[i] = (uint8)(parameterB * i + parameterA);
            if (r_line_x[i] > 187)
            {
                r_line_x[i] = 187;

            }
        }
    }
    else if (type == 0)             //��
    {
        for (int i = startline; i < endline; i++)
        {
            m_line_x[i] = (uint8)((l_line_x[i] / 2 + r_line_x[i] / 2));
            if (m_line_x[i] < 0)
            {
                m_line_x[i] = 0;
            }
            if (m_line_x[i] > 185)
            {
                m_line_x[i] = 185;
            }
        }
    }
}
/****************************END*****************************************/


//�������
void CircleTest(uint8 type)
{//1��2��
    if(type == 1)    //�󻷵����
    {
        uint8 y1;
        for(uint8 i=MT9V03X_H-1; i >MT9V03X_H-20; i--)
        {
            if(l_line_x[i] ==0 && l_line_x[i -1] ==0)   //����·��ڿ�
            {
                y1 =i;
                break;
            }
        }
        for(uint8 i=y1 -10; i>10; i--)   //y1 -10
        {
            if(image_01[i][0] ==0 && image_01[i -1][0] ==0 && image_01[i-2][0] ==0)   //�ҵ��Ϸ��ڵ�
            {
                y1 =i;
                break;
            }
        }//�ҵ���ɫԲ������ʼ�߶�����

        for(uint8 i=y1; y1>10; i--)
        {
            if( (l_line_x[y1-2] -l_line_x[y1] <10) &&  (l_line_x[y1-4] -l_line_x[y1] <20) &&(l_line_x[y1-6] -l_line_x[y1] <30) )  //����Բ������������û��ʮ�ֿ�����������룩
            {
                road_type.straight      = 0;
                road_type.bend          = 0;
                road_type.Ramp          = 0;
                //                road_type.Cross         = 0;
                road_type.L_Cross       = 0;
                road_type.R_Cross       = 0;
                road_type.LeftCirque    = 1;
                road_type.RightCirque   = 0;
                //                road_type.Fork_in       = 0;
                //                road_type.Fork_on       = 0;
                //                road_type.Fork_out      = 0;
                road_type.Barn_l_out    = 0;
                road_type.Barn_r_out    = 0;
                road_type.Barn_l_in     = 0;
                road_type.Barn_r_in     = 0;
            }
        }
    }
}
/****************************END*****************************************/

//�ж϶�����λ�ú���Ŀ
uint8 diuxian_hang=0, budiuxian_hang=0;
void Diuxian_weizhi_test(uint8 type, uint8 startline, uint8 endline)
{//1��2��
    diuxian_hang=0, budiuxian_hang=0;
    if(type ==1)
    {
        for(uint8 y=startline; y>=endline; y--)
        {
            budiuxian_hang += l_search_flag[y];
            diuxian_hang = (startline-endline+1) -budiuxian_hang;
        }
    }
    if(type ==2)
    {
        for(uint8 y=startline; y>=endline; y--)
        {
            budiuxian_hang += r_search_flag[y];
            diuxian_hang = (startline-endline+1) -budiuxian_hang;
        }
    }
}
/****************************END*****************************************/

//�ұ��߹յ�
uint8 l_guaidain_x1=0, l_guaidain_y1=0;//һ�յ�
uint8 r_guaidain_x1=0, r_guaidain_y1=0;
uint8 l_guaidain_x2=0, l_guaidain_y2=0;//���յ�
uint8 r_guaidain_x2=0, r_guaidain_y2=0;
uint8 Bianxian_guaidian_num(uint8 type, uint8 startline, uint8 endline)
{
    uint8 bianxian_guaidian_num = 0;

    //һ�յ�
    l_guaidain_x1 =0;
    l_guaidain_y1 =0;
    r_guaidain_x1 =0;
    r_guaidain_y1 =0;
    //���յ�
    l_guaidain_x2 =0;
    l_guaidain_y2 =0;
    r_guaidain_x2 =0;
    r_guaidain_y2 =0;

    //����߹յ�
    if(type == 1)
    {
        if(l_line_x[MT9V03X_H -1] !=0)         //���¹յ����
        {
            //��һ���յ�
            for(uint8 y=startline; y>endline; y--)
            {
                if(fabs(l_line_x[y] -l_line_x[y+1])<4 && (l_line_x[y] -l_line_x[y-3] >8))
                {
                    l_guaidain_x1 = l_line_x[y];
                    l_guaidain_y1 = y;
                    bianxian_guaidian_num++;
                    break;
                }
            }
            //�ڶ����յ�
            if(bianxian_guaidian_num == 1)
            {
                for(uint8 y=l_guaidain_y1-20; y>endline; y--)
                {
                    if(fabs(l_line_x[y] -l_line_x[y-1])<4 && (l_line_x[y] -l_line_x[y+3] >8))
                    {
                        l_guaidain_x2 = l_line_x[y];
                        l_guaidain_y2 = y;
                        bianxian_guaidian_num++;
                        break;
                    }
                }
            }
        }
    }

    //�ұ��߹յ�
    if(type == 2)
    {
        if(r_line_x[MT9V03X_H -1] !=MT9V03X_W -1)         //���¹յ����
        {
            //��һ���յ�
            for(uint8 y=startline; y>endline; y--)
            {
                if(fabs(r_line_x[y] -r_line_x[y+1]) <4 && (r_line_x[y]- r_line_x[y-3]< -8))    //�������Ҫ�����ϸ�һ��
                {
                    r_guaidain_x1 = r_line_x[y];
                    r_guaidain_y1 = y;
                    bianxian_guaidian_num++;
                    break;
                }
            }
            //�ڶ����յ�
            if(bianxian_guaidian_num == 1)
            {
                for(uint8 y=endline; y>r_guaidain_y1-20; y++)
                {
                    if(fabs(r_line_x[y] -r_line_x[y-1]) <4 && (r_line_x[y]- r_line_x[y+3]< -8))    //�������Ҫ�����ϸ�һ��
                    {
                        r_guaidain_x2 = r_line_x[y];
                        r_guaidain_y2 = y;
                        bianxian_guaidian_num++;
                        break;
                    }
                }
            }
        }
    }

    return bianxian_guaidian_num;
}
/****************************END*****************************************/

//�ұ��߹յ�
uint8 l_guaidain_x=0, l_guaidain_y=0;
uint8 r_guaidain_x=0, r_guaidain_y=0;
void Bianxian_guaidian(uint8 type, uint8 startline, uint8 endline )
{ //ÿ��ֻ��һ���յ�����
    l_guaidain_x =0;
    l_guaidain_y =0;
    r_guaidain_x =0;
    r_guaidain_y =0;

    //����߹յ�
    if(type ==1)
    {
        if(l_line_x[MT9V03X_H -1] !=0)         //���¹յ����
        {
            for(uint8 y=startline; y>endline; y--)
            {
                if(fabs(l_line_x[y] -l_line_x[y+1])<4 && (l_line_x[y] -l_line_x[y-3] >8))
                {
                    l_guaidain_y = y;
                    l_guaidain_x = l_line_x[y];
                    break;
                }
            }
        }
    }

    //�ұ��߹յ�
    if(type == 2)
    {
        if(r_line_x[MT9V03X_H -1] !=MT9V03X_W -1)         //���¹յ����
        {
            for(uint8 y=startline; y>endline; y--)
            {
                if(fabs(r_line_x[y] -r_line_x[y+1]) <4 && (r_line_x[y]- r_line_x[y-3]< -8))    //�������Ҫ�����ϸ�һ��
                {
                    r_guaidain_y = y;
                    r_guaidain_x = r_line_x[y];
                    break;
                }
            }
        }
    }
}
/****************************END*****************************************/

uint8 bianxian_guaidian_l=0;
uint8 bianxian_guaidian_r=0;
uint8 RoundaboutGetArc(uint8 type, uint8 num)
{//array���������飬typeģʽ��num�жϵ�ǿ��
    uint8 inc = 0, dec = 0;
    switch (type) //�����
    {
        case 1:
            for (uint8 i = MT9V03X_H - 1; i > 10; i--)
            {
                if (l_line_x[i] !=0 && l_line_x[i - 1] !=0)   //�����δ����
                {
                    if (inc < num)
                    {
                        if (l_line_x[i] < l_line_x[i - 1])  //�������������
                        {
                            inc++;
                        }
                    }
                    else
                    {
                        if (l_line_x[i] > l_line_x[i - 1])   //�������������
                        {
                            dec++;
                        }
                    }

                    /* �л��� */
                    if (inc >= num && dec >= num)
                    {
                        bianxian_guaidian_l = i + num;
                        return 1;
                    }
                }
                else
                {
                    inc = 0;
                    dec = 0;
                }
            }
            break;

        case 2:  //�ұ���
            for (uint8 i = MT9V03X_H -1; i > 10; i--)
            {
                if (r_line_x[i] != MT9V03X_W -1 && r_line_x[i - 1] != MT9V03X_W -1)   //�����δ����
                {
                    if (inc < num)
                    {
                        if (r_line_x[i] > r_line_x[i - 1])  //�ұ�����������
                        {
                            inc++;
                        }
                    }
                    else
                    {
                        if (r_line_x[i] < r_line_x[i - 1])   //�ұ�����������
                        {
                            dec++;
                        }
                    }
                    /* �л��� */
                    if (inc >= num && dec >= num)
                    {
                        bianxian_guaidian_r = i + num;
                        return 1;
                    }
                }
                else
                {
                    inc = 0;
                    dec = 0;
                }
            }
            break;
    }

    return 0;
}
/****************************END*****************************************/


//������V�ֵ׵�
uint8 sancha_x=0, sancha_x_zhengque=0;
uint8 sancha_y=0, sancha_y_zhengque=0;
void Sancha_didian()
{
    float k1,k2;
    sancha_x=0;
    sancha_y=0;

    if(sancha_y_zhengque<60)
    {
        k1=0.35;k2=0.65;
    }
    else
    {
        k1=0.1;k2=0.9;
    }

    for(uint8 x=k1*MT9V03X_W; x <k2*MT9V03X_W; x++)
    {
        for(uint8 y=MT9V03X_H -1; y>10; y--)
        {
            if(image_01[y][x] ==0 && image_01[y-1][x] ==0 && image_01[y-2][x] ==0)
            {
                sancha_x = x;
                sancha_y = y;
                break;
            }
        }
        if( sancha_y >sancha_y_zhengque)
        {
            sancha_y_zhengque = sancha_y;
            sancha_x_zhengque = sancha_x;
        }
    }
}
/****************************END*****************************************/

//����ǰհ��(����)
uint8 qianzhang =0;
void Qianzhang(void)   //�������ٸ�
{
    for(uint8 y =MT9V03X_H -1; y>10; y--)
    {
        if(image_01[y][MT9V03X_W/2]==0 && image_01[y-1][MT9V03X_W/2]==0 && image_01[y-2][MT9V03X_W/2]==0)
        {
            qianzhang = y;
        }
    }
}
/****************************END*****************************************/

uint8 Num=0;
//�ж�Բ������ʮ�֣�����Բ���ĵ�һ��Բ����
uint8 Cirque_or_Cross(uint8 type, uint8 startline)
{//1Ϊ��Բ����2Ϊ��Բ��
    uint8 num =0;
    if(type ==1)
    {
        for(uint8 y=startline; y<startline+10; y++)
        {
            for(uint8 x=l_line_x[y]; x>1; x--)
            {
                if(image_01[y][x] !=0)
                {
                    num ++;
                }
            }
        }
    }
    if(type ==2)
    {
        for(uint8 y=startline; y<startline+10; y++)
        {
            for(uint8 x=r_line_x[y]; x<MT9V03X_W -2; x++)
            {
                if(image_01[y][x] !=0)
                {
                    num ++;
                }
            }
        }
    }
    Num=num;
    return num;
}
/*********************END*****************************************/


//�����󻷵�
uint8 huandao_stage=1;  //�����׶α�־λ
void Handle_Left_Cirque()
{
    //�ж���Բ����λ��
    switch(huandao_stage)
    {
        case 1:
            Diuxian_weizhi_test(1, MT9V03X_H -1, MT9V03X_H -20);
            if(budiuxian_hang >=15)
            {
                Diuxian_weizhi_test(1, MT9V03X_H -20, MT9V03X_H -40);
                if(diuxian_hang >=15)
                {
                    huandao_stage = 2;  //�ڶμӰ׶Σ�1�׶Σ�������ֱ�У�
                }
            }
            break;
        case 2:
            Diuxian_weizhi_test(1,MT9V03X_H -1, MT9V03X_H -20);
            if(diuxian_hang >=15 && huandao_stage ==2)
            {
                Diuxian_weizhi_test(1, MT9V03X_H -20, MT9V03X_H -40);
                if(budiuxian_hang >=15)
                {
                    huandao_stage =3;  //�׶μӺڶΣ�2�׶Σ�������ֱ�У�
                }
            }
            break;
        case 3:
            Diuxian_weizhi_test(1, MT9V03X_H -20, MT9V03X_H -40);
            if(budiuxian_hang >=15 && huandao_stage ==3)
            {
                Diuxian_weizhi_test(1, MT9V03X_H -40, MT9V03X_H -60);
                if(diuxian_hang >=1)
                {
                    huandao_stage =4;  //�ڶμӰ׶Σ�3�׶Σ������߽�����
                }
            }
            break;
        case 4:
            annulus_s1 += Distance_Measure();
            if( annulus_s1 > 5500)   //���������ִ���һ��ֵ�����뻷�����棨���ֵ���Ը�СһЩ������·�����ã�
            {
                huandao_stage =5;     //����Բ����
                annulus_s1 =0;
            }
            break;
        case 5:
            Diuxian_weizhi_test(2,MT9V03X_H -40, MT9V03X_H -60);
            if(diuxian_hang >=10 && huandao_stage ==5)
            {
                huandao_stage =6;     //����
            }
            break;
        case 6:
            annulus_s1 += Distance_Measure();
            if(annulus_s1 >3000)//����
            {
                huandao_stage =7;
                annulus_s1 =0;
            }
            break;
        case 7:
            annulus_s1 += Distance_Measure();
            if(annulus_s1 >5000)  //����һ�ξ��벢���߻����������(���б�־λ����)
            {
                road_type.LeftCirque = 0;
                huandao_stage =1;
                annulus_s1 =0;
                l_guaidain_x=0, l_guaidain_y=0;
                r_guaidain_x=0, r_guaidain_y=0;
                BUZZER_OFF;
            }
            break;
    }

    //����
    if(huandao_stage ==1)    //����߲�����
    {
        for(uint8 y =MT9V03X_H -30; y >=MT9V03X_H -40; y--)  //ֻ��80�е�������ȣ�������
        {
            l_line_x[y] = r_line_x[y] - kuandu_saidao[MT9V03X_H-1 -y] - 5;
            r_line_x[y] = r_line_x[y] - 5;
        }
    }
    else if((huandao_stage ==2) || (huandao_stage ==3))
    {
        for(uint8 y =MT9V03X_H -30; y >=MT9V03X_H -40; y--)  //ֻ��80�е�������ȣ�������
        {
            l_line_x[y] = r_line_x[y] - kuandu_saidao[MT9V03X_H-1 -y] - 10;
            r_line_x[y] = r_line_x[y] - 10;
        }
    }
    else if(huandao_stage ==4)   //������Ȳ���
    {
        for(uint8 y =MT9V03X_H -30; y >=MT9V03X_H -40; y--)  //ֻ��80�е�������ȣ�������
        {
            r_line_x[y] = l_line_x[y] + kuandu_saidao[MT9V03X_H-1 -y];   //������һ��ƫ��������Сһ�������
//            l_line_x[y] = l_line_x[y] ;
        }
    }
    else if(huandao_stage ==5)   //�ڻ���
    {
        ;//�������
    }
    else if(huandao_stage ==6)   //������
    {
//                if(r_line_x[MT9V03X_H -1] != (MT9V03X_W-1))        //���¹յ����&&(MT9V03X_H<50)
//                {
//                    for(uint8 y=MT9V03X_H -1; y>10; y--)
//                    {
//                        if(fabs(r_line_x[y] -r_line_x[y+1])<4 && (r_line_x[y]- r_line_x[y-6] < -6))    //�������Ҫ�����ϸ�һ��
//                        {
//                            r_guaidain_y = y;   //û�������ѭ��
//                            r_guaidain_x = r_line_x[y];
//                            break;
//                        }
//                    }
//                }
//                else
//                {
        r_guaidain_y = MT9V03X_H -1;
        r_guaidain_x = MT9V03X_W-10;
//                }

        for(uint8 y =MT9V03X_H -50; y>10; y--)
        {
            if((image_01[y][2] == 0) && (image_01[y-1][2]==0) && (image_01[y-2][2]==0))
            {
                l_guaidain_y = y;
                l_guaidain_x = 3;
                break;
            }
        }

        La_zhixian(r_guaidain_x, r_guaidain_y, l_guaidain_x, l_guaidain_y, r_line_x);
    }
    else if(huandao_stage==7)
    {
        for(uint8 y =MT9V03X_H-1; y>20; y--)
        {
            l_line_x[y] = r_line_x[y] - kuandu_saidao[MT9V03X_H-1 -y] - 5;     //����·��ƫ��,Ϊ��׼ȷʶ�𻷵�,����ƫ����ʹ��������������
            r_line_x[y] = r_line_x[y] - 5;
        }
    }
}
/*****************************�󻷵��������*****************************************/

//�����һ���
void Handle_Right_Cirque()
{
    //�ж���Բ����λ��
    switch(huandao_stage)
    {
        case 1:
            Diuxian_weizhi_test(2, MT9V03X_H -1, MT9V03X_H -20);
            if(budiuxian_hang >=15)
            {
                Diuxian_weizhi_test(2, MT9V03X_H -20, MT9V03X_H -40);
                if(diuxian_hang >=15)
                {
                    huandao_stage =2;  //�ڶμӰ׶Σ�1�׶Σ�������ֱ�У�
                }
            }
            break;

        case 2:
            Diuxian_weizhi_test(2,MT9V03X_H -1, MT9V03X_H -20);
            if(diuxian_hang >=15 && huandao_stage ==2)
            {
                Diuxian_weizhi_test(2, MT9V03X_H -20, MT9V03X_H -40);
                if(budiuxian_hang >=15)
                {
                    huandao_stage =3;  //�׶μӺڶΣ�2�׶Σ�������ֱ�У�
                }
            }
            break;

        case 3:
            Diuxian_weizhi_test(2, MT9V03X_H -20, MT9V03X_H -40);
            if(budiuxian_hang >=15 && huandao_stage ==3)
            {
                Diuxian_weizhi_test(2, MT9V03X_H -40, MT9V03X_H -60);
                if(diuxian_hang >=1)
                {
                    huandao_stage =4;  //�ڶμӰ׶Σ�3�׶Σ������߽�����
                }
            }
            break;
        case 4:
            annulus_s1 += Distance_Measure();
            if( annulus_s1 >5500)   //���������ִ���һ��ֵ�����뻷�����棨���ֵ���Ը�СһЩ������·�����ã�
            {
                huandao_stage =5;     //����Բ����
                annulus_s1 =0;
            }
            break;
        case 5:
            Diuxian_weizhi_test(1,MT9V03X_H -40, MT9V03X_H -60);
            if(diuxian_hang >=10 && huandao_stage ==5)
            {
                huandao_stage =6;     //����
            }
            break;
        case 6:
            annulus_s1 += Distance_Measure();
            if(annulus_s1 >3000)//����
            {
                huandao_stage =7;
                annulus_s1 =0;
            }
            break;
        case 7:
            annulus_s1 += Distance_Measure();
            if(annulus_s1 >5000)  //����һ�ξ��벢���߻����������(���б�־λ����)
            {
                road_type.RightCirque = 0;
                huandao_stage =1;
                annulus_s1 =0;
                l_guaidain_x=0, l_guaidain_y=0;
                r_guaidain_x=0, r_guaidain_y=0;
                BUZZER_OFF;
            }
            break;
    }

    //����
    if(huandao_stage ==1)    //����߲�����
    {
        for(uint8 y =MT9V03X_H -30; y >=MT9V03X_H -40; y--)  //ֻ��80�е�������ȣ�������
        {
            r_line_x[y] = l_line_x[y] + kuandu_saidao[MT9V03X_H-1 -y] +5;
            l_line_x[y] = l_line_x[y] + 5;
        }
    }
    else if((huandao_stage ==2) || (huandao_stage ==3))
    {
        for(uint8 y =MT9V03X_H -30; y >=MT9V03X_H -40; y--)  //ֻ��80�е�������ȣ�������
        {
            r_line_x[y] = l_line_x[y] + kuandu_saidao[MT9V03X_H-1 -y] + 10;
            l_line_x[y] = l_line_x[y] + 10;
        }
    }
    else if(huandao_stage ==4)   //������Ȳ���
    {
        for(uint8 y =MT9V03X_H -30; y >=MT9V03X_H -40; y--)  //ֻ��80�е�������ȣ�������
        {
            l_line_x[y] = r_line_x[y] - kuandu_saidao[MT9V03X_H-1 -y] + 5;  //������һ��ƫ��������Сһ�������
            r_line_x[y] = r_line_x[y] + 5;
        }
    }
    else if(huandao_stage ==5)   //�ڻ���
    {
        ;//�������
    }
    else if(huandao_stage ==6)   //������
    {
//        if(l_line_x[MT9V03X_H -1] != 0)         //���¹յ����
//        {
//            for(uint8 y=MT9V03X_H -1; y>10; y--)
//            {
//                if(fabs(l_line_x[y] -l_line_x[y+1])<4 && (l_line_x[y]- l_line_x[y-6] >6))    //�������Ҫ�����ϸ�һ��
//                {
//                    l_guaidain_y = y;   //û�������ѭ��
//                    l_guaidain_x = l_line_x[y];
//                    break;
//                }
//
//            }
//        }
//        else
//        {
            l_guaidain_y = MT9V03X_H -1;
            l_guaidain_x = 10;
//        }

        for(uint8 y =MT9V03X_H -50; y>10; y--)
        {
            if((image_01[y][MT9V03X_W -2] == 0) && (image_01[y-1][MT9V03X_W -2]==0) && (image_01[y-2][MT9V03X_W -2]==0))
            {
                r_guaidain_y = y;
                r_guaidain_x = MT9V03X_W -1;
                break;
            }
        }
        La_zhixian(l_guaidain_x, l_guaidain_y, r_guaidain_x, r_guaidain_y, l_line_x);
    }
    else if(huandao_stage==7)
    {
        for(uint8 y =MT9V03X_H-1; y>20; y--)
        {
            r_line_x[y] = l_line_x[y] + kuandu_saidao[MT9V03X_H-1 -y];     //����·��ƫ��,Ϊ��׼ȷʶ�𻷵�,����ƫ����ʹ��������������
        }
    }
}
/*****************************�һ�������*****************************************/

int16 fork_s1 = 0;
int16 fork_s2 = 0;
//��������
uint8 sancha_stage = 1;//����׶α�־λ
void Handle_Fork()
{
//    fork_s2 += Distance_Measure();
//    if(fork_s2 > 10000)
//    {
//        fork_s2 = 0;
//        if(sancha_stage == 1)
//        {
//            BUZZER_OFF;
//            road_type.Fork = 0;
//        }
//    }
    //�ж�������ڵ�λ��
    switch(sancha_stage)
    {
        case 1:
            if(RoundaboutGetArc(1, 15) && RoundaboutGetArc(2, 15))
            {
                if(bianxian_guaidian_l >= 55 && bianxian_guaidian_r >= 55)
                {
                    bianxian_guaidian_l =0;
                    bianxian_guaidian_r =0;
                    sancha_stage = 2;
                }
            }
            break;
        case 2:
            Sancha_didian();
            fork_s1 += Distance_Measure();
            if(fork_s1 >= 2500)
            {
                fork_s1 = 0;
                sancha_y_zhengque =0;
                sancha_stage = 3;
            }
            break;
        case 3:
            if(RoundaboutGetArc(1, 10) && RoundaboutGetArc(2, 10))
            {
                if(bianxian_guaidian_l >= 50 && bianxian_guaidian_r >= 50)//���⳵����
                {
                    sancha_stage = 4;
                }
            }
            break;
        case 4:
            Sancha_didian();
            fork_s1 += Distance_Measure();
            if(fork_s1 >= 4000)
            {
                fork_s1 = 0;
                sancha_y_zhengque =0;
                bianxian_guaidian_l =0;
                bianxian_guaidian_r =0;
                sancha_stage = 5;
            }
            break;
    }

    //����
    if(sancha_stage == 1)
    {
        ;//�������
    }
    else if(sancha_stage == 2)
    {
        if(sancha_y_zhengque > 5)
        {
            if(sancha_y_zhengque > 80)
            {
                sancha_y_zhengque = 57;
            }
//            l_line_x[y] = r_line_x[y] - kuandu_saidao[MT9V03X_H-1 -y];
            La_zhixian(MT9V03X_W -1, MT9V03X_H -1, sancha_x_zhengque, sancha_y_zhengque, r_line_x);//������
        }
    }
    else if(sancha_stage == 3)
    {
        ;//�������
    }
    else if(sancha_stage == 4)
    {
        if(sancha_y_zhengque > 5)
        {
            if(sancha_y_zhengque > 80)
            {
                sancha_y_zhengque = 57;
            }
            La_zhixian(MT9V03X_W -1, MT9V03X_H -1, sancha_x_zhengque, sancha_y_zhengque, r_line_x);//������
        }
    }
    else if(sancha_stage == 5)
    {
        BUZZER_OFF;
        road_type.Fork = 0;
        sancha_stage = 1;
    }
}
/****************************���������****************************************/

//������бʮ��
void Handle_L_Cross()
{
    switch(xieshizi_stage)
    {
        case 1:
        {
            if(l_lose_value<20)
            {
                xieshizi_stage =2;
            }
        }
        break;
        case 2:
        {
            if(l_lose_value>40 && r_lose_value>22)  //ǰհ����Ҫ��
            {
                xieshizi_stage =3;
            }
        }
        break;
        case 3:
        {
            annulus_s1 += Distance_Measure();
            if(annulus_s1 >2000)
            {
                road_type.L_Cross =0;
                annulus_s1 =0;
                xieshizi_stage =1;
                r_guaidain_x =0;
                r_guaidain_y =0;
                l_guaidain_x =0;
                l_guaidain_y =0;
                BUZZER_OFF;
            }
        }
        break;
    }
    //����
    if(xieshizi_stage ==1 || xieshizi_stage ==3)
    {
        for(uint8 y =MT9V03X_H-1; y>20; y--)
        {
            l_line_x[y] = r_line_x[y] - kuandu_saidao[MT9V03X_H-1 -y];
        }
    }
    else if(xieshizi_stage ==2)
    {
        ;//��������
    }
    //    else if(xieshizi_stage ==3)
    //    {
    //        if(r_line_x[MT9V03X_H -1] != 0)//���¹յ���ڣ�ȡ�յ������
    //        {
    //            for(uint8 y=MT9V03X_H -1; y>30; y--)
    //            {
    //                if(fabs(r_line_x[y] -r_line_x[y+1])<4 && (r_line_x[y]- r_line_x[y-6] <-8))
    //                {
    //                    r_guaidain_x = r_line_x[y];
    //                    r_guaidain_y = y;
    //                    break;
    //                }
    //            }
    //        }
    //        else
    //        {
    //            r_guaidain_x = MT9V03X_W-5;
    //            r_guaidain_y = 60;
    //        }
    //
    //        for(uint8 y = (r_guaidain_y-10); y>10; y--)
    //        {
    //            if(image_01[y][r_guaidain_x] ==0 && image_01[y-1][r_guaidain_x]==0 && image_01[y-2][r_guaidain_x]==0)
    //            {
    //                r_guaidain_y = y;
    //                break;
    //            }
    //        }
    //        La_zhixian(1, MT9V03X_H -1, r_guaidain_x, r_guaidain_y, l_line_x);
    //    }
}
/****************************��бʮ�ִ������*****************************************/

//������бʮ��
uint8 xieshizi_stage=1;
void Handle_R_Cross()
{
    switch(xieshizi_stage)
    {
        case 1:
        {
            if(r_lose_value<20)
            {
                xieshizi_stage =2;
            }
        }
        break;
        case 2:
        {
            if(l_lose_value>22 && r_lose_value>40)  //ǰհ����Ҫ��
            {
                xieshizi_stage =3;
            }
        }
        break;
        case 3:
        {
            annulus_s1 += Distance_Measure();
            if(annulus_s1 >2000)
            {
                road_type.R_Cross =0;
                annulus_s1 =0;
                xieshizi_stage =1;
                r_guaidain_x =0;
                r_guaidain_y =0;
                l_guaidain_x =0;
                l_guaidain_y =0;
                BUZZER_OFF;
            }
        }
        break;
    }
    //����
    if(xieshizi_stage ==1 || xieshizi_stage ==3)
    {
        for(uint8 y =MT9V03X_H-1; y>20; y--)
        {
            r_line_x[y] = l_line_x[y] + kuandu_saidao[MT9V03X_H-1 -y];
        }
    }
    else if(xieshizi_stage ==2)
    {
        ;//��������
    }
    //        else if(xieshizi_stage ==3)
    //        {
    //            if(l_line_x[MT9V03X_H -1] != 0)//���¹յ���ڣ�ȡ�յ������
    //            {
    //                for(uint8 y=MT9V03X_H -1; y>30; y--)
    //                {
    //                    if(fabs(l_line_x[y] -l_line_x[y+1])<4 && (l_line_x[y]- l_line_x[y-6] > 8))
    //                    {
    //                        l_guaidain_x = l_line_x[y];
    //                        l_guaidain_y = y;
    //                        break;
    //                    }
    //                }
    //            }
    //            else
    //            {
    //                l_guaidain_x = 5;
    //                l_guaidain_y = 60;
    //            }
    //
    //            for(uint8 y = (r_guaidain_y-10); y>10; y--)
    //            {
    //                if(image_01[y][l_guaidain_x] ==0 && image_01[y-1][l_guaidain_x]==0 && image_01[y-2][l_guaidain_x]==0)
    //                {
    //                    l_guaidain_y = y;
    //                    break;
    //                }
    //            }
    //            La_zhixian(MT9V03X_W-1, MT9V03X_H -1, l_guaidain_x, l_guaidain_y, r_line_x);
    //        }

}
/****************************��бʮ�ִ������*****************************************/

int16 cross_s = 0;
//����ʮ��
void Handle_Cross(void)
{
    cross_s += Distance_Measure();
    if(cross_s > 2000)
    {
        //        BUZZER_OFF;
        cross_s = 0;
        road_type.Cross = 0;
    }

    Check_Cross_Guaidian(1);
    if(cross_left[0] !=0 && cross_left[1] !=0)     //������¹յ㶼����
    {//ʮ��һ�׶�
        La_zhixian(l_line_x[cross_left[0]], cross_left[0], l_line_x[cross_left[1]], cross_left[1], l_line_x);    //����������ĺ�������
    }
    else if(cross_left[0] ==0 && cross_left[1] !=0)     //ֻ���ѵ����ϵ㣨ʮ���У�
    {//ʮ�ֶ��׶�
        Regression(1, cross_left[1]-10, cross_left[1]);
        Hua_Xian(1, cross_left[1], MT9V03X_H-3, parameterB, parameterA);
    }

    Check_Cross_Guaidian(2);
    if(cross_right[0] !=0 && cross_right[1] !=0)     //�Ҳ����¹յ㶼����
    {//ʮ��һ�׶�
        La_zhixian(r_line_x[cross_right[0]], cross_right[0], r_line_x[cross_right[1]], cross_right[1], r_line_x);    //����������ĺ�������
    }
    else if(cross_right[0] ==0 && cross_right[1] !=0)     //ֻ���ѵ����ϵ㣨ʮ���У�
    {//ʮ�ֶ��׶�
        Regression(2, cross_right[1]-10, cross_right[1]);
        Hua_Xian(2, cross_right[1], MT9V03X_H-3, parameterB, parameterA);
    }
}

uint8 cross_left[2]  ={0, 0};  //��һ���±��ͼ���·��ĵ��yֵ
uint8 cross_right[2] ={0, 0};
void Check_Cross_Guaidian(uint8 type)  //��ʮ�ֹյ�
{
    cross_left[0] =cross_left[1] =cross_right[0] =cross_right[1] =0;

    if (type ==1)
    {
        if(image_01[MT9V03X_H -1][2] ==0)   //���¹յ����
        {
            for(uint8 y1 =MT9V03X_H -1; y1>=2; y1--)
            {
                if(fabs(l_line_x[y1] -l_line_x[y1-1])<4 && (l_line_x[y1] -l_line_x[y1-10])>10 && l_line_x[y1-10] ==0)
                {
                    cross_left[0]  =y1;
                    break;
                }
            }

            if(cross_left[0] !=0)   //���¹յ��ҵ���,���������Ϲյ�
            {
                for(uint8 y2 =cross_left[0] -1; y2>=2; y2--)
                {
                    if(fabs(l_line_x[y2] -l_line_x[y2-1])<4 && l_line_x[y2] -l_line_x[y2-5]< -10 && l_line_x[y2] ==0)
                    {
                        cross_left[1]  =y2-5 -5;
                        break;
                    }
                }
            }
        }
        else  //���¹յ㲻����
        {
            for(uint8 y =MT9V03X_H -1; y>=2; y--)
            {
                if(fabs(l_line_x[y] -l_line_x[y-1])<4 && (l_line_x[y] -l_line_x[y-10]) <-10 && l_line_x[y] ==0)
                {
                    cross_left[0]  =0;
                    cross_left[1]  =y-5 -5;
                    break;
                }
            }
        }
    }
    else if(type ==2)
    {
        if(image_01[MT9V03X_H -1][MT9V03X_W -2] ==0)   //���¹յ����
        {
            for(uint8 y1 =MT9V03X_H -1; y1>=2; y1--)
            {
                if(fabs(r_line_x[y1] -r_line_x[y1-1])<4 && (r_line_x[y1] -r_line_x[y1-10]) <-10 && r_line_x[y1-10] ==MT9V03X_W -1)
                {
                    cross_right[0]  =y1;
                    break;
                }
            }
            if(cross_right[0] !=0)   //���¹յ��ҵ���,���������Ϲյ�
            {
                for(uint8 y2 =cross_right[0] -1; y2>=2; y2--)
                {
                    if(fabs(r_line_x[y2] -r_line_x[y2-1])<4 && (r_line_x[y2] -r_line_x[y2-5]) >10 && r_line_x[y2] ==MT9V03X_W -1)
                    {
                        cross_right[1]  =y2-5 -5;
                        break;
                    }
                }
            }
        }
        else  //���¹յ㲻����
        {
            for(uint8 y =MT9V03X_H -1; y>=2; y--)
            {
                if(fabs(r_line_x[y] -r_line_x[y-1])<4 && r_line_x[y] -r_line_x[y-10] >10 && r_line_x[y] ==MT9V03X_W -1)
                {
                    cross_right[0]  =0;
                    cross_right[1]  =y-5 -5;
                    break;
                }
            }
        }
    }
}
/****************************ʮ�ִ������*****************************************/


//���⺯��
uint8 Garage_T=0;  //�����ʱ
void Handle_Barn_Out(uint8 type)
{//1��2��
    if(type ==1)
    {
        while (Garage_T <40)//ֱ��
        {
            Set_Steer(0);
            Set_Motor(3000, 3000);
            Garage_T++;
            systick_delay_ms(STM0,10);
        }
        while (Garage_T<80)//ת��
        {
            Set_Steer(STEER_LIM);
            Set_Motor(3000, 3000);
            Garage_T++;
            systick_delay_ms(STM0,10);
        }
        if(Garage_T>=80)   //�������,��־λ����
        {
            Garage_T =0;
            flag.start =1;
        }
    }

    else if(type ==2)
    {
        while (Garage_T <40)//ֱ��
        {
            Set_Steer(0);
            Set_Motor(3000, 3000);
            Garage_T++;
            systick_delay_ms(STM0,10);
        }
        while (Garage_T <80)   //ת��
        {
            Set_Steer(-STEER_LIM);
            Set_Motor(3000, 3000);
            Garage_T++;
            systick_delay_ms(STM0,10);
        }
        if(Garage_T>=80)   //�������,��־λ����
        {
            Set_Steer(0);
            Garage_T =0;
            flag.start =1;
        }
    }
}

int8 pass_barn = 1;
int16 close_check_ku_s = 0;
int16 jinku_s = 0;
//��⺯��
void Handle_Barn_in(uint8 type)
{//1�����,2�����
    if(type == 1)
    {
        if(pass_barn == 1)//��һ�ξ�����
        {
            for(uint8 y =MT9V03X_H-1; y>20; y--)
            {
                l_line_x[y] = r_line_x[y] - kuandu_saidao[MT9V03X_H-1 -y];//������
            }
            close_check_ku_s += Distance_Measure();
            if(close_check_ku_s > 4000)//����һ�ξ��룬�򿪿����־λ
            {
                pass_barn++;
                road_type.Barn_l_in = 0;
                flag.open_check_ku = 1;
                close_check_ku_s = 0;
            }
        }
        else if(pass_barn == 2)//�ڶ��ξ�����
        {
            jinku_s += Distance_Measure();
            if(jinku_s > 4000)  //����һ�ξ���ͣ��
            {
                flag.stop = 1;
                jinku_s=0;
            }
        }
    }

    else if(type == 2)
    {
        if(pass_barn == 1)//��һ�ξ�����
        {
            for(uint8 y =MT9V03X_H-1; y>20; y--)
            {
                r_line_x[y] = l_line_x[y] + kuandu_saidao[MT9V03X_H-1 -y];//������
            }
            close_check_ku_s += Distance_Measure();
            if(close_check_ku_s > 4000)//����һ�ξ��룬�򿪿����־λ
            {
                pass_barn++;
                road_type.Barn_r_in = 0;
                flag.open_check_ku = 1;
                close_check_ku_s = 0;
            }
        }
        else if(pass_barn == 2)//�ڶ��ξ�����
        {
            jinku_s += Distance_Measure();
            if(jinku_s > 4000)//����һ�ξ���ͣ��
            {
                flag.stop = 1;
                jinku_s=0;
            }
        }
    }
}


uint8 length=0;
void Mid_Col(void)
{
    int i;
    for(i=MT9V03X_H;i>1;i--)
    {
        if(image_01[i][80]==0 && image_01[i-1][80]==0 && image_01[i-2][80]==0)
        {
            break;
        }
    }
    length=MT9V03X_H-i;
    //        return length;
}

void Outside_protect(void)
{
    uint8 j=0;

    for(uint8 i = l_line_x[MT9V03X_H-1]; i < r_line_x[MT9V03X_H-1]; i++)      //�������
    {
        if(image_01[MT9V03X_H-1][i] == 0)
        {
            j++;
            if(j>120)
            {
                flag.stop = 1;
            }
        }
    }
}

int16 Sum1;
void HDPJ_lvbo(uint8 data[], uint8 N, uint8 size)
{
    Sum1 = 0;
    for(uint8 j =0; j <size; j++)
    {
        if(j <N /2)
        {
            for(uint8 k =0; k <N; k++)
            {
                Sum1 +=data[j +k];
            }
            data[j] =Sum1 /N;
        }
        else
            if(j <size -N /2)
            {
                for(uint8 k =0; k <N /2; k++)
                {
                    Sum1 +=(data[j +k] +data[j -k]);
                }
                data[j] = Sum1 /N;
            }
            else
            {
                for(uint8 k =0; k <size -j; k++)
                {
                    Sum1 +=data[j +k];
                }
                for(uint8 k =0; k <(N -size +j); k++)
                {
                    Sum1 +=data[j -k];
                }
                data[j] = Sum1 /N;
            }
        Sum1 = 0;
    }
}

uint8 sudu_yingzi=0;    //�ٶ�����
void Check_Zhidao(void)
{//����Ҫ����������ֹ��
    uint8 inc =0, dec =0;
    sudu_yingzi =0;

    for(uint8 y =MT9V03X_H-1; y >1; y--)
    {
        if((l_line_x[y] <=l_line_x[y-1]) && l_line_x[y] !=0)   //���߲����߲ż���ֱ������
        {
            inc++;
        }

        if((r_line_x[y] >=r_line_x[y-1]) && r_line_x[y] !=MT9V03X_W -1)
        {
            dec++;
        }
    }

    if(inc>=dec)
    {
        sudu_yingzi =dec;
    }
    else
    {
        sudu_yingzi =inc;
    }
}

uint8 Tututu(uint8 type)
{
    if (type ==1)
    {
        for(uint8 y1 =MT9V03X_H -1; y1>=30; y1=y1-4)
        {
            if(fabs(l_line_x[y1] -l_line_x[y1-1])<4 && (l_line_x[y1] -l_line_x[y1-10]) >8)
            {
                return 1;
            }
        }
    }
    else if (type ==2)
    {
        for(uint8 y1 =MT9V03X_H -1; y1>=30; y1=y1-4)
        {
            if(fabs(r_line_x[y1] -r_line_x[y1-1])<4 && (r_line_x[y1] -r_line_x[y1-10]) <-8)
            {
                return 1;
            }
        }
    }
    return 0;
}




#pragma section all restore

