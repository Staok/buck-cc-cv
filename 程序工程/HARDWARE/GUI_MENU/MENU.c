#include "MENU.h"
#include "lcd.h"
#include "GUI.h"
#include "BG.h"
#include "SYS_INF_CTRL.h"
#include "ds18b20.h"
#include "tsensor.h"
#include <string.h>
#include "SelfChecking.h"
#include "timer.h"
#include "stdio.h"
#include "delay.h"

u8 KeyPos;

//u8 is_Key_Down;//是否已经按下，用于单次有效按键
//u8 is_Key_Left;
//u8 is_Key_Up;
//u8 is_Key_Center;
//u8 is_Key_Right;

u8 is_Coarse;//是否粗调，电压电流步进由0.01改为0.1，时间由1改为10

u8 Key_CenterCount;//长按计数，松开清0，长按中间3S切换模式
u8 is_Key_Center_LongPrass;//长按标志，松开清0
u8 Key_UseUp;

//菜单指针
u8 Menu_Chr_Pos;
u8 Menu_DC_Pos;

void clearAllKeyInf(void)
{
    KeyPos = Key_Null;
    Key_UseUp = 0;
    Key_CenterCount = 0;
}

/*********
基础界面绘制START
*********/
void DrawPageHead(u8 *str)
{
	//绘制固定栏up
	LCD_Fill(0,0,lcddev.width,20,BLUE);
	//绘制固定栏down
	LCD_Fill(0,lcddev.height-20,lcddev.width,lcddev.height,BLUE);
	//POINT_COLOR=WHITE;
	Gui_StrCenter(0,2,WHITE,BLUE,str,16,1);//居中显示
	Gui_StrCenter(0,lcddev.height-18,WHITE,BLUE,"Staok恒压恒流源",16,1);//居中显示
	//绘制测试区域
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,BLACK);
}

void DrawPowerModePage(void)
{
	DrawPageHead("电源模式");
	Gui_Drawbmp16(70,260,gImage_BG);
	DrawCommonInf();
}

void DrawChargeModePage()
{
	DrawPageHead("平衡充电模式");
	Gui_Drawbmp16(70,260,gImage_BG);
	DrawCommonInf();
	Draw_Refresh_NumofBatCells();
	POINT_COLOR=WHITE;
	Show_Str(10,20+10+16+10+16+10+32+32+20+16+10+16+10,WHITE,BLACK,"电池类型:LiPO",16,1);
	Show_Str(10+16*4+5*8+10,20+10+16+10+16+10+32+32+20+16+10+16+10,WHITE,BLACK,"电池节数:",16,1);
	POINT_COLOR=BLUE;
	LCD_DrawRectangle(6,20+10+16+10+16+10+32+32+20+16+10+16+10-2,lcddev.width-6,20+10+16+10+16+10+32+32+20+16+10+16+10+16+2);
	
	Draw_Refresh_BalanceChrInf();
	Gui_StrCenter(0,20+10+16+10+16+10+32+32+20+16+10+16+10+16+5+16+2+16+2,RED,BLACK,"[1]  [2]  [3]  [4]",16,1);
	
}

void DrawCommonInf(void)
{
	POINT_COLOR = WHITE;
	LCD_DrawRectangle(6,20+10-2,lcddev.width-6,20+10+16+2);
	LCD_DrawRectangle(6,20+10+16+10-2,lcddev.width-6,20+10+16+10+16+2);
	
	Draw_Refresh_ONOFF();
	Draw_Refresh_PowSignal();
	Draw_Refresh_CCCV();
	
	//实时和设定电压电流的显示框架,不用变
	Show_Str(10,20+10+16+10+16+10,RED,BLACK,"电流",32,1);//82
	Show_Str(10,20+10+16+10+16+10+32+10,RED,BLACK,"电压",32,1);//124
	POINT_COLOR=RED;
	BACK_COLOR=BLACK;
//	LCD_ShowString(10+32+32+8+8*4+8,20+10+16+10+16+10+8,16,"A",1);
//	LCD_ShowString(10+32+32+8+8*4+8,20+10+16+10+16+10+32+10+8,16,"V",1);
	LCD_DrawRectangle(6,20+10+16+10+16+10-2,10+32+32+8+8*4+8+8+5,20+10+16+10+16+10+32+10+32+2);
//	POINT_COLOR=RED;
//	BACK_COLOR=BLACK;
//	LCD_ShowString(10+32+32+8+8*4+8+28+8*4+8,20+10+16+10+16+10+8,16,"A",1);
//	LCD_ShowString(10+32+32+8+8*4+8+28+4*8+8,20+10+16+10+16+10+32+10+8,16,"V",1);
	Show_Str(10+32+32+8+8*4+8+28+8*3+16,111,RED,BLACK,"设定",16,1);
	LCD_DrawRectangle(10+32+32+8+8*4+8+28-5,20+10+16+10+16+10-2,lcddev.width-6,20+10+16+10+16+10+32+10+32+2);
	Draw_Refresh_CurVol();
	
	//
	POINT_COLOR=WHITE;
	Show_Str(10,20+10+16+10+16+10+32+32+20,WHITE,BLACK,"温度:",16,1);
	POINT_COLOR=BLUE;
	LCD_DrawRectangle(6,20+10+16+10+16+10+32+32+20-2,lcddev.width-6,20+10+16+10+16+10+32+32+20+16+2);
	Draw_Refresh_Tem();
	
	//
	POINT_COLOR=WHITE;
	Show_Str(10,20+10+16+10+16+10+32+32+20+16+10,WHITE,BLACK,"运行时间:",16,1);
	POINT_COLOR=BLUE;
	LCD_DrawRectangle(6,20+10+16+10+16+10+32+32+20+16+10-2,lcddev.width-6,20+10+16+10+16+10+32+32+20+16+10+16+2);
	Draw_Refresh_RuningTime();
}
/*********
基础界面绘制END
*********/



/*********
区块信息更新START
*********/
void Draw_Refresh_NumofBatCells(void)//OK
{
	char BatCells_STR[1];
//	myitoa(DCDC_Val.NumofBatCells,BatCells_STR,10);
    sprintf(BatCells_STR,"%1d",DCDC_Val.NumofBatCells);
	POINT_COLOR=WHITE;
	LCD_ShowString(10+16*4+5*8+10+4*16+8+10,20+10+16+10+16+10+32+32+20+16+10+16+10,16,(u8*)BatCells_STR,0); //0~4
}

void Draw_Refresh_BalanceChrInf(void)//OK
{
//	char BATCellVol_1_int_srt[1],BATCellVol_1_tail_srt[2];//四个电池的电压整数和小数部分的字符串
//	char BATCellVol_2_int_srt[1],BATCellVol_2_tail_srt[2];
//	char BATCellVol_3_int_srt[1],BATCellVol_3_tail_srt[2];
//	char BATCellVol_4_int_srt[1],BATCellVol_4_tail_srt[2];
//    int tail_1,tail_2,tail_3,tail_4;//存放小数的100倍数
	
	char CELL_DISCHR_SHOW_STR[18]={'\0'};//放电状态和电池电压一行字符串
	char CELL_VOL_SHOW_STR[27]={'\0'};
	
//	myitoa((int)(DCDC_Val.BATCellVol_1),BATCellVol_1_int_srt,10);//分别整数和小数部分转换为字符串
//    tail_1 = ((int)(DCDC_Val.BATCellVol_1*100))%100;
//	myitoa(tail_1,BATCellVol_1_tail_srt,10);
//	
//	strcat(CELL_VOL_SHOW_STR,BATCellVol_1_int_srt);//电压信息字符串组合
//	strcat(CELL_VOL_SHOW_STR,".");
//    if(tail_1>=10){strcat(CELL_VOL_SHOW_STR,BATCellVol_1_tail_srt);
//    }else{  strcat(CELL_VOL_SHOW_STR,"0");
//            strcat(CELL_VOL_SHOW_STR,BATCellVol_1_tail_srt);}
//	strcat(CELL_VOL_SHOW_STR,"V ");

	if(DCDC_Val.BATCellDisChr_1)//放电开关信息字符串组合
	{
		strcat(CELL_DISCHR_SHOW_STR,"ON  ");
	}else{strcat(CELL_DISCHR_SHOW_STR,"OFF  ");}
	
	if(DCDC_Val.BATCellDisChr_2)
	{
		strcat(CELL_DISCHR_SHOW_STR,"ON  ");
	}else{strcat(CELL_DISCHR_SHOW_STR,"OFF  ");}
	
	if(DCDC_Val.BATCellDisChr_3)
	{
		strcat(CELL_DISCHR_SHOW_STR,"ON  ");
	}else{strcat(CELL_DISCHR_SHOW_STR,"OFF  ");}
	
	if(DCDC_Val.BATCellDisChr_4)
	{
		strcat(CELL_DISCHR_SHOW_STR,"ON");
	}else{strcat(CELL_DISCHR_SHOW_STR,"OFF");}
    //四个电压均应在4.20以内
	sprintf(CELL_VOL_SHOW_STR,"%.2fV %.2fV %.2fV %.2fV",DCDC_Val.BATCellVol_1,\
    DCDC_Val.BATCellVol_2,DCDC_Val.BATCellVol_3,DCDC_Val.BATCellVol_4);
    
	Gui_StrCenter(0,20+10+16+10+16+10+32+32+20+16+10+16+10+16+5,RED,BLACK,(u8*)CELL_DISCHR_SHOW_STR,16,0);
	Gui_StrCenter(0,20+10+16+10+16+10+32+32+20+16+10+16+10+16+5+16+2,RED,BLACK,(u8*)CELL_VOL_SHOW_STR,16,0);
}

void Draw_Refresh_ONOFF(void)//OK
{
	if(CTRL_DCDC_ON_OFF == DCDC_ON){
		Gui_StrCenter(0,30,GREEN,BLACK,"状态:运行",16,0);}else{
		Gui_StrCenter(0,30,RED,BLACK,"状态:停机",16,0);}
}

void Draw_Refresh_PowSignal(void)//OK
{
	static u8 _inverse = 1;
	if(CTRL_DCDC_ON_OFF == DCDC_ON)//运行中
	{
		if(_inverse){
			_inverse=!_inverse;
			gui_circle(lcddev.width-20,38,GREEN,5,1);
		}else{
			_inverse=!_inverse;
			gui_circle(lcddev.width-20,38,BLACK,5,1);
			gui_circle(lcddev.width-20,38,WHITE,5,0);
		}
	}else{
		gui_circle(lcddev.width-20,38,BLACK,5,1);
		gui_circle(lcddev.width-20,38,WHITE,5,0);
		gui_circle(lcddev.width-20,38,RED,4,1);
	}
}

void Draw_Refresh_CCCV(void)//OK
{
	//char ERROR_CODE_STR[1];
	char ERROR_CODE_SHOW_STR[9]={'\0'};
	if(CTRL_DCDC_CC_CV == DCDC_CV){
		Gui_StrCenter(0,56,WHITE,BLACK,"恒压",16,0);}else{
		Gui_StrCenter(0,56,WHITE,BLACK,"恒流",16,0);}
    sprintf(ERROR_CODE_SHOW_STR,"ErrCode:%1d",Error_code);
//	myitoa(Error_code,ERROR_CODE_STR,10);
//	strcat(ERROR_CODE_SHOW_STR,"ErrCode:");
//	strcat(ERROR_CODE_SHOW_STR,ERROR_CODE_STR);
	Show_Str(lcddev.width-90,56,WHITE,BLACK,(u8*)ERROR_CODE_SHOW_STR,16,0);
}

void Draw_Refresh_CurVol(void)//OK
{
//    char BAT_Cur_int_srt[1],BAT_Cur_tail_srt[2];//电流和电压整数和小数部分的字符串
//	char BAT_Vol_int_srt[1],BAT_Vol_tail_srt[2];
//	char BAT_SetCur_int_srt[1],BAT_SetCur_tail_srt[2];
//	char BAT_SetVol_int_srt[1],BAT_SetVol_tail_srt[2];
	char BAT_Cur_srt[5]={'\0'};//->0.00~4.00A
    char BAT_Vol_srt[5]={'\0'};//->0.00~9.99~20.0V
    char BAT_SetCur_srt[5]={'\0'};//->0.00~4.00A
    char BAT_SetCurSup_srt[5]={'\0'};
    char BAT_CurLim_srt[5]={'\0'};
    char BAT_SetVol_srt[5]={'\0'};//->0.00~9.99~20.0V
    char BAT_ChrVol_srt[5]={'\0'};//
	
//	myitoa((int)(DCDC_Val.Cur),BAT_Cur_int_srt,10);//分别整数和小数部分转换为字符串
//	myitoa(((int)(DCDC_Val.Cur*100))%100,BAT_Cur_tail_srt,10);
//	
//	myitoa((int)(DCDC_Val.SetCur),BAT_SetCur_int_srt,10);
//	myitoa(((int)(DCDC_Val.SetCur*100))%100,BAT_Vol_tail_srt,10);
//	
//    if(DCDC_Val.Vol >= 10.0)
//    {
//        myitoa((int)(DCDC_Val.Vol),BAT_Vol_int_srt,10);
//        myitoa((((int)(DCDC_Val.Vol*100))%100)%10,BAT_Vol_tail_srt,10);//小数只取一位
//    }else
//    {
//        myitoa((int)(DCDC_Val.Vol),BAT_Vol_int_srt,10);
//        myitoa(((int)(DCDC_Val.Vol*100))%100,BAT_Vol_tail_srt,10);//小数取两位
//    }
//	strcat(BAT_Cur_srt,BAT_Cur_int_srt);//字符串组合
//    strcat(BAT_Cur_srt,".");
//    strcat(BAT_Cur_srt,BAT_Cur_tail_srt);
    
    sprintf(BAT_Cur_srt,"%.2fA",DCDC_Val.Cur);//->0.00~4.00A
    sprintf(BAT_SetCur_srt,"%.2fA",DCDC_Val.SetCur);
    
//    if(DCDC_Val.Vol >= 10.0)
//    {
//        sprintf(BAT_Vol_srt,"%.1f",DCDC_Val.Vol);//->0.00~9.99~20.00V
//    }else{  
    sprintf(BAT_Vol_srt,"%.2fV ",DCDC_Val.Vol);
    
//    if(DCDC_Val.SetVol >= 10.0)
//    {
//        sprintf(BAT_SetVol_srt,"%.1f",DCDC_Val.SetVol);
//    }else{  
    sprintf(BAT_SetVol_srt,"%.2fV ",DCDC_Val.SetVol);
    sprintf(BAT_ChrVol_srt,"%.2fV ",DCDC_Val.ChrVol);
    sprintf(BAT_SetCurSup_srt,"%.2fV",FinishCur);
    
    //实时电流电压正确显示
	POINT_COLOR=WHITE;
	LCD_ShowString(10+32+32+8,20+10+16+10+16+10+8,16,(u8*)BAT_Cur_srt,0);//实时电流值，四位
	LCD_ShowString(10+32+32+8,20+10+16+10+16+10+32+10+8,16,(u8*)BAT_Vol_srt,0);//实时电压值，四位
    
    //设定电流正确显示
    if(CTRL_DCDC_Mode == DCDC_Mode_Chr)
    {
        if(SYS_Chr_State == Chr_State_sup)
        {
            LCD_ShowString(10+32+32+8+8*4+8+28,20+10+16+10+16+10+8,16,(u8*)BAT_SetCurSup_srt,0);//设定电流值，四位
        }else if((SYS_Chr_State == Chr_State_CC)||(SYS_Chr_State == Chr_State_trickle)){
            LCD_ShowString(10+32+32+8+8*4+8+28,20+10+16+10+16+10+8,16,(u8*)BAT_SetCur_srt,0);//设定电流值，四位
        }else if(SYS_Chr_State == Chr_State_CV){
            sprintf(BAT_CurLim_srt,"%.2fV",DCDC_Val.Cur_Limit);
            LCD_ShowString(10+32+32+8+8*4+8+28,20+10+16+10+16+10+8,16,(u8*)BAT_CurLim_srt,0);
        }
    }else if(CTRL_DCDC_Mode == DCDC_Mode_DC)
    {
        LCD_ShowString(10+32+32+8+8*4+8+28,20+10+16+10+16+10+8,16,(u8*)BAT_SetCur_srt,0);
    }
    
    //设定电压正确显示
    if(CTRL_DCDC_Mode == DCDC_Mode_Chr)
    {
        if(SYS_Chr_State == Chr_State_Finish){
            LCD_ShowString(10+32+32+8+8*4+8+28,20+10+16+10+16+10+32+10+8,16,"0.00",0);//设定电压值，四位
        }else{
            LCD_ShowString(10+32+32+8+8*4+8+28,20+10+16+10+16+10+32+10+8,16,(u8*)BAT_ChrVol_srt,0);//设定电压值，四位
        }
    }else if(CTRL_DCDC_Mode == DCDC_Mode_DC)
    {
        LCD_ShowString(10+32+32+8+8*4+8+28,20+10+16+10+16+10+32+10+8,16,(u8*)BAT_SetVol_srt,0);//设定电压值，四位
    }
}

void Draw_Refresh_Tem(void)//OK,要打开DS18B20
{
	short Temperature_DEV_tail;
	//short Temperature_MCU_tail;
	char DEV_t[10]={'\0'};
    //char MCU_t[10]={'\0'};
	//Temperature_DEV = 28;
	//Temperature_MCU = 25;
    //Temperature_DEV_tail = 8;
    //Temperature_MCU_tail = 8;
	if(Error_code != Error_ds18b20_isn_init)
	{
		Temperature_DEV = (DS18B20_Get_Temp()/10);//每100ms读取电池组温度，原函数返回百位数
		Temperature_DEV_tail = (DS18B20_Get_Temp()%10);//得到第一位小数
	}
	//Temperature_MCU = (Get_Temprate()/100);//得到MCU温度值的整数部分（内部温敏只表大概），原函数返回千位数
	//Temperature_MCU_tail = (Get_Temprate()%100)%10;//得到第一位小数

	sprintf(DEV_t,"DEV %2d.%1d℃",Temperature_DEV,Temperature_DEV_tail);
    //sprintf(MCU_t,"MCU %2d.%1d℃",Temperature_MCU,Temperature_MCU_tail);
	Show_Str(10+16*2+8+10,20+10+16+10+16+10+32+32+20,BLACK,WHITE,(u8*)DEV_t,16,0);//叠加覆盖式显示新数据
	//Show_Str(10+16*2+8+10+8*8+16+10,20+10+16+10+16+10+32+32+20,WHITE,BLACK,(u8*)MCU_t,16,0);
}

void Draw_Refresh_RuningTime(void)//OK
{
//	char RuningMins_STR[3],SET_MAX_RuningMins_STR[3];
	char RuningTime_SHOW_STR[13] = {'\0'};
	
//	myitoa(Timer_IT_flags._1min,RuningMins_STR,10);
//	myitoa(SetMaxRuningMins,SET_MAX_RuningMins_STR,10);//设定上限600min
//	
//	strcat(RuningTime_SHOW_STR,RuningMins_STR);
//	strcat(RuningTime_SHOW_STR,"min/");
//	strcat(RuningTime_SHOW_STR,SET_MAX_RuningMins_STR);
//	strcat(RuningTime_SHOW_STR,"min");
    
    sprintf(RuningTime_SHOW_STR,"%dmin/%dmin",Timer_IT_flags._1min,SetMaxRuningMins);
    
	POINT_COLOR=WHITE;
	LCD_ShowString(10+16*4+8+20,20+10+16+10+16+10+32+32+20+16+10,16,(u8*)RuningTime_SHOW_STR,0);
}

void Draw_Refresh_All(void)//OK
{
	switch(CTRL_DCDC_Mode)
	{
		case DCDC_Mode_DC:
			Draw_Refresh_ONOFF();
			Draw_Refresh_CurVol();
			Draw_Refresh_CCCV();
			Draw_Refresh_Tem();
			Draw_Refresh_RuningTime();
			break;
		
		case DCDC_Mode_Chr:
			Draw_Refresh_ONOFF();
			Draw_Refresh_CurVol();
			Draw_Refresh_CCCV();
			Draw_Refresh_Tem();
			Draw_Refresh_RuningTime();
			Draw_Refresh_BalanceChrInf();
			Draw_Refresh_NumofBatCells();
			break;
	}
}
/*********
区块信息更新END
*********/

/*********
界面操作响应START
*********/
//定时器标志位周期性进入，在主while里面和按键读取放一起
void Opera_DC_Menu(void)
{
    if((CTRL_DCDC_ON_OFF == DCDC_ON)&&(Error_code == 0))
    {
        Sys_AutoRun_DC_Output();
    }
    
	if(KeyPos == Key_Center)
    {
        Key_UseUp = 1;
        KeyPos = Key_Null;
        is_Coarse = !is_Coarse;
    }
    
    if(KeyPos == Key_Down)
    {
        Key_UseUp = 1;
        KeyPos = Key_Null;
        
        if(++Menu_DC_Pos > Menu_DC_MaxItem)
        {
            Menu_DC_Pos = 1;
        }
        MENU_Refrash_Pos();
    }
    
    if(KeyPos == Key_Up)
    {
        Key_UseUp = 1;
        KeyPos = Key_Null;
        
        if(--Menu_DC_Pos < 1)
        {
            Menu_DC_Pos = Menu_DC_MaxItem;
        }
        MENU_Refrash_Pos();
    }
    
    if(KeyPos == Key_Left)
    {
        Key_UseUp = 1;
        KeyPos = Key_Null;
        switch(Menu_DC_Pos)
        {
            case Menu_DC_DCDC_ONOFF:
                if(CTRL_DCDC_ON_OFF == DCDC_OFF)
                {CTRL_DCDC_ON_OFF = DCDC_ON;}else{CTRL_DCDC_ON_OFF = DCDC_OFF;ShutSecondNow = Timer_IT_flags._1sec;}
                break;
            case Menu_DC_SetCur:
                CTRL_DCDC_CC_CV = DCDC_CC;//动电流立马设为恒流模式
                //DCDC_Val.SetVol = DCDC_Val.Vol_Limit;
                if(is_Coarse)
                {DCDC_Val.SetCur -= 0.1;}else{DCDC_Val.SetCur -= 0.01;}
                //if(DCDC_Val.SetCur > 3){DCDC_Val.SetCur = 3.0;}
                if(DCDC_Val.SetCur < DCDC_Val.Cur_MinLimit){DCDC_Val.SetCur = DCDC_Val.Cur_MinLimit;} 
                break;
            case Menu_DC_SetVol:
                CTRL_DCDC_CC_CV = DCDC_CV;//动电压立马设为恒压模式
                //DCDC_Val.SetCur = DCDC_Val.Cur_Limit;
                if(is_Coarse)
                {DCDC_Val.SetVol -= 0.1;}else{DCDC_Val.SetVol -= 0.01;}
                if(DCDC_Val.SetVol < DCDC_Val.Vol_MinLimit){DCDC_Val.SetVol = DCDC_Val.Vol_MinLimit;}
                break;
            case Menu_DC_SetTime:
                if(is_Coarse)
                {SetMaxRuningMins -= 10;}else{SetMaxRuningMins -= 1;}
                //if(SetMaxRuningMins > MaxRuningMins){SetMaxRuningMins = MaxRuningMins;}
                if(SetMaxRuningMins < (Timer_IT_flags._1min+5)){SetMaxRuningMins = Timer_IT_flags._1min+5;}
                break;
            default:break;
        }
    }
    
    if(KeyPos == Key_Right)
    {
        Key_UseUp = 1;
        KeyPos = Key_Null;
        switch(Menu_DC_Pos)
        {
            case Menu_DC_DCDC_ONOFF:
                if(CTRL_DCDC_ON_OFF == DCDC_OFF)
                {CTRL_DCDC_ON_OFF = DCDC_ON;}else{CTRL_DCDC_ON_OFF = DCDC_OFF;ShutSecondNow = Timer_IT_flags._1sec;}
                break;
            case Menu_DC_SetCur:
                CTRL_DCDC_CC_CV = DCDC_CC;//动电流立马设为恒流模式
                //DCDC_Val.SetVol = DCDC_Val.Vol_Limit;
                if(is_Coarse)
                {DCDC_Val.SetCur += 0.1;}else{DCDC_Val.SetCur += 0.01;}
                //if(DCDC_Val.SetCur > 3){DCDC_Val.SetCur = 3.0;}
                if(DCDC_Val.SetCur > DCDC_Val.Cur_Limit){DCDC_Val.SetCur = DCDC_Val.Cur_Limit;} 
                break;
            case Menu_DC_SetVol:
                CTRL_DCDC_CC_CV = DCDC_CV;//动电流立马设为恒流模式
                //DCDC_Val.SetCur = DCDC_Val.Cur_Limit;
                if(is_Coarse)
                {DCDC_Val.SetVol += 0.1;}else{DCDC_Val.SetVol += 0.01;}
                if(DCDC_Val.SetVol > DCDC_Val.Vol_Limit){DCDC_Val.SetVol = DCDC_Val.Vol_Limit;}
                break;
            case Menu_DC_SetTime:
                if(is_Coarse)
                {SetMaxRuningMins += 10;}else{SetMaxRuningMins += 1;}
                if(SetMaxRuningMins > MaxRuningMins){SetMaxRuningMins = MaxRuningMins;}
                break;
            default:break;
        }
    }
    

}

void Opera_Chr_Menu(void)
{
	if((CTRL_DCDC_ON_OFF == DCDC_ON)&&(Error_code == 0))
    {
        Sys_AutoRun_Chr_Sequence();
    }
    if(KeyPos == Key_Center)
    {
        Key_UseUp = 1;
        KeyPos = Key_Null;
        is_Coarse = !is_Coarse;
    }
    
    if(KeyPos == Key_Down)
    {
        Key_UseUp = 1;
        KeyPos = Key_Null;
        
        if(++Menu_Chr_Pos > Menu_Chr_MaxItem)
        {
            Menu_Chr_Pos = 1;
        }
        MENU_Refrash_Pos();
    }
    
    if(KeyPos == Key_Up)
    {
        Key_UseUp = 1;
        KeyPos = Key_Null;
        
        if(--Menu_Chr_Pos < 1)
        {
            Menu_Chr_Pos = Menu_Chr_MaxItem;
        }
        MENU_Refrash_Pos();
    }
    
    if(KeyPos == Key_Left)
    {
        Key_UseUp = 1;
        KeyPos = Key_Null;
        switch(Menu_Chr_Pos)
        {
            case Menu_Chr_DCDC_ONOFF:
                if(DCDC_Val.NumofBatCells != 0){
                    if(CTRL_DCDC_ON_OFF == DCDC_OFF)
                    {CTRL_DCDC_ON_OFF = DCDC_ON;}else{CTRL_DCDC_ON_OFF = DCDC_OFF;ShutSecondNow = Timer_IT_flags._1sec;}
                }
                break;
            case Menu_Chr_SetCur:
                if(CTRL_DCDC_ON_OFF == DCDC_OFF)
                {
                    if(is_Coarse)
                    {DCDC_Val.SetCur -= 0.1;}else{DCDC_Val.SetCur -= 0.01;}
                    //if(DCDC_Val.SetCur > 3){DCDC_Val.SetCur = 3.0;}
                    if(DCDC_Val.SetCur < DCDC_Val.Cur_MinLimit){DCDC_Val.SetCur = DCDC_Val.Cur_MinLimit;} 
                }
                break;
            case Menu_Chr_SetTime:
                if(is_Coarse)
                {SetMaxRuningMins -= 10;}else{SetMaxRuningMins -= 1;}
                //if(SetMaxRuningMins > MaxRuningMins){SetMaxRuningMins = MaxRuningMins;}
                if(SetMaxRuningMins < (Timer_IT_flags._1min+5)){SetMaxRuningMins = Timer_IT_flags._1min+5;}
                break;
            default:break;
        }
    }
    
    if(KeyPos == Key_Right)
    {
        Key_UseUp = 1;
        KeyPos = Key_Null;
        switch(Menu_Chr_Pos)
        {
            case Menu_Chr_DCDC_ONOFF:
                if(DCDC_Val.NumofBatCells != 0){
                    if(CTRL_DCDC_ON_OFF == DCDC_OFF)
                    {CTRL_DCDC_ON_OFF = DCDC_ON;}else{CTRL_DCDC_ON_OFF = DCDC_OFF;ShutSecondNow = Timer_IT_flags._1sec;}
                }
                break;
            case Menu_Chr_SetCur:
                if(CTRL_DCDC_ON_OFF == DCDC_OFF)
                {
                    if(is_Coarse)
                    {DCDC_Val.SetCur += 0.1;}else{DCDC_Val.SetCur += 0.01;}
                    if(DCDC_Val.SetCur > DCDC_Val.Cur_Limit){DCDC_Val.SetCur = DCDC_Val.Cur_Limit;}
                    //if(DCDC_Val.SetCur < 0){DCDC_Val.SetCur = 0;}                    
                }
                break;
            case Menu_Chr_SetTime:
                if(is_Coarse)
                {SetMaxRuningMins += 10;}else{SetMaxRuningMins += 1;}
                if(SetMaxRuningMins > MaxRuningMins){SetMaxRuningMins = MaxRuningMins;}
                //if(SetMaxRuningMins < (Timer_IT_flags._1min+5)){SetMaxRuningMins = Timer_IT_flags._1min+5;}
                break;
            default:break;
        }
    }
}

void MENU_Refrash_Pos(void)
{
    switch(CTRL_DCDC_Mode)
    {
        case DCDC_Mode_DC:
            gui_circle(lcddev.width-70,38,BLACK,5,1);//全清
            gui_circle(150+55,98,BLACK,5,1);
            gui_circle(150+55,140,BLACK,5,1);
            gui_circle(102+105,200,BLACK,5,1);
            switch(Menu_DC_Pos)
            {
                case Menu_DC_DCDC_ONOFF:
                    gui_circle(lcddev.width-70,38,WHITE,5,1);//添加对应POS
                    break;
                case Menu_DC_SetCur:
                    gui_circle(150+55,98,WHITE,5,1);
                    break;
                case Menu_DC_SetVol:
                    gui_circle(150+55,140,WHITE,5,1);
                    break;
                case Menu_DC_SetTime:
                    gui_circle(102+105,200,WHITE,5,1);
                    break;
                default:break;
            }
            break;
        case DCDC_Mode_Chr:
            gui_circle(lcddev.width-70,38,BLACK,5,1);
            gui_circle(150+55,98,BLACK,5,1);
            gui_circle(102+105,200,BLACK,5,1);
            switch(Menu_Chr_Pos)
            {
                case Menu_Chr_DCDC_ONOFF:
                    gui_circle(lcddev.width-70,38,WHITE,5,1);
                    break;
                case Menu_Chr_SetCur:
                    gui_circle(150+55,98,WHITE,5,1);
                    break;
                case Menu_Chr_SetTime:
                    gui_circle(102+105,200,WHITE,5,1);
                    break;
                default:break;
            }
            break;
        default:break;
    }
        
    
}

/*********
界面操作响应END
*********/


//整形转字符，stm32不支持标准的itoa,这里自己实现
//char *myitoa(int value, char *string, int radix)
//{
//    int     i, d;
//    int     flag = 0;
//    char    *ptr = string;

//    /* This implementation only works for decimal numbers. */
//    if (radix != 10)
//    {
//        *ptr = 0;
//        return string;
//    }

//    if (!value)
//    {
//        *ptr++ = 0x30;
//        *ptr = 0;
//        return string;
//    }

//    /* if this is a negative value insert the minus sign. */
//    if (value < 0)
//    {
//        *ptr++ = '-';

//        /* Make the value positive. */
//        value *= -1;
//    }

//    for (i = 10000; i > 0; i /= 10)
//    {
//        d = value / i;

//        if (d || flag)
//        {
//            *ptr++ = (char)(d + 0x30);
//            value -= (d * i);
//            flag = 1;
//        }
//    }

//    /* Null terminate the string. */
//    *ptr = 0;

//    return string;

//} /* NCL_Itoa */

//字符串转整形，stm32不支持标准的atoi,这里自己实现
//int myatoi(const char *str)
//{
//	int s=0;
//	uint8_t falg=0;
//	
//	while(*str==' ')
//	{
//		str++;
//	}

//	if(*str=='-'||*str=='+')
//	{
//		if(*str=='-')
//		falg=1;
//		str++;
//	}

//	while(*str>='0'&&*str<='9')
//	{
//		s=s*10+*str-'0';
//		str++;
//		if(s<0)
//		{
//			s=2147483647;
//			break;
//		}
//	}
//	return s*(falg?-1:1);
//}







