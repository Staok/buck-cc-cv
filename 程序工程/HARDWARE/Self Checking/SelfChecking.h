#ifndef __SELFCHECKING_H
#define __SELFCHECKING_H	 
#include "sys.h"

extern short Temperature_DEV;//读出的温度有符号
extern short Temperature_MCU;

/*故障代码
0:->目前无故障；
1:->温度测量初始化失败；
2:->DCDC芯片没有正常关闭；
3:->DCDC芯片没有正常打开；
4:->电池温度过高或者MCU温度过高
*/
#define Error_ds18b20_isn_init 1
#define Error_DCDC_isn_shut 2
#define Error__reserve 3
#define Error_Tem_Over_Scope 4
#define Error_DCDC_Output_isn_Normal 5

extern u8 Error_code;
extern u16 ShutSecondNow;

//自检DCDC是否正常关闭，用于在程序关闭DCDC芯片之后的自检
//u8 IS_DCDC_OFF(void);
//自检温度是否在正常范围
u8 is_Tem_Over_Scope(void);
//自检当DCDC芯片开启时，再判断现在处于恒流还是恒压模式，再各自判断输出是否正常
u8 is_DCDC_Output_Normal(void);

#define Error_check_num 2//循环诊断里面的诊断函数项数目

void dealWith_Output_isn_Normal(void);
//void dealWith_DCDC_isn_Shut(void);
void dealWith_ds18b20_isn_init(void);
void dealWith_Tem_Over_Scope(void);

		 				    
#endif

