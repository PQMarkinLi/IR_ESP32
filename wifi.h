/*
 * wifi.h
 *
 *  Created on: Dec 19, 2017
 *      Author: markin
 */

#ifndef MAIN_WIFI_H_
#define MAIN_WIFI_H_
extern EventGroupHandle_t wifi_event_group;

extern TaskHandle_t xTaskSmartconfig;

extern const int WIFI_CONNECTED_BIT ;
extern const int WIFI_DISCONNECTED_BIT ;
extern const int WIFI_REQUEST_STA_CONNECT_BIT;
extern const int WIFI_STA_GOT_IP_BIT;
extern const int ESPTOUCH_DONE_BIT ;
extern const int UDP_CONNCETED_SUCCESS;
extern const int SEND_IRDA_BIT ;
extern const int LEARN_IRDA_BIT ;
extern char *ipaddr;

extern char irmac[13];
TaskHandle_t * const pxSmartConfig;
void initialise_wifi(void);
void smartconfig_example_task(void * pvParameters);
//void smartconfigTask(void *pvParameters);
#endif /* MAIN_WIFI_H_ */
