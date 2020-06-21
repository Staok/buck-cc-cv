#include "SelfChecking.h"
#include "delay.h"
#include "timer.h"
#include "beep.h"
#include "ds18b20.h"
#include "tsensor.h"
#include "SYS_INF_CTRL.h"


u8 Error_code = 0;

u16 ShutSecondNow = 0;

/*********
自检函数START
*********/
//自检DCDC是否正常关闭，用于在程序关闭DCDC芯片之后的自检，在关闭输出后只写ShutSecondNow = Timer_IT_flags._1sec;即可，
//u8 IS_DCDC_OFF(void)
//{
//    
//	//CTRL_DCDC_ON_OFF = DCDC_OFF;
//	//delay_ms(5000);//延时根据实际情况修改，应比正常电压降低到1V以下再长一点点
//    if(((Timer_IT_flags._1sec - ShutSecondNow) > 5)&&(CTRL_DCDC_Mode == DCDC_Mode_DC))
//    {
//        if((DCDC_Val.Cur > 0.1)&&(DCDC_Val.Vol > 1))//如果还有持续的较大的输出电压，表示没有DCDC关掉
//        {
//            Error_code = Error_DCDC_isn_shut;//只能在处理故障函数里面把错误代码置0（无故障），检测函数只给错，不能删错！
//            return 0;
//        }else{ return 1;}//返回1表示成功，无错
//    }
//    return 1;
//}
//自检温度是否在正常范围
u8 is_Tem_Over_Scope(void)
{
//	if((Temperature_DEV < 5)||(Temperature_DEV > 40) || (Temperature_MCU < 5)||(Temperature_MCU > 50))
//	{
//		Error_code = Error_Tem_Over_Scope;
//		return 0;
//	}else{ return 1;}
	if((Temperature_DEV < 5)||(Temperature_DEV > 40))
	{
		Error_code = Error_Tem_Over_Scope;
		return 0;
	}else{ return 1;}
}
//自检当DCDC芯片开启时，再判断现在处于恒流还是恒压模式，再各自判断输出是否正常
u8 is_DCDC_Output_Normal(void)
{
	if(CTRL_DCDC_ON_OFF == DCDC_ON)
	{
		switch(CTRL_DCDC_Mode)//压流不超过极限，压流不过小于设定值
		{
			case DCDC_Mode_DC:
				if((DCDC_Val.Cur > DCDC_Val.Cur_Limit + 0.1)||(DCDC_Val.Vol > DCDC_Val.Vol_Limit + 1))
				{
					Error_code = Error_DCDC_Output_isn_Normal; //如果电压异常，标志错误代码
					return 0; //返回错误
				}
                if((DCDC_Val.Cur < DCDC_Val.SetCur-0.5)||(DCDC_Val.Vol < DCDC_Val.SetVol-1))
                {
                    Error_code = Error_DCDC_Output_isn_Normal;
                    return 0;
                }
				break;
			case DCDC_Mode_Chr:
				if((DCDC_Val.Cur > DCDC_Val.Cur_Limit + 0.1)||(DCDC_Val.Vol > (4.2*DCDC_Val.NumofBatCells+0.2))) //+xx是裕量
				{
					Error_code = Error_DCDC_Output_isn_Normal;
					return 0;
				}
                //如果在补充充电状态，电流不能小于设定电流的一半
                if((SYS_Chr_State == Chr_State_sup)&&(DCDC_Val.Cur < FinishCur/2.0))
                {
                    Error_code = Error_DCDC_Output_isn_Normal;
                    return 0;
                }
                //如果在恒流或者涓流充电状态，电流不能小于设定值-0.5
                if(((SYS_Chr_State == Chr_State_CC)||(SYS_Chr_State == Chr_State_trickle))&&(DCDC_Val.Cur < DCDC_Val.SetCur-0.5))
                {
                    Error_code = Error_DCDC_Output_isn_Normal;
                    return 0;
                }
				break;
			default:break;
		}
        return 1;//返回无误
	}else if(CTRL_DCDC_ON_OFF == DCDC_OFF)
    {
        //CTRL_DCDC_ON_OFF = DCDC_OFF;
        //自检DCDC是否正常关闭，用于在程序关闭DCDC芯片之后的自检，在关闭输出后只写ShutSecondNow = Timer_IT_flags._1sec;即可
        if(((Timer_IT_flags._1sec - ShutSecondNow) > 5)&&(CTRL_DCDC_Mode == DCDC_Mode_DC))
        {
            if((DCDC_Val.Cur > 0.1)&&(DCDC_Val.Vol > 1))//如果还有持续的较大的输出电压，表示没有DCDC关掉
            {
                Error_code = Error_DCDC_isn_shut;//只能在处理故障函数里面把错误代码置0（无故障），检测函数只给错，不能删错！
                return 0;
            }else{ return 1;}//返回1表示成功，无错
        }
        return 1;
    }
    return 1;
}
/*********
自检函数END
*********/




/*********
故障处理函数START
*********/
//报警形式：有关输出不正常或者温度不正常的，立即关闭DCDC，周期性的鸣响蜂鸣器报警
//屏幕的ErrorCode显示是什么错误，程序上每隔300ms或者1s就重新检查，如果恢复正常，报警解除，但需要手动开启DCDC
void dealWith_Output_isn_Normal(void)
{
	CTRL_DCDC_ON_OFF = DCDC_OFF;
	CTRL_buzzer_ON_OFF = Buzzer_ON;
	//显示“没有正常开启输出！功率纽子开关打开否？可以酌情手动重启”
//	while(Error_code)
//	{
		if(Timer_IT_flags._1sec_flag == 1)
		{
			//Timer_IT_flags._1sec_flag = 0;//主循环里后面的程序会清标志
			//buzzer_once;
			if(is_DCDC_Output_Normal())
			{
				Error_code = 0;
				;//屏幕清除警告，恢复正常
				CTRL_buzzer_ON_OFF = Buzzer_OFF;
			}
		}
//	}
}
//void dealWith_DCDC_isn_Shut(void)
//{
//	CTRL_DCDC_ON_OFF = DCDC_OFF;
//	CTRL_buzzer_ON_OFF = Buzzer_ON;
//	//"没有正常关闭输出，已经停机，可酌情手动关机"
////	while(Error_code)
////	{
//		if(Timer_IT_flags._1sec_flag == 1)
//		{
//			//Timer_IT_flags._1sec_flag = 0;//主循环里后面的程序会清标志
//			
//			if(IS_DCDC_OFF())
//			{
//				Error_code = 0;
//				;//屏幕清除警告，恢复正常
//				CTRL_buzzer_ON_OFF = Buzzer_OFF;
//			}
//		}
////	}
//}
//处理DS18B20初始化故障
void dealWith_ds18b20_isn_init(void)//屏幕显示，并循环重新初始化，直到初始化完成就清除故障标志
{
	//CTRL_DCDC_ON_OFF = DCDC_OFF;
	//CTRL_buzzer_ON_OFF = Buzzer_ON;
	;//屏幕显示警告"温度检测模块初始化未完成，已经停机，正在不断重试，可酌情手动重启"
	//while(Error_code)
	//{
		if(Timer_IT_flags._1sec_flag == 1)
		{
			//Timer_IT_flags._1sec_flag = 0;//主循环里后面的程序会清标志
			//buzzer_once;
			if(DS18B20_Init())//DS18B20初始化，返回1表示出错
			{
				Error_code = Error_ds18b20_isn_init;
			}else{ Error_code = 0;}//屏幕清除警告，恢复正常
		}
	//}
	//CTRL_buzzer_ON_OFF = Buzzer_OFF;
	//CTRL_DCDC_ON_OFF = DCDC_ON;
}
//处理两种温度任一超范围故障
void dealWith_Tem_Over_Scope(void)//屏幕显示，同时询问是否重新初始化，直到初始化完成就清除故障标志
{
	CTRL_DCDC_ON_OFF = DCDC_OFF;
	CTRL_buzzer_ON_OFF = Buzzer_ON;
	;//屏幕显示警告"温度超范围，已经停机，请酌情手动关机等待温度下降再开机"
//	while(Error_code)
//	{
		if(Timer_IT_flags._300msec_flag == 1)
		{
			//Timer_IT_flags._300msec_flag = 0;//主循环里后面的程序会清标志
			//buzzer_once;
			if(is_Tem_Over_Scope())
			{
				Error_code = 0;
				;//屏幕清除警告，恢复正常
				CTRL_buzzer_ON_OFF = Buzzer_OFF;
				//CTRL_DCDC_ON_OFF = DCDC_ON;
			}
		}
//	}

}
/*********
故障处理函数END
*********/


