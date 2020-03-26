#ifndef __SYS_INF_CTRL_H__
#define __SYS_INF_CTRL_H__
#include "sys.h"

//两个温度，一个设备(DS18B20读的)，一个内部AD温度读的，
//显示的时候读取，每1s刷新，刷新显示时代小数，每300ms诊断，诊断时只用整数
extern short Temperature_DEV;//带符号的整数,小数部分在读取并显示时单独获取
extern short Temperature_MCU;
extern u16 MaxRuningMins;
extern u16 SetMaxRuningMins;
//有三个分钟时间，可设定最大时间，当下设定时间和定时器记录的运行时间

//DCDC开关
#define DCDC_ON 1
#define DCDC_OFF 0//写0关闭DCDC
#define CTRL_DCDC_ON_OFF PAout(8)

//DCDC处于电源还是充电模式
#define DCDC_Mode_DC 1
#define DCDC_Mode_Chr 0
extern u8 CTRL_DCDC_Mode;

//DCDC处于恒压还是恒流状态
#define DCDC_CC 0
#define DCDC_CV 1//写1为选择电压反馈，恒压
#define CTRL_DCDC_CC_CV PAout(1)

#define Adc_Average_times 5


//充电阶段状态
extern u8 SYS_Chr_State;
#define Chr_State_sup 1//补充充电，某节电池电压低于正常最小电压，此时不开启平衡模块
#define Chr_State_CC 2//恒流充电，每节电池电压都正常(且没有满电)，并且电池组电压小于满电电压
#define Chr_State_CV 3//恒压充电，当电池组电压达到满电电压
#define Chr_State_trickle 4//涓流充电，充满电后，等待几分钟到十几分钟，电池组电压回落到涓流充电电压阈值，再从恒流开始充电
#define Chr_State_Finish 5//满电标志，如果电池不取，则会在电压下降到检流充电阈值时再次充电

extern float LowVol_th;//补充充电阈值
extern float FinishCur;//恒压充电下，充满电时充电电流阈值
extern float TriVol_th;//涓流充电，再充电阈值

__packed struct DCDC_INF
{
	float Cur,SetCur;//实际读取值和设定值
	float Vol,SetVol,ChrVol;
    float Cur_Limit,Vol_Limit;//压流限制
    float Cur_MinLimit,Vol_MinLimit;//压流限制
	float AD5Key_Value;
	float BATCellVol_1,BATCellVol_2,BATCellVol_3,BATCellVol_4;//每节电池的电压
	u8 BATCellDisChr_1,BATCellDisChr_2,BATCellDisChr_3,BATCellDisChr_4;//每节电池是否开启放电，1为是
	u8 NumofBatCells;
};

extern struct DCDC_INF DCDC_Val;
	
void Sys_ParameterInit(void);//参数初始化
void Sys_AutoRun_Chr_Sequence(void);//——————充电程序
void Sys_AutoRun_DC_Output(void);//—————————根据恒流恒压和压流信息输出

void Sys_ReadinALLInf(void);//一次性读入所有信息


#define _373_CS PBout(1)
#define _373_EN 1
#define _373_disEN 0

#define Balance_1 PBout(8)//低
#define Balance_2 PBout(9)
#define Balance_3 PBout(10)
#define Balance_4 PBout(11)//高
#define Balance_ON 0
#define Balance_OFF 1

//通道选择
#define ch_C0 0
#define ch_C1 1
#define ch_C2 2
#define ch_C3 3
#define ch_C4 4
#define ch_Cur 6
#define ch_Vol 7

void SetDAC_Value(float DAC_Value);//设定DAC输出值
void Sel_CD4051_Ch(u8 ch);
void Balance_ctrl(void);
void decideBatCells(void);
void initMCP4822(void);




#endif

