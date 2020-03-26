#include "sys_io.h"
#include "delay.h"
/*
包含：（非模块化的为本项目专门定制的外设功能）
DCDC芯片开关控制线——PA8--高电平关闭DCDC芯片
恒流恒压模式控制线——PA1--低电平设置恒流反馈模式，反之恒压	
五向按键ADKey——AD由高到低，下左上中右，ADC1_IN8->ADKey
373锁存器使能——PB1->373EN，锁存器扩展了平衡模块四个MOS和4051的地址ABC
设备温度——PA15->DS18B20
来自4051的多个模拟量，ADC_IN0->MulVol，通道0到4是C0到C4，通道6和7是DCDC的电流和电压

SPI(一个软件使能)
MOSI-PA7、MISO-PA6、SCK-PA5、SS-PA4(DA芯片)

串口2的初始化——TX-PA2、RX-PA3，通过双刀双掷开关选择接往ESP8266-07的串口1或者引出的排针
*/


void SYS_IO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
//	EXTI_InitTypeDef EXTI_InitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|\
	RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|\
	RCC_APB2Periph_GPIOC, ENABLE);
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
    
	//STEP和KEY外部中断初始化
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource0);
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource13);
//	
//	EXTI_InitStructure.EXTI_Line=EXTI_Line0;
//	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
//	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//	EXTI_Init(&EXTI_InitStructure);
//	EXTI_InitStructure.EXTI_Line=EXTI_Line13;
//	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
//	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//	EXTI_Init(&EXTI_InitStructure);
	
//	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);
//	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);
	
	//GPIOA初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    //SPI1引脚初始化
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;//MISO
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA,&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_7;//SCK MOSI
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA,&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;//SS
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA,&GPIO_InitStructure);
    
    GPIO_SetBits(GPIOA,GPIO_Pin_4);//设高
    
    //SPI1初始化
    SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;//只发送不接收
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;//时钟极性：空闲时低电平，数据在时钟上升沿传入从机
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;//时钟相位，第一个时钟跳边沿开始传输，根据MCP4822的协议
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;//使能引脚软件设置，非硬件
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;//36M/256=140.625KHz
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1,&SPI_InitStructure);
    SPI_Cmd(SPI1,ENABLE);
    
    //SPI1_ReadWriteByte(0xff);//最大的作用就是维持MOSI为高电平，而且这句话也不是必须的
    
	
	GPIO_ResetBits(GPIOA,GPIO_Pin_8);//关闭DCDC
	GPIO_SetBits(GPIOA,GPIO_Pin_1);//设定恒压MODE
	
	//GPIOB初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    //在此之前，必须先配置LCD的IO，即把LCD_Init()放到SYS_IO_Init()前面！
    
    GPIO_SetBits(GPIOB,GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11);//关闭四路平衡PMOS
    GPIO_ResetBits(GPIOB,GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14);//4051地址选000
    GPIO_SetBits(GPIOB,GPIO_Pin_1);//开启373锁存器
    delay_ms(5);
    GPIO_ResetBits(GPIOB,GPIO_Pin_1);//锁存373输出端，关闭锁存器
}



void TestLED_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
}

//通过SPI1口发送一个数据，返回1表示成功
u8 SPI1_WriteByte(u16 TxData)
{
    u8 retry=0;
    u16 tx_buf;
    SPI_SS = 0;
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //等待发送区空
    {
        retry++;
        if(retry>200)return 0;
    }
    
    tx_buf = TxData;
    delay_ms(1);
    SPI_I2S_SendData(SPI1, (u8)(tx_buf >> 8)); //通过外设 SPIx 发送数据
    delay_ms(1);
    SPI_I2S_SendData(SPI1, (u8)TxData);
    delay_ms(1);
    SPI_SS = 1;
    return 1;
}

//通过SPI1口发送一个数据，同时接收一个数据
//u8 SPI1_ReadWriteByte(u8 TxData)
//{
//    u8 retry=0;
//    SPI_SS = 0;
//    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //等待发送区空
//    {
//        retry++;
//        if(retry>200)return 0;
//    }
//    SPI_I2S_SendData(SPI1, TxData); //通过外设 SPIx 发送一个数据
//    retry=0;
//    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) //等待接收
//    { 
//        retry++;
//        if(retry>200)return 0;
//    }
//    SPI_SS = 1;
//    return SPI_I2S_ReceiveData(SPI1); //返回通过 SPIx 最近接收的数据
//}




