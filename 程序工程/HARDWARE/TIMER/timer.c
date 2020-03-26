#include "timer.h"
#include "wdg.h"
#include "beep.h"
#include "SelfChecking.h"
#include "SYS_INF_CTRL.h"
#include "MENU.h"
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

struct TIM3_IT_FLAGS Timer_IT_flags;


#define TIM3_IT_PSC (14400-1)
#define TIM3_1MS_IT_ARR (5-1)
#define TIM3_10MS_IT_ARR (50-1)// 72Mhz/14400=0.0002s  0.0002*50=0.01s
#define TIM3_100MS_IT_ARR (500-1)

#define ARR TIM3_10MS_IT_ARR //在这里设定中断定时时间
#define PSC TIM3_IT_PSC

//通用定时器3中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器3!
void TIM3_Int_Init(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能
	
	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = ARR; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =PSC; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //先占优先级1级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  //从优先级1级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器

	Timer_IT_flags._10msec_flag = 0;
	Timer_IT_flags._100msec_flag = 0;
	Timer_IT_flags._1sec_flag = 0;
	Timer_IT_flags._10msec = 0;
	Timer_IT_flags._100msec = 0;
	Timer_IT_flags._1sec = 0;
    Timer_IT_flags._300msec_flag = 0;
    Timer_IT_flags._1min_flag = 0;
    Timer_IT_flags._1min = 0;
    _OutofTime_Running_flag = 0;

	TIM_Cmd(TIM3, ENABLE);  //使能TIMx
}


//通用定时器2中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器2!用于蜂鸣器鸣叫，暂时没啥其他作用
void TIM2_Int_Init(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //时钟使能
	
	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = 5-1; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =14400-1; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;  //先占优先级3级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器
    
    TIM_Cmd(TIM2, ENABLE);  //使能TIMx
}

u8 _OutofTime_Running_flag;
u8 run_once = 1;
//定时器3中断服务程序
void TIM3_IRQHandler(void)
{
	
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		/*********
		定时器分频START
		*********/
		Timer_IT_flags._10msec_flag = 1;
		if(++Timer_IT_flags._10msec >= 10)
		{
			Timer_IT_flags._10msec = 0;
			Timer_IT_flags._100msec_flag = 1;
			Timer_IT_flags._100msec++;
			
		}
		
		if(Timer_IT_flags._100msec % 3 == 0)
		{
			Timer_IT_flags._300msec_flag = 1;
            IWDG_Feed();//来喂狗了
		}
		
		if(Timer_IT_flags._100msec >= 10)
		{
			Timer_IT_flags._100msec = 0;
			Timer_IT_flags._1sec_flag = 1;
			Timer_IT_flags._1sec++;
		}
		
		if(Timer_IT_flags._1sec >= 60)
		{
            Timer_IT_flags._1sec = 0;
			Timer_IT_flags._1min_flag = 1;
			Timer_IT_flags._1min++;
			
			
			if((Timer_IT_flags._1min > SetMaxRuningMins)){
			Timer_IT_flags._1min = 0;_OutofTime_Running_flag = 1;
			CTRL_DCDC_ON_OFF = DCDC_OFF;}//超过时间，强制关闭
		}
		/*********
		定时器分频END
		*********/
	}
}


//定时器2中断服务程序
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    	/*********
		蜂鸣器响应START
		*********/
		if(is_buzzer_once)  //叫唤一声
		{
			if(Timer_IT_flags._100msec % 3 != 0)
			{	buzzer = ~buzzer;}else{	buzzer = 0;is_buzzer_once = 0;}
		}
		
		if(is_buzzer_bibi)  //1秒间歇叫唤
		{
			run_once = 1;
			if(Timer_IT_flags._1sec % 2 == 0)
			{
				buzzer = ~buzzer;
			}else{buzzer = 0;}  //0为关闭
		}else{
			if(run_once){	buzzer = 0;run_once = 0;}
		}
		/*********
		蜂鸣器响应END
		*********/
    }
}












