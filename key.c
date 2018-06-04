/*
 * key.c
 *
 *  Created on: Dec 11, 2017
 *      Author: markin
 *
 *      KEY_SET_IO  	(GPIO_NUM_5)
 */


#include "esp_system.h"
#include "driver/gpio.h"

#include "key.h"

uint8_t keybuff[KEYBUFFSIZE] = {0};
uint8_t key_indexW = 0;
uint8_t key_indexR = 0;
uint8_t length = 0;
static uint32_t keyCounterL = 0, keyCounterS = 0;
uint8_t keyState = 0;

#define KEY_IN() (gpio_get_level(KEY_SET_IO))

/*
extern void vPortEnterCritical( void ); //ENTER ISR
extern void vPortExitCritical( void );  //EXIT ISR
*/

void initialise_key()
{
	gpio_pad_select_gpio(KEY_SET_IO);
	gpio_set_direction(KEY_SET_IO,GPIO_MODE_INPUT);
	gpio_set_pull_mode(KEY_SET_IO,GPIO_PULLUP_ONLY);
}



void keyScan_writeBuff(uint8_t keyValue)
{
    if(length >= KEYBUFFSIZE )
        return;

 //   vPortEnterCritical();

    length ++;
    keybuff[key_indexW] = keyValue;
    printf("%d\r\n",keybuff[key_indexW]);
    if(++key_indexW >= KEYBUFFSIZE)
        key_indexW = 0;

//    vPortExitCritical();
}


char keyScan_readBuff(void)
{
	uint8_t key;
    if(length == 0) return 0;

//    vPortEnterCritical();

    length --;
    key = keybuff[key_indexR];
    printf("%d\r\n",keybuff[key_indexW]);
    if(++key_indexR >= KEYBUFFSIZE)
        key_indexR = 0;

//    vPortExitCritical();

    return key;
}
uint8_t get_keystate(void)
{
	return keyState;
}
void keyScan(void)
{
    // scan key and write key code to buffer.
    if(keyState == LONG_KEY)
        keyCounterL++;
    else
        keyCounterL =0;

    if(keyState == SHORT_KEY)
        keyCounterS++;
    else
        keyCounterS =0;


    switch(keyState)
    {
        case NO_KEY:
            if(KEY_IN() == 1) keyState = SHORT_KEY;
            else keyState = NO_KEY;
        break;

        case SHORT_KEY:
            if(KEY_IN() != 1) // release key, short key
            {
                keyScan_writeBuff(KEY_CODE + SHORT_KEY_CODE);
                keyState = NO_KEY;
            }
            else
            {
                if(keyCounterS >= 2000/10)
                {
                    keyCounterS = 0;
                    keyScan_writeBuff(KEY_CODE + FIRSTLONG_KEY_CODE);
                    keyState = LONG_KEY;
                }
            }
        break;

        case LONG_KEY:
            if(KEY_IN() != 1) // release key, short key
            {
                keyState = NO_KEY;
            }
            else
            {
                if(keyCounterL >= 250/10)
                {
                    keyCounterL = 0;
                    keyScan_writeBuff(KEY_CODE + AFTERLONG_KEY_CODE);
                }
            }
        break;
    }

}
