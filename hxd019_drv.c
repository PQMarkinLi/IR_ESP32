/*
 * hxd019_drv.c
 *
 *  Created on: Jan 15, 2018
 *      Author: markin
 */
#include <string.h>

#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include "esp_smartconfig.h"
#include "driver/gpio.h"
#include "esp_spi_flash.h"

#include "hxd019_drv.h"


#define pwdlen 63
unsigned char haha;
unsigned int count_0=0;
 unsigned char ReceiveBuf[64] = {0};	//接收数据(指令)存放数组
//xdata unsigned char ReceiveBuf1[230] = {0};	//接收数据(指令)存放数组
 unsigned char ReceiveBuf2[230] = {0};	//接收数据(指令)存放数组
 unsigned char ReceiveCount = 0;		//接收数据(指令)字节数
 unsigned char ReceiveFlag = 0;		//接收数据(指令)标志位

 uint8_t hxddat[17]={0x00,0x01,0x02,0x03,0x10,0x11,0x12,0x13,0x20,0x21,0x22,0x23,0x30,0x31,0x32,0x33,0x44};//数据处理表
 /*		  、、		 0	   1   2	 3	  4	   5   6	 7	 8	   9	A	B	  C   D  	E   F
 	数据说明：0-0x00,1-0x01,2-0x02,3-0x03,4-0x10,5-0x11,6-0x12,
 	7-0x13,8-0x20,9-0x21,A-0x22,B-0x23,C-0x30,D-0x31,E-0x32,F-0x33
 */
  unsigned char data1[40]={
 0x30 ,0x03 ,0x40 ,0x17 ,0x00 ,0x33 ,0x11 ,0x00 ,0x28 ,0x00 ,0x66 ,0x00 ,0x26 ,0x00 ,0x22 ,0x00 ,0x26 ,
 0x00 ,0xC4, 0x00 ,0x27 ,0x00 ,0x90 ,0x00 ,0x26 ,0x00 ,0x37 ,0x00 ,0x27 ,0x00 ,0xFF ,0xFF ,0xFF ,0xFF ,
 0xFF,0xFF,0xFF,0xFF,0x01,0x22};
 /*
 code unsigned char data1[39]={
 0x30 ,0x03 ,0x40 ,0x17 ,0x00 ,0x33 ,0x11 ,0x01 ,0x28 ,0x02 ,0x66 ,0x00 ,0x26 ,0x00 ,0x22 ,0x00 ,0x26 ,
 0x00 ,0xC4, 0x04 ,0x27 ,0x00 ,0x90 ,0x09 ,0x26 ,0x00 ,0x00 ,0x00 ,0x27 ,0x00 ,0xFF ,0xFF ,0xFF ,0xFF ,
 0xFF,0xFF,0xFF,0xFF,0x01};
 */
  unsigned char data2[59]={
 0x00 ,0x00 ,0x11 ,0x15 ,0x00 ,0x00 ,0xF0 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
 0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
 0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
 0x00 ,0x00 ,0x00 ,0x01 ,0x00 ,0x06 ,0x75 ,0x01};


  /****
    **  数据处理原则：因模块限制，最大只能发送0x33，类似4进制，不符合ascii码表，因此做如下数据处理 ；
    **		发送：	  1、将实际数值分离；
    **				  2、提取十位数以及个位数，查表，分开放，占两个字节，如a为0x61，则发送时发送hxddat[6]、hxddat[1]；
    			接收	  3、 查表，确定实际数值，即元素在数组中占的位
    **				  4、  待更新
    **  程序流程：一、按键获取数据，放入数组
    				二、数据处理发送
  				三、主控协调工作流程


  ****/
  void IRDA_delay_10us(czx_vu32 t){//ÑÓÊ±10us
  //40us delay
  	do
  	{
  		for(uint8_t i=200;i>1;i--)
  		{
  			;
  		}
  		t--;
  	}while(t>0);
  }
  void IRDA_delay_1ms(czx_vu32 t){//ÑÓÊ±1ms
  	vTaskDelay (t / portTICK_RATE_MS );
  }
  UINT8 hxdRwork(unsigned char *buf)
  {

	unsigned char i,j,k,m=1;
	unsigned char ten=0,unit=0,err=0,buz[2]={0};
	UINT16  recv=0,comp=0;
/*  	 time0_init();
  	Serial_Init();
  	remote_poweron_init();*/
  	 Learn_start2();
  //	Delay10ms(6);
  	IRDA_delay_1ms(60);
  	memset(ReceiveBuf,0,sizeof(ReceiveBuf));

  	memset(ReceiveBuf2,0,sizeof(ReceiveBuf2));
  	while(!IRDA_BUSY_S());
  	if(IRDA_BUSY_S()==1)
  	{
//  		readI2C2(ReceiveBuf2);
  		readI2C(ReceiveBuf2,&err);  //进入学习后，busy由低变高，读数据
  		IRDA_delay_1ms(60);
  	}

  /*
  	出错处理,只有连续发送相同数据才有效
  */
  /*******************start**********************/
  	if(err!=0xF0)	   //接收数据出错了	   00 21 11 22 00 42 22 44
  	{
  		m=0;
  		for(i=0,k=0;i<2;i++)
  		{
  		 	for(j=0;j<17;j++)
  		 	{
  		 		if(ReceiveBuf2[k]==hxddat[j])
  				{
  					ten=j;
  					break;
  				}
  		 	}
  			for(j=0;j<17;j++)
  			{
  				if(ReceiveBuf2[1+k]==hxddat[j])
  				{
  					unit=j;
  					break;
  				}
  			}
  			if(j==0x11)//没找到
  			{
  				for(j=0;j<128;j++)
  				{
  					if(ReceiveBuf2[j+4]%2==0)
  					ReceiveBuf2[j+4]=ReceiveBuf2[j+4]>>1;
  				}
  				goto err;
  			}

  		 	buz[i]=ten*0x10+unit;

  		 	k=k+2;
  		}

  		comp=(buz[0]<<8)+buz[1];

  		recv=0;

  		for(i=0;i<128;i++)
  		{
  			if(ReceiveBuf2[i+4]%2==0)
  			ReceiveBuf2[i+4]=ReceiveBuf2[i+4]>>1;
  			recv=recv+ReceiveBuf2[i+4];			    //计算接收到128个字符的和
  		}

  		for(i=0;i<128;i++)
  		{
  			if(recv<comp)							 //接收到的和跟实际发送和的大小比较，小于说明处理时多处理了
  			{
  				recv=0;
  				if(ReceiveBuf2[i+4]%2==0)
  				ReceiveBuf2[4+i]=ReceiveBuf2[4+i]<<1;
  				for(j=0;j<128;j++)
  				{
  					recv=recv+ReceiveBuf2[j+4];
  				}
  			}else
  			{

  				break;
  			}


  		}

  	}


 /******************end*************************/

 err:
  	for(i=0,k=0;i<pwdlen;i++)
  	{
  		 for(j=0;j<16;j++)
  		 {
  		 	if(ReceiveBuf2[k+4]==hxddat[j])
  			{
  				ten=j;
  			}
  			if(ReceiveBuf2[5+k]==hxddat[j])
  			{
  				unit=j;

  			}

  		 }
  		 buf[i]=ten*0x10+unit;
  		 k=k+2;
  	}

 //  	SendData('b');
/*  	printf("\r\n");
  	for(i=0;i<sizeof(ReceiveBuf2);i++)
  	{
  		printf("%02x ",ReceiveBuf2[i]);
  	}*/

  	IRDA_delay_1ms(60);
//  	reset51();
  	return m;

  }
  /*
  需要发送的数据，以及长度
  返回值：空
  */
 void hxdSwork(unsigned char buf[],unsigned char len)
// void hxdSwork(void)
 {
    //初始化变量

 		 unsigned char i,j;
 		 UINT16  rovc=0;
 		 unsigned char ten=0,unit=0;
 		 unsigned char che[1]={0};
 		 unsigned char check=0;
 		//初始化函数
/*  		time0_init();
  		Serial_Init();
  		remote_poweron_init();*/
/* 		memset(ReceiveBuf,0,sizeof(ReceiveBuf));

 		memset(ReceiveBuf2,0,sizeof(ReceiveBuf2));*/

 			/****************test by xiaohym***********/
 			for(i=0,j=4;i<len;i++)
 			{

 				ten=buf[i]>>4;	  //提取16进制十位数
 				unit=buf[i]&0x0f;	  //提取16进制的个位数
 				if(j==68)
 				j=j+2;
 				ReceiveBuf2[j]=hxddat[ten];
 				ReceiveBuf2[j+1]=hxddat[unit];

 				j=j+2;

 			}


 			for(i=4;i<132;i++)			 //计算所有发送数据的和,128个，0x80个
 			{
 				rovc=rovc+ReceiveBuf2[i];

 			}


 			check=rovc>>8;		  //  提取和的高8位
 			ten=check>>4;
 			unit=check&0x0f;
 			ReceiveBuf2[0]=hxddat[ten];
 			ReceiveBuf2[1]=hxddat[unit];


 			check=(UINT8)rovc;	  //  提取和的低8位

 			ten=check>>4;
 			unit=check&0x0f;
 			ReceiveBuf2[2]=hxddat[ten];
 			ReceiveBuf2[3]=hxddat[unit];

 			for(i=0;i<40;i++)
 			{
 				check=check+data1[i];
 			}
 			for(i=0;i<59;i++)
 			{
 				check=check+data2[i];
 			}

 		//	SendData(rovc);

 //			check=0x13;	   //若改变data1以及data2的值，此处校验码需更改


 			che[0]=check;

 			IRDA_delay_1ms(100);
 //			Delay10ms(1);

 			writeI2C(data1,sizeof(data1),ReceiveBuf2,sizeof(ReceiveBuf2),data2,sizeof(data2),che);
 			//delay_200ms();
 			//Delay10ms(120);			 //连续发送间隔必须等待时间，协调发送与接收
 			IRDA_delay_1ms(100);
/* 			IRDA_delay_1ms(400);
 			IRDA_delay_1ms(400);*/
 			IRDA_delay_1ms(20);
 			IRDA_delay_1ms(20);

 //ss			reset51();
 }




 gpio_config_t io_conf;
 void	IRDA_SET_SDA_IN(void){
 /*	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
 	io_conf.mode = GPIO_MODE_INPUT;
 	io_conf.pin_bit_mask = (1<<IRDA_SDA_PIN) ;
 	io_conf.pull_down_en = 0;
 	io_conf.pull_up_en = 0;
 	gpio_config(&io_conf);*/

     gpio_pad_select_gpio(IRDA_SDA_PIN);
     gpio_set_direction(IRDA_SDA_PIN,GPIO_MODE_INPUT);
  //   gpio_set_pull_mode(IRDA_SDA_PIN,GPIO_PULLUP_PULLDOWN);
 }
 void	IRDA_SET_SDA_OUT(void){
 /*	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
 	io_conf.mode = GPIO_MODE_OUTPUT;
 	io_conf.pin_bit_mask = (1<<IRDA_SDA_PIN) ;
 	io_conf.pull_down_en = 0;
 	io_conf.pull_up_en = 0;
 	gpio_config(&io_conf);*/

     gpio_pad_select_gpio(IRDA_SDA_PIN);
     gpio_set_direction(IRDA_SDA_PIN,GPIO_MODE_OUTPUT);
 //    gpio_set_pull_mode(IRDA_SDA_PIN,GPIO_PULLUP_ONLY);

 }
 void	IRDA_SET_SCL_OUT(void){
 /*	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
 	io_conf.mode = GPIO_MODE_OUTPUT;
 	io_conf.pin_bit_mask = (1<<IRDA_SCL_PIN) ;
 	io_conf.pull_down_en = 0;
 	io_conf.pull_up_en = 0;
 	gpio_config(&io_conf);*/
     gpio_pad_select_gpio(IRDA_SCL_PIN);
     gpio_set_direction(IRDA_SCL_PIN,GPIO_MODE_OUTPUT);
 //    gpio_set_pull_mode(IRDA_SCL_PIN,GPIO_PULLUP_ONLY);


 }
 void	IRDA_SET_BUSY_IN(void){

 /*	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
 	io_conf.mode = GPIO_MODE_INPUT;
 	io_conf.pin_bit_mask = (1<<IRDA_BUSY_PIN) ;
 	io_conf.pull_down_en = 0;
 	io_conf.pull_up_en = 0;
 	gpio_config(&io_conf);*/

     gpio_pad_select_gpio(IRDA_BUSY_PIN);
     gpio_set_direction(IRDA_BUSY_PIN,GPIO_MODE_INPUT);
     gpio_set_pull_mode(IRDA_BUSY_PIN,GPIO_PULLUP_ONLY);
 }
 void	IRDA_SET_BUSY_OUT(void){
 /*	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
 	io_conf.mode = GPIO_MODE_OUTPUT;
 	io_conf.pin_bit_mask = (1<<IRDA_BUSY_PIN) ;
 	io_conf.pull_down_en = 0;
 	io_conf.pull_up_en = 0;
 	gpio_config(&io_conf);*/

     gpio_pad_select_gpio(IRDA_BUSY_PIN);
     gpio_set_direction(IRDA_BUSY_PIN,GPIO_MODE_OUTPUT);
     gpio_set_pull_mode(IRDA_BUSY_PIN,GPIO_PULLUP_ONLY);
 }
 void	IRDA_INIT(){//ÉÏµç³õÊŒ»¯
 	IRDA_SDA_H();
 	IRDA_SET_SDA_OUT();
 	IRDA_SCL_H();
 	IRDA_SET_SCL_OUT();
 	IRDA_SET_BUSY_IN();
 }

 czx_u8	IRDA_getACKsign(void){
 	czx_vu8	ack;
 	IRDA_SET_SDA_IN();
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_SCL_H();
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	ack	=	IRDA_GET_DATA();
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_SCL_L();
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	return ack;
 }
 void	IRDA_sendACKsign(void){
 	IRDA_SET_SDA_OUT();
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_SDA_L();
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_SCL_H();
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_SCL_L();
 }
 czx_u16	IRDA_open(void){
 	IRDA_SET_SDA_OUT();
 	IRDA_SET_SCL_OUT();
 	IRDA_SCL_H();
 	IRDA_SDA_H();
 	return IRDA_NO_ERROR;
 }

 czx_u16	IRDA_start(void){
 	IRDA_SET_SDA_OUT();
 	IRDA_SET_SCL_OUT();
 	IRDA_SCL_H();
 	IRDA_SDA_H();
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_SDA_L();
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_SCL_L();
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	return true;
 }
 czx_u16	IRDA_stop(void){
 	IRDA_SET_SDA_OUT();
 	IRDA_SET_SCL_OUT();
 	IRDA_SCL_L();
 	IRDA_SDA_L();
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_SCL_H();
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_SDA_H();
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	return IRDA_NO_ERROR;
 }
/* UINT16 I2CWriteData(unsigned char bData )
 {
 	UINT8 Data_Bit,ACKSign,TmpDat;
 	char i;	//kal_int8 i;


 	IRDA_SCL_L();
 	IRDA_delay_10us(IRDA_DELAY_40US);

 	for(i=7;i>=0;i--)
 	{
 		IRDA_delay_10us(IRDA_DELAY_40US);

 		Data_Bit=(bData>>i)& 0x01;

 		if(Data_Bit)
 		IRDA_SDA_H();
 		else
 		IRDA_SDA_L();

 		IRDA_delay_10us(IRDA_DELAY_40US);
 		IRDA_SCL_H();
 		IRDA_delay_10us(IRDA_DELAY_40US);
 		IRDA_SCL_L();
 	}

 	ACKSign=IRDA_GET_ACK();

 	return ACKSign;
 }*/
 UINT16 I2CWriteData(unsigned char bData ){
 	czx_vu8	d_bit;
 	//czx_vu8 i;
         signed char i ;
         IRDA_SET_SDA_OUT();
    IRDA_SCL_L();
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	for(i=7;i>=0;i--)
 	{
 		IRDA_delay_10us(IRDA_DELAY_40US);

 		d_bit=((bData>>i)& 0x01);
 		if(d_bit){
 			IRDA_SDA_H();
 		}
 		else{
 			IRDA_SDA_L();
 		}
 		IRDA_delay_10us(IRDA_DELAY_40US);
 		IRDA_SCL_H();
 		IRDA_delay_10us(IRDA_DELAY_40US);
 		IRDA_SCL_L();
 	}
 	return IRDA_getACKsign();
 }
 ///urc_send
 void writeI2C(unsigned char *data1, UINT8 count1,unsigned char *data2, UINT8 count2,unsigned char *data3, UINT8 count3,unsigned char *data4)		//hxd;通用写
 {
 	UINT8 i;
 	UINT8 j = 0;
 	unsigned char iBuffer;

 	IRDA_open();
 	IRDA_delay_10us(IRDA_DELAY_40US);

 	IRDA_SCL_L();
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_SCL_H();

 	IRDA_delay_1ms(IRDA_DELAY_20MS); //20ms	//14
  //	delay_20ms();

 	IRDA_start();
 	IRDA_delay_10us(IRDA_DELAY_40US);


 	for(i = 0; i < count1; i++)	//count=7,初值
 	{
 		iBuffer = data1[i];
 		I2CWriteData(iBuffer);
 		IRDA_delay_10us(IRDA_DELAY_40US);
 	}
 	for(i = 0; i < count2; i++)	//count=7,初值
 	{
 		iBuffer = data2[i];
 		I2CWriteData(iBuffer);
 		IRDA_delay_10us(IRDA_DELAY_40US);
 	}
 	for(i = 0; i < count3; i++)	//count=7,初值
 	{
 		iBuffer = data3[i];
 		I2CWriteData(iBuffer);
 		IRDA_delay_10us(IRDA_DELAY_40US);
 	}
 	//写校验码
 	iBuffer = data4[0];
 	I2CWriteData(iBuffer);
 	IRDA_delay_10us(IRDA_DELAY_40US);

 	IRDA_delay_10us(IRDA_DELAY_40US);

 	IRDA_stop();
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_close();
 	IRDA_delay_10us(IRDA_DELAY_40US);
 }

 void writeI2C2(unsigned char *data2, UINT8 count)		//hxd;通用写
 {
 	UINT8 i;
 	UINT8 j = 0;
 	unsigned char iBuffer;

 	IRDA_open();
 	IRDA_delay_10us(IRDA_DELAY_40US);

 	IRDA_SCL_L();
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_SCL_H();

 	IRDA_delay_1ms(IRDA_DELAY_20MS); //20ms	//14

 	IRDA_start();
 	IRDA_delay_10us(IRDA_DELAY_40US);


 	for(i = 0; i < count; i++)	//count=7,初值
 	{
 		iBuffer = data2[i];
 		I2CWriteData(iBuffer);
 		IRDA_delay_10us(IRDA_DELAY_40US);
 	}


 	IRDA_delay_10us(IRDA_DELAY_40US);

 	IRDA_stop();
 	IRDA_delay_10us(IRDA_DELAY_40US);

 	IRDA_close();
 	IRDA_delay_10us(IRDA_DELAY_40US);

 }

 void Learn_start2(void)
 {

 	IRDA_open();
 	IRDA_delay_10us(IRDA_DELAY_40US);

 	IRDA_SCL_L();
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_SCL_H();

 	IRDA_delay_1ms(IRDA_DELAY_20MS); //20ms	//14
 // 	delay_20ms();

 	IRDA_start();
 	IRDA_delay_10us(IRDA_DELAY_40US);

 	I2CWriteData(0x30);
 	IRDA_delay_10us(IRDA_DELAY_40US);

 	I2CWriteData(0x20);
 	IRDA_delay_10us(IRDA_DELAY_40US);

 	I2CWriteData(0x50);
 	IRDA_delay_10us(IRDA_DELAY_40US);

 	IRDA_delay_10us(IRDA_DELAY_40US);

 	IRDA_stop();
 	IRDA_delay_10us(IRDA_DELAY_40US);

 	IRDA_close();
 	IRDA_delay_10us(IRDA_DELAY_40US);

 }
 UINT16 I2CReadData(UINT8* pbData)
 {
 //	UINT8 Data_Bit; UINT8 ACKSign;
 	UINT8 readdata = 0;
 	UINT8 i=8;
 	IRDA_SET_SDA_IN();

 	while (i--)
 	{
 		readdata<<=1;

 		IRDA_SCL_H();
 		IRDA_delay_10us(IRDA_DELAY_40US);

 		readdata |= IRDA_GET_DATA() ; //读不到时可以这样试：readdata |= IRDA_GET_DATA()?0x01:0x00;
 //		readdata |= IRDA_GET_DATA()?0x01:0x00;
 		IRDA_SCL_L();
 		IRDA_delay_10us(IRDA_DELAY_40US);
 		IRDA_delay_10us(IRDA_DELAY_40US);		//hxd;加delay
 	}
 	IRDA_SCL_L();
 	IRDA_delay_10us(IRDA_DELAY_40US);

 	*pbData = readdata;

 	IRDA_sendACKsign();

 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);		//hxd;1G以上的系统要加,test


 	return I2CERR_NO_ERROR;
 }
 //===================
 UINT8  readI2C(unsigned char* readtempbuf,unsigned char *ch)     //UINT8
 {
 	UINT8 bValue;
 	UINT8 i=0,j=0;
 	UINT8 checksum;

 	IRDA_open();
 	IRDA_delay_10us(IRDA_DELAY_40US);

 	IRDA_SCL_L();
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_SCL_H();

 	IRDA_delay_1ms(IRDA_DELAY_20MS); //20ms
 //	delay_20ms();


 	IRDA_start();
 	IRDA_delay_10us(IRDA_DELAY_40US);

 	//----------------------------------------
 	//write
 	I2CWriteData(0x30);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	//address point
 	I2CWriteData(0x62);
 	IRDA_delay_10us(IRDA_DELAY_40US);

 	//---------------------------------------
 	//read
 	IRDA_start();
 	IRDA_delay_10us(IRDA_DELAY_40US);

 	I2CWriteData(0x31);
 	IRDA_delay_10us(IRDA_DELAY_40US);

 	I2CReadData(&bValue);			//wjs;read:FCS(1B)
 	IRDA_delay_10us(IRDA_DELAY_40US);			//wjs;1G以上的系统要加

 	if(bValue != 0x00)
 	{
 		IRDA_stop();
 		IRDA_delay_10us(IRDA_DELAY_40US);
 		IRDA_close();
 		IRDA_delay_10us(IRDA_DELAY_40US);
 	//	SendData(dat[0]);
 	//	kal_prompt_trace(MOD_TST, "remote_study_type_error");
 		return 0;
 	}

 	i = 0;
 //	readtempbuf[i] = bValue;
 //	kal_prompt_trace(MOD_TST, "remote_I2C_data[%d] = %d",i,readtempbuf[i]);
 	checksum = 0xc3;

/* 	for(i = 1,j=0; i < 230; i++)			//wjs;read:learndata(109B)+120=230,更改后只接收89个，后面数据丢弃
 	{
 //		printf("2");
 		I2CReadData(&bValue);

 		IRDA_delay_10us(IRDA_DELAY_40US);
 		readtempbuf[i]=bValue;
 		if(i>=39&&i<171)
 	//	if(i>=38&&i<106)
 		{
 		readtempbuf[j] = bValue;
 	//	kal_prompt_trace(MOD_TST, "remote_I2C_data[%d] = %d",i,readtempbuf[i]);
 		j++;
 		}

 		if(i==177)
 		{
 			*ch = bValue;
 		}
 	//	checksum += bValue;
 	}*/

 	for(i = 1; i < 230; i++)			//wjs;read:learndata(109B)+120=230
 	{
 		I2CReadData(&bValue);
 		IRDA_delay_10us(IRDA_DELAY_40US);
 		readtempbuf[i] = bValue;
 		printf("%02x ",bValue);
 	//	kal_prompt_trace(MOD_TST, "remote_I2C_data[%d] = %d",i,readtempbuf[i]);
 		checksum += bValue;
 		if(i==177)
 		{
 			*ch = bValue;
 		}

 	}

 	I2CReadData(&bValue);		//wjs;read:CK(1B)	?????
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	//	kal_prompt_trace(MOD_MMI, "remote_read_checksum = %d",bValue);
 	//	kal_prompt_trace(MOD_MMI, "remote_count_checksum = %d",checksum);

 	IRDA_stop();
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_close();
 	IRDA_delay_10us(IRDA_DELAY_40US);
 //暂时不校
 //	if(bValue != checksum)
 //	{
 //	//	kal_prompt_trace(MOD_MMI, "remote_study_checksum_error");
 //	return 0;lg,
 //	}
 //	else
 //	{
 //	//	kal_prompt_trace(MOD_MMI, "remote_study_checksum_ok");
 //		return 1;
 //	}
 	return 1;
 }
 uint8_t readI2C2(uint8_t* readtempbuf)     //UINT8
 {
 	UINT8 bValue;
 	UINT8 i=0;
 	UINT8 checksum;

 	IRDA_open();
 	IRDA_delay_10us(IRDA_DELAY_40US);

 	IRDA_SCL_L();
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_SCL_H();
 	IRDA_delay_1ms(IRDA_DELAY_20MS); //20ms

 	IRDA_start();
 	IRDA_delay_10us(IRDA_DELAY_40US);

 	//----------------------------------------
 	//write
 	I2CWriteData(0x30);
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	//address point
 	I2CWriteData(0x62);
 	IRDA_delay_10us(IRDA_DELAY_40US);

 	//---------------------------------------
 	//read
 	IRDA_start();
 	IRDA_delay_10us(IRDA_DELAY_40US);

 	I2CWriteData(0x31);
 	IRDA_delay_10us(IRDA_DELAY_40US);

 	I2CReadData(&bValue);			//wjs;read:FCS(1B)
 	IRDA_delay_10us(IRDA_DELAY_40US);			//wjs;1G以上的系统要加

 	if(bValue != 0x00)
 	{
 		IRDA_stop();
 		IRDA_delay_10us(IRDA_DELAY_40US);
 		IRDA_close();
 		IRDA_delay_10us(IRDA_DELAY_40US);
 		return 0;
 	}
 	i = 0;
 	readtempbuf[i] = bValue;
 	checksum = 0xc3;

 	for(i = 1; i < 230; i++)			//wjs;read:learndata(109B)+120=230
 	{
 		I2CReadData(&bValue);
 		IRDA_delay_10us(IRDA_DELAY_40US);
 		readtempbuf[i] = bValue;
 //		kal_prompt_trace(MOD_TST, "remote_I2C_data[%d] = %d",i,readtempbuf[i]);
 		checksum += bValue;
 	}
 	I2CReadData(&bValue);		//wjs;read:CK(1B)	?????
 	IRDA_delay_10us(IRDA_DELAY_40US);

 	IRDA_stop();
 	IRDA_delay_10us(IRDA_DELAY_40US);
 	IRDA_close();
 	IRDA_delay_10us(IRDA_DELAY_40US);
// //暂时不校
// 	if(bValue != checksum)
// 	{
// 	//	kal_prompt_trace(MOD_MMI, "remote_study_checksum_error");
// 	return 0;
// 	}
// 	else
// 	{
// 	//	kal_prompt_trace(MOD_MMI, "remote_study_checksum_ok");
// 		return 1;
// 	}
 	return 1;
 }













