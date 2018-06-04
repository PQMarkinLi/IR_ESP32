/*
 * key.h
 *
 *  Created on: Dec 11, 2017
 *      Author: markin
 */

#ifndef MAIN_KEY_H_
#define MAIN_KEY_H_

#define KEY_SET_IO  	(GPIO_NUM_5)
#define KEYBUFFSIZE 4

#define NO_KEY 0
#define SHORT_KEY 1
#define LONG_KEY 2

#define KEY_CODE (unsigned char)0x00
#define SHORT_KEY_CODE (unsigned char)0x01
#define FIRSTLONG_KEY_CODE (unsigned char)0x02
#define AFTERLONG_KEY_CODE (unsigned char)0x03


void initialise_key(void);
void keyScan(void);
char keyScan_readBuff(void);
uint8_t get_keystate(void);
#endif /* MAIN_KEY_H_ */
