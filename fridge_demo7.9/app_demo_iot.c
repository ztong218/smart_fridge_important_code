/*
 * Copyright (c) 2022 HiSilicon (Shanghai) Technologies CO., LIMITED.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "..\..\..\..\device\hisilicon\hispark_pegasus\sdk_liteos\platform\os\Huawei_LiteOS\components\lib\libc\musl\include\time.h"
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <hi_task.h>
#include <string.h>
#include <hi_wifi_api.h>
#include <hi_mux.h>
#include <hi_io.h>
#include <hi_gpio.h>
#include "iot_config.h"
#include "iot_log.h"
#include "iot_main.h"
#include "iot_profile.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "cJSON.h"
#include "food_data.h"

/* DHT11模块 & LED模块 */
#include <unistd.h>
#include "iot_gpio_ex.h"
#include "iot_gpio.h"
#include "iot_uart.h"
#include "hi_uart.h"
#include "iot_watchdog.h"
#include "iot_errno.h"
#include "hi_time.h"
#include "ssd1306_fonts.h"
#include "oled_ssd1306.h"
#include "iot_i2c.h"
#include"ohos_types.h"
#include "hi_io.h"
#include "iot_pwm.h"
#include "hi_pwm.h"

/* UART模块 */
#include <hi_stdlib.h>
#include <hisignalling_protocol.h>
#include <hi_uart.h>
#include <app_demo_uart.h>
#include <iot_uart.h>
#include <hi_types_base.h>
#include <hi_early_debug.h>
#include "hisignalling_protocol.h"
#include "globals.h"

extern hi_u8 category[30];

/* attribute initiative to report */
#define TAKE_THE_INITIATIVE_TO_REPORT
#define ONE_SECOND                          (1000)
/* oc request id */
#define CN_COMMADN_INDEX                    "commands/request_id="
#define WECHAT_SUBSCRIBE_LIGHT              "light"
#define WECHAT_SUBSCRIBE_LIGHT_ON_STATE     "1"
#define WECHAT_SUBSCRIBE_LIGHT_OFF_STATE    "0"
#define WECHAT_SUBSCRIBE_MOTOR              "motor"
#define WECHAT_SUBSCRIBE_MOTOR_ON_STATE     "1"
#define WECHAT_SUBSCRIBE_MOTOR_OFF_STATE    "0"
#define WECHAT_SUBSCRIBE_DETECT             "detect"
#define WECHAT_SUBSCRIBE_DETECT_ON_STATE    "1"
#define WECHAT_SUBSCRIBE_DETECT_OFF_STATE   "0"

/* DHT11模块 */
#define UART_BUFF_SIZE 100
#define U_SLEEP_TIME   100000
#define DHT11_GPIO  IOT_IO_NAME_GPIO_11
IotGpioValue DHT11_DQ_IN;
typedef unsigned char u8;

/* MOTOR模块 */
#define MOTOR_GPIO_1  IOT_IO_NAME_GPIO_6
#define MOTOR_GPIO_2  IOT_IO_NAME_GPIO_7

/* OLED模块 */
#define OLED_I2C_IDX 0
#define IOT_I2C_IDX_BAUDRATE (400 * 1000)
#define INS5902_I2C_IDX 0

#define DECIMA               10
#define HEX                  16

#define OLED_WIDTH    (128)
#define OLED_I2C_ADDR 0x78 // 默认地址为 0x78
#define OLED_I2C_CMD 0x00 // 0000 0000       写命令
#define OLED_I2C_DATA 0x40 // 0100 0000(0x40) 写数据
#define OLED_I2C_BAUDRATE (400 * 1000) // 400k

#define BATTERY_REGISTER         0x21
#define BATTERY_SWITCH           0x80

#define LED_LOOP    10
#define DELYA_MS    1000

int32_t temperature;
int32_t humidity;
_Bool detectStart = 0;   //判断是否需要拍摄新的照片，开始检测食物信息
char string[16] = {0};
int g_ligthStatus = -1;
int g_motorStatus = -1;
int g_detectStatus = -1;
typedef void (*FnMsgCallBack)(hi_gpio_value val);

typedef struct FunctionCallback {
    hi_bool  stop;
    hi_u32 conLost;
    hi_u32 queueID;
    hi_u32 iotTaskID;
    FnMsgCallBack    msgCallBack;
}FunctionCallback;
FunctionCallback g_functinoCallback;


/* OLED模块 */
#define DELAY_100_MS (100 * 1000)
char* itoa(int num,char* str,int radix)
{
    char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";//索引表
    unsigned unum;//存放要转换的整数的绝对值,转换的整数可能是负数
    int i=0,j,k;//i用来指示设置字符串相应位，转换之后i其实就是字符串的长度；转换后顺序是逆序的，有正负的情况，k用来指示调整顺序的开始位置;j用来指示调整顺序时的交换。
 
    //获取要转换的整数的绝对值
    if(radix==10&&num<0)//要转换成十进制数并且是负数
    {
        unum=(unsigned)-num;//将num的绝对值赋给unum
        str[i++]='-';//在字符串最前面设置为'-'号，并且索引加1
    }
    else unum=(unsigned)num;//若是num为正，直接赋值给unum
 
    //转换部分，注意转换后是逆序的
    do
    {
        str[i++]=index[unum%(unsigned)radix];//取unum的最后一位，并设置为str对应位，指示索引加1
        unum/=radix;//unum去掉最后一位
 
    }while(unum);//直至unum为0退出循环
 
    str[i]='\0';//在字符串最后添加'\0'字符，c语言字符串以'\0'结束。
 
    //将顺序调整过来
    if(str[0]=='-') k=1;//如果是负数，符号不用调整，从符号后面开始调整
    else k=0;//不是负数，全部都要调整
 
    char temp;//临时变量，交换两个值时用到
    for(j=k;j<=(i-1)/2;j++)//头尾一一对称交换，i其实就是字符串的长度，索引最大值比长度少1
    {
        temp=str[j];//头部赋值给临时变量
        str[j]=str[i-1+k-j];//尾部赋值给头部
        str[i-1+k-j]=temp;//将临时变量的值(其实就是之前的头部值)赋给尾部
    }
 
    return str;//返回转换后的字符串
 
}

typedef struct {
    /* Pointer to the buffer storing data to send */
    unsigned char *sendBuf;
    /* Length of data to send */
    unsigned int  sendLen;
    /* Pointer to the buffer for storing data to receive */
    unsigned char *receiveBuf;
    /* Length of data received */
    unsigned int  receiveLen;
} IotI2cData;

static uint32_t I2cWiteByte(uint8_t regAddr, uint8_t byte)
{
    unsigned int id = OLED_I2C_IDX;
    uint8_t buffer[] = {regAddr, byte};
    IotI2cData i2cData = {0};

    i2cData.sendBuf = buffer;
    i2cData.sendLen = sizeof(buffer) / sizeof(buffer[0]);

    return IoTI2cWrite(id, OLED_I2C_ADDR, i2cData.sendBuf, i2cData.sendLen);
}

/*
 * @brief Write a command byte to OLED device.
 * @param cmd the commnad byte to be writen.
 * @return Returns {@link IOT_SUCCESS} if the operation is successful;
 * returns an error code defined in {@link wifiiot_errno.h} otherwise.
 */
static uint32_t WriteCmd(uint8_t cmd)
{
    return I2cWiteByte(OLED_I2C_CMD, cmd);
}

/*
 * @brief Write a data byte to OLED device.
 * @param cmd the data byte to be writen.
 * @return Returns {@link IOT_SUCCESS} if the operation is successful;
 * returns an error code defined in {@link wifiiot_errno.h} otherwise.
 */
static uint32_t WriteData(uint8_t data)
{
    return I2cWiteByte(OLED_I2C_DATA, data);
}

/*
 * @brief ssd1306 OLED Initialize.
 */
uint32_t OledInit(void)
{
    static const uint8_t initCmds[] = {
        0xAE, // --display off
        0x00, // ---set low column address
        0x10, // ---set high column address
        0x40, // --set start line address
        0xB0, // --set page address
        0x81, // contract control
        0xFF, // --128
        0xA1, // set segment remap
        0xA6, // --normal / reverse
        0xA8, // --set multiplex ratio(1 to 64)
        0x3F, // --1/32 duty
        0xC8, // Com scan direction
        0xD3, // -set display offset
        0x00,
        0xD5, // set osc division
        0x80,
        0xD8, // set area color mode off
        0x05,
        0xD9, // Set Pre-Charge Period
        0xF1,
        0xDA, // set com pin configuartion
        0x12,
        0xDB, // set Vcomh
        0x30,
        0x8D, // set charge pump enable
        0x14,
        0xAF, // --turn on oled panel
    };

    IoTGpioInit(13); /* 初始化gpio13 */
    IoSetFunc(13, 6); /* gpio13使用6功能 */
    IoTGpioInit(14); /* 初始化gpio14 */
    IoSetFunc(14, 6); /* gpio14使用6功能 */

    IoTI2cInit(0, OLED_I2C_BAUDRATE);

    for (size_t i = 0; i < ARRAY_SIZE(initCmds); i++) {
        uint32_t status = WriteCmd(initCmds[i]);
        if (status != IOT_SUCCESS) {
            return status;
        }
    }
    return IOT_SUCCESS;
}

void OledSetPosition(uint8_t x, uint8_t y)
{
    WriteCmd(0xb0 + y);
    /* 在0xf0右移4位，与0x10或，实现了写数据 */
    /* Shift 4 bits to the right at 0xf0, and 0x10 or, to write data */
    WriteCmd(((x & 0xf0) >> 4) | 0x10);
    WriteCmd(x & 0x0f);
}

/*
 * 全屏填充
 * Full Screen Fill
 */
void OledFillScreen(uint8_t fillData)
{
    /* 循环8次实现横屏填充 */
    /* Cycle 8 times to achieve horizontal screen filling */
    for (uint8_t m = 0; m < 8; m++) {
        WriteCmd(0xb0 + m);
        WriteCmd(0x00);
        WriteCmd(0x10);
        /* 循环128次实现竖屏填充 */
        /* Loop 128 times to achieve vertical screen filling */
        for (uint8_t n = 0; n < 128; n++) {
            WriteData(fillData);
        }
    }
}

/**
 * @brief 8*16 typeface
 * @param x: write positon start from x axis
 * @param y: write positon start from y axis
 * @param ch: write data
 * @param font: selected font
 */
void OledShowChar(uint8_t x, uint8_t y, uint8_t ch, Font font)
{
    uint8_t c = ch - ' '; // 得到偏移后的值，Get the offset value
    uint8_t b = x;
    uint8_t d = y;
    if (b > OLED_WIDTH - 1) {
        b = 0;
        d = d + 2; /* 得到偏移后的值2 Get the offset value 2 */
    }

    if (font == FONT8_X16) {
        OledSetPosition(b, d);
        for (uint8_t i = 0; i < 8; i++) { /* 循环8次实现横屏填充 Cycle 8 times to achieve horizontal screen filling */
            WriteData(g_f8X16[c * 16 + i]); /* 循环16次实现横屏填充 Cycle 16 times to realize horizontal screen filling */
        }

        OledSetPosition(b, d + 1);
        for (uint8_t i = 0; i < 8; i++) { /* 循环8次实现横屏填充 Cycle 8 times to achieve horizontal screen filling */
            /* 循环16次实现横屏填充8列 Cycle 16 times to fill 8 columns in horizontal screen */
            WriteData(g_f8X16[c * 16 + i + 8]);
        }
    } else {
        OledSetPosition(b, d);
        for (uint8_t i = 0; i < 6; i++) { /* 循环6次实现横屏填充 Cycle 6 times to achieve horizontal screen filling */
            WriteData(g_f6X8[c][i]);
        }
    }
}

void OledShowString(uint8_t x, uint8_t y, const char* str, Font font)
{
    uint8_t j = 0;
    uint8_t b = x;
    uint8_t d = y;
    if (str == NULL) {
        printf("param is NULL,Please check!!!\r\n");
        return;
    }

    while (str[j]) {
        OledShowChar(b, d, str[j], font);
        b += 8; /* 循环8次实现横屏填充 Cycle 8 times to achieve horizontal screen filling */
        if (b > 120) { /* 循环120次实现横屏填充 Cycle 120 times to achieve horizontal screen filling */
            b = 0;
            d += 2; /* 循环2次实现横屏填充 Cycle twice to realize horizontal screen filling */
        }
        j++;
    }
}


/* DHT11模块 */
u8 GPIOGETINPUT(IotIoName id,IotGpioValue *val)
{
    IoTGpioGetInputVal(id,val);
    return *val;
}

void DHT11_IO_OUT(void) {
     //设置GPIO_11为输出模式
    IoTGpioSetDir(DHT11_GPIO, IOT_GPIO_DIR_OUT);
}

void DHT11_IO_IN(void) {
    IoTGpioSetDir(DHT11_GPIO, IOT_GPIO_DIR_IN);
    IoSetPull( DHT11_GPIO, IOT_IO_PULL_NONE);
}

void DHT11_Rst(void) {
    DHT11_IO_OUT();
    IoTGpioSetOutputVal(DHT11_GPIO, 0);//设置GPIO输出低电平 
    hi_udelay(20000);//拉低至少18ms
    IoTGpioSetOutputVal(DHT11_GPIO, 1);//设置GPIO输出高电平
    hi_udelay(35);     	//主机拉高20~40us
}

u8  DHT11_Check(void) {
   u8  retry = 0;
    DHT11_IO_IN();//SET INPUT	 
    while (GPIOGETINPUT(DHT11_GPIO, &DHT11_DQ_IN) && retry < 100) {
        retry++;
       hi_udelay(1);
    }
    if (retry >= 100) return 1;
    else retry = 0;

    while ((!GPIOGETINPUT(DHT11_GPIO, &DHT11_DQ_IN)) && retry < 100) {
        retry++;
       hi_udelay(1);
    }
    if (retry >= 100) return 1;
    return 0;
}

u8 DHT11_Read_Bit(void) 			 
{
 	u8 retry=0;
  while(GPIOGETINPUT(DHT11_GPIO,&DHT11_DQ_IN)&&retry<100){//等待变为低电平
        retry++;
        hi_udelay(1);
    }
    retry=0;
    while((!GPIOGETINPUT(DHT11_GPIO,&DHT11_DQ_IN))&&retry<100){//等待变高电平
        retry++;
        hi_udelay(1);
    }
    hi_udelay(40);//等待40us	//用于判断高低电平，即数据1或0
    if(GPIOGETINPUT(DHT11_GPIO,&DHT11_DQ_IN))return 1; else return 0;
}

u8 DHT11_Read_Byte(void) {
    u8  i, dat;
    dat = 0;
    for (i = 0; i < 8; i++) {
        dat <<= 1;
        dat |= DHT11_Read_Bit();
    }
    return dat;
}

u8 DHT11_Read_Data(u8 *temp, u8 *humi) {
    u8 buf[5] = {0};
    u8 i;
    DHT11_Rst();
    if (DHT11_Check() == 0)//读取40位数据
     {
        for (i = 0; i < 5; i++) {
            buf[i] = DHT11_Read_Byte();
        }
        if ((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4])//数据校验
         {
            *humi = buf[0];
            *temp = buf[2];
        }
    } else {
        return 1;
    }
    return 0;
}

u8 DHT11_Init(void)
{	 
	//初始化GPIO
	IoTGpioInit(DHT11_GPIO);
    //设置GPIO_11的复用功能为普通GPIO
	 IoSetFunc(DHT11_GPIO, IOT_IO_FUNC_GPIO_11_GPIO);
    //设置GPIO_11为输入模式
    IoTGpioSetDir(DHT11_GPIO, IOT_GPIO_DIR_IN);
	//设置GPIO_11输出高电平
    IoTGpioSetOutputVal(DHT11_GPIO, 1);	

    DHT11_Rst();  //复位DHT11s
	return DHT11_Check();//DHT11_Check();//等待DHT11的回应
} 


/* UART模块 */
UartDefConfig uartDefConfig = {0};

static void Uart1GpioCOnfig(void)
{
#ifdef ROBOT_BOARD
    IoSetFunc(HI_IO_NAME_GPIO_5, IOT_IO_FUNC_GPIO_5_UART1_RXD);
    IoSetFunc(HI_IO_NAME_GPIO_6, IOT_IO_FUNC_GPIO_6_UART1_TXD);
    /* IOT_BOARD */
#elif defined (EXPANSION_BOARD)
    IoSetFunc(HI_IO_NAME_GPIO_0, IOT_IO_FUNC_GPIO_0_UART1_TXD);
    IoSetFunc(HI_IO_NAME_GPIO_1, IOT_IO_FUNC_GPIO_1_UART1_RXD);
#endif
}

int SetUartRecvFlag(UartRecvDef def)
{
    if (def == UART_RECV_TRUE) {
        uartDefConfig.g_uartReceiveFlag = HI_TRUE;
    } else {
        uartDefConfig.g_uartReceiveFlag = HI_FALSE;
    }
    
    return uartDefConfig.g_uartReceiveFlag;
}

int GetUartConfig(UartDefType type)
{
    int receive = 0;

    switch (type) {
        case UART_RECEIVE_FLAG:
            receive = uartDefConfig.g_uartReceiveFlag;
            break;
        case UART_RECVIVE_LEN:
            receive = uartDefConfig.g_uartLen;
            break;
        default:
            break;
    }
    return receive;
}

void ResetUartReceiveMsg(void)
{
    (void)memset_s(uartDefConfig.g_receiveUartBuff, sizeof(uartDefConfig.g_receiveUartBuff),
        0x0, sizeof(uartDefConfig.g_receiveUartBuff));
}

unsigned char *GetUartReceiveMsg(void)
{
    return uartDefConfig.g_receiveUartBuff;
}


// 打印食物信息  
void printFoodInfo(const FoodData* fruit) {  
    printf("Name: %s, Quantity: %d, Remaining Shelf Life: %d, Storage Date: %s\n",  
           fruit->name, fruit->quantity, fruit->period, fruit->storageDate);  
}  

// 创建一个数组来存储Food结构体  
FoodData food[8]; // 设置有8种食物 
// expiredFoodData ExpiredFood[10]; //存放过期食物
// expiredFoodData willExpiredFood[10]; //存放即将过期食物

size_t foodCount = sizeof(food) / sizeof(food[0]);
int currentYear, currentMonth, currentDay;
int storageYear, storageMonth, storageDay;
int bestData[8] = {7,3,10,15,15,10,10,30};

// 初始化食物信息
void FoodInit()
{
    int foodCount = 0; // 当前已存储的食物数量  

    // 初始化一些食物信息  
    strcpy_s(food[foodCount].name, strlen("Apple") + 1, "Apple");
    food[foodCount].quantity = 0;  
    food[foodCount].period = 0;  
    // food[foodCount].days = 0;  
    strcpy_s(food[foodCount].storageDate, strlen("2024-07-01") + 1, "2024-07-01");  // 用于测试剩余保质期-即将过期
    foodCount++;  

    strcpy_s(food[foodCount].name, strlen("Banana") + 1, "Banana");  
    food[foodCount].quantity = 0;  
    food[foodCount].period = 0;  
    // food[foodCount].days = 0;  
    strcpy_s(food[foodCount].storageDate, strlen("2024-07-02") + 1, "2024-07-02");  // 用于测试剩余保质期-已经过期
    foodCount++;  

    strcpy_s(food[foodCount].name, strlen("Lemon") + 1, "Lemon");
    food[foodCount].quantity = 0;  
    food[foodCount].period = 0;  
    // food[foodCount].days = 0;  
    // strcpy_s(food[foodCount].storageDate, strlen("2024-06-01") + 1, "2024-06-01");  
    foodCount++;  

    strcpy_s(food[foodCount].name, strlen("carrot") + 1, "carrot");  
    food[foodCount].quantity = 0;  
    food[foodCount].period = 0;  
    // food[foodCount].days = 0;  
    // strcpy_s(food[foodCount].storageDate, strlen("2024-06-05") + 1, "2024-06-05");  
    foodCount++;  

    strcpy_s(food[foodCount].name, strlen("daikon") + 1, "daikon");
    food[foodCount].quantity = 0;  
    food[foodCount].period = 0;  
    // food[foodCount].days = 0;  
    // strcpy_s(food[foodCount].storageDate, strlen("2024-06-01") + 1, "2024-06-01");  
    foodCount++;  

    strcpy_s(food[foodCount].name, strlen("pepper") + 1, "pepper");  
    food[foodCount].quantity = 0;  
    food[foodCount].period = 0;  
    // food[foodCount].days = 0;  
    // strcpy_s(food[foodCount].storageDate, strlen("2024-06-05") + 1, "2024-06-05");  
    foodCount++;  

    strcpy_s(food[foodCount].name, strlen("eggplant") + 1, "eggplant");
    food[foodCount].quantity = 0;  
    food[foodCount].period = 0;  
    // food[foodCount].days = 0;  
    // strcpy_s(food[foodCount].storageDate, strlen("2024-07-05") + 1, "2024-07-05");  
    foodCount++;  

    strcpy_s(food[foodCount].name, strlen("potato") + 1, "potato");  
    food[foodCount].quantity = 0;  
    food[foodCount].period = 0;  
    // food[foodCount].days = 0;  
    // strcpy_s(food[foodCount].storageDate, strlen("2024-06-05") + 1, "2024-06-05");  
    foodCount++;  

    // 打印所有食物信息  
    printf("stored food information:\n");
    for (int i = 0; i < foodCount; i++) {  
        printFoodInfo(&food[i]);  
    }  
    printf("food information output end\n");
}

// 相机开始检测，更新食物信息
void FoodTask(){
    sendFlag = detectStart;
    // if(detectStart == 1){
    //     food[0].period = 7; 
    //     // food[1].period = 3; 
    //     food[6].period = 10; 
    //     // strcpy_s(food[0].storageDate, strlen("2024-07-05") + 1, "2024-07-05");  
    //     // strcpy_s(food[1].storageDate, strlen("2024-07-05") + 1, "2024-07-05");  
    //     // strcpy_s(food[6].storageDate, strlen("2024-07-05") + 1, "2024-07-05");  
    // }
    detectStart = 0;
}

// TODO: 以下时间计算相关函数待测试
// 判断是否为闰年
bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// 获取一个月的天数
int getMonthDays(int year, int month) {
    switch (month) {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12:
            return 31;
        case 4: case 6: case 9: case 11:
            return 30;
        case 2:
            return isLeapYear(year) ? 29 : 28;
        default:
            return 0;
    }
}

// 计算从年初到指定日期的天数
int getDayOfYear(int year, int month, int day) {
    int days = day;
    for (int i = 1; i < month; ++i) {
        days += getMonthDays(year, i);
    }
    return days;
}

// 计算两个日期之间的天数差
int calculateDaysDifference(int year1, int month1, int day1, int year2, int month2, int day2) {
    int dayOfYear1 = getDayOfYear(year1, month1, day1);
    int dayOfYear2 = getDayOfYear(year2, month2, day2);
    
    // 如果年份相同，直接返回两个日期的天数差
    if (year1 == year2) {
        return dayOfYear2 - dayOfYear1;
    }
    
    // 计算跨年份的天数差
    int daysDifference = 0;
    for (int y = year1; y < year2; ++y) {
        int daysInYear = isLeapYear(y) ? 366 : 365;
        if (y == year1) {
            daysDifference += dayOfYear1;
            daysInYear -= dayOfYear1;
        } else if (y == year2 - 1) {
            daysInYear -= getDayOfYear(year2, month2, 1); // 减去年初到month2的天数
            daysDifference += daysInYear;
        } else {
            daysDifference += daysInYear;
        }
    }
    
    return daysDifference + dayOfYear2; // 加上year2的天数
}

// 获取当前日期的函数，需要年份参数
void getCurrentDate(int *year, int *month, int *day) {
    time_t t = time(NULL);
    struct tm *now = localtime(&t);
    if (year != NULL) *year = now->tm_year + 1900; // tm_year是从1900年开始的，所以需要+1900
    if (month != NULL) *month = now->tm_mon + 1;    // tm_mon是从0开始的，所以需要+1
    if (day != NULL) *day = now->tm_mday;           // tm_mday是实际的日期
}

// 自定义实现strftime函数，格式化日期
// 将整数转换为字符串，并填充0以满足所需的长度
void int_to_str(char *buffer, int value, int length) {
    sprintf(buffer, "%0*d", length, value);
}

// 自定义的strftime函数，仅支持"%Y-%m-%d"格式
char* my_strftime(char *buffer, const struct tm *local_time) {
    if (buffer == NULL || local_time == NULL) {
        return NULL;
    }
    // 清空字符串，准备更新
    memset(buffer, 0, 20);
    // 将年份转换为字符串
    int_to_str(buffer, local_time->tm_year + 1900, 4);
    // 填充年份后的分隔符'-'
    strcat(buffer, "-");
    // 将月份转换为字符串
    int_to_str(strchr(buffer, '\0'), local_time->tm_mon + 1, 2);
    // 填充月份后的分隔符'-'
    strcat(buffer, "-");
    // 将日期转换为字符串
    int_to_str(strchr(buffer, '\0'), local_time->tm_mday, 2);

    return buffer;
}


// 获取当前日期的函数，返回当前的年月日[字符串]
static char dateStr[20];    // 在函数外定义静态字符串，用于存储格式化的日期
char* getCurrentDateString() {
    // 获取当前时间
    time_t now = time(NULL);
    // 转换为本地时间
    struct tm *local_time = localtime(&now);
    // 清空字符串，准备更新
    memset(dateStr, 0, sizeof(dateStr));
    // 使用strftime格式化日期并存储到静态字符串中
    // strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", local_time);
    // sprintf_s(dateStr, sizeof(dateStr), "%Y-%m-%d", local_time);
    // 使用自定义的my_strftime函数格式化日期
    my_strftime(dateStr, local_time);
    printf("Current date: %s\n", dateStr);//用于测试是否正确得到年月日信息
    // 返回格式化的日期字符串
    return dateStr;
}

// 辅助函数，解析日期字符串并返回月份和日期
void parseDate(const char *dateStr, int *year, int *month, int *day) {
    sscanf(dateStr, "%d-%d-%d", year, month, day);
}

// TODO:检测食物是否过期(待测试)
void FoodCheck(){
    getCurrentDate(currentYear, currentMonth, currentDay);
    for (size_t i = 0; i < foodCount; ++i) {
        // 计算剩余保质期
        parseDate(food[i].storageDate, &storageYear, &storageMonth, &storageDay);
        food[i].period = bestData[i] - calculateDaysDifference(storageYear, storageMonth, storageDay,
                                                 currentYear, currentMonth, currentDay);
    }
    // 打印结果
    for (size_t i = 0; i < foodCount; ++i) {
        printf("食材名称: %s, 剩余保质期: %d天\n", food[i].name, food[i].period);
    }
}


/* mqtt模块 */
static void DeviceConfigInit_light(hi_gpio_value val)
{
    hi_io_set_func(HI_IO_NAME_GPIO_9, HI_IO_FUNC_GPIO_9_GPIO);
    hi_gpio_set_dir(HI_GPIO_IDX_9, HI_GPIO_DIR_OUT);
    hi_gpio_set_ouput_val(HI_GPIO_IDX_9, val);
}

static void DeviceConfigInit_motor(hi_gpio_value val)
{
    //风扇1
    hi_io_set_func(HI_IO_NAME_GPIO_6, HI_IO_FUNC_GPIO_6_GPIO);
    hi_gpio_set_dir(HI_GPIO_IDX_6, HI_GPIO_DIR_OUT);
    hi_gpio_set_ouput_val(HI_GPIO_IDX_6, val);
    //风扇2
    hi_io_set_func(HI_IO_NAME_GPIO_7, HI_IO_FUNC_GPIO_7_GPIO);
    hi_gpio_set_dir(HI_GPIO_IDX_7, HI_GPIO_DIR_OUT);
    hi_gpio_set_ouput_val(HI_GPIO_IDX_7, val);
}

static int  DeviceMsgCallback(FnMsgCallBack msgCallBack)
{
    g_functinoCallback.msgCallBack = msgCallBack;
    return 0;
}

static void wechatControlDeviceMsg_light(hi_gpio_value val)
{
    DeviceConfigInit_light(val);
}

static void wechatControlDeviceMsg_motor(hi_gpio_value val)
{
    DeviceConfigInit_motor(val);
}

static void wechatControlDeviceMsg_detect(hi_gpio_value val)
{
    // DeviceConfigInit_detect(val);
    detectStart = val;
}

// < this is the callback function, set to the mqtt, and if any messages come, it will be called
// < The payload here is the json string
static void DemoMsgRcvCallBack(int qos, const char *topic, const char *payload)
{
    IOT_LOG_DEBUG("RCVMSG:QOS:%d TOPIC:%s PAYLOAD:%s\r\n", qos, topic, payload);
    /* 云端下发命令后，板端的操作处理 */
    if (strstr(payload, WECHAT_SUBSCRIBE_LIGHT) != NULL) {
        if (strstr(payload, WECHAT_SUBSCRIBE_LIGHT_OFF_STATE) != NULL) {
            wechatControlDeviceMsg_light(HI_GPIO_VALUE1);
            g_ligthStatus = HI_FALSE;
        } else {
            wechatControlDeviceMsg_light(HI_GPIO_VALUE0);
            g_ligthStatus = HI_TRUE;
        }
    }
    else if (strstr(payload, WECHAT_SUBSCRIBE_MOTOR) != NULL) {
        if (strstr(payload, WECHAT_SUBSCRIBE_MOTOR_OFF_STATE) != NULL) {
            wechatControlDeviceMsg_motor(HI_GPIO_VALUE0);
            g_motorStatus = HI_TRUE;
        } else {
            wechatControlDeviceMsg_motor(HI_GPIO_VALUE1);
            g_motorStatus = HI_FALSE;
        }
    }
    else if (strstr(payload, WECHAT_SUBSCRIBE_DETECT) != NULL) {
        if (strstr(payload, WECHAT_SUBSCRIBE_DETECT_OFF_STATE) != NULL) {
            wechatControlDeviceMsg_detect(false);
            g_detectStatus = HI_FALSE;
        } else {
            wechatControlDeviceMsg_detect(true);
            g_detectStatus = HI_TRUE;
        }
    }
    return HI_NULL;
}

/* publish sample */
hi_void IotPublishSample(void)
{
    /* reported attribute */
    WeChatProfile weChatProfile = {
        .subscribeType = "type",
        .status.subState = "state",
        .status.subReport = "reported",
        .status.reportVersion = "version",
        .status.Token = "clientToken",

        // /* report motor */
        // .reportAction.subDeviceActionMotor = "motor",
        // .reportAction.motorActionStatus = 0, /* 0 : motor off */

        /* report temperature */
        .reportAction.subDeviceActionTemperature = "temperature",
        .reportAction.temperatureData = temperature, /* temperature data */
        /* report humidity */
        .reportAction.subDeviceActionHumidity = "humidity",
        .reportAction.humidityActionData = humidity, /* humidity data */
        /* report detectStart */
        .reportAction.subDeviceActionDetect = "detectStart",
        .reportAction.detectActionStatus = detectStart, /* detectStart */
    };

    /* report food */
    //所有食物
    weChatProfile.reportAction.subDeviceActionFood = "food";
    for (int i = 0; i < (sizeof(food) / sizeof(food[0])); i++) {  
        weChatProfile.reportAction.foodActionData[i] = food[i];  
    }
    //已过期食物
    // weChatProfile.reportAction.subDeviceActionFood_Expired = "ExpiredFood";
    // for (int i = 0; i < (sizeof(ExpiredFood) / sizeof(ExpiredFood[0])); i++) {  
    //     weChatProfile.reportAction.ExpiredFoodActionData[i] = ExpiredFood[i];  
    // }
    //即将过期食物
    // weChatProfile.reportAction.subDeviceActionFood_willExpired = "willExpiredFood";
    // for (int i = 0; i < (sizeof(willExpiredFood) / sizeof(willExpiredFood[0])); i++) {  
    //     weChatProfile.reportAction.willExpiredFoodActionData[i] = willExpiredFood[i];  
    // }

    /* report light */
    if (g_ligthStatus == HI_TRUE) {
        weChatProfile.reportAction.subDeviceActionLight = "light";
        weChatProfile.reportAction.lightActionStatus = 1; /* 1: light on */
    } else if (g_ligthStatus == HI_FALSE) {
        weChatProfile.reportAction.subDeviceActionLight = "light";
        weChatProfile.reportAction.lightActionStatus = 0; /* 0: light off */
    } else {
        weChatProfile.reportAction.subDeviceActionLight = "light";
        weChatProfile.reportAction.lightActionStatus = 0; /* 0: light off */
    }
    /* report motor */
    if (g_motorStatus == HI_TRUE) {
        weChatProfile.reportAction.subDeviceActionMotor = "motor";
        weChatProfile.reportAction.motorActionStatus = 1; /* 1: motor on */
    } else if (g_ligthStatus == HI_FALSE) {
        weChatProfile.reportAction.subDeviceActionMotor = "motor";
        weChatProfile.reportAction.motorActionStatus = 0; /* 0: motor off */
    } else {
        weChatProfile.reportAction.subDeviceActionMotor = "motor";
        weChatProfile.reportAction.motorActionStatus = 0; /* 0: motor off */
    }
    /* profile report */
    IoTProfilePropertyReport(CONFIG_USER_ID, &weChatProfile);
}

// < this is the demo main task entry,here we will set the wifi/cjson/mqtt ready and
// < wait if any work to do in the while
static hi_void *DemoEntry(const char *arg, char *param)
{
    /* LED初始化 */
    IoSetFunc(IOT_IO_NAME_GPIO_13, IOT_IO_FUNC_GPIO_13_I2C0_SDA);
    IoSetFunc(IOT_IO_NAME_GPIO_14, IOT_IO_FUNC_GPIO_14_I2C0_SCL);
    IoTI2cInit(INS5902_I2C_IDX, IOT_I2C_IDX_BAUDRATE);
    IoTI2cSetBaudrate(INS5902_I2C_IDX, IOT_I2C_IDX_BAUDRATE);
    /* ssd1306 config init */
    OledInit();
    OledFillScreen(0);

    /* DHT11初始化 */
    while(DHT11_Init())
	{
		printf("DHT11 Init Error!!\r\n");
 		usleep(100000);
	}		
    printf("DHT11 Init Successful!!\r\n");

    /* MOTOR初始化 */
    // 初始化GPIO
    IoTGpioInit(MOTOR_GPIO_1);
    IoTGpioInit(MOTOR_GPIO_2);
    // 设置GPIO为输出方向
    IoTGpioSetDir(MOTOR_GPIO_1, IOT_GPIO_DIR_OUT);
    IoTGpioSetDir(MOTOR_GPIO_2, IOT_GPIO_DIR_OUT);

    /* 食物检测 */
    FoodInit();

    WifiStaReadyWait();
    cJsonInit();
    IoTMain();
    /* 云端下发回调 */
    IoTSetMsgCallback(DemoMsgRcvCallBack);
    /* 主动上报 */
#ifdef TAKE_THE_INITIATIVE_TO_REPORT
    while (1) {
        /* 用户可以在这调用发布函数进行发布，需要用户自己写调用函数 */
        /* DHT11模块 */
        DHT11_Read_Data(&temperature,&humidity);
        OledShowString(1, 1, "temperature", 1);
        OledShowString(1, 3, itoa(temperature,string,10), 1);
        OledShowString(1, 5, "humidity", 1);
        OledShowString(1, 7, itoa(humidity,string,10), 1); 

        // 自动调整温湿度
        // if(humidity > 80){
        //     // g_motorStatus = HI_TRUE;
        //     //风扇1
        //     hi_io_set_func(HI_IO_NAME_GPIO_6, HI_IO_FUNC_GPIO_6_GPIO);
        //     hi_gpio_set_dir(HI_GPIO_IDX_6, HI_GPIO_DIR_OUT);
        //     hi_gpio_set_ouput_val(HI_GPIO_IDX_6, 1);
        //     //风扇2
        //     hi_io_set_func(HI_IO_NAME_GPIO_7, HI_IO_FUNC_GPIO_7_GPIO);
        //     hi_gpio_set_dir(HI_GPIO_IDX_7, HI_GPIO_DIR_OUT);
        //     hi_gpio_set_ouput_val(HI_GPIO_IDX_7, 1);
        // }
        // else if(humidity < 70){
        //     //风扇1
        //     hi_io_set_func(HI_IO_NAME_GPIO_6, HI_IO_FUNC_GPIO_6_GPIO);
        //     hi_gpio_set_dir(HI_GPIO_IDX_6, HI_GPIO_DIR_OUT);
        //     hi_gpio_set_ouput_val(HI_GPIO_IDX_6, 0);
        //     //风扇2
        //     hi_io_set_func(HI_IO_NAME_GPIO_7, HI_IO_FUNC_GPIO_7_GPIO);
        //     hi_gpio_set_dir(HI_GPIO_IDX_7, HI_GPIO_DIR_OUT);
        //     hi_gpio_set_ouput_val(HI_GPIO_IDX_7, 0);
        // }

        printf("test start\n");
        // printf("MOTOR_GPIO_1: ", MOTOR_GPIO_1, "--------", "MOTOR_GPIO_2: ", MOTOR_GPIO_2, "\n");
        printf("Temperature: %d, Humidity: %d, detectStart: %d\n", temperature, humidity, detectStart, "\n");
        printf("test end\n");
        IotPublishSample(); // 发布例程
		
        /* UART模块 */
        hi_u8 uartBuff[UART_BUFF_SIZE] = {0};
        hi_unref_param(param);
        printf("Initialize uart demo successfully, please enter some datas via DEMO_UART_NUM port...\n");
        Uart1GpioCOnfig();
        for (;;) {
            uartDefConfig.g_uartLen = IoTUartRead(DEMO_UART_NUM, uartBuff, UART_BUFF_SIZE);
            mysend();
            if ((uartDefConfig.g_uartLen > 0) && (uartBuff[0] == 0xaa) && (uartBuff[1] == 0x55)) {
                if (GetUartConfig(UART_RECEIVE_FLAG) == HI_FALSE) {
                    (void)memcpy_s(uartDefConfig.g_receiveUartBuff, uartDefConfig.g_uartLen,
                        uartBuff, uartDefConfig.g_uartLen);
                    (void)SetUartRecvFlag(UART_RECV_TRUE);
                }
                // usleep(100000);
                TaskMsleep(1000);
                //更新每种食物的数量&存储日期&剩余保质期
                //TODO:更新食物的<存储日期>和<常规保质期>（待测试）
                printf("category[0]:%x",category[0],"\n");
                printf("category[1]:%x",category[1],"\n");
                printf("category[2]:%x",category[2],"\n");
                if (category[0]!=0){
                    for(int i = 0; i < 30; i++){
                        switch (category[i]){
                            case 97:{
                                //苹果
                                printf("list-----category:%x,num:%x\n,",category[i],number[i]);
                                food[0].quantity = number[i];
                                food[0].period = 7;
                                strcpy_s(food[0].storageDate, sizeof(dateStr) + 1, getCurrentDateString());
                                printf("food[0].storageDate:%s\n",food[0].storageDate);
                                break;
                            }
                            case 98:{
                                //香蕉
                                food[1].quantity = number[i];
                                food[1].period = 3;
                                strcpy_s(food[1].storageDate, sizeof(dateStr) + 1, getCurrentDateString());
                                printf("food[0].storageDate:%s\n",food[1].storageDate);
                                break;
                            }
                            case 99:{
                                //柠檬
                                food[2].quantity = number[i];
                                food[2].period = 10;
                                strcpy_s(food[2].storageDate, sizeof(dateStr) + 1, getCurrentDateString());
                                printf("food[0].storageDate:%s\n",food[2].storageDate);
                                break;
                            }
                            case 100:{
                                //胡萝卜
                                food[3].quantity = number[i];
                                food[3].period = 15;
                                strcpy_s(food[3].storageDate, sizeof(dateStr) + 1, getCurrentDateString());
                                printf("food[0].storageDate:%s\n",food[3].storageDate);
                                break;
                            }
                            case 103:{
                                //白萝卜
                                food[4].quantity = number[i];
                                food[4].period = 15;
                                strcpy_s(food[4].storageDate, sizeof(dateStr) + 1, getCurrentDateString());
                                printf("food[0].storageDate:%s\n",food[4].storageDate);
                                break;
                            }
                            case 102:{
                                //青椒
                                food[5].quantity = number[i];
                                food[5].period = 10;
                                strcpy_s(food[5].storageDate, sizeof(dateStr) + 1, getCurrentDateString());
                                printf("food[0].storageDate:%s\n",food[5].storageDate);
                                break;
                            }
                            case 101:{
                                //茄子
                                food[6].quantity = number[i];
                                food[6].period = 10;
                                strcpy_s(food[6].storageDate, sizeof(dateStr) + 1, getCurrentDateString());
                                printf("food[0].storageDate:%s\n",food[6].storageDate);
                                break;
                            }
                            case 104:{
                                //土豆
                                food[7].quantity = number[i];
                                food[7].period = 30;
                                strcpy_s(food[7].storageDate, sizeof(dateStr) + 1, getCurrentDateString());
                                printf("food[0].storageDate:%s\n",food[7].storageDate);
                                break;
                            }
                            default:{
                                printf("category[i]:%s",category[i],"\n");
                                break;
                            }
                        }
                    }}
            }
            else{
                break;
            }

            // if(count_rec!=0){
            // printf("main:category:%c\n",category[count_rec-1]);
            // printf("main:number:%x\n", number[count_rec-1]);
            // }
            TaskMsleep(20); /* 20:sleep 20ms */
        }

        /* 食物检测 */
        FoodTask();
        /* 检查食物是否过期 */
        FoodCheck();

#endif
        TaskMsleep(ONE_SECOND);
    }
    return NULL;
}

// < This is the demo entry, we create a task here,
// and all the works has been done in the demo_entry
#define CN_IOT_TASK_STACKSIZE  0x1000
#define CN_IOT_TASK_PRIOR 25
#define CN_IOT_TASK_NAME "IOTDEMO"

static void AppDemoIot(void)
{
    /* UART模块 */
    hi_u32 ret = 0;

    IotUartAttribute uartAttr = {
        .baudRate = 115200, /* baudRate: 115200 */
        .dataBits = 8, /* dataBits: 8bits */
        .stopBits = 1, /* stop bit */
        .parity = 0,
    };
    /* Initialize uart driver */
    ret = IoTUartInit(DEMO_UART_NUM, &uartAttr);
    if (ret != HI_ERR_SUCCESS) {
        printf("Failed to init uart! Err code = %d\n", ret);
        return;
    }

    /* Create a task to handle iot */
    osThreadAttr_t attr;
    IoTWatchDogDisable();

    attr.name = "IOTDEMO";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = CN_IOT_TASK_STACKSIZE;
    attr.priority = CN_IOT_TASK_PRIOR;

    /* Create a task to handle uart communication */
    osThreadAttr_t attr_UART = {0};
    attr_UART.stack_size = UART_DEMO_TASK_STAK_SIZE;
    attr_UART.priority = UART_DEMO_TASK_PRIORITY;
    attr_UART.name = (hi_char*)"uart demo";

    if (osThreadNew((osThreadFunc_t)DemoEntry, NULL, &attr) == NULL) {
        printf("[mqtt] Falied to create IOTDEMO!\n");
        printf("Falied to create uart demo task!\n");
        printf("Failed to create Motor Task!\n");
    }
}

SYS_RUN(AppDemoIot);