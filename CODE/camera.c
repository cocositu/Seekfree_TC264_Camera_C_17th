#include "camera.h"
#pragma section all "cpu1_dsram"

uint8 search_line_end = 10;     //搜线终止行

int16 X1=0,X2=0,Y1=0,Y2=0;   //显示一些变量(调元素的时候用来看一些变量的值)
uint8 image_01[MT9V03X_H][MT9V03X_W];
uint8 image_yuanshi[MT9V03X_H][MT9V03X_W];


float offset_quanzhong [15] = {0.96, 0.92, 0.88, 0.83, 0.77,
                               0.71, 0.65, 0.59, 0.53, 0.47,
                               0.47, 0.47, 0.47, 0.47, 0.47,};  //偏差权重

uint8 kuandu_saidao [MT9V03X_H-1] = {
        106, 105, 104, 103, 102, 101, 100, 99, 98, 97,
        96,  95,  94,  93,  92,  91,  90,  89,  88,  87,
        86,  85,  84,  83,  82,  81,  80,  79,  78,  77,
        76,  75,  74,  73,  72,  71,  70,  69,  68,  67,
        66,  65,  64,  63,  62,  61,  60,  59,  58,  57,
        56,  55,  54,  53,  52,  51,  50,  49,  48,  47,
        46,  45,  44,  43,  42,  41,  40,  39,  38,  37,
        35,  33,  31,  29,  27,   25,  23,  21,  19,  17 }; //前80行的赛道宽度

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

//摄像头处理全流程
void Camera_Display(void)
{
    if(mt9v03x_finish_flag==1)
    {
        Transfer_Camera();
        mt9v03x_finish_flag = 0;                   //在图像使用完毕后  务必清除标志位，否则不会开始采集下一幅图像
        Get01change_Dajin();                       //二值化
        Pixle_Filter();                            //腐蚀（像素滤波）
        Search_Line(0);                             //初次搜线
        Element_Test();                            //元素识别
        Element_Handle();                          //元素处理
        Check_Zhidao();
//        HDPJ_lvbo(m_line_x, 20, MT9V03X_H -1);     //中线滤波
//        Calculate_Offset();                        //偏差计算
        Calculate_Offset_1();                        //偏差计算

        for(uint8 i=(MT9V03X_H -1); i>search_line_end; i--)
        {
            m_line_x[i] = (l_line_x[i] + r_line_x[i])/2;
        }
        if(flag.image_show)
        {            //屏幕显示
            Blacking();
        }
    }
}

/****************************END*****************************************/


//元素处理
uint16 distance_L_Cross_out=0;
uint16 distance_R_Cross_out=0;
uint8 flag_L_shizi_R_turn=0;
uint8 flag_R_shizi_L_turn=0;

int16 annulus_s1=0;
int16 jiaodu_jifen =0;
int16 annulus_jinku_s =0;
void  Element_Handle()
{
    /**************入库**********************/
    if(road_type.Barn_l_in)
    {
        Handle_Barn_in(1);
    }
    else if(road_type.Barn_r_in)
    {
        Handle_Barn_in(2);
    }
    /***************圆环*********************/
    if(road_type.LeftCirque)
    {
        Handle_Left_Cirque();
    }
    else if(road_type.RightCirque)
    {
        Handle_Right_Cirque();
    }
    /****************斜十字********************/
    if(road_type.L_Cross)
    {
        Handle_L_Cross();
    }
    else if(road_type.R_Cross)
    {
        Handle_R_Cross();
    }
    /****************三岔********************/
    if(road_type.Fork)
    {
        Handle_Fork();
    }
    /*************十字***********************/
    if(road_type.Cross)
    {
        Handle_Cross();
    }
}
/****************************END*****************************************/


//元素识别
int16 l_line_qulv=0, r_line_qulv=0;
void Element_Test(void)
{
    l_line_qulv = 1000*Process_Curvity(l_line_x[MT9V03X_H-10], MT9V03X_H-10, l_line_x[MT9V03X_H-25], MT9V03X_H-25, l_line_x[MT9V03X_H-40], MT9V03X_H-40);
    r_line_qulv = 1000*Process_Curvity(r_line_x[MT9V03X_H-10], MT9V03X_H-10, r_line_x[MT9V03X_H-25], MT9V03X_H-25, r_line_x[MT9V03X_H-40], MT9V03X_H-40);

    Mid_Col();
    Outside_protect();

    //判断左右车库
    if(flag.open_check_ku)
    {
        check_cheku(45,40,2);//车库检测
    }

    if(!road_type.Fork && !road_type.LeftCirque && !road_type.RightCirque && !road_type.L_Cross
            && !road_type.R_Cross && !road_type.Cross && !road_type.Barn_l_in && !road_type.Barn_r_in)
    {
        //判断左右车库
        if(flag_starting_line)
        {
            if(garage_direction ==1)     //左车库
            {
                road_type.Barn_l_in  =1;
                road_type.straight = 0;
                road_type.bend = 0;
                garage_direction =0;     //清楚车库方向标志位
            }
            else if(garage_direction ==2)     //右车库
            {
                road_type.Barn_r_in  =1;
                road_type.straight = 0;
                road_type.bend = 0;
                garage_direction =0;     //清楚车库方向标志位
            }
        }
        //判断左斜十字和左圆环
        else if(length > 60 && l_lose_value >30 && r_lose_value <3 && fabs(r_line_qulv) <4 && !Tututu(2))//
        {
            BUZZER_ON;
            if(Cirque_or_Cross(1,l_losemax) >= 38 && road_type.LeftCirque ==0 && road_type.L_Cross ==0 && road_type.Fork==0)  //左侧丢线位置y的下方有较多白点,判断为圆环
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
        //右斜十字和右圆环
        else if(length > 60 && r_lose_value >30 && l_lose_value <3 && fabs(l_line_qulv) <4 && !Tututu(1))//
        {
            BUZZER_ON;
            if(Cirque_or_Cross(2,r_losemax) > 40 && road_type.RightCirque ==0 && road_type.R_Cross ==0 && road_type.Fork==0)  //左侧丢线位置y的下方有较多白点,判断为圆环
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
        //检测入三岔
        else if(RoundaboutGetArc(1, 15) && RoundaboutGetArc(2, 15))
        {
//            BUZZER_ON;
            road_type.Fork =1;
            road_type.straight = 0;
            road_type.bend = 0;
        }
        //检测十字
        else if(length > 60 && r_lose_value >20 && l_lose_value > 20)
        {
            //            BUZZER_ON;
            road_type.Cross = 1;
            road_type.straight = 0;
            road_type.bend = 0;
        }
        //排除特殊元素，判断直道和弯道
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


//*******************图像数组转存函数************************************//
void Transfer_Camera()
{
    for(uint8 y=0; y<MT9V03X_H; y++)       //存储到一个新数组，后续处理（High为120，Width为188，刷新率为50）
    {
        for(uint8 x=0; x<MT9V03X_W; x++)
        {
            image_yuanshi[y][x] = mt9v03x_image[y][x];
        }
    }
}
/****************************END*****************************************/

uint8 kuan[MT9V03X_H];
//基础搜线函数
uint8 m_line_x[MT9V03X_H];                                    //储存赛道中线的列
uint8 r_line_y[MT9V03X_H], l_line_y[MT9V03X_H];               //储存左右边界的行数
uint8 r_line_x[MT9V03X_H], l_line_x[MT9V03X_H];               //储存原始图像的左右边界的列数
uint8 r_second_line_x[MT9V03X_H], l_second_line_x[MT9V03X_H]; //储存补线之后的左右边界值（还未用到）
uint8 m_second_line_x[MT9V03X_H];                             //储存赛道中线的列
uint8 r_lose_value=0, l_lose_value=0;                         //左右丢线数，后面注意要走一次清零
uint8 r_search_flag[MT9V03X_H], l_search_flag[MT9V03X_H];     //是否搜到线的标志
uint8 height, r_width, l_width;                               //循环变量名
uint8 r_losemax,l_losemax;
void Search_Line(uint8 type)     //0为普通模式，1为斑马线模式（只有搜线起始横坐标不同）
{
    uint8 l_flag=0,r_flag=0;
    uint8 l_search_start, r_search_start;                   //搜线起始列坐标
    uint8 r_searchget_flage, l_searchget_flage;             //搜到线时的标志位
    r_searchget_flage=1; l_searchget_flage=1;               //开始搜索是默认为上次搜到线
    r_lose_value=0; l_lose_value=0;                         //左右丢线数，后面注意要走一次清零


    for(height=(MT9V03X_H -1); height>search_line_end; height--)
    {
        //确定每行的搜线起始横坐标
        if (type == 0)//普通模式
        {
            if( (height>MT9V03X_H-5) || ( (l_line_x[height+1] == 0) && (r_line_x[height+1] == MT9V03X_W -1) && (height <MT9V03X_H-4) )  )   //前四行，或者左右都丢线的行
            {
                l_search_start = MT9V03X_W/2;
                r_search_start = MT9V03X_W/2;
            }
            else if((l_line_x[height+1] !=0) && (r_line_x[height+1] !=MT9V03X_W -1) && (height <MT9V03X_H-4))   //左右都不丢线
            {
                l_search_start = l_line_x[height+1]+7;
                r_search_start = r_line_x[height+1]-7;
            }
            else if((l_line_x[height+1] != 0 && r_line_x[height+1] == MT9V03X_W -1) && (height <MT9V03X_H-4))   //左不丢线,右丢线
            {
                l_search_start = l_line_x[height+1]+7;
                r_search_start = MT9V03X_W/2;
            }
            else if((l_line_x[height+1] == 0 && r_line_x[height+1] != MT9V03X_W -1) && (height <MT9V03X_H-4))   //右不丢线,左丢线
            {
                l_search_start = MT9V03X_W/2;
                r_search_start = r_line_x[height+1]-7;
            }
        }

        if( (image_01[height][MT9V03X_W/2] ==0) && (image_01[height -1][MT9V03X_W/2] ==0) && (image_01[height -2][MT9V03X_W/2] ==0) && (height <MT9V03X_H-40)) //搜线终止条件
        {
            search_line_end = height+1;
            break;
        }
        else
        {
            search_line_end = 10;
        }


        for(l_width=l_search_start; l_width>1; l_width--)      //左边搜线
        {
            if(image_01[height][l_width -2]==0 && image_01[height][l_width -1]==0 && image_01[height][l_width] !=0 && l_width>2)
            {   //黑黑白
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

        for(r_width=r_search_start; r_width<(MT9V03X_W -2); r_width++)      //右边搜线
        {
            if( image_01[height][r_width ] !=0 && image_01[height][r_width +1]==0 && image_01[height][r_width +2]==0 && r_width<MT9V03X_W-3)
            {   //白黑黑
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


//二值化
uint8 Threshold;  //阈值
uint8 Threshold_static = 160;   //阈值静态下限
uint16 Threshold_detach = 300;  //阳光算法分割阈值(光强越强,该值越大)
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

            if (image_yuanshi[y][x] >thre)         //数值越大，显示的内容越多，较浅的图像也能显示出来
                image_01[y][x] = 255;  //白
            else
                image_01[y][x] = 0;  //黑
        }
    }
}
/****************************END*****************************************/


///求二值化阈值
//-------------------------------------------------------------------------------------------------------------------
//  @param      image  图像数组
//  @param      clo    宽
//  @param      row    高
//  @param      pixel_threshold 阈值分离
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
    uint8* data = image;  //指向像素数据的指针
    for (i = 0; i < GrayScale; i++)
    {
        pixelCount[i] = 0;
        pixelPro[i] = 0;
    }

    uint32 gray_sum = 0;
    //统计灰度级中每个像素在整幅图像中的个数
    for (i = 0; i < height; i += 1)
    {
        for (j = 0; j < width; j += 1)
        {
            // if((sun_mode&&data[i*width+j]<pixel_threshold)||(!sun_mode))
            //{
            pixelCount[(
                    int)data[i * width + j]]++;  //将当前的点的像素值作为计数数组的下标
            gray_sum += (int)data[i * width + j];  //灰度值总和
            //}
        }
    }

    //计算每个像素值的点在整幅图像中的比例
    for (i = 0; i < GrayScale; i++)
    {
        pixelPro[i] = (float)pixelCount[i] / pixelSum;
    }

    //遍历灰度级[0,255]
    float w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;
    w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;
    for (j = 0; j < pixel_threshold; j++)
    {
        w0 +=
                pixelPro[j];  //背景部分每个灰度值的像素点所占比例之和 即背景部分的比例
        u0tmp += j * pixelPro[j];  //背景部分 每个灰度值的点的比例 *灰度值

        w1 = 1 - w0;
        u1tmp = gray_sum / pixelSum - u0tmp;

        u0 = u0tmp / w0;    //背景平均灰度
        u1 = u1tmp / w1;    //前景平均灰度
        u = u0tmp + u1tmp;  //全局平均灰度
        deltaTmp = w0 * pow((u0 - u), 2) + w1 * pow((u1 - u), 2);
        if (deltaTmp > deltaMax)
        {
            deltaMax = deltaTmp;
            threshold = (uint8)j;     //本来这里没有强制类型转换的,我自己加的
        }
        if (deltaTmp < deltaMax)
        {
            break;
        }
    }
    return threshold;
}
/****************************END*****************************************/


//减少运算量的二值化
uint8 my_adapt_threshold(uint8 *image, uint16 width, uint16 height)   //注意计算阈值的一定要是原图像
{
#define GrayScale 256
    int pixelCount[GrayScale];
    float pixelPro[GrayScale];
    int i, j, pixelSum = width * height/4;
    uint8 threshold = 0;
    uint8* data = image;  //指向像素数据的指针
    for (i = 0; i < GrayScale; i++)
    {
        pixelCount[i] = 0;
        pixelPro[i] = 0;
    }

    uint32 gray_sum=0;
    //统计灰度级中每个像素在整幅图像中的个数
    for (i = 0; i < height; i+=2)
    {
        for (j = 0; j < width; j+=2)
        {
            pixelCount[(int)data[i * width + j]]++;  //将当前的点的像素值作为计数数组的下标
            gray_sum+=(int)data[i * width + j];       //灰度值总和
        }
    }

    //计算每个像素值的点在整幅图像中的比例

    for (i = 0; i < GrayScale; i++)
    {
        pixelPro[i] = (float)pixelCount[i] / pixelSum;
    }

    //遍历灰度级[0,255]
    float w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;


    w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;
    for (uint8 j = 0; j < GrayScale; j++)
    {

        w0 += pixelPro[j];  //背景部分每个灰度值的像素点所占比例之和   即背景部分的比例
        u0tmp += j * pixelPro[j];  //背景部分 每个灰度值的点的比例 *灰度值

        w1=1-w0;
        u1tmp=gray_sum/pixelSum-u0tmp;

        u0 = u0tmp / w0;              //背景平均灰度
        u1 = u1tmp / w1;              //前景平均灰度
        u = u0tmp + u1tmp;            //全局平均灰度
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


//画边线和中线(液晶画点,如果屏幕显示的图像进行了缩放就不能适用)
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


//****************************偏差计算*******************************//
int16 offset;                    //摄像头处理得到的偏差
//int16 shizi_s1;
//void Calculate_Offset()
//{
//
//    uint16 total_midcourt_line =0;            //计算中线值
//    uint16 mid_value = MT9V03X_W/2 -1 ;       //屏幕中间
//
//    for(uint8 y=74; y<78; y++)                //累加控制行的中线值
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
      for(uint8 y =MT9V03X_H -1; y >=10; y--)    //利用近大远小的权重计算偏差(调试完成)
      {
          m_line_x[y] =1.0*(l_line_x[y] +r_line_x[y])/2;    //这句代码并不是没有用,千万不可以删
      }

      HDPJ_lvbo(m_line_x, 20, MT9V03X_H -1);   //平均滑动滤波

      for(uint8 y =MT9V03X_H -30; y >=MT9V03X_H -40; y--)    //利用近大远小的权重计算偏差(调试完成)
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



//三点法计算赛道曲率
float Process_Curvity(uint8 x1, uint8 y1, uint8 x2, uint8 y2, uint8 x3, uint8 y3)
{
    float K;
    int S_of_ABC = ((x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1)) / 2;
    //面积的符号表示方向
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


//自己写的开方函数
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


//识别斑马线，判断车库方向（不用区别左右斑马线）(在拾牙慧者的基础上改进了一下)
uint8 flag_starting_line =0;
uint8 garage_direction =0;
void check_cheku(uint8 start_point, uint8 end_point ,uint8 qiangdu)
{ //garage_direction=1为左车库，garage_direction=2为右车库
    uint8 times = 0;
    uint8 baise_hang = 0;                              //白色行数，判断车库方向使用
    flag_starting_line =0;                             //在03.18的基础上增加了两行
    garage_direction =0;

    for (uint8 height =start_point; height >=end_point; height--)  //判断斑马线
    {
        uint8 black_blocks_l =0, black_blocks_r =0;
        uint8 cursor_l =0, cursor_r =0;

        for (uint8 width_l =MT9V03X_W/2, width_r =MT9V03X_W/2; width_l >=1 && width_r<MT9V03X_W-2; width_l--,width_r++)
        {
            if (image_01[height][width_l] ==0 )  //!=0 不执行
            {
                if (cursor_l >20)
                {
                    break;    //当黑色元素超过栈长度的操作   //这个break有问题吧
                }
                else
                {
                    cursor_l++;
                    X1=cursor_l;
                }
            }
            else
            {
                if (cursor_l >= qiangdu && cursor_l <= qiangdu+4)     //这个范围是不是给小了(4,8) > (4,12)
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
                    break;    //当黑色元素超过栈长度的操作
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

    if(flag_starting_line ==1)         //搜到斑马线线再进行车库方向的判断
    {
        flag.open_check_ku = 0;       //搜到斑马线关闭车库检测
        for (uint8 height =start_point; height >=end_point; height--)
        {
            if(image_01[height][0] !=0)
            {
                baise_hang++;
            }
        }
        if(baise_hang >(start_point -end_point-2))       //左侧白色多，车库在左，否则在右(1左，2右)
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


//腐蚀（像素滤波,去除偶尔出现的噪点,效果有限）(这个函数有问题,因为当时二值化时用1表示的白,现在用255表示白,得改一下)
void Pixle_Filter()
{
    for (uint8 height = 10; height < MT9V03X_H-10; height++)
    {
        for (uint8 width = 10; width < MT9V03X_W -10; width = width + 1)
        {
            if ((image_01[height][width] == 0) && (image_01[height - 1][width] + image_01[height + 1][width] +image_01[height][width + 1] + image_01[height][width - 1] >=3*255))
            { //一个黑点的上下左右的白点大于等于三个，令这个点为白
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


//找车库拐点(简单粗暴版)
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


//本来就是同一边界上的两点拉线
void La_zhixian(uint8 x_down, uint8 y_down, uint8 x_up, uint8 y_up, uint8 *array)    //输入两个点的横纵坐标
{
    for(uint8 i=y_down; i >(y_up-1); i--)         //试试看能不能写>=
    {
        int16 X;
        X = (y_down -i) *(x_up -x_down) /(y_down -y_up) ;
        array[i] = array[y_down] + X;
    }
}
/****************************END*****************************************/


//最小二乘法计算斜率和截距
float parameterA=0, parameterB=0;
void Regression(uint8 type, uint8 startline, uint8 endline)
{//一左，二右，0中间
    uint8 i = 0;
    uint8 sumlines = endline - startline;
    int16 sumX = 0;
    int16 sumY = 0;
    float averageX = 0;
    float averageY = 0;
    float sumUp = 0;
    float sumDown = 0;

    if (type == 0)               //拟合中线
    {
        for (i = startline; i < endline; i++)
        {
            sumX += i;
            sumY += m_line_x[i];
        }
        if (sumlines != 0)
        {
            averageX = sumX*1.0 / sumlines;     //x的平均值
            averageY = sumY*1.0 / sumlines;     //y的平均值
        }
        for (i = startline; i < endline; i++)
        {
            sumUp += (m_line_x[i] - averageY) * (i - averageX);
            sumDown += (i - averageX) * (i - averageX);
        }
        parameterB = sumUp / sumDown;
        parameterA = averageY - parameterB * averageX;
    }
    else if (type == 1)         //拟合左线
    {
        for (i = startline; i < endline; i++)
        {
            sumX += i;
            sumY += l_line_x[i];
        }
        averageX = sumX*1.0 / sumlines;     //x的平均值
        averageY = sumY*1.0 / sumlines;     //y的平均值
        for (i = startline; i < endline; i++)
        {
            sumUp += (l_line_x[i] - averageY) * (i - averageX);
            sumDown += (i - averageX) * (i - averageX);
        }
        parameterB = sumUp / sumDown;
        parameterA = averageY - parameterB * averageX;
    }
    else if (type == 2)             //拟合右线
    {
        for (i = startline; i < endline; i++)
        {
            sumX += i;
            sumY += r_line_x[i];
        }
        averageX = sumX*1.0 / sumlines;     //x的平均值
        averageY = sumY*1.0 / sumlines;     //y的平均值
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


//最小二乘法划线
void Hua_Xian(uint8 type, uint8 startline, uint8 endline, float parameterB, float parameterA)
{//一左，二右
    if (type == 1) //左
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
    else if (type == 2)            //右
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
    else if (type == 0)             //中
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


//环岛检测
void CircleTest(uint8 type)
{//1左，2右
    if(type == 1)    //左环岛检测
    {
        uint8 y1;
        for(uint8 i=MT9V03X_H-1; i >MT9V03X_H-20; i--)
        {
            if(l_line_x[i] ==0 && l_line_x[i -1] ==0)   //跨过下方黑块
            {
                y1 =i;
                break;
            }
        }
        for(uint8 i=y1 -10; i>10; i--)   //y1 -10
        {
            if(image_01[i][0] ==0 && image_01[i -1][0] ==0 && image_01[i-2][0] ==0)   //找到上方黑点
            {
                y1 =i;
                break;
            }
        }//找到黑色圆环的起始高度坐标

        for(uint8 i=y1; y1>10; i--)
        {
            if( (l_line_x[y1-2] -l_line_x[y1] <10) &&  (l_line_x[y1-4] -l_line_x[y1] <20) &&(l_line_x[y1-6] -l_line_x[y1] <30) )  //利用圆环横坐标增加没有十字快的特征（猜想）
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

//判断丢线行位置和数目
uint8 diuxian_hang=0, budiuxian_hang=0;
void Diuxian_weizhi_test(uint8 type, uint8 startline, uint8 endline)
{//1左2右
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

//找边线拐点
uint8 l_guaidain_x1=0, l_guaidain_y1=0;//一拐点
uint8 r_guaidain_x1=0, r_guaidain_y1=0;
uint8 l_guaidain_x2=0, l_guaidain_y2=0;//二拐点
uint8 r_guaidain_x2=0, r_guaidain_y2=0;
uint8 Bianxian_guaidian_num(uint8 type, uint8 startline, uint8 endline)
{
    uint8 bianxian_guaidian_num = 0;

    //一拐点
    l_guaidain_x1 =0;
    l_guaidain_y1 =0;
    r_guaidain_x1 =0;
    r_guaidain_y1 =0;
    //二拐点
    l_guaidain_x2 =0;
    l_guaidain_y2 =0;
    r_guaidain_x2 =0;
    r_guaidain_y2 =0;

    //左边线拐点
    if(type == 1)
    {
        if(l_line_x[MT9V03X_H -1] !=0)         //左下拐点存在
        {
            //第一个拐点
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
            //第二个拐点
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

    //右边线拐点
    if(type == 2)
    {
        if(r_line_x[MT9V03X_H -1] !=MT9V03X_W -1)         //右下拐点存在
        {
            //第一个拐点
            for(uint8 y=startline; y>endline; y--)
            {
                if(fabs(r_line_x[y] -r_line_x[y+1]) <4 && (r_line_x[y]- r_line_x[y-3]< -8))    //这个条件要更加严格一点
                {
                    r_guaidain_x1 = r_line_x[y];
                    r_guaidain_y1 = y;
                    bianxian_guaidian_num++;
                    break;
                }
            }
            //第二个拐点
            if(bianxian_guaidian_num == 1)
            {
                for(uint8 y=endline; y>r_guaidain_y1-20; y++)
                {
                    if(fabs(r_line_x[y] -r_line_x[y-1]) <4 && (r_line_x[y]- r_line_x[y+3]< -8))    //这个条件要更加严格一点
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

//找边线拐点
uint8 l_guaidain_x=0, l_guaidain_y=0;
uint8 r_guaidain_x=0, r_guaidain_y=0;
void Bianxian_guaidian(uint8 type, uint8 startline, uint8 endline )
{ //每边只有一个拐点的情况
    l_guaidain_x =0;
    l_guaidain_y =0;
    r_guaidain_x =0;
    r_guaidain_y =0;

    //左边线拐点
    if(type ==1)
    {
        if(l_line_x[MT9V03X_H -1] !=0)         //左下拐点存在
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

    //右边线拐点
    if(type == 2)
    {
        if(r_line_x[MT9V03X_H -1] !=MT9V03X_W -1)         //右下拐点存在
        {
            for(uint8 y=startline; y>endline; y--)
            {
                if(fabs(r_line_x[y] -r_line_x[y+1]) <4 && (r_line_x[y]- r_line_x[y-3]< -8))    //这个条件要更加严格一点
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
{//array待处理数组，type模式，num判断的强度
    uint8 inc = 0, dec = 0;
    switch (type) //左边线
    {
        case 1:
            for (uint8 i = MT9V03X_H - 1; i > 10; i--)
            {
                if (l_line_x[i] !=0 && l_line_x[i - 1] !=0)   //最低行未丢线
                {
                    if (inc < num)
                    {
                        if (l_line_x[i] < l_line_x[i - 1])  //左边线向右延申
                        {
                            inc++;
                        }
                    }
                    else
                    {
                        if (l_line_x[i] > l_line_x[i - 1])   //左边线向左延申
                        {
                            dec++;
                        }
                    }

                    /* 有弧线 */
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

        case 2:  //右边线
            for (uint8 i = MT9V03X_H -1; i > 10; i--)
            {
                if (r_line_x[i] != MT9V03X_W -1 && r_line_x[i - 1] != MT9V03X_W -1)   //最低行未丢线
                {
                    if (inc < num)
                    {
                        if (r_line_x[i] > r_line_x[i - 1])  //右边线向左延申
                        {
                            inc++;
                        }
                    }
                    else
                    {
                        if (r_line_x[i] < r_line_x[i - 1])   //右边线向右延申
                        {
                            dec++;
                        }
                    }
                    /* 有弧线 */
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


//三岔找V字底点
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

//计算前瞻量(中线)
uint8 qianzhang =0;
void Qianzhang(void)   //不够用再改
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
//判断圆环还是十字（利用圆环的第一段圆弧）
uint8 Cirque_or_Cross(uint8 type, uint8 startline)
{//1为左圆环，2为右圆环
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


//处理左环岛
uint8 huandao_stage=1;  //环岛阶段标志位
void Handle_Left_Cirque()
{
    //判断在圆环的位置
    switch(huandao_stage)
    {
        case 1:
            Diuxian_weizhi_test(1, MT9V03X_H -1, MT9V03X_H -20);
            if(budiuxian_hang >=15)
            {
                Diuxian_weizhi_test(1, MT9V03X_H -20, MT9V03X_H -40);
                if(diuxian_hang >=15)
                {
                    huandao_stage = 2;  //黑段加白段（1阶段）（补线直行）
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
                    huandao_stage =3;  //白段加黑段（2阶段）（补线直行）
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
                    huandao_stage =4;  //黑段加白段（3阶段）（拉线进环）
                }
            }
            break;
        case 4:
            annulus_s1 += Distance_Measure();
            if( annulus_s1 > 5500)   //编码器积分大于一个值，进入环岛里面（这个值可以给小一些，车身路径不好）
            {
                huandao_stage =5;     //进入圆环内
                annulus_s1 =0;
            }
            break;
        case 5:
            Diuxian_weizhi_test(2,MT9V03X_H -40, MT9V03X_H -60);
            if(diuxian_hang >=10 && huandao_stage ==5)
            {
                huandao_stage =6;     //出环
            }
            break;
        case 6:
            annulus_s1 += Distance_Measure();
            if(annulus_s1 >3000)//积分
            {
                huandao_stage =7;
                annulus_s1 =0;
            }
            break;
        case 7:
            annulus_s1 += Distance_Measure();
            if(annulus_s1 >5000)  //积分一段距离并补线环岛处理结束(所有标志位清零)
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

    //处理
    if(huandao_stage ==1)    //半宽补线补左线
    {
        for(uint8 y =MT9V03X_H -30; y >=MT9V03X_H -40; y--)  //只有80行的赛道宽度，够用了
        {
            l_line_x[y] = r_line_x[y] - kuandu_saidao[MT9V03X_H-1 -y] - 5;
            r_line_x[y] = r_line_x[y] - 5;
        }
    }
    else if((huandao_stage ==2) || (huandao_stage ==3))
    {
        for(uint8 y =MT9V03X_H -30; y >=MT9V03X_H -40; y--)  //只有80行的赛道宽度，够用了
        {
            l_line_x[y] = r_line_x[y] - kuandu_saidao[MT9V03X_H-1 -y] - 10;
            r_line_x[y] = r_line_x[y] - 10;
        }
    }
    else if(huandao_stage ==4)   //赛道宽度补线
    {
        for(uint8 y =MT9V03X_H -30; y >=MT9V03X_H -40; y--)  //只有80行的赛道宽度，够用了
        {
            r_line_x[y] = l_line_x[y] + kuandu_saidao[MT9V03X_H-1 -y];   //随便给的一个偏移量，给小一点会内切
//            l_line_x[y] = l_line_x[y] ;
        }
    }
    else if(huandao_stage ==5)   //在环岛
    {
        ;//无需操作
    }
    else if(huandao_stage ==6)   //出环岛
    {
//                if(r_line_x[MT9V03X_H -1] != (MT9V03X_W-1))        //右下拐点存在&&(MT9V03X_H<50)
//                {
//                    for(uint8 y=MT9V03X_H -1; y>10; y--)
//                    {
//                        if(fabs(r_line_x[y] -r_line_x[y+1])<4 && (r_line_x[y]- r_line_x[y-6] < -6))    //这个条件要更加严格一点
//                        {
//                            r_guaidain_y = y;   //没进入这个循环
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
            l_line_x[y] = r_line_x[y] - kuandu_saidao[MT9V03X_H-1 -y] - 5;     //车辆路径偏右,为了准确识别环岛,采用偏移量使车身更加贴近左侧
            r_line_x[y] = r_line_x[y] - 5;
        }
    }
}
/*****************************左环岛处理结束*****************************************/

//处理右环岛
void Handle_Right_Cirque()
{
    //判断在圆环的位置
    switch(huandao_stage)
    {
        case 1:
            Diuxian_weizhi_test(2, MT9V03X_H -1, MT9V03X_H -20);
            if(budiuxian_hang >=15)
            {
                Diuxian_weizhi_test(2, MT9V03X_H -20, MT9V03X_H -40);
                if(diuxian_hang >=15)
                {
                    huandao_stage =2;  //黑段加白段（1阶段）（补线直行）
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
                    huandao_stage =3;  //白段加黑段（2阶段）（补线直行）
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
                    huandao_stage =4;  //黑段加白段（3阶段）（拉线进环）
                }
            }
            break;
        case 4:
            annulus_s1 += Distance_Measure();
            if( annulus_s1 >5500)   //编码器积分大于一个值，进入环岛里面（这个值可以给小一些，车身路径不好）
            {
                huandao_stage =5;     //进入圆环内
                annulus_s1 =0;
            }
            break;
        case 5:
            Diuxian_weizhi_test(1,MT9V03X_H -40, MT9V03X_H -60);
            if(diuxian_hang >=10 && huandao_stage ==5)
            {
                huandao_stage =6;     //出环
            }
            break;
        case 6:
            annulus_s1 += Distance_Measure();
            if(annulus_s1 >3000)//积分
            {
                huandao_stage =7;
                annulus_s1 =0;
            }
            break;
        case 7:
            annulus_s1 += Distance_Measure();
            if(annulus_s1 >5000)  //积分一段距离并补线环岛处理结束(所有标志位清零)
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

    //处理
    if(huandao_stage ==1)    //半宽补线补左线
    {
        for(uint8 y =MT9V03X_H -30; y >=MT9V03X_H -40; y--)  //只有80行的赛道宽度，够用了
        {
            r_line_x[y] = l_line_x[y] + kuandu_saidao[MT9V03X_H-1 -y] +5;
            l_line_x[y] = l_line_x[y] + 5;
        }
    }
    else if((huandao_stage ==2) || (huandao_stage ==3))
    {
        for(uint8 y =MT9V03X_H -30; y >=MT9V03X_H -40; y--)  //只有80行的赛道宽度，够用了
        {
            r_line_x[y] = l_line_x[y] + kuandu_saidao[MT9V03X_H-1 -y] + 10;
            l_line_x[y] = l_line_x[y] + 10;
        }
    }
    else if(huandao_stage ==4)   //赛道宽度补线
    {
        for(uint8 y =MT9V03X_H -30; y >=MT9V03X_H -40; y--)  //只有80行的赛道宽度，够用了
        {
            l_line_x[y] = r_line_x[y] - kuandu_saidao[MT9V03X_H-1 -y] + 5;  //随便给的一个偏移量，给小一点会内切
            r_line_x[y] = r_line_x[y] + 5;
        }
    }
    else if(huandao_stage ==5)   //在环岛
    {
        ;//无需操作
    }
    else if(huandao_stage ==6)   //出环岛
    {
//        if(l_line_x[MT9V03X_H -1] != 0)         //左下拐点存在
//        {
//            for(uint8 y=MT9V03X_H -1; y>10; y--)
//            {
//                if(fabs(l_line_x[y] -l_line_x[y+1])<4 && (l_line_x[y]- l_line_x[y-6] >6))    //这个条件要更加严格一点
//                {
//                    l_guaidain_y = y;   //没进入这个循环
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
            r_line_x[y] = l_line_x[y] + kuandu_saidao[MT9V03X_H-1 -y];     //车辆路径偏右,为了准确识别环岛,采用偏移量使车身更加贴近左侧
        }
    }
}
/*****************************右环岛处理*****************************************/

int16 fork_s1 = 0;
int16 fork_s2 = 0;
//处理三岔
uint8 sancha_stage = 1;//三岔阶段标志位
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
    //判断在三岔口的位置
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
                if(bianxian_guaidian_l >= 50 && bianxian_guaidian_r >= 50)//避免车身不稳
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

    //处理
    if(sancha_stage == 1)
    {
        ;//无需操作
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
            La_zhixian(MT9V03X_W -1, MT9V03X_H -1, sancha_x_zhengque, sancha_y_zhengque, r_line_x);//拉右线
        }
    }
    else if(sancha_stage == 3)
    {
        ;//无需操作
    }
    else if(sancha_stage == 4)
    {
        if(sancha_y_zhengque > 5)
        {
            if(sancha_y_zhengque > 80)
            {
                sancha_y_zhengque = 57;
            }
            La_zhixian(MT9V03X_W -1, MT9V03X_H -1, sancha_x_zhengque, sancha_y_zhengque, r_line_x);//拉右线
        }
    }
    else if(sancha_stage == 5)
    {
        BUZZER_OFF;
        road_type.Fork = 0;
        sancha_stage = 1;
    }
}
/****************************三岔处理结束****************************************/

//处理左斜十字
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
            if(l_lose_value>40 && r_lose_value>22)  //前瞻量需要调
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
    //处理
    if(xieshizi_stage ==1 || xieshizi_stage ==3)
    {
        for(uint8 y =MT9V03X_H-1; y>20; y--)
        {
            l_line_x[y] = r_line_x[y] - kuandu_saidao[MT9V03X_H-1 -y];
        }
    }
    else if(xieshizi_stage ==2)
    {
        ;//不做处理
    }
    //    else if(xieshizi_stage ==3)
    //    {
    //        if(r_line_x[MT9V03X_H -1] != 0)//右下拐点存在，取拐点横坐标
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
/****************************左斜十字处理结束*****************************************/

//处理右斜十字
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
            if(l_lose_value>22 && r_lose_value>40)  //前瞻量需要调
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
    //处理
    if(xieshizi_stage ==1 || xieshizi_stage ==3)
    {
        for(uint8 y =MT9V03X_H-1; y>20; y--)
        {
            r_line_x[y] = l_line_x[y] + kuandu_saidao[MT9V03X_H-1 -y];
        }
    }
    else if(xieshizi_stage ==2)
    {
        ;//不做处理
    }
    //        else if(xieshizi_stage ==3)
    //        {
    //            if(l_line_x[MT9V03X_H -1] != 0)//右下拐点存在，取拐点横坐标
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
/****************************右斜十字处理结束*****************************************/

int16 cross_s = 0;
//处理十字
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
    if(cross_left[0] !=0 && cross_left[1] !=0)     //左侧上下拐点都存在
    {//十字一阶段
        La_zhixian(l_line_x[cross_left[0]], cross_left[0], l_line_x[cross_left[1]], cross_left[1], l_line_x);    //输入两个点的横纵坐标
    }
    else if(cross_left[0] ==0 && cross_left[1] !=0)     //只能搜到左上点（十字中）
    {//十字二阶段
        Regression(1, cross_left[1]-10, cross_left[1]);
        Hua_Xian(1, cross_left[1], MT9V03X_H-3, parameterB, parameterA);
    }

    Check_Cross_Guaidian(2);
    if(cross_right[0] !=0 && cross_right[1] !=0)     //右侧上下拐点都存在
    {//十字一阶段
        La_zhixian(r_line_x[cross_right[0]], cross_right[0], r_line_x[cross_right[1]], cross_right[1], r_line_x);    //输入两个点的横纵坐标
    }
    else if(cross_right[0] ==0 && cross_right[1] !=0)     //只能搜到右上点（十字中）
    {//十字二阶段
        Regression(2, cross_right[1]-10, cross_right[1]);
        Hua_Xian(2, cross_right[1], MT9V03X_H-3, parameterB, parameterA);
    }
}

uint8 cross_left[2]  ={0, 0};  //第一个下标存图像下方的点的y值
uint8 cross_right[2] ={0, 0};
void Check_Cross_Guaidian(uint8 type)  //找十字拐点
{
    cross_left[0] =cross_left[1] =cross_right[0] =cross_right[1] =0;

    if (type ==1)
    {
        if(image_01[MT9V03X_H -1][2] ==0)   //左下拐点存在
        {
            for(uint8 y1 =MT9V03X_H -1; y1>=2; y1--)
            {
                if(fabs(l_line_x[y1] -l_line_x[y1-1])<4 && (l_line_x[y1] -l_line_x[y1-10])>10 && l_line_x[y1-10] ==0)
                {
                    cross_left[0]  =y1;
                    break;
                }
            }

            if(cross_left[0] !=0)   //左下拐点找到了,接着找左上拐点
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
        else  //左下拐点不存在
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
        if(image_01[MT9V03X_H -1][MT9V03X_W -2] ==0)   //右下拐点存在
        {
            for(uint8 y1 =MT9V03X_H -1; y1>=2; y1--)
            {
                if(fabs(r_line_x[y1] -r_line_x[y1-1])<4 && (r_line_x[y1] -r_line_x[y1-10]) <-10 && r_line_x[y1-10] ==MT9V03X_W -1)
                {
                    cross_right[0]  =y1;
                    break;
                }
            }
            if(cross_right[0] !=0)   //右下拐点找到了,接着找右上拐点
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
        else  //右下拐点不存在
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
/****************************十字处理结束*****************************************/


//出库函数
uint8 Garage_T=0;  //出库计时
void Handle_Barn_Out(uint8 type)
{//1左2右
    if(type ==1)
    {
        while (Garage_T <40)//直走
        {
            Set_Steer(0);
            Set_Motor(3000, 3000);
            Garage_T++;
            systick_delay_ms(STM0,10);
        }
        while (Garage_T<80)//转弯
        {
            Set_Steer(STEER_LIM);
            Set_Motor(3000, 3000);
            Garage_T++;
            systick_delay_ms(STM0,10);
        }
        if(Garage_T>=80)   //出库结束,标志位清零
        {
            Garage_T =0;
            flag.start =1;
        }
    }

    else if(type ==2)
    {
        while (Garage_T <40)//直走
        {
            Set_Steer(0);
            Set_Motor(3000, 3000);
            Garage_T++;
            systick_delay_ms(STM0,10);
        }
        while (Garage_T <80)   //转弯
        {
            Set_Steer(-STEER_LIM);
            Set_Motor(3000, 3000);
            Garage_T++;
            systick_delay_ms(STM0,10);
        }
        if(Garage_T>=80)   //出库结束,标志位清零
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
//入库函数
void Handle_Barn_in(uint8 type)
{//1左入库,2右入库
    if(type == 1)
    {
        if(pass_barn == 1)//第一次经过库
        {
            for(uint8 y =MT9V03X_H-1; y>20; y--)
            {
                l_line_x[y] = r_line_x[y] - kuandu_saidao[MT9V03X_H-1 -y];//补左线
            }
            close_check_ku_s += Distance_Measure();
            if(close_check_ku_s > 4000)//积分一段距离，打开库检测标志位
            {
                pass_barn++;
                road_type.Barn_l_in = 0;
                flag.open_check_ku = 1;
                close_check_ku_s = 0;
            }
        }
        else if(pass_barn == 2)//第二次经过库
        {
            jinku_s += Distance_Measure();
            if(jinku_s > 4000)  //积分一段距离停车
            {
                flag.stop = 1;
                jinku_s=0;
            }
        }
    }

    else if(type == 2)
    {
        if(pass_barn == 1)//第一次经过库
        {
            for(uint8 y =MT9V03X_H-1; y>20; y--)
            {
                r_line_x[y] = l_line_x[y] + kuandu_saidao[MT9V03X_H-1 -y];//补右线
            }
            close_check_ku_s += Distance_Measure();
            if(close_check_ku_s > 4000)//积分一段距离，打开库检测标志位
            {
                pass_barn++;
                road_type.Barn_r_in = 0;
                flag.open_check_ku = 1;
                close_check_ku_s = 0;
            }
        }
        else if(pass_barn == 2)//第二次经过库
        {
            jinku_s += Distance_Measure();
            if(jinku_s > 4000)//积分一段距离停车
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

    for(uint8 i = l_line_x[MT9V03X_H-1]; i < r_line_x[MT9V03X_H-1]; i++)      //左边搜线
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

uint8 sudu_yingzi=0;    //速度因子
void Check_Zhidao(void)
{//这里要考虑搜线终止行
    uint8 inc =0, dec =0;
    sudu_yingzi =0;

    for(uint8 y =MT9V03X_H-1; y >1; y--)
    {
        if((l_line_x[y] <=l_line_x[y-1]) && l_line_x[y] !=0)   //两边不丢线才计算直道长度
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

