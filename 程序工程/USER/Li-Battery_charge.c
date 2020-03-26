//  　　　┏┓　　　┏┓
//  　　┏┛┻━━━┛┻┓
//  　　┃　　　　　　　┃
//  　　┃　　　━　　　┃
//  　　┃　┗┳　┳┛　┃
//  　　┃　　　　　　　┃
//  　　┃　　　┻　　　┃
//  　　┃　　　　　　  ┃
//  　　┗━┓　　　┏━┛Codes are far away from bugs with the Mysterious Animal protecting
//  　　　　┃　　　┃    神兽保佑,代码无bug
//  　　　　┃　　　┃
//  　　　　┃　　　┗━━━┓
//  　　　　┃              ┣┓
//  　　　　┃              ┏┛
//  　　　　┗┓┓┏━┳┓┏┛
//  　　　　　┃┫┫　┃┫┫
//  　　　　　┗┻┛　┗┻┛
#include "sys.h"
#include "delay.h"
#include "beep.h"
#include "ds18b20.h" 
#include "timer.h"
#include "usart.h"
#include "sys_io.h"
#include "wdg.h"
#include "tsensor.h"
#include "adc.h"
#include "lcd.h"
#include "GUI.h"

#include "SelfChecking.h"
#include "MENU.h"
#include "SYS_INF_CTRL.h"

//DS18B20读取先用一个，减小工作量

/*********
中断优先级说明：
抢占优先级：主定时器(TIM3)中断(1) > 串口接收中断(2) > TIM2蜂鸣器鸣叫中断(3)
*********/

/********
LCDTFT界面操作说明：
长按中间键切换电源或者平衡充模式；
短按中间件切换粗调还是细调；
上下切换调参项目，左右调该参；
平衡充模式下，开启状态，电流不可调；
********/

#define USART1_bound 9600
#define USART2_bound 115200

//是否响应故障
#define ResponseFault 0
#define CheckFault 0

//其他文件变量声明


int main(void)
{
	static u8 Error_check_now = 1;
    //float i = 1;
    static u8 RunOnce = 1;
//		u16 len;
    delay_init();
    LCD_Init();
	SYS_IO_Init();
	//开机默认模式设定：DCDC关闭、恒压调节、电源模式
	Sys_ParameterInit();
	uart_init(USART1_bound);
	//T_Adc_Init();//不用
    Adc_Init();
    initMCP4822();//关闭DAC芯片的B通道按照设定输出A通道DA
    BEEP_Init();
    TIM2_Int_Init();
    TestLED_init();
    
	if(CTRL_DCDC_Mode == DCDC_Mode_DC)
	{
		DrawPowerModePage();
	}else if(CTRL_DCDC_Mode == DCDC_Mode_Chr)
	{
		DrawChargeModePage();
	}
    
	delay_ms(369);
	if(DS18B20_Init())	//DS18B20初始化	
	{
		Error_code = Error_ds18b20_isn_init;
	}
    
    Sys_ReadinALLInf();//开机读取一遍数据
    is_DCDC_Output_Normal();//上电自检
	//is_Tem_Over_Scope();
    
    Draw_Refresh_All();//刷新界面
    
	delay_ms(369);
	TIM3_Int_Init();//默认选择10MS定时中断
    delay_ms(119);
	IWDG_Init();//默认选择需每1S喂狗
    
//    SetDAC_Value(0.90009);
    
//    PBout(6) = 1;//不选LCD
//    Balance_1 = Balance_ON;
//    Balance_2 = Balance_OFF;
//    Balance_3 = Balance_ON;
//    Balance_4 = Balance_OFF;
//    _373_CS = _373_EN;//拉高更新输出
//    delay_ms(5);
//    _373_CS = _373_disEN;
//    
//    
//    Sel_CD4051_Ch(ch_C3);

//    while(1){;}
//   while(1)
//   {
//       for(i = 170;i > 30;i--)
//       {
//           SetDAC_Value(i/100.0);//SetDAC_Value给定值不能超过1.8,对于电压 1.8*1.1/10*(10+91) = 20.0
//           delay_ms(300);
//       }
//   }
    buzzer_once;//响一声表示系统初始化完毕
	while(1)
	{
		/*********
		集中响应故障START
		*********/
		if(ResponseFault)
		{
			switch(Error_code)//把容易出现且时间紧急的故障放在前面，按照重要性排序
			{
				case Error_DCDC_Output_isn_Normal://如果在这故障，是在while循环中对输出的自检中得到
					dealWith_Output_isn_Normal();
					break;
				case Error_DCDC_isn_shut://如果在这故障，是在程序关闭DCDC芯片后自检得到
					dealWith_Output_isn_Normal();//检查输出是否关闭故障的函数转移到检查输出是否正常里面了
					break;
				case Error_Tem_Over_Scope://如果在这故障，是在while循环中对电池组和MCU温度的自检中得到
					dealWith_Tem_Over_Scope();
					break;
				case Error_ds18b20_isn_init://如果在这故障，是在上电后DS18B20初始化的自检中得到
					dealWith_ds18b20_isn_init();
					break;
				case 0:break;
			}
		}
		/*********
		集中响应故障END
		*********/
        
		if(Timer_IT_flags._1sec_flag == 1)
		{
			Timer_IT_flags._1sec_flag = 0;
			//IO_TestLED = !IO_TestLED;//灯被LCD征用
            
			/*********
			刷新屏幕上的运行指示灯
			*********/
			Draw_Refresh_PowSignal();
		}
		
		if(Timer_IT_flags._300msec_flag == 1)
		{
			Timer_IT_flags._300msec_flag = 0;
			
            if(CheckFault)
            {
                /*********
                集中故障巡检START
                *********/
                //主要写成判断语句，不会占用太多时间
                switch(Error_check_now)
                {
                    case 1:is_DCDC_Output_Normal();break;
                    case 2:is_Tem_Over_Scope();break;
                    default:break;
                }
                if(++Error_check_now > Error_check_num)
                {
                    Error_check_now = 1;
                }
                /*********
                集中故障巡检END
                *********/
            }

            /*********
			读入所有信息
			*********/
            Sys_ReadinALLInf();
            /*********
			刷新界面信息
			*********/
			Draw_Refresh_All();
            
		}
		
        //按键处理
        if(Timer_IT_flags._100msec_flag == 1)
        {
            Timer_IT_flags._100msec_flag = 0;
            
            DCDC_Val.AD5Key_Value = Get_Adc_Average(MCU_AD_5Key_Ch,2);
            
            if(DCDC_Val.AD5Key_Value > 3500)//单次，用后立马清,并设Key_UseUp = 1;
            {
                RunOnce = 1;
                if(!Key_UseUp)
                {KeyPos = Key_Down;
                buzzer_once;
                }
            }else if(DCDC_Val.AD5Key_Value > 3020)
            {
                RunOnce = 1;
                if(!Key_UseUp)
                {KeyPos = Key_Left;
                //buzzer_once;
                }
            }else if(DCDC_Val.AD5Key_Value > 2640)
            {
                RunOnce = 1;
                if(!Key_UseUp)
                {KeyPos = Key_Up;
                buzzer_once;
                }
            }else if(DCDC_Val.AD5Key_Value > 2330)
            {
                RunOnce = 1;
                if(!Key_UseUp)
                {KeyPos = Key_Center;
                buzzer_once;
                }
                
                if(++Key_CenterCount > 15)//长按
                {
                    Key_CenterCount = 0;
                    is_Key_Center_LongPrass = 1;
                }
            }else if(DCDC_Val.AD5Key_Value > 1830)
            {
                RunOnce = 1;
                if(!Key_UseUp)
                {KeyPos = Key_Right;
                //buzzer_once;
                }
            }else {if(RunOnce == 1){clearAllKeyInf();RunOnce = 0;}}
            
            
            if(is_Key_Center_LongPrass)//切换模式
            {
                is_Key_Center_LongPrass = 0;
                CTRL_DCDC_ON_OFF = DCDC_OFF;
                delay_ms(10);
                
                if(CTRL_DCDC_Mode == DCDC_Mode_DC)
                {
                    CTRL_DCDC_Mode = DCDC_Mode_Chr;
                    CTRL_DCDC_CC_CV = DCDC_CC;
                    DrawChargeModePage();
                }else{
                    CTRL_DCDC_Mode = DCDC_Mode_DC;
                    CTRL_DCDC_CC_CV = DCDC_CV;
                    DrawPowerModePage();
                }
                MENU_Refrash_Pos();
                buzzer_once;
            }
            
            switch(CTRL_DCDC_Mode)
            {
                case DCDC_Mode_DC:Opera_DC_Menu();break;
                case DCDC_Mode_Chr:Opera_Chr_Menu();break;
                default:break;
            }
        }
        
        
        //超过设定运行时间，进入死机
		while(_OutofTime_Running_flag){
            CTRL_DCDC_ON_OFF = DCDC_OFF;
            DrawPageHead("TIME OUT!Please shutdown.");
			CTRL_buzzer_ON_OFF = Buzzer_ON;
            while(1){;}}
		      
        //串口1测试用，回传所接收信息
//		if(USART_RX_STA&0x8000)
//		{					   
//			len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
//			printf("\r\n您发送的消息为:\r\n");
//			printf("\r\n%s\r\n",USART_RX_BUF);
//			printf("\r\n您发送的消息长度为:\r\n");
//			printf("\r\n%d\r\n",len);
//			while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
////			for(t=0;t<len;t++)
////			{
////				USART_SendData(USART1, USART_RX_BUF[t]);//向串口1发送数据
////				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
////			}
//			printf("\r\n\r\n");//插入换行
//			USART_RX_STA=0;
//		}
		
		
	}
}


