#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//定时器 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////   

__packed struct TIM3_IT_FLAGS
{
	u8 _10msec_flag;
	u16 _10msec;
	u8 _100msec_flag;
	u16 _100msec;
	u8 _300msec_flag;
	u8 _1sec_flag;
	u16 _1sec;
	u8 _1min_flag;
	u16 _1min;
};
extern struct TIM3_IT_FLAGS Timer_IT_flags;
extern u8 _OutofTime_Running_flag;
extern u8 run_once;

void TIM3_Int_Init(void);
void TIM2_Int_Init(void);
 
#endif
