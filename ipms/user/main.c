#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "led.h"
#include "key.h"
#include "usart.h"
#include "delay.h"
#include "lcd.h"
#include "ov7670.h"
#include "string.h"
#include "serial_image.h"
#include "FSR.h"

//#define  WIFI

// 停车流程状态机枚举
typedef enum {
    STATE_IDLE = 0,              // 空闲状态，等待红外感应
    STATE_VEHICLE_DETECTED,      // 红外检测到车辆
    STATE_CAMERA_PREVIEW,        // 摄像头预览
    STATE_COUNTDOWN_CAPTURE,     // 5秒倒计时
    STATE_CAPTURING,             // 拍照上传
    STATE_WAITING_DELAY,         // 发送完毕后等待1秒
    STATE_WAITING_RESULT,        // 等待识别结果
    STATE_DISPLAY_RESULT,        // 显示车牌结果
    STATE_PAYMENT_WAITING,       // 等待支付
    STATE_GATE_OPEN,             // 打开闸门
    STATE_GATE_WAIT,             // 等待车辆通过
    STATE_MONITOR_PARKING,       // 监控车位状态
    STATE_CAR_PARKED,            // 车辆停稳
    STATE_CAR_LEAVING,           // 车辆离开
    STATE_RETURN_IDLE            // 返回空闲
} ParkingState;

// 全局状态变量
volatile ParkingState current_state = STATE_IDLE;
volatile ParkingState previous_state = STATE_IDLE;
volatile vu8 gate_state = 0;              // 闸门状态: 0-关闭, 1-开启
volatile vu8 parking_occupied = 0;        // 车位状态: 0-空闲, 1-有车
volatile vu8 capture_ready = 0;           // 拍照就绪标志
volatile u32 state_timer = 0;             // 状态计时器(毫秒)
volatile u32 countdown_seconds = 0;       // 倒计时秒数
volatile u8 last_display_countdown = 255;  // 上次显示的倒计时秒数(初始值255确保首次显示)
volatile u8 key2_sent = 0;                // KEY2发送标志
volatile u8 display_plate_info = 0;       // 显示车牌信息标志
volatile u32 display_countdown = 0;       // 车牌信息显示倒计时
volatile u8 parking_spots_redraw = 0;      // 车位显示强制重绘标志
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
vu8 cur_status=0;
vu8 LED_flag=0;
//vu32 a=0;
//vu32 b=0;
vu16 AA=0,BB=0;
vu16 color=0;
vu16 color_save=0;//保存上一个点的值
vu8 R=0,G=0,B=0;//颜色变量
vu8 TableChangePoint_240[240];//定义点240个
vu8 Max_ChangePoint_240=0,Min_ChangePoint_240=0,Max_bChangePoint=0,Min_bChangePoint=0;//最大最小初始末点,最小初始末点
vu8 a_Continue=0,b_Continue=0;//记录凹凸点重叠计数器
vu8 flag_aMax=0;//末值比标志
vu8 Max_aChangePoint_reset=0,Min_aChangePoint_reset=0;//最大最小比较次数
vu16 Length_card=0,Width_card=0;//车的长和宽
vu8 Max_aChangePoint_reset_1=0,Min_aChangePoint_reset_1=0;//最近一次的值
vu8 flag_MaxMinCompare=0;//Max_aChangePoint_reset_1和Max_aChangePoint_reset的标志
vu8 TableChangePoint_320[320];//定义点320个
float V=0.00,S=0.00,H=0.00;//定义HSV值
vu16 Min_blue=0;
vu16 Max_blue=0;//定义蓝色区间颜色的最大最小值参数
vu16 k1=0,kk1=0,k2=0,kk2=0,k3=0,kk3=0,k4=0,kk4=0,k5=0,kk5=0,k6=0,kk6=0,k7=0,kk7=0,k8=0,kk8=0;//各拐点线段
extern vu8 Table[6300];//定义字符串 框10+26个*150 = 5400 字节
extern vu8 talble_0[150];//字符3,省简称
extern vu8 table_yu[32];//渝
extern vu8 table_min[32];//民
extern vu8 table_lu[32];//鲁
extern vu8 table_zhe[32];//浙
extern vu8 table_shan[32];//陕
extern vu8 table_cuan[32];//川
vu8 R_a=0,G_a=0,B_a=0;//均值

vu8 table_picture[150];//定义一张图片数据
vu8 table_char[36]={0,1,2,3,4,5,6,7,8,9,'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',};
vu8 table_char_char[36]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',};
vu8 table_card[5][8]={	//定义5个车牌的二维数组
{0,0,0,0,0,0,0,0},		//最后一列用于时间戳
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
};
vu8 tim3_num=0;//TIM3中断计数

vu8 table_cardMeasure[7];//保存车的长度
void Show_Card(vu8 i);//显示第几个车牌
void Show_Title();//显示标题

// 接收后端数据的相关变量
volatile u8 receive_index = 0;
volatile u8 receive_complete = 0;
u8 receive_buffer[128];

// 车牌信息结构
typedef struct {
    char plate[20];
    char color[10];
    char in_time[25];
    char out_time[25];
    char status[10];
    u32 fee;  // 停车费用（元）
} PlateInfo;

PlateInfo current_plate;

// 车位信息定义
#define PARKING_SPOTS_COUNT  6
typedef struct {
    u8 occupied;           // 是否被占用
    u32 occupied_time;      // 占用持续时间(毫秒)
    char plate[20];        // 车牌号
} ParkingSpot;

ParkingSpot parking_spots[PARKING_SPOTS_COUNT];

// 省份拼音首字母到汉字的映射表
// 格式: 拼音首字母, 对应汉字(2字节UTF-8), 显示索引
// 需要从字库数组中查找对应的字模数据

// 车牌省份简称字库 (31个省简称, 每个16x16点阵, 32字节)
// 索引: 0=京, 1=津, 2=沪, 3=渝, 4=冀, 5=豫, 6=云, 7=辽, 8=黑, 9=湘,
//       10=皖, 11=鲁, 12=新, 13=苏, 14=浙, 15=赣, 16=鄂, 17=粤, 18=桂, 19=琼,
//       20=川, 21=贵, 22=藏, 23=陕, 24=甘, 25=青, 26=宁, 27=蒙, 28=晋, 29=警, 30=学
const u8 province_font[31][32] = {
    // 京 (0) - 需要添加实际点阵数据
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 津 (1)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 沪 (2)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 渝 (3)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 冀 (4)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 豫 (5)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 云 (6)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 辽 (7)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 黑 (8)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 湘 (9)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 皖 (10)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 鲁 (11)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 新 (12)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 苏 (13)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 浙 (14)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 赣 (15)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 鄂 (16)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 粤 (17)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 桂 (18)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 琼 (19)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 川 (20)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 贵 (21)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 藏 (22)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 陕 (23)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 甘 (24)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 青 (25)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 宁 (26)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 蒙 (27)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 晋 (28)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 警 (29)
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
    // 学 (30) - 教练车牌
    {0x00,0x00,0x20,0x04,0x10,0x08,0x10,0x08,0x11,0xFC,0x11,0x04,0x11,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x40,0x02,0x00,0x00},
};

// 拼音首字母查找表
typedef struct {
    const char* pinyin;
    u8 index;
} PinyinMap;

const PinyinMap pinyin_table[] = {
    {"J", 0},   // 京
    {"T", 1},   // 津
    {"H", 2},   // 沪
    {"Y", 3},   // 渝
    {"HB", 4},  // 冀
    {"YN", 6},  // 云
    {"LN", 7},  // 辽
    {"HLJ", 8}, // 黑
    {"HN", 9},  // 湘
    {"HN", 19}, // 琼
    {"AH", 10}, // 皖
    {"SD", 11}, // 鲁
    {"XJ", 12}, // 新
    {"JS", 13}, // 苏
    {"ZJ", 14}, // 浙
    {"JX", 15}, // 赣
    {"GD", 17}, // 粤
    {"GX", 18}, // 桂
    {"SC", 20}, // 川
    {"GZ", 21}, // 贵
    {"XZ", 22}, // 藏
    {"SN", 23}, // 陕
    {"GS", 24}, // 甘
    {"QH", 25}, // 青
    {"NX", 26}, // 宁
    {"NM", 27}, // 蒙
    {"SX", 28}, // 晋
    {"JJ", 29}, // 警
    {"X", 30},  // 学
};

// LCD绘制矩形函数声明
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2, u16 color);
u8 GetIndexFromPinyin(const char* pinyin);

// 函数前置声明
void ParsePlateInfo(u8* data);
void DisplayPlateInfo(PlateInfo* info);
void DisplayParkingSpots(void);
void ScanParkingSpots(void);
void LCD_ShowString_Fixed(u16 x, u16 y, const char* str);
char* FindString(char* source, char* target);
u8 GetStringLen(char* str);
u32 ParseFee(char* str);
void USART1_IRQHandler(void);

void MYRCC_DeInit(void)//复位所有时钟
{
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC->APB1RSTR = 0x00000000;//复位所有
	RCC->APB2RSTR = 0x00000000;

	RCC->AHBENR = 0x00000014;  //睡眠模式寄存器SRAM时钟使能.其他关闭.
	RCC->APB2ENR = 0x00000000; //关闭所有时钟
	RCC->APB1ENR = 0x00000000;
	RCC->CR |= 0x00000001;     //使能内部时钟HSION
	RCC->CFGR &= 0xF8FF0000;   //复位SW[1:0],HPRE[3:0],PPRE1[2:0],PPRE2[2:0],ADCPRE[1:0],MCO[2:0]
	RCC->CR &= 0xFEF6FFFF;     //复位HSEON,CSSON,PLLON
	RCC->CR &= 0xFFFBFFFF;     //复位HSEBYP
	RCC->CFGR &= 0xFF80FFFF;   //复位PLLSRC, PLLXTPRE, PLLMUL[3:0] and USBPRE
	RCC->CIR = 0x00000000;     //关闭所有中断
	/* Enable the TIM3 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //副优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化NVIC寄存器
}
void Stm32_Clock_Init(vu8 PLL)//系统时钟初始化函数  pll:选定的倍频因子2~16
{
	unsigned char temp=0;
	MYRCC_DeInit();		  //复位所有时钟
	RCC->CR|=0x00010000;  //外部时钟使能HSEON
	while(!(RCC->CR>>17));//等待外部时钟就绪
	RCC->CFGR=0X00000400; //APB1=DIV2;APB2=DIV1;AHB=DIV1;
	PLL-=2;//留出2个位
	RCC->CFGR|=PLL<<18;   //设置PLL值 2~16
	RCC->CFGR|=1<<16;	  //PLLSRC ON
	FLASH->ACR|=0x32;	  //FLASH 2个延时周期

	RCC->CR|=0x01000000;  //PLLON
	while(!(RCC->CR>>25));//等待PLL锁定
	RCC->CFGR|=0x00000002;//PLL作为系统时钟
	while(temp!=0x02)     //等待PLL作为系统时钟设置成功
	{
		temp=RCC->CFGR>>2;
		temp&=0x03;
	}
}

void LED()//LED指示灯-闪烁
{
	GPIO_WriteBit(LED1_GPIO_PORT, LED1_GPIO_PIN,LED_flag>>7);
	LED_flag=~LED_flag;
}

void Data_LCD_Display()//数据显示
{
	vu16 a=0,b=0;
	u32 timeout = 0;
	LCD_SetWindows(0,0,320,240);//设置显示窗口
	GPIO_WriteBit(LCD_RS_PORT, LCD_RS_PIN,1);//标志寄存器写数据

	// 等待VSYNC变低（带超时保护）
	timeout = 0;
	while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0)==1) {
		if(++timeout > 0xFFFF) return;  // 超时退出
	}

	GPIO_WriteBit(FIFO_WRST_PORT, FIFO_WRST_PIN, 0);    //FIFO写指针复位
	GPIO_WriteBit(FIFO_WRST_PORT, FIFO_WRST_PIN, 1);    //
	GPIO_WriteBit(FIFO_WR_PORT, FIFO_WR_PIN, 1);        //FIFO写使能

	// 等待第二帧同步信号（带超时保护）
	timeout = 0;
	while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0)==0) {
		if(++timeout > 0xFFFF) return;  // 超时退出
	}
	timeout = 0;
	while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0)==1) {
		if(++timeout > 0xFFFF) return;  // 超时退出
	}
	GPIO_WriteBit(FIFO_WR_PORT, FIFO_WR_PIN, 0); //FIFO写入停止

	FIFO_Reset_Read_Addr();

	for (a=0;a<240;a++)
	{
		for(b=0;b<320;b++)
		{
			GPIOC->BRR =1<<4;  //PC5 - FIFO_WRST写指针
			AA=GPIOA->IDR;	   //读取上一行高字节
			GPIOC->BSRR =1<<4; //PC5 - FIFO_WRST写使能

			GPIOC->BRR =1<<4;  //PC5 - FIFO_WRST写指针
			BB=GPIOA->IDR&0x00ff;	//读取下一行低字节
			GPIOC->BSRR =1<<4;

			color=(AA<<8)|BB;  //得到该点颜色

			LCD_DATA_PORT->ODR = color;  //LCD写入该点数据

			GPIOC->BRR =1<<11;  //LCD写数据控制//PC12 - TFT-RS 寄存器/数据标志：0:写寄存器   1:写数据
			GPIOC->BSRR =1<<11; //LCD写数据控制
		}
	}
}


//extern u8 ov_sta;   //在ov7670.c中定义，当ov7670摄像头采集完一帧图片时ov_sta=1

void send_pic_using_USART() // 使用串口发送图片
{
	vu16 a=0,b=0;
	u32 timeout = 0;
	LCD_SetWindows(0,0,320,240);//设置显示窗口
	GPIO_WriteBit(LCD_RS_PORT, LCD_RS_PIN,1);//标志寄存器写数据

	// 等待VSYNC变低（带超时保护）
	timeout = 0;
	while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0)==1) {
		if(++timeout > 0xFFFF) return;  // 超时退出
	}

	GPIO_WriteBit(FIFO_WRST_PORT, FIFO_WRST_PIN, 0);    //FIFO写指针复位
	GPIO_WriteBit(FIFO_WRST_PORT, FIFO_WRST_PIN, 1);    //
	GPIO_WriteBit(FIFO_WR_PORT, FIFO_WR_PIN, 1);        //FIFO写使能

	// 等待第二帧同步信号（带超时保护）
	timeout = 0;
	while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0)==0) {
		if(++timeout > 0xFFFF) return;  // 超时退出
	}
	timeout = 0;
	while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0)==1) {
		if(++timeout > 0xFFFF) return;  // 超时退出
	}
	GPIO_WriteBit(FIFO_WR_PORT, FIFO_WR_PIN, 0); //FIFO写入停止

	FIFO_Reset_Read_Addr();

	u8 pixel1, pixel2;
	u32 j=0;

	// 发送帧头
	USART1_Send_Byte(0x01);
	USART1_Send_Byte(0xfe);
	for (a=0;a<240;a++)
	{
		for(b=0;b<320;b++)
		{
			GPIOC->BRR =1<<4;  //PC5 - FIFO_WRST写指针
			AA=GPIOA->IDR;	   //读取上一行高字节
			GPIOC->BSRR =1<<4; //PC5 - FIFO_WRST写使能

			GPIOC->BRR =1<<4;  //PC5 - FIFO_WRST写指针
			BB=GPIOA->IDR&0x00ff;	//读取下一行低字节
			GPIOC->BSRR =1<<4;

			// 发送数据
			USART1_Send_Byte(AA);
			USART1_Send_Byte(BB);
		}
	}
	// 发送帧尾
	USART1_Send_Byte(0xfe);
	USART1_Send_Byte(0x01);
}





void RGB_HSV(vu16 num)//RGB565转HSV
{

	float max=0.00,min=0.00;
	vu8 r=0,g=0,b=0;
	r=(num>>11)*255/31;g=((num>>5)&0x3f)*255/63;b=(num&0x1f)*255/31;

	max=r;min=r;
	if(g>=max)max=g;
	if(b>=max)max=b;
	if(g<=min)min=g;
	if(b<=min)min=b;

	V=100*max/255;//转换为百分比
	S=100*(max-min)/max;//放大100倍显示
	if(max==r) H=(g-b)/(max-min)*60;
	if(max==g) H=120+(b-r)/(max-min)*60;
	if(max==b) H=240+(r-g)/(max-min)*60;
	if(H<0) H=H+360;
}


//定时器3中断服务函数
void TIM3_IRQHandler(void)
{
	if(TIM3->SR&0X0001)//溢出中断
	{
		LED();

	}

	TIM3->SR&=~(1<<0);//清除中断标志位
}
void TIM3_Configuration(void)
	{
	/* TIM3 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	/* ---------------------------------------------------------------
	TIM3CLK = PCLK1=36MHz
	TIM3CLK = 36 MHz, Prescaler = 7200, TIM3 counter clock = 5K,计数器频率为5K,计数周期为10K
	--------------------------------------------------------------- */
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 2000; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数器上限值	 默认值为10000为1000ms
	TIM_TimeBaseStructure.TIM_Prescaler =(12800-1); //设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

	/* Enables the Update event for TIM3 */
	//TIM_UpdateDisableConfig(TIM3,ENABLE); 	//使能 TIM3 更新事件

	/* TIM IT enable */
		TIM_ITConfig(  //使能或者失能指定的TIM中断
		TIM3, //TIM2
		TIM_IT_Update  |  //TIM 中断源
		TIM_IT_Trigger,   //TIM 触发中断源
		ENABLE  //使能
		);

	/* TIM3 enable counter */
	TIM_Cmd(TIM3, ENABLE);  //使能TIMx计数器
}

// LCD绘制矩形函数实现
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2, u16 color)
{
	u16 i;
	for(i = x1; i <= x2; i++) {
		LCD_DrawPoint(i, y1, color);
		LCD_DrawPoint(i, y2, color);
	}
	for(i = y1; i <= y2; i++) {
		LCD_DrawPoint(x1, i, color);
		LCD_DrawPoint(x2, i, color);
	}
}

// 状态机处理函数
void Process_State_Machine(void)
{
    u8 key;
    
    switch(current_state) {
        
        // ========== 空闲状态，显示车位 ==========
        case STATE_IDLE:
            ScanParkingSpots();  // 扫描车位状态
            DisplayParkingSpots();

            if(HW_Scan(0) == KEY_PRESSED) {
                current_state = STATE_CAMERA_PREVIEW;
                countdown_seconds = 0;          // 重置倒计时
                last_display_countdown = 255;    // 重置显示标志
                LCD_Fill(0x6666);
                delayms(500);
            }
            break;
        
        // ========== 摄像头预览 + 3秒倒计时 ==========
        case STATE_CAMERA_PREVIEW:
            // 只在第一次进入时捕获图像，避免反复重置FIFO导致卡住
            if(last_display_countdown == 255) {
                Data_LCD_Display();  // 捕获一帧图像显示
            }

            // 使用叠加模式显示倒计时，不闪烁
            if(last_display_countdown != countdown_seconds) {
                last_display_countdown = countdown_seconds;
                u8 display_num = 3 - countdown_seconds;  // 显示 3,2,1
                // 在屏幕中心位置显示大号倒计时数字，使用叠加模式(1)
                LCD_ShowChar(144, 100, display_num + '0', 1);  // 叠加模式显示
            }
            
            state_timer += 50;
            if(state_timer >= 1000) {
                state_timer = 0;
                countdown_seconds++;
                
                if(countdown_seconds >= 3) {
                    current_state = STATE_CAPTURING;
                    countdown_seconds = 0;
                    last_display_countdown = 255;  // 重置，为下次显示做准备
                }
            }
            break;
        
        // ========== 拍照并上传 ==========
        case STATE_CAPTURING:
            GPIO_WriteBit(FIFO_WR_PORT, FIFO_WR_PIN, 0);
            delayms(100);
            send_pic_using_USART();  // 发送图像数据
            LCD_Fill(0x0000);  // 黑屏
            state_timer = 0;
            current_state = STATE_WAITING_DELAY;  // 等待1秒后再接收
            break;

        // ========== 发送完毕后等待1秒 ==========
        case STATE_WAITING_DELAY:
            LCD_ShowString_Fixed(80, 120, "Waiting...");  // 调试信息
            state_timer += 50;
            if(state_timer >= 20) {  // 20 * 50ms = 1000ms = 1秒
                state_timer = 0;
                current_state = STATE_WAITING_RESULT;
            }
            break;

        // ========== 等待车牌识别结果 ==========
        case STATE_WAITING_RESULT:
            LCD_ShowString_Fixed(60, 100, "State:WAITING_RESULT");
            LCD_ShowNum(60, 130, receive_complete, 3);  // 显示receive_complete值

            if(receive_complete) {
                receive_complete = 0;
                ParsePlateInfo(receive_buffer);
                LCD_Fill(0x0000);  // 黑屏
                DisplayPlateInfo(&current_plate);  // 显示车牌信息
                current_state = STATE_DISPLAY_RESULT;
            }

            state_timer += 50;
            if(state_timer >= 600) {  // 600 * 50ms = 30秒超时
                state_timer = 0;
                LCD_Fill(0x6666);
                current_state = STATE_CAMERA_PREVIEW;
                countdown_seconds = 0;
                last_display_countdown = 255;
            }
            break;
        
        // ========== 显示车牌识别结果 ==========
        case STATE_DISPLAY_RESULT:
            state_timer += 50;
            if(state_timer >= 100) {  // 100 * 50ms = 5秒后进入支付页面
                state_timer = 0;
                LCD_Fill(0x0000);
                LCD_ShowString_Fixed(50, 80, "Payment Required");
                LCD_ShowString_Fixed(30, 120, "Scan QR to Pay");
                current_state = STATE_PAYMENT_WAITING;
            }
            break;

        // ========== 等待支付 ==========
        case STATE_PAYMENT_WAITING:
            LCD_ShowString_Fixed(50, 160, "Fee: ");
            LCD_ShowNum(100, 160, current_plate.fee, 3);  // 显示费用
            LCD_ShowChar(130, 160, 'Y', 0);
            LCD_ShowChar(142, 160, 'u', 0);
            LCD_ShowChar(154, 160, 'a', 0);
            LCD_ShowChar(166, 160, 'n', 0);

            // 检查是否收到支付成功消息
            if(receive_complete) {
                receive_complete = 0;
                if(FindString((char*)receive_buffer, "PAY_SUCCESS")) {
                    LCD_Fill(0x0000);
                    LCD_ShowString_Fixed(60, 100, "Payment Success!");
                    LCD_ShowString_Fixed(40, 140, "Gate Opening...");
                    delayms(1500);
                    current_state = STATE_GATE_OPEN;
                }
            }

            // 60秒超时，返回空闲
            state_timer += 50;
            if(state_timer >= 1200) {  // 60秒超时
                state_timer = 0;
                LCD_Fill(0x6666);
                current_state = STATE_IDLE;
            }
            break;

        // ========== 打开闸门 ==========
        case STATE_GATE_OPEN:
            gate_state = 1;
            servo_ctrl(1);
            // 立即显示车位信息（parking_spots_redraw 触发首次重绘）
            ScanParkingSpots();  // 扫描车位状态
            DisplayParkingSpots();  // 显示车位状态
            state_timer += 50;
            if(state_timer >= 200) {  // 200 * 50ms = 10秒后进入下一状态
                state_timer = 0;
                current_state = STATE_GATE_WAIT;
            }
            break;

        // ========== 等待车辆通过 ==========
        case STATE_GATE_WAIT:
            ScanParkingSpots();  // 扫描车位状态
            DisplayParkingSpots();  // 显示车位状态
            state_timer += 50;
            if(state_timer >= 5000) {  // 5秒后关闭闸门
                gate_state = 0;
                servo_ctrl(0);
                current_state = STATE_MONITOR_PARKING;
            }
            break;
        
        // ========== 监控车位状态 ==========
        case STATE_MONITOR_PARKING:
            ScanParkingSpots();  // 扫描车位状态
            DisplayParkingSpots();  // 显示车位状态
            if(FSR_Scan(1) == KEY_PRESSED) {
                current_state = STATE_CAR_PARKED;
            }
            break;
        
        // ========== 车辆停稳 ==========
        case STATE_CAR_PARKED:
            parking_occupied = 1;
            DisplayParkingSpots();  // 显示车位状态
            current_state = STATE_CAR_LEAVING;
            state_timer = 0;
            break;

        // ========== 车辆离开 ==========
        case STATE_CAR_LEAVING:
            DisplayParkingSpots();  // 显示车位状态
            if(FSR_Scan(1) == KEY_RELEASED) {
                parking_occupied = 0;
                current_state = STATE_IDLE;
            }

            state_timer += 50;
            if(state_timer >= 60000) {
                current_state = STATE_IDLE;
            }
            break;
    }
}

int main(void)
{
    unsigned int num = 0;
    unsigned int i;
    unsigned int j;
    
    Stm32_Clock_Init(16);  // 初始化时钟
    Led_init();             // 初始化 LED
    Lcd_Gpio_Init();
    LCD_Init();
    Key_init();             // 初始化 KEY1 PA8
    OV7670_Gpio_Init();     // OV7670口初始化
    GPIO_WriteBit(FIFO_OE_PORT, FIFO_OE_PIN, 0);
    USART1_init();          // 初始化串口
    
    // 初始化红外传感器、舵机和FSR压力传感器
    HW_GPIO_Init();     // 初始化红外传感器GPIO (PA15)
    SERVO_Init();       // 初始化舵机GPIO (PA11)
    FSR_IO_Init();      // 初始化FSR压力传感器GPIO (PC14)
    
    TIM3_Configuration();  // 10Khz的计数频率
    LCD_Fill(0x6666);
    while(!Sensor_init());
    
    // 初始化状态
    current_state = STATE_IDLE;
    gate_state = 0;
    parking_occupied = 0;
    servo_ctrl(0);

    // 初始化车位状态
    for(i = 0; i < PARKING_SPOTS_COUNT; i++) {
        parking_spots[i].occupied = 0;
        parking_spots[i].occupied_time = 0;
        for(j = 0; j < 20; j++) {
            parking_spots[i].plate[j] = '\0';
        }
    }
    
    // 启动界面简化
    LCD_Fill(0x0000);
    LCD_ShowString(20, 100, "IPMS Ready");
    delayms(1000);
    LCD_Fill(0x6666);
    
    while(1) {
        // 处理状态机
        Process_State_Machine();
        
        // 检查按键（用于舵机手动控制）
        u8 key = KEY_Scan(0);
        if(key == KEY1_PRES) {
            gate_state = 1;
            servo_ctrl(1);
        }
        if(key == KEY2_PRES) {
            gate_state = 0;
            servo_ctrl(0);
        }

        // 处理接收到的后端数据
        if(receive_complete) {
            receive_complete = 0;
            // 检查是否是舵机命令
            if(FindString((char*)receive_buffer, "GATE:") != 0) {
                if(FindString((char*)receive_buffer, "GATE:OPEN") != 0) {
                    LCD_ShowString_Fixed(60, 180, "Gate: OPEN ");
                    servo_ctrl(1);  // 抬杆
                } else if(FindString((char*)receive_buffer, "GATE:CLOSE") != 0) {
                    LCD_ShowString_Fixed(60, 180, "Gate: CLOSE");
                    servo_ctrl(0);  // 落杆
                }
            } else {
                ParsePlateInfo(receive_buffer);
            }
        }
    }
}

// 获取字符串长度
u8 GetStringLen(char* str)
{
	u8 len = 0;
	while(str[len] != '\0') len++;
	return len;
}

// 查找子串位置
char* FindString(char* source, char* target)
{
	u8 i, j;
	for(i = 0; source[i] != '\0'; i++) {
		u8 match = 1;
		for(j = 0; target[j] != '\0'; j++) {
			if(source[i + j] != target[j]) {
				match = 0;
				break;
			}
		}
		if(match) return &source[i];
	}
	return 0;
}

// 根据拼音首字母获取汉字索引
u8 GetIndexFromPinyin(const char* pinyin)
{
	u8 i;
	for(i = 0; i < sizeof(pinyin_table)/sizeof(PinyinMap); i++) {
		if(pinyin_table[i].pinyin[0] == pinyin[0]) {
			if(pinyin_table[i].pinyin[1] == '\0' || pinyin[1] == '\0') {
				if(pinyin_table[i].pinyin[1] == '\0' && pinyin[1] == '\0') {
					return pinyin_table[i].index;
				}
			} else if(pinyin_table[i].pinyin[1] == pinyin[1]) {
				return pinyin_table[i].index;
			}
		}
	}
	return 0xFF; // 未找到
}

// 在LCD上显示16x16汉字
void LCD_ShowChineseChar(u16 x, u16 y, u8 index)
{
	u16 i, j;
	u8 font_data[32];
	u8 tmpreg;
	
	if(index >= 31) return; // 超出范围
	
	// 复制字模数据
	for(i = 0; i < 32; i++) {
		font_data[i] = province_font[index][i];
	}
	
	// 显示16x16点阵
	for(i = 0; i < 16; i++) {
		for(j = 0; j < 8; j++) {
			tmpreg = font_data[2*i];
			if(tmpreg & (0x80 >> j)) {
				LCD_DrawPoint(x + j, y + i, 0xFFFF); // 白色前景
			}
		}
		for(j = 0; j < 8; j++) {
			tmpreg = font_data[2*i + 1];
			if(tmpreg & (0x80 >> j)) {
				LCD_DrawPoint(x + 8 + j, y + i, 0xFFFF); // 白色前景
			}
		}
	}
}

// USART1中断服务函数
void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
		u8 c = USART_ReceiveData(USART1);

		if(c == '\n' || receive_index >= 127) {
			receive_buffer[receive_index] = '\0';
			receive_index = 0;

			// 检查是否是车牌信息、出库信息或舵机命令
			if(FindString((char*)receive_buffer, "PLATE:") != 0 ||
			   FindString((char*)receive_buffer, "EXIT_OK:") != 0 ||
			   FindString((char*)receive_buffer, "GATE:") != 0) {
				receive_complete = 1;
			}
		} else if(c != '\r') {
			receive_buffer[receive_index++] = c;
		}
	}
}

// 解析车牌信息
void ParsePlateInfo(u8* data)
{
	u8 i;
	char* ptr;
	char* end;
	u8 is_exit_ok_format = 0;

	// 清空结构体
	for(i = 0; i < 20; i++) current_plate.plate[i] = '\0';
	for(i = 0; i < 10; i++) current_plate.color[i] = '\0';
	for(i = 0; i < 25; i++) current_plate.in_time[i] = '\0';
	for(i = 0; i < 25; i++) current_plate.out_time[i] = '\0';
	for(i = 0; i < 10; i++) current_plate.status[i] = '\0';

	// 检查是否是EXIT_OK格式
	if(FindString((char*)data, "EXIT_OK:") != 0) {
		is_exit_ok_format = 1;
		// EXIT_OK:JSF05EV8,FEE:10.00,IN:...,OUT:... 格式
		ptr = FindString((char*)data, "EXIT_OK:");
		if(ptr) {
			ptr += 8;  // 跳过 "EXIT_OK:"
			end = FindString(ptr, ",");
			if(end) {
				u8 len = end - ptr;
				if(len > 19) len = 19;
				for(i = 0; i < len; i++) {
					current_plate.plate[i] = ptr[i];
				}
				current_plate.plate[len] = '\0';
			}
			// EXIT_OK格式没有COLOR和STATUS，设置status为out
			for(i = 0; i < 10; i++) current_plate.status[i] = '\0';
			current_plate.status[0] = 'o';
			current_plate.status[1] = 'u';
			current_plate.status[2] = 't';
		}
	}

	// 解析PLATE（标准格式）
	if(!is_exit_ok_format) {
		ptr = FindString((char*)data, "PLATE:");
		if(ptr) {
			ptr += 6;
			end = FindString(ptr, ",");
			if(end) {
				u8 len = end - ptr;
				if(len > 19) len = 19;
				for(i = 0; i < len; i++) {
					current_plate.plate[i] = ptr[i];
				}
				current_plate.plate[len] = '\0';  // 添加字符串终止符
			}
		}
	}

	// 解析COLOR
	ptr = FindString((char*)data, "COLOR:");
	if(ptr) {
		ptr += 6;
		end = FindString(ptr, ",");
		if(end) {
			u8 len = end - ptr;
			if(len > 9) len = 9;
			for(i = 0; i < len; i++) {
				current_plate.color[i] = ptr[i];
			}
			current_plate.color[len] = '\0';  // 添加字符串终止符
		}
	}

	// 解析IN时间
	ptr = FindString((char*)data, "IN:");
	if(ptr) {
		ptr += 3;
		end = FindString(ptr, ",");
		if(end) {
			u8 len = end - ptr;
			if(len > 24) len = 24;
			for(i = 0; i < len; i++) {
				current_plate.in_time[i] = ptr[i];
			}
			current_plate.in_time[len] = '\0';  // 添加字符串终止符
		}
	}

	// 解析OUT时间
	ptr = FindString((char*)data, "OUT:");
	if(ptr) {
		ptr += 4;
		end = FindString(ptr, ",");
		if(end) {
			u8 len = end - ptr;
			if(len > 24) len = 24;
			for(i = 0; i < len; i++) {
				current_plate.out_time[i] = ptr[i];
			}
			current_plate.out_time[len] = '\0';  // 添加字符串终止符
		}
	}

	// 解析STATUS
	ptr = FindString((char*)data, "STATUS:");
	if(ptr) {
		ptr += 7;
		end = FindString(ptr, "\r");
		if(!end) end = FindString(ptr, "\n");
		if(!end) end = ptr + GetStringLen(ptr);
		if(end) {
			u8 len = end - ptr;
			if(len > 9) len = 9;
			for(i = 0; i < len; i++) {
				current_plate.status[i] = ptr[i];
			}
			current_plate.status[len] = '\0';  // 添加字符串终止符
		}
	}

	// 解析FEE（停车费用）
	ptr = FindString((char*)data, "FEE:");
	if(ptr) {
		ptr += 4;
		current_plate.fee = ParseFee(ptr);
	}
}

// 解析FEE（停车费用）
u32 ParseFee(char* str) {
    u32 fee = 0;
    while(*str >= '0' && *str <= '9') {
        fee = fee * 10 + (*str - '0');
        str++;
    }
    return fee;
}

// 显示车位布局
void DisplayParkingSpots(void) {
    u8 i, row, col;
    u16 x, y;
    u16 box_width = 95;
    u16 box_height = 70;
    u16 start_x = 10;
    u16 start_y = 30;
    u16 gap_x = 10;
    u16 gap_y = 15;
    u16 color_empty = 0x07E0;  // 绿色-空闲
    u16 color_occupied = 0xF800;  // 红色-占用
    static u8 first_call = 1;  // 首次显示标志
    static u8 last_occupied[6] = {0};  // 上次占用状态

    // 如果需要强制重绘，重置状态
    if(parking_spots_redraw) {
        parking_spots_redraw = 0;
        first_call = 1;
        for(i = 0; i < PARKING_SPOTS_COUNT; i++) {
            last_occupied[i] = 255;  // 设置为无效值，确保强制重绘
        }
    }

    // 首次显示时完全重绘
    if(first_call) {
        LCD_Fill(0x0000);  // 黑屏
        // 显示标题
        LCD_ShowString_Fixed(100, 5, "PARKING STATUS");
    }

    for(i = 0; i < PARKING_SPOTS_COUNT; i++) {
        row = i / 3;  // 0或1
        col = i % 3;  // 0,1,2

        x = start_x + col * (box_width + gap_x);
        y = start_y + row * (box_height + gap_y);

        // 首次显示或状态变化时重绘方框
        if(first_call || last_occupied[i] != parking_spots[i].occupied) {
            // 绘制方框（空闲为绿色，占用为红色）
            if(parking_spots[i].occupied) {
                LCD_DrawRectangle(x, y, x + box_width, y + box_height, color_occupied);
            } else {
                LCD_DrawRectangle(x, y, x + box_width, y + box_height, color_empty);
            }
            last_occupied[i] = parking_spots[i].occupied;
        }

        // 首次显示时绘制车位号 "P1", "P2", ... "P6"
        if(first_call) {
            LCD_ShowChar(x + 35, y + 5, 'P', 0);
            LCD_ShowChar(x + 47, y + 5, '1' + i, 0);
        }
    }

    if(first_call) {
        first_call = 0;
    }
}

// 扫描车位状态
// FSR默认高电平，被压时拉低
void ScanParkingSpots(void) {
    u8 fsr_state;
    unsigned int i;

    // 扫描FSR传感器状态
    fsr_state = FSR_Scan(1);

    if(fsr_state == KEY_RELEASED) {
        // FSR被压住（低电平），车位占用
        parking_spots[0].occupied = 1;
        if(GetStringLen(parking_spots[0].plate) == 0) {
            // 复制当前车牌到车位
            for(i = 0; i < 20 && current_plate.plate[i] != '\0'; i++) {
                parking_spots[0].plate[i] = current_plate.plate[i];
            }
            parking_spots[0].plate[i] = '\0';
        }
    } else {
        // FSR释放（高电平），车位空闲
        parking_spots[0].occupied = 0;
        for(i = 0; i < 20; i++) {
            parking_spots[0].plate[i] = '\0';
        }
    }
}

// 在LCD上显示车牌信息
void DisplayPlateInfo(PlateInfo* info)
{
	// 清屏为黑色
	LCD_Fill(0x0000);

	// 显示标题框
	LCD_DrawRectangle(10, 10, 310, 250, 0x001F);  // 蓝色边框

	// 显示 "Vehicle Info" 标题
	LCD_ShowChar(130, 20, 'V', 0);
	LCD_ShowChar(142, 20, 'e', 0);
	LCD_ShowChar(154, 20, 'h', 0);
	LCD_ShowChar(166, 20, 'i', 0);
	LCD_ShowChar(178, 20, 'c', 0);
	LCD_ShowChar(190, 20, 'l', 0);
	LCD_ShowChar(202, 20, 'e', 0);

	// 显示车牌标签
	LCD_ShowChar(30, 50, 'P', 0);
	LCD_ShowChar(42, 50, 'l', 0);
	LCD_ShowChar(54, 50, 'a', 0);
	LCD_ShowChar(66, 50, 't', 0);
	LCD_ShowChar(78, 50, 'e', 0);
	LCD_ShowChar(90, 50, ':', 0);

	// 显示车牌 (直接显示ASCII字符，不依赖字库)
	if(GetStringLen(info->plate) > 0) {
		LCD_ShowString_Fixed(110, 50, info->plate);
	}

	// 显示颜色标签
	LCD_ShowChar(30, 80, 'C', 0);
	LCD_ShowChar(42, 80, 'o', 0);
	LCD_ShowChar(54, 80, 'l', 0);
	LCD_ShowChar(66, 80, 'o', 0);
	LCD_ShowChar(78, 80, 'r', 0);
	LCD_ShowChar(90, 80, ':', 0);
	LCD_ShowString_Fixed(110, 80, info->color);

	// 显示入库时间标签
	LCD_ShowChar(30, 110, 'I', 0);
	LCD_ShowChar(42, 110, 'n', 0);
	LCD_ShowChar(54, 110, ':', 0);
	LCD_ShowString_Fixed(75, 110, info->in_time);

	// 显示出库时间标签
	LCD_ShowChar(30, 140, 'O', 0);
	LCD_ShowChar(42, 140, 'u', 0);
	LCD_ShowChar(54, 140, 't', 0);
	LCD_ShowChar(66, 140, ':', 0);
	LCD_ShowString_Fixed(85, 140, info->out_time);

	// 显示状态标签
	LCD_ShowChar(30, 170, 'S', 0);
	LCD_ShowChar(42, 170, 't', 0);
	LCD_ShowChar(54, 170, 'a', 0);
	LCD_ShowChar(66, 170, 't', 0);
	LCD_ShowChar(78, 170, 'u', 0);
	LCD_ShowChar(90, 170, 's', 0);
	LCD_ShowChar(102, 170, ':', 0);
	LCD_ShowString_Fixed(120, 170, info->status);

	// 显示停车费用标签
	LCD_ShowChar(30, 200, 'F', 0);
	LCD_ShowChar(42, 200, 'e', 0);
	LCD_ShowChar(54, 200, 'e', 0);
	LCD_ShowChar(66, 200, ':', 0);
	LCD_ShowNum(90, 200, info->fee, 6);  // 显示费用（6位数字）
	LCD_ShowChar(150, 200, 'Y', 0);  // 元

	// 根据状态显示不同颜色的边框
	if(GetStringLen(info->status) >= 2 && info->status[0] == 'i' && info->status[1] == 'n') {
		LCD_DrawRectangle(10, 10, 310, 250, 0x07E0);  // 绿色边框表示在场
	} else {
		LCD_DrawRectangle(10, 10, 310, 250, 0xF800);  // 红色边框表示离场
	}
}

// 显示字符串到LCD
void LCD_ShowString_Fixed(u16 x, u16 y, const char* str)
{
	u16 i = 0;
	while(str[i] != '\0') {
		LCD_ShowChar(x + i * 12, y, str[i], 0);
		i++;
	}
}
