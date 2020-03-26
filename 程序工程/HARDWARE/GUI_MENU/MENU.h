#ifndef __MENU_H__
#define __MENU_H__

#include "sys.h"

extern short Temperature_DEV;//带符号的整数
extern short Temperature_MCU;


//ADKey状态
extern u8 KeyPos;//按键标志，
#define Key_Null 0
#define Key_Down 1
#define Key_Left 2
#define Key_Up 3
#define Key_Center 4
#define Key_Right 5

//extern u8 is_Key_Down;//是否已经按下，用于单次有效按键
//extern u8 is_Key_Left;
//extern u8 is_Key_Up;
//extern u8 is_Key_Center;
//extern u8 is_Key_Right;

extern u8 is_Coarse;//是否粗调，电压电流步进由0.01改为0.1，时间由1改为10

extern u8 Key_CenterCount;//长按计数，松开清0，长按中间3S切换模式
extern u8 is_Key_Center_LongPrass;//长按标志，松开清0
extern u8 Key_UseUp;

void clearAllKeyInf(void);


void DrawPageHead(u8 *str);//页面上显示“电源模式”或者“平衡充电模式”
void DrawPowerModePage(void);//恒流恒压源的基础界面
void DrawChargeModePage(void);//平衡充的基础界面（即在基础信息界面的下面增加平衡模块信息显示）
void DrawCommonInf(void);//两个模式共有的信息界面

void Draw_Refresh_NumofBatCells(void);
void Draw_Refresh_BalanceChrInf(void);//更新平衡模块信息显示
void Draw_Refresh_ONOFF(void);//更新“运行”或“停机”状态信息
void Draw_Refresh_PowSignal(void);//在运行时模拟绿灯闪，在停机时显示红灯
void Draw_Refresh_CCCV(void);//更新“恒流/恒压中...”信息
void Draw_Refresh_CurVol(void);//更新实时读取的电流电压信息
void Draw_Refresh_Tem(void);//更新实时温度读取信息
void Draw_Refresh_RuningTime(void);//更新运行时间信息
void Draw_Refresh_All(void);//更新当前屏幕上所有变量信息

//界面操作响应
void Opera_DC_Menu(void);
void Opera_Chr_Menu(void);
void MENU_Refrash_Pos(void);

//平衡充模式菜单参数
extern u8 Menu_Chr_Pos;
#define Menu_Chr_DCDC_ONOFF 1
#define Menu_Chr_SetCur 2
#define Menu_Chr_SetTime 3

#define Menu_Chr_MaxItem 3

//电源模式菜单参数
extern u8 Menu_DC_Pos;
#define Menu_DC_DCDC_ONOFF 1
#define Menu_DC_SetCur 2//Set哪个，就进入哪个的控制
#define Menu_DC_SetVol 3
#define Menu_DC_SetTime 4

#define Menu_DC_MaxItem 4


//char *myitoa(int value, char *string, int radix);
//int myatoi(const char *str);

#endif

