/*
 * hxd.h
 *
 *  Created on: Dec 15, 2017
 *      Author: markin
 */

#ifndef _HXD_H_
#define _HXD_H_
/*
 * HXD019D.h
 *
 *  Created on: Dec 15, 2017
 *      Author: markin
 */


/*****************************************************
*
*ÖÆ×÷ÈË£º
*
* µ¥Î»:    ÉîÛÚÊÐºêÐŸŽï¿ÆŒŒÓÐÏÞ¹«ËŸ
*
* ²¿ÃÅ:   ŒŒÊõ²¿
*
* Ä¿µÄ£º  STM32ÓëHXD019ÍšÑ¶
*
* ÈÕÆÚ:   2013ÄêÏÄ
******************************************************/




/*

*/

#include"DataType.h"
#define     IRDA_RECV_NUM 230
#define		IRDA_NO_ERROR	0
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
czx_u8	IRDA_getACKsign(void);
void	IRDA_sendACKsign(void);

czx_u16	IRDA_open(void);
#define		IRDA_close()  IRDA_open()

czx_u16	IRDA_start(void);
czx_u16	IRDA_stop(void);
czx_u16	IRDA_write(czx_vu8 d);
void		IRDA_tx_data(czx_vu8 *d,czx_vu8 len);//·¢ËÍÊýŸÝ
czx_u16		IRDA_read(czx_vu8 *d);
void		IRDA_learn_start(void);
czx_u8		IRDA_rx_data(czx_vu8 *d);
czx_u8		IRDA_learn_data_in_out(czx_vu8*learn_data_out);
czx_u8		IRDA_get_remote_study_data(czx_vu8 *cmd_data);

//czx_vu8		IRDA_learn_data_in_out(czx_vu8	*pd);//Ñ§Ï°Ò£¿Ø
UINT16 I2CReadData(UINT8* pbData);
uint8_t readI2C(uint8_t* readtempbuf) ;
#endif /* MAIN_HXD_H_ */
