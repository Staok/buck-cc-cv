#include "SYS_INF_CTRL.h"
#include "sys_io.h"
#include "SelfChecking.h"
#include "ds18b20.h"
#include "tsensor.h"
#include "adc.h"
#include "delay.h"
#include "MENU.h"
#include "timer.h"

short Temperature_DEV;//带符号的整数
short Temperature_MCU;
u16 MaxRuningMins;
u16 SetMaxRuningMins;

float LowVol_th;//补充充电阈值
float FinishCur;//恒压充电下，充满电时充电电流阈值
float TriVol_th;//涓流充电，再充电阈值

u8 CTRL_DCDC_Mode;
u8 SYS_Chr_State;

struct DCDC_INF DCDC_Val;

//开机默认参数设定
void Sys_ParameterInit(void)
{
	CTRL_DCDC_ON_OFF = DCDC_OFF;
	CTRL_DCDC_CC_CV = DCDC_CV;//开机充电模式，恒流准备状态
	CTRL_DCDC_Mode = DCDC_Mode_DC;
    
	MaxRuningMins = 600;				//调节运行时间限制的上限，不必改
	SetMaxRuningMins = 120;				
    
    DCDC_Val.NumofBatCells = 0;//电池节数
	
    DCDC_Val.Cur_Limit = 2.0;
    DCDC_Val.Vol_Limit = 20.0;
    DCDC_Val.Cur_MinLimit = 0.50;//电流传感纹波达32mv最大，设定电流太小就不能稳定
    DCDC_Val.Vol_MinLimit = 3.00;
    
	DCDC_Val.Cur = 0;//最大占五位的数    ->0.00~3.00A
	DCDC_Val.Vol = 0;//              ->0.00~9.99~20.00V  下同
	DCDC_Val.SetCur = DCDC_Val.Cur_Limit;
    DCDC_Val.SetVol = 12.60;//恒压状态
    DCDC_Val.ChrVol = 0;//充电模式设定电压
	DCDC_Val.BATCellVol_1 = 0;
	DCDC_Val.BATCellVol_2 = 0;
	DCDC_Val.BATCellVol_3 = 0;
	DCDC_Val.BATCellVol_4 = 0;
    
	DCDC_Val.BATCellDisChr_1 = 0;
	DCDC_Val.BATCellDisChr_2 = 0;
	DCDC_Val.BATCellDisChr_3 = 0;
	DCDC_Val.BATCellDisChr_4 = 0;
    
    LowVol_th = 3.5;//补充充电阈值
    FinishCur = 0.5;//恒压充电下，充满电时充电电流阈值，不能小于DCDC_Val.Cur_MinLimit
    TriVol_th= 4.1;//涓流充电，再充电阈值
    
    DCDC_Val.AD5Key_Value = 100.0;
    
    SYS_Chr_State = Chr_State_CC;//默认恒流状态
    
    KeyPos = 0;
//    is_Key_Down = 0;
//    is_Key_Left = 0;
//    is_Key_Up = 0;
//    is_Key_Center = 0;
//    is_Key_Right = 0;
    is_Coarse = 0;
    Key_CenterCount = 0;
    is_Key_Center_LongPrass = 0;
    Key_UseUp = 0;
    
    Menu_Chr_Pos = 1;
    Menu_DC_Pos = 1;

	Temperature_DEV = 27;
	Temperature_MCU = 27;
}

//充电程序，菜单选择平衡充模式并开启后跳到这里
void Sys_AutoRun_Chr_Sequence(void)
{
    static u16 Finish_min;//离开本段函数值不变
    
    /********
    检查电池状态，判断应处于什么充电阶段START
    ********/
    //确定是否进行补充充电(有电池电压过低)，修改参数
    switch(DCDC_Val.NumofBatCells)
    {
        case 1:if(DCDC_Val.BATCellVol_1 < LowVol_th){SYS_Chr_State = Chr_State_sup;}
            break;
        case 2:if((DCDC_Val.BATCellVol_1 < LowVol_th)||(DCDC_Val.BATCellVol_2 < LowVol_th)){SYS_Chr_State = Chr_State_sup;}
            break;
        case 3:if((DCDC_Val.BATCellVol_1 < LowVol_th)||(DCDC_Val.BATCellVol_2 < LowVol_th)||(DCDC_Val.BATCellVol_3 < LowVol_th))\
            {SYS_Chr_State = Chr_State_sup;}
            break;
        case 4:if((DCDC_Val.BATCellVol_1 < LowVol_th)||(DCDC_Val.BATCellVol_2 < LowVol_th)||(DCDC_Val.BATCellVol_3 < LowVol_th)||\
            (DCDC_Val.BATCellVol_4 < LowVol_th)){SYS_Chr_State = Chr_State_sup;}
            break;
        default:break;
    }
    //如果不处于涓流并且电压小于满电电压，则选择恒流充电状态
    if((SYS_Chr_State != Chr_State_sup)&&(DCDC_Val.Vol < (DCDC_Val.NumofBatCells*4.20)))
    {
        SYS_Chr_State = Chr_State_CC;
    }
    //如果不处于涓流并且电压达到满电电压，则选择恒压充电状态
    if((SYS_Chr_State != Chr_State_sup)&&(DCDC_Val.Vol >= (DCDC_Val.NumofBatCells*4.20)))
    {
        SYS_Chr_State = Chr_State_CV;
    }
    //处于恒压阶段，并且满电标志，充电完成
    if((SYS_Chr_State == Chr_State_CV)&&(DCDC_Val.Cur <= FinishCur))
    {
        SYS_Chr_State = Chr_State_Finish;
        Finish_min = Timer_IT_flags._1min;//记录当前运行时间
    }
    //充电完成后，5分钟后，并且电压小于涓流充电阈值，开始涓流充电
    if((SYS_Chr_State == Chr_State_Finish)&&(((Timer_IT_flags._1min - Finish_min) >= 5))&&(DCDC_Val.Vol < TriVol_th))
    {
        CTRL_DCDC_ON_OFF = DCDC_ON;
        SYS_Chr_State = Chr_State_trickle;
    }
    /********
    检查电池状态，判断应处于什么充电阶段END
    ********/
    
    
    
    if(SYS_Chr_State == Chr_State_sup)
    {
        //补充充电
        CTRL_DCDC_CC_CV = DCDC_CC;
        delay_ms(30);
        SetDAC_Value((FinishCur*0.185+2.5)*10.0/16.8);//以FinishCur电流补充充电
    }
    if((SYS_Chr_State == Chr_State_CC)||(SYS_Chr_State == Chr_State_trickle))//判断电压现在应处于恒流充电状态(条件包括涓流和恒流)
    {
        //设定恒流状态以及相应参数
        //电流值根据设定值
        CTRL_DCDC_CC_CV = DCDC_CC;//保证整个过程中不变线
        delay_ms(30);
        Sys_AutoRun_DC_Output();//输出,设定相应恒流电流
        //DCDC_Val.SetVol = 4.2*DCDC_Val.NumofBatCells;//电压设定到最大
    }
    if(SYS_Chr_State == Chr_State_CV)
    {
        //设定恒流状态以及相应参数
        //电压值根据平衡模块读取有几节电池定的满电电压
        CTRL_DCDC_CC_CV = DCDC_CV;
        delay_ms(30);
        SetDAC_Value((DCDC_Val.ChrVol/101.0)*10.0);
    }
    if(SYS_Chr_State == Chr_State_Finish)
    {
        CTRL_DCDC_CC_CV = DCDC_CV;
        delay_ms(30);
        //SetDAC_Value((0/101.0)*10.0);//充满电后设置输出电压为0
        CTRL_DCDC_ON_OFF = DCDC_OFF;
    }
    
}

//菜单选择电源模式并开启后跳到这里
void Sys_AutoRun_DC_Output(void)
{
    float SetDAC_Value_CV,SetDAC_Value_CC,SetDAC_Temp;
    switch(CTRL_DCDC_CC_CV)
    {
        case DCDC_CV:
            // x为实际电压，y为反馈电压
            SetDAC_Value_CV = ((DCDC_Val.SetVol - 0.04)/101.0)*10.0;//分压电阻传感，做修正
            SetDAC_Value_CV /= 1.1;//滤波器增益
            SetDAC_Value(SetDAC_Value_CV);
            break;
        case DCDC_CC:
            //y = 0.3974x - 0.0112  x实际电流，y为INA194输出电压
            //y = 0.0973x2 + 0.7926x + 0.1542 x为实际电流 y为设定电流
            //根据实际来，焊接器件与理论计算比较不符了
            //SetDAC_Value_CC = (DCDC_Val.SetCur*0.185+2.5)*10.0/16.8;//电流传感芯片传感ACS712-05B
            //SetDAC_Value_CC /= 1.1;
            SetDAC_Temp = DCDC_Val.SetCur*DCDC_Val.SetCur*0.0973 + DCDC_Val.SetCur*0.7926 + 0.1542;
            SetDAC_Value_CC = SetDAC_Temp*0.3974 - 0.0112;
            SetDAC_Value_CC /= 1.1;
            SetDAC_Value(SetDAC_Value_CC);
            break;
        default:break;
    }
}

//读取DCDC电压电流、平衡模块的电压以及设备和MCU温度，供其他函数调用！
//这个函数和刷屏函数并行平等的放在主WHILE里面，互不干扰
void Sys_ReadinALLInf(void)
{
    //从74HC373控制CD4051选择读取模拟量
    Sel_CD4051_Ch(ch_Cur);
    delay_ms(5);//必要加延时
    DCDC_Val.Cur = (Get_Adc_Average(MCU_AD_MUL_Ch,Adc_Average_times)*3.3)/4096.0;
    DCDC_Val.Cur = (DCDC_Val.Cur + 0.0112)/0.3974;//修正
    //y = 0.0924x3 - 0.4267x2 + 1.5327x - 0.271
    DCDC_Val.Cur = 0.0924*DCDC_Val.Cur*DCDC_Val.Cur*DCDC_Val.Cur - 0.4267*DCDC_Val.Cur*DCDC_Val.Cur + 1.5327*DCDC_Val.Cur - 0.271;//修正
    if(DCDC_Val.Cur < 0){DCDC_Val.Cur = 0;}
    
    Sel_CD4051_Ch(ch_Vol);
    delay_ms(5);
    DCDC_Val.Vol = ((Get_Adc_Average(MCU_AD_MUL_Ch,Adc_Average_times)*3.3)/4096.0)/10.0*101.0;
    DCDC_Val.Vol = DCDC_Val.Vol + 0.3;//修正
    if(DCDC_Val.Vol < 1.5){DCDC_Val.Vol = 0;}
    
    //在这里只读温度的整数部分用于诊断错误，LCD显示温度那里现读带小数
    if(Error_code != Error_ds18b20_isn_init)
    {
        Temperature_DEV = (DS18B20_Get_Temp()/10);//每100ms读取电池组温度
    }
    //Temperature_MCU = (Get_Temprate()/100);//得到MCU温度值的整数部分（内部温敏只表大概）
    
    
    if(CTRL_DCDC_Mode == DCDC_Mode_Chr)
    {
        
//        Sel_CD4051_Ch(ch_C1);
//        DCDC_Val.BATCellVol_1 = (Get_Adc_Average(MCU_AD_MUL_Ch,Adc_Average_times)*3.3)/4096.0;
//        
//        Sel_CD4051_Ch(ch_C2);
//        DCDC_Val.BATCellVol_2 = (Get_Adc_Average(MCU_AD_MUL_Ch,Adc_Average_times)*3.3)/4096.0;
//        
//        Sel_CD4051_Ch(ch_C3);
//        DCDC_Val.BATCellVol_3 = (Get_Adc_Average(MCU_AD_MUL_Ch,Adc_Average_times)*3.3)/4096.0;
//        
//        Sel_CD4051_Ch(ch_C4);
//        DCDC_Val.BATCellVol_4 = (Get_Adc_Average(MCU_AD_MUL_Ch,Adc_Average_times)*3.3)/4096.0;
        
        //判断有几节电池连接在平衡头
        decideBatCells();
        //平衡开启关闭控制
        Balance_ctrl();
    }
}

//设置12位DAC，传入要设定的电压值小数(小数位不可多于两位)
void SetDAC_Value(float DAC_Value)
{
    /*
    b15 高B，低A
    b14 无关
    b13 高增益1，低2
    b12 高输出，低关通道
    */
    
    u16 Digit_Out;
    DAC_Value -= 0.009;//修正
    if(DAC_Value > 2){DAC_Value = 2.0;}else if(DAC_Value < 0){DAC_Value = 0;}
    Digit_Out = (u16)((DAC_Value*4096)/2.048);
    Digit_Out &= 0x0FFF;//清高四位
    Digit_Out |= 0x7000;//高四位写0111
    SPI1_WriteByte(Digit_Out);
}

void initMCP4822(void)
{
    SPI1_WriteByte(0xE000);//关闭B通道
    
    if(CTRL_DCDC_CC_CV == DCDC_CV)
    {
        Sys_AutoRun_DC_Output();
    }
    
}


//选择CD4051模拟传入通道和平衡模块放电控制
void Sel_CD4051_Ch(u8 ch)
{
    //顺序 C B A  或者 A2 A1 A0
    //PB12 13 14 对应 A B C
    PBout(6) = 1;//不选LCD
    switch(ch)
    {
        case ch_C0:GPIO_ResetBits(GPIOB,GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14);//000
            break;
        case ch_C4:GPIO_ResetBits(GPIOB,GPIO_Pin_12|GPIO_Pin_13);GPIO_SetBits(GPIOB,GPIO_Pin_14);//100
            break;
        case ch_C2:GPIO_ResetBits(GPIOB,GPIO_Pin_12|GPIO_Pin_14);GPIO_SetBits(GPIOB,GPIO_Pin_13);//010
            break;
        case ch_Cur:GPIO_ResetBits(GPIOB,GPIO_Pin_12);GPIO_SetBits(GPIOB,GPIO_Pin_14|GPIO_Pin_13);//110
            break;
        case ch_C1:GPIO_ResetBits(GPIOB,GPIO_Pin_13|GPIO_Pin_14);GPIO_SetBits(GPIOB,GPIO_Pin_12);//001
            break;
        case ch_C3:GPIO_ResetBits(GPIOB,GPIO_Pin_14);GPIO_SetBits(GPIOB,GPIO_Pin_12|GPIO_Pin_13);//011
            break;
        case ch_Vol:GPIO_SetBits(GPIOB,GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14);//111
            break;
        default:break;
    }
    _373_CS = _373_EN;//拉高更新输出
    delay_ms(5);
    _373_CS = _373_disEN;
}

void Balance_ctrl(void)
{
    //比较每节电池电压，只对最大的一节放电，最大电压的有并列则哪个都不放电
    switch(DCDC_Val.NumofBatCells)
    {
        case 2:if(DCDC_Val.BATCellVol_2 > DCDC_Val.BATCellVol_1){
        DCDC_Val.BATCellDisChr_2 = 1;DCDC_Val.BATCellDisChr_1 = 0;
        }else{DCDC_Val.BATCellDisChr_2 = 0;DCDC_Val.BATCellDisChr_1 = 0;}
            break;
        
        case 3:if((DCDC_Val.BATCellVol_3 > DCDC_Val.BATCellVol_1)&&(DCDC_Val.BATCellVol_3 > DCDC_Val.BATCellVol_2)){
            DCDC_Val.BATCellDisChr_3 = 1;DCDC_Val.BATCellDisChr_2 = 0;DCDC_Val.BATCellDisChr_1 = 0;
        }else if((DCDC_Val.BATCellVol_2 > DCDC_Val.BATCellVol_1)&&(DCDC_Val.BATCellVol_2 > DCDC_Val.BATCellVol_3)){
            DCDC_Val.BATCellDisChr_2 = 1;DCDC_Val.BATCellDisChr_3 = 0;DCDC_Val.BATCellDisChr_1 = 0;
        }else if((DCDC_Val.BATCellVol_1 > DCDC_Val.BATCellVol_2)&&(DCDC_Val.BATCellVol_1 > DCDC_Val.BATCellVol_3))
        {
            DCDC_Val.BATCellDisChr_1 = 1;DCDC_Val.BATCellDisChr_2 = 0;DCDC_Val.BATCellDisChr_3 = 0;
        }else{DCDC_Val.BATCellDisChr_3 = 0;DCDC_Val.BATCellDisChr_2 = 0;DCDC_Val.BATCellDisChr_1 = 0;}
            break;
        
        case 4:if((DCDC_Val.BATCellVol_4 > DCDC_Val.BATCellVol_1)&&(DCDC_Val.BATCellVol_4 > DCDC_Val.BATCellVol_2)&&(DCDC_Val.BATCellVol_4 > DCDC_Val.BATCellVol_3))
        {
            DCDC_Val.BATCellDisChr_4 = 1;DCDC_Val.BATCellDisChr_3 = 0;DCDC_Val.BATCellDisChr_2 = 0;DCDC_Val.BATCellDisChr_1 = 0;
        }else if((DCDC_Val.BATCellVol_3 > DCDC_Val.BATCellVol_1)&&(DCDC_Val.BATCellVol_3 > DCDC_Val.BATCellVol_2)&&(DCDC_Val.BATCellVol_3 > DCDC_Val.BATCellVol_4))
        {
            DCDC_Val.BATCellDisChr_3 = 1;DCDC_Val.BATCellDisChr_4 = 0;DCDC_Val.BATCellDisChr_2 = 0;DCDC_Val.BATCellDisChr_1 = 0;
        }else if((DCDC_Val.BATCellVol_2 > DCDC_Val.BATCellVol_1)&&(DCDC_Val.BATCellVol_2 > DCDC_Val.BATCellVol_3)&&(DCDC_Val.BATCellVol_2 > DCDC_Val.BATCellVol_4))
        {
            DCDC_Val.BATCellDisChr_2 = 1;DCDC_Val.BATCellDisChr_4 = 0;DCDC_Val.BATCellDisChr_3 = 0;DCDC_Val.BATCellDisChr_1 = 0;
        }else if((DCDC_Val.BATCellVol_1 > DCDC_Val.BATCellVol_2)&&(DCDC_Val.BATCellVol_1 > DCDC_Val.BATCellVol_3)&&(DCDC_Val.BATCellVol_2 > DCDC_Val.BATCellVol_4))
        {
            DCDC_Val.BATCellDisChr_1 = 1;DCDC_Val.BATCellDisChr_4 = 0;DCDC_Val.BATCellDisChr_3 = 0;DCDC_Val.BATCellDisChr_2 = 0;
        }else{
            DCDC_Val.BATCellDisChr_2 = 0;DCDC_Val.BATCellDisChr_4 = 0;DCDC_Val.BATCellDisChr_3 = 0;DCDC_Val.BATCellDisChr_1 = 0;
        }
            break;
        default:break;
    }
    
    //控制放电
    PBout(6) = 1;//不选LCD
    if(DCDC_Val.BATCellDisChr_1){Balance_1 = Balance_ON;}else{Balance_1 = Balance_OFF;}
    if(DCDC_Val.BATCellDisChr_2){Balance_2 = Balance_ON;}else{Balance_2 = Balance_OFF;}
    if(DCDC_Val.BATCellDisChr_3){Balance_3 = Balance_ON;}else{Balance_3 = Balance_OFF;}
    if(DCDC_Val.BATCellDisChr_4){Balance_4 = Balance_ON;}else{Balance_4 = Balance_OFF;}
    _373_CS = _373_EN;//拉高更新输出
    delay_ms(5);
    _373_CS = _373_disEN;
}

void decideBatCells(void)
{
    if(CTRL_DCDC_Mode == DCDC_Mode_Chr)
    {
        if(DCDC_Val.BATCellVol_1 < 1){DCDC_Val.NumofBatCells = 0;}
        else if(DCDC_Val.BATCellVol_2 < 1){DCDC_Val.NumofBatCells = 1;}
        else if(DCDC_Val.BATCellVol_3 < 1){DCDC_Val.NumofBatCells = 2;}
        else if(DCDC_Val.BATCellVol_4 < 1){DCDC_Val.NumofBatCells = 3;}
        else{DCDC_Val.NumofBatCells = 4;}
        DCDC_Val.ChrVol = DCDC_Val.NumofBatCells*4.20;
    }
    
    
}



