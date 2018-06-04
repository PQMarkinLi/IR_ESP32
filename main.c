/*
 * main.c
 *
 *  Created on: Dec 19, 2017
 *      Author: markin
 *      add udp 40400 192.168.1.255
 *      add tcp server port 10001
 */
#include <string.h>
#include <stdint.h>
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
#include "port/arpa/inet.h"

#include "testUART1.h"

#include "wifi.h"
#include "tcp.h"
#include "udp.h"
#include "key.h"
#include "led.h"
#include "hxd019_drv.h"




const int SEND_IRDA_BIT = BIT6;
const int LEARN_IRDA_BIT = BIT7;
unsigned int hexlearndata[465] = {0};
uint8_t learn_times;
int txirData_len = 0;
xQueueHandle TcpQueue;
xQueueHandle IrQueue;
int Learn_Task_flag = 0;
TaskHandle_t TCP_LearnIR = NULL;

/*const int KEY_LONG_PRESS_BIT = BIT9;
const int KEY_SHORT_PRESS_BIT = BIT10;*/
//IR
uint8_t irda_data[2048];
uint8_t learndata[232]; // chu li hou de 232 bytes
int irtxlen=0;
uint8_t *irda_data_tcp;
uint8_t learn_flag=3;
/*unsigned char hxddat[17]={
		0x00,0x01,0x02,0x03,0x10,0x11,0x12,0x13,0x20,0x21,0x22,0x23,0x30,0x31,0x32,0x33,0x44
	   0		1	2	3	 4	  5	   6	7	  8	   9	A	B	  C	   D	E	F
};*/
unsigned char buf_LS[]={0x30,0x00,0x01,0xEA,0x0A,0x4C,0x65,0x00,0x00,0xD6};
unsigned char buf_5[]={0x30,0x00,0x01,0x1c,0xE3,0x00,0xFF,0x20,0x88,0xD7};
unsigned char buf_3[]={
		0x01,0x1C,0xE3,0x1F,0xE0,0x37,0xC8,0x1E,0xE1,0x15,0xEA,
		0x0A,0xF5,0x0F,0xF0,0x01,0xFE,0x02,0xFD,0x03,0xFC,0x04,
		0xFB,0x05,0xFA,0x06,0xF9,0x07,0xF8,0x08,0xF7,0x09,0xF6,
		0x12,0xED,0x45,0xBA,0x0C,0xF3,0x12,0xED,0x0D,0xF2,0x0B,
		0xF4,0x10,0xEF,0x11,0xEE,0x0E,0xF1,0x4C,0x65,0x00,0x00
};
unsigned char buf_4[]={
		0x01,0x19,0xE6,0x10,0xEF,0x2A,0xD5,0x14,0xEB,0x45,0xBA,0x01,0xFE,
		0x03,0xFC,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,
		0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0x29,0xD6,
		0x0E,0xF1,0x12,0xED,0x10,0xEF,0x11,0xEE,0x13,0xEC,0x14,0xEB,0x00,
		0xFF,0x00,0x00
};


unsigned char buf_6[232] ={
		0x30,0x03,0x52,0x47,0x00,0x3A,0x16,0x01,0x30,0x02,0x1F,0x00,0x26,0x00,0x69,0x00,0x23,0x00,0x44,0x0A,
		0x23,0x00,0x8C,0x00,0x36,0x02,0x00,0x00,0x1F,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x01,0x12,
		0x21,0x12,0x12,0x12,0x11,0x22,0x11,0x21,0x21,0x11,0x12,0x12,0x12,0x22,0x23,0x45,0xF0,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x07,0x76,0x3F,0x00};//3288·¢OK


unsigned char buf_xfxf[232] ={
		0x30,0x03,
		0x54, 0x4F, 0x00, 0x3A, 0x23, 0x01, 0x1E, 0x01, 0x5D, 0x00,
		0x25, 0x00, 0x99, 0x00, 0x29, 0x00, 0xA1, 0x00, 0x2A, 0x00,
		0x63, 0x03, 0x26, 0x00, 0x8F, 0x00, 0x49, 0x02, 0xD5, 0x0C,
		0x24, 0x00, 0x00, 0x00, 0x22, 0x00, 0x01, 0x11, 0x11, 0x12,
		0x11, 0x21, 0x21, 0x12, 0x11, 0x01, 0x11, 0x11, 0x12, 0x11,
		0x31, 0x21, 0x12, 0x14, 0x56, 0x57, 0xF0, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x06, 0x76, 0x00,
		0x00
};
unsigned char buf_xf[232] = {
		0x30,
		0x03,
		0x53, 0x4B, 0x00, 0x3B, 0x1B, 0x01, 0x19, 0x01, 0x5C, 0x00,
		0x21, 0x00, 0x99, 0x00, 0x21, 0x00, 0x5B, 0x03, 0x21, 0x00,
		0x8B, 0x00, 0x43, 0x02, 0x00, 0x00, 0x1E, 0x00, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x11, 0x11, 0x12,
		0x11, 0x21, 0x21, 0x12, 0x11, 0x01, 0x11, 0x11, 0x12, 0x11,
		0x21, 0x21, 0x12, 0x13, 0x45, 0xF0, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x76, 0x24
};

unsigned char CharToHex(unsigned char bHex)
{
    if(bHex <= 0x09)
    {
        bHex += 0x30;
    }
    else if((bHex>=10)&&(bHex<=15))//Capital
    {
        bHex += 0x37;
    }
    else
    {
        bHex = 0xff;
    }
    return bHex;
}
unsigned char HexToChar(unsigned char bChar)
{
    if((bChar>=0x30)&&(bChar<=0x39))
    {
        bChar -= 0x30;
    }
    else if((bChar>=0x41)&&(bChar<=0x46)) // Capital
    {
        bChar -= 0x37;
    }
    else if((bChar>=0x61)&&(bChar<=0x66)) //littlecase
    {
        bChar -= 0x57;
    }
    else
    {
        bChar = 0xff;
    }
    return bChar;
}
unsigned char Char2ToHex(unsigned char aChar,unsigned char bChar)
{
    if((aChar>=0x30)&&(aChar<=0x39))
    {
    	aChar -= 0x30;
    }
    else if((aChar>=0x41)&&(aChar<=0x46)) // Capital
    {
    	aChar -= 0x37;
    }
    else if((aChar>=0x61)&&(aChar<=0x66)) //littlecase
    {
    	aChar -= 0x57;
    }
    else
    {
    	aChar = 0xff;
    }
    if((bChar>=0x30)&&(bChar<=0x39))
    {
        bChar -= 0x30;
    }
    else if((bChar>=0x41)&&(bChar<=0x46)) // Capital
    {
        bChar -= 0x37;
    }
    else if((bChar>=0x61)&&(bChar<=0x66)) //littlecase
    {
        bChar -= 0x57;
    }
    else
    {
        bChar = 0xff;
    }
    aChar=(aChar<<4)+bChar;
    return aChar;
}
//IR SEND

void vTaskIRDA(void * pvParameters)
{
	static const char *TAG = "irdatx:";
	static uint8_t txirdata[512];
	while(1)
	{
		ESP_LOGI(TAG, "IRDA tx start");
		txirdata[0]=HexToChar(irda_data[0]);
		for(int i=0;i<irtxlen;i=i+2)
		{
			txirdata[i/2]=Char2ToHex(irda_data[i],irda_data[i+1]);
		}

		xEventGroupSetBits(wifi_event_group,SEND_IRDA_BIT);
		vTaskDelay( 500 / portTICK_RATE_MS);
		UINT8 datalen=irtxlen/2;
		ESP_LOGI(TAG, "irtxlen: %d datalen: %d \r\n",irtxlen,datalen);
		writeI2C2(txirdata,datalen);
		for(int i=0;i<datalen;i++)
		{
			printf("%02X ",txirdata[i]);
		}

		ESP_LOGI(TAG, "IRDA tx over");
		xEventGroupClearBits(wifi_event_group,SEND_IRDA_BIT);
		vTaskDelete(NULL);
	}
	vTaskDelete(NULL);
}
void vTaskIRDA1(void * pvParameters)
{
	static const char *TAG = "irda:";
	uint8_t irtxdata[232];
//	IRDA_INIT();
	vTaskDelay( 500 / portTICK_RATE_MS);
	for(int i=0;i<txirData_len;i++)
	{
		irtxdata[i]=irda_data[i];
		printf("%02X ",irtxdata[i]);
	}
	ESP_LOGI(TAG, "IRDA tx irtxdata !");
	while(1)
	{
		ESP_LOGI(TAG, "IRDA tx start");
		xEventGroupSetBits(wifi_event_group,SEND_IRDA_BIT);
		vTaskDelay( 500 / portTICK_RATE_MS);
		writeI2C2(irtxdata,txirData_len);
		vTaskDelay( 1000 / portTICK_RATE_MS);
		ESP_LOGI(TAG, "IRDA tx over");
		xEventGroupClearBits(wifi_event_group,SEND_IRDA_BIT);
		//vTaskDelay( 5000 / portTICK_RATE_MS);
		vTaskDelete(NULL);
	}
	vTaskDelete(NULL);

}
void vTaskIRDA_LS(void * pvParameters)
{
	static const char *TAG = "irda:";


	while(1)
	{
		ESP_LOGI(TAG, "IRDA tx start");
		xEventGroupSetBits(wifi_event_group,SEND_IRDA_BIT);
		vTaskDelay( 500 / portTICK_RATE_MS);
		writeI2C2(buf_LS,sizeof(buf_LS));
		ESP_LOGI(TAG, "IRDA tx over");
		for(int i=0;i<sizeof(buf_LS);i++)
		{
			printf("%02X ",buf_LS[i]);
		}
		xEventGroupClearBits(wifi_event_group,SEND_IRDA_BIT);
		vTaskDelete(NULL);
	}
	vTaskDelete(NULL);
}
void vTaskIRDA_XF(void * pvParameters)
{
	static const char *TAG = "irda:";


	while(1)
	{
		ESP_LOGI(TAG, "IRDA tx start");
		buf_xf[231]=0x00;
		for(int i=0;i<232;i++)
		{
			buf_xf[231]+=buf_xf[i];
		}
		xEventGroupSetBits(wifi_event_group,SEND_IRDA_BIT);
		vTaskDelay( 500 / portTICK_RATE_MS);
		writeI2C2(buf_xf,sizeof(buf_xf));
		ESP_LOGI(TAG, "IRDA tx over");
		for(int i=0;i<sizeof(buf_xf);i++)
		{
			printf("%02X ",buf_xf[i]);
		}
		xEventGroupClearBits(wifi_event_group,SEND_IRDA_BIT);
		vTaskDelete(NULL);
	}
	vTaskDelete(NULL);
}
//IR LEARN
void vTasklearnIRDA1(void * pvParameters)
{
	static const char *TAG = "irda:";
	while(1)
	{
//		ESP_LOGI(TAG, "learn waiting");
//		if(Learn_Task_flag)
		{

//			IRDA_INIT();
			vTaskDelay( 1000 / portTICK_RATE_MS);
			unsigned char sum = 0x00;
			xEventGroupSetBits(wifi_event_group, LEARN_IRDA_BIT);
			ESP_LOGI(TAG, "learn start");
			IRDA_SET_BUSY_IN();//设置引脚为输入

			memset(irda_data,0,sizeof(irda_data));
			Learn_start2();
			vTaskDelay( 50 / portTICK_RATE_MS);
			while(!IRDA_BUSY_S());
			learn_flag = readI2C2(irda_data);

			learndata[0] = 0x30;
			sum += learndata[0];
			learndata[1] = 0x03;
			sum += learndata[1];
			uint8_t i=1;
			for(;i<230;i++)
			{
				learndata[i+1]=irda_data[i];
				sum += irda_data[i];
				printf("0x%02X, ",learndata[i]);
			}
			learndata[231] = sum;
			printf("0x%02X, ",learndata[231]);
			learn_times++;
			if(learn_flag != 0x00)
			{
				learn_times = 0;
			}
	//		printf("learn_flag : %d \r\n",learn_flag);
			sum = 0;
			xEventGroupClearBits(wifi_event_group, LEARN_IRDA_BIT);
			ESP_LOGI(TAG, "learn over");
			vTaskDelay( 500 / portTICK_RATE_MS);
			ESP_LOGI(TAG,"learn_times:%d",learn_times);
			if(learn_times > 9)
			{
				learn_times = 0;
				Learn_Task_flag = 0;
				TCP_LearnIR = NULL;
//				vTaskDelete(NULL);
			}
			vTaskDelete(NULL);
		}
	}
	Learn_Task_flag = 0;
	vTaskDelete(NULL);
}
// status led: blue led,red led
void vTaskKEY(void * pvParameters)
{
	static const char *TAG = "key";
	ESP_LOGI(TAG, "key scan");
    EventBits_t uxBits;
	while(1)
	{
		vTaskDelay( 200 / portTICK_RATE_MS);
		if(gpio_get_level(KEY_SET_IO)==1)
		{

		}
		else
		{
			vTaskDelay(1000 / portTICK_RATE_MS);
			// test learn IR

			if(gpio_get_level(KEY_SET_IO)==0)
			{
				while(gpio_get_level(KEY_SET_IO)==0);
				gpio_set_level(2,1);
				xEventGroupSetBits(wifi_event_group,WIFI_REQUEST_STA_CONNECT_BIT);
				uxBits=xEventGroupWaitBits(wifi_event_group, WIFI_REQUEST_STA_CONNECT_BIT, false, true, portMAX_DELAY);
				if(uxBits & WIFI_REQUEST_STA_CONNECT_BIT){
					ESP_LOGI(TAG, "key set wifi smartconfig");
					ESP_ERROR_CHECK( esp_wifi_disconnect() );
					xTaskCreate(&smartconfig_example_task, "smartconfig_example_task", 4096, NULL, XTASK6, pxSmartConfig);
				}
				ESP_LOGI(TAG, "key press");
				xEventGroupClearBits(wifi_event_group,WIFI_REQUEST_STA_CONNECT_BIT);
			}
			else
			{
				xEventGroupSetBits(wifi_event_group, LEARN_IRDA_BIT);
				//if(Learn_Task_flag == 0 )
				if(TCP_LearnIR == NULL)
					xTaskCreate(&vTasklearnIRDA1, "vTasklearnIRDA1", 2048 , NULL, XTASK7, TCP_LearnIR);
			}
		}
	}
	vTaskDelete(NULL);
}
void vTaskLED(void * pvParameters)
{
	static const char *TAG = "led";
	 EventBits_t uxBits;
	ESP_LOGI(TAG, "led scan");
	while(1)
	{
		vTaskDelay( 200 / portTICK_RATE_MS);
		uxBits = xEventGroupWaitBits(wifi_event_group,  WIFI_CONNECTED_BIT|WIFI_DISCONNECTED_BIT|SEND_IRDA_BIT|LEARN_IRDA_BIT|WIFI_REQUEST_STA_CONNECT_BIT, false, false, portMAX_DELAY);
		if(uxBits & WIFI_CONNECTED_BIT) {
			 if(uxBits & SEND_IRDA_BIT) {
				ESP_LOGI(TAG, "SEND_IRDA_BIT");
				gpio_set_level(LED_BLUE_IO,0);
				vTaskDelay(500 / portTICK_RATE_MS);
			}

			 if(uxBits & LEARN_IRDA_BIT) {
				gpio_set_level(LED_BLUE_IO,0);
				vTaskDelay(300 / portTICK_RATE_MS);
				gpio_set_level(LED_BLUE_IO,1);
				vTaskDelay(300 / portTICK_RATE_MS);
			}
			 else
			 {
				gpio_set_level(LED_BLUE_IO,1);
				gpio_set_level(LED_RED_IO,0);
			 }

		}
		 if(uxBits & WIFI_REQUEST_STA_CONNECT_BIT) {
			gpio_set_level(LED_BLUE_IO,0);
			gpio_set_level(LED_RED_IO,1);
			vTaskDelay(300 / portTICK_RATE_MS);
			gpio_set_level(LED_RED_IO,0);

			vTaskDelay(300 / portTICK_RATE_MS);
		}
		 if(uxBits & WIFI_DISCONNECTED_BIT) {
					gpio_set_level(LED_RED_IO,1);
					gpio_set_level(LED_BLUE_IO,0);
		}
	}
	vTaskDelete(NULL);
}


void app_main()
{
	TaskHandle_t TCP_server = NULL;
	ESP_ERROR_CHECK( nvs_flash_init() );
	initialise_led();
	initialise_key();
	initialise_wifi();
	IRDA_INIT();
	vTaskDelay(1000 / portTICK_RATE_MS);// 3s
	learn_times = 0;
	//chuang jian LED KEY TASK
	xTaskCreate(&vTaskKEY,"vTaskKEY",2048,NULL,XTASK1,NULL);
	xTaskCreate(&vTaskLED,"vTaskLED",2048,NULL,XTASK2,NULL);
	xTaskCreate(&socket_server_tcp, "socket_server_tcp", 4096 ,NULL , XTASK5, TCP_server);
	//yunxing udp zhudong fasong UDP
	xTaskCreate(&udp_send_ip, "udp_send_ip", 4096 ,NULL , XTASK4, NULL);
//	xTaskCreate(&vTasklearnIRDA1, "vTasklearnIRDA1", 2048 ,NULL , XTASK10, &TCP_LearnIR);

	xTaskCreate(&udp_Device_Management, "udp_Device_Management", 4096 ,NULL , XTASK3, NULL);
	while (1)
	{
		static const char *TAG = "loop";
		vTaskDelay(3000 / portTICK_RATE_MS);
		ESP_LOGI(TAG, "current heap size:%d", esp_get_free_heap_size());
		;
	}
}


