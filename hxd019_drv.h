/*
 * hxd019_drv.h
 *
 *  Created on: Jan 15, 2018
 *      Author: markin
 */

#ifndef MAIN_HXD019_DRV_H_
#define MAIN_HXD019_DRV_H_

#include 	"DataType.h"
#define     IRDA_RECV_NUM 230
#define		IRDA_NO_ERROR	0
#define 	I2CERR_NO_ERROR	0
#define		IRDA_PORT			I2C1
/*
#define		IRDA_PORT_CLK		RCC_APB1Periph_I2C1
#define		IRDA_PORT_IO		GPIOB
#define		IRDA_PORT_IO_CLK	RCC_APB2Periph_GPIOB
*/
#define		IRDA_SCL_PIN		GPIO_NUM_26
#define		IRDA_SDA_PIN		GPIO_NUM_25
#define		IRDA_BUSY_PIN		GPIO_NUM_21

#define		IRDA_DELAY_40US		4
#define		IRDA_DELAY_20MS		20

#define		IRDA_SCL_H()	gpio_set_level(IRDA_SCL_PIN,1)
#define		IRDA_SCL_L()	gpio_set_level(IRDA_SCL_PIN,0)
#define		IRDA_SDA_H()	gpio_set_level(IRDA_SDA_PIN,1)
#define		IRDA_SDA_L()	gpio_set_level(IRDA_SDA_PIN,0)

#define		IRDA_GET_ACK()	gpio_get_level(IRDA_SDA_PIN)
#define		IRDA_GET_DATA()	gpio_get_level(IRDA_SDA_PIN)

#define		IRDA_BUSY_H()	gpio_set_level(IRDA_BUSY_PIN,1)
#define		IRDA_BUSY_L()	gpio_set_level(IRDA_BUSY_PIN,0)

#define		IRDA_BUSY_S()	gpio_get_level(IRDA_BUSY_PIN)

/*
#define         IRDA_LED_OFF()  gpio_set_level(GPIO_NUM_32,0)
#define         IRDA_LED_ON()   gpio_set_level(GPIO_NUM_32,1)
*/
#ifndef UINT8
typedef unsigned char UINT8;
#endif

#ifndef UINT16
typedef unsigned short UINT16;
#endif

void	IRDA_INIT();
void	IRDA_delay_10us(czx_vu32 t);
void	IRDA_delay_1ms(czx_vu32 t);
void	IRDA_SET_SDA_OUT(void);
void	IRDA_SET_SDA_IN(void);
void	IRDA_SET_SCL_OUT(void);
void	IRDA_SET_BUSY_IN(void);
void	IRDA_SET_BUSY_OUT(void);
UINT8	IRDA_getACKsign(void);
void	IRDA_sendACKsign(void);

UINT16	IRDA_open(void);
#define		IRDA_close()  IRDA_open()

UINT16	IRDA_start(void);
UINT16	IRDA_stop(void);
/*UINT16	IRDA_write(UINT8 d);
void		IRDA_tx_data(UINT8 *d,UINT8 len);//·¢ËÍÊýŸÝ
UINT16		IRDA_read(UINT8 *d);
void		IRDA_learn_start(void);

UINT8		IRDA_rx_data(UINT8 *d);
UINT8		IRDA_learn_data_in_out(UINT8*learn_data_out);
UINT8		IRDA_get_remote_study_data(UINT8 *cmd_data);

//UINT8		IRDA_learn_data_in_out(UINT8	*pd);//Ñ§Ï°Ò£¿Ø
UINT16 I2CReadData(UINT8* pbData);
UINT8 readI2C(UINT8* readtempbuf) ;*/
UINT8 hxdRwork(unsigned char *buf);
void hxdSwork(unsigned char buf[],unsigned char len);
void Learn_start2(void);
UINT8  readI2C(unsigned char* readtempbuf,unsigned char *ch) ;
uint8_t readI2C2(uint8_t* readtempbuf)  ;
UINT16 I2CWriteData(unsigned char bData);
void writeI2C(unsigned char *data1, UINT8 count1,unsigned char *data2, UINT8 count2,unsigned char *data3, UINT8 count3,unsigned char *data4);		//hxd;通用写
void writeI2C2(unsigned char *data2, UINT8 count);

#endif /* MAIN_HXD019_DRV_H_ */
