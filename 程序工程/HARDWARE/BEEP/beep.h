#ifndef __BEEP_H
#define __BEEP_H	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//蜂鸣器驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/2
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 

extern u8 is_buzzer_once;
extern u8 is_buzzer_bibi;

#define buzzer_once is_buzzer_once = 1;  //蜂鸣器叫唤一声

#define Buzzer_ON 1//间歇叫唤打开
#define Buzzer_OFF 0 //间歇叫唤关闭
#define CTRL_buzzer_ON_OFF is_buzzer_bibi


#define buzzer PCout(13)	   //需在1ms定时器里面设定震荡鸣叫，注意退出之前应给低电平

void BEEP_Init(void);
		 				    
#endif

