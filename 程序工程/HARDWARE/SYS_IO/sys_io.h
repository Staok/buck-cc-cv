#ifndef __SYS_IO_H
#define __SYS_IO_H	 
#include "sys.h"
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

#define IO_TestLED PBout(12)
#define SPI_SS PAout(4)


void SYS_IO_Init(void);
void TestLED_init(void);
//u8 SPI1_ReadWriteByte(u8 TxData);
u8 SPI1_WriteByte(u16 TxData);
		 				    
#endif

