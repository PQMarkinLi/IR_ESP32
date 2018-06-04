/* Esptouch example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
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

#include <string.h>
#include <sys/socket.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "errno.h"
#include "port/arpa/inet.h"


#include "testUART1.h"




#include "wifi.h"
#include "key.h"
#include "led.h"
//#include "hxd.h"



const int WIFI_CONNECTED_BIT = BIT0;
const int WIFI_DISCONNECTED_BIT = BIT1;
const int WIFI_LINKED_BIT = BIT2;
const int WIFI_REQUEST_STA_CONNECT_BIT = BIT3;
const int WIFI_STA_GOT_IP_BIT = BIT4;
const int ESPTOUCH_DONE_BIT = BIT5;
const int UDP_CONNCETED_SUCCESS = BIT8;
/* FreeRTOS event group to signal when we are connected & ready to make a request */
EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
/* @brief indicate that the ESP32 is currently connected. */

bool truebit=true;
static const char *TAG = "sc";
const char wifi_manager_nvs_namespace[] = "espwifimgr";
//
wifi_config_t* wifi_manager_config_sta = NULL;
int wificonnect_flag=0;
wifi_config_t *basic_config;
wifi_config_t *wifi_config ;
void smartconfig_example_task(void * parm);
static void sc_callback(smartconfig_status_t status, void *pdata);
uint8_t disconnected_num=0;
uint8_t smart_config_flag =0;
char *ipaddr="";
char irmac[13]="";
uint8_t ir_mac[6];
static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
    	disconnected_num=0;
    	ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
 //   	xEventGroupWaitBits(wifi_event_group, WIFI_LINKED_BIT, false, true, portMAX_DELAY);
    	if(esp_wifi_connect()!=ESP_OK)
    	{
    		xEventGroupSetBits(wifi_event_group, WIFI_REQUEST_STA_CONNECT_BIT);
    	}
    	break;
    case SYSTEM_EVENT_STA_STOP:
    	disconnected_num=0;
    	ESP_LOGI(TAG, "SYSTEM_EVENT_STA_STOP");
    	break;
    case SYSTEM_EVENT_STA_GOT_IP:
    	disconnected_num=0;
    	ESP_LOGI(TAG, "event_handler:SYSTEM_EVENT_STA_GOT_IP!");
    	ESP_LOGI(TAG, "got ip:%s\n",
    	ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));

    	ipaddr=ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip);

    	xEventGroupClearBits(wifi_event_group, WIFI_DISCONNECTED_BIT);
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        xEventGroupSetBits(wifi_event_group, WIFI_STA_GOT_IP_BIT);
        break;
    case SYSTEM_EVENT_STA_LOST_IP:
    	disconnected_num=0;
    	esp_wifi_connect();
    	ESP_LOGI(TAG, "SYSTEM_EVENT_STA_LOST_IP");
    	break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
    	ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        xEventGroupSetBits(wifi_event_group, WIFI_DISCONNECTED_BIT);
        if(smart_config_flag)
        {
        	xEventGroupClearBits(wifi_event_group, WIFI_DISCONNECTED_BIT);
        	xEventGroupSetBits(wifi_event_group, WIFI_REQUEST_STA_CONNECT_BIT);
        }
    	if(disconnected_num<15)
    	{
    		esp_wifi_connect();
    		disconnected_num ++;
    	}
    	else if(disconnected_num>=15)
    	{
    		xEventGroupClearBits(wifi_event_group, WIFI_REQUEST_STA_CONNECT_BIT);
    		gpio_set_level(2,0);
    		smart_config_flag=0;
    		xEventGroupSetBits(wifi_event_group, WIFI_DISCONNECTED_BIT);
    		vTaskDelete(pxSmartConfig);
    	}

        break;
    case SYSTEM_EVENT_SCAN_DONE:
    	ESP_LOGI(TAG, "SYSTEM_EVENT_SCAN_DONE");
    	break;
    case SYSTEM_EVENT_STA_CONNECTED:
    	gpio_set_level(2,0);
    	ESP_LOGI(TAG, "SYSTEM_EVENT_STA_CONNECTED");
    	ESP_ERROR_CHECK(esp_wifi_get_mac(ESP_IF_WIFI_STA,ir_mac));
    	sprintf(irmac,"%02X%02X%02X%02X%02X%02X",
    			ir_mac[0],ir_mac[1],ir_mac[2],ir_mac[3],ir_mac[4],ir_mac[5]);
    	xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    	disconnected_num=0;
    	break;

    default:
        break;
    }
    return ESP_OK;
}


static void sc_callback(smartconfig_status_t status, void *pdata)
{
	switch (status) {
	        case SC_STATUS_WAIT:
	            ESP_LOGI(TAG, "SC_STATUS_WAIT");
	            break;
	        case SC_STATUS_FIND_CHANNEL:
	            ESP_LOGI(TAG, "SC_STATUS_FINDING_CHANNEL");
	            break;
	        case SC_STATUS_GETTING_SSID_PSWD:
	            ESP_LOGI(TAG, "SC_STATUS_GETTING_SSID_PSWD");
	            smartconfig_type_t *type = pdata;
	            if(*type == SC_TYPE_ESPTOUCH)
	            {
	            	 ESP_LOGI(TAG, "SC_TYPE_ESPTOUCH");
	            }
	            else
	            {
	            	ESP_LOGI(TAG, "SC_TYPE_AIRKISS");
	            }
	            break;
	        case SC_STATUS_LINK:
	            ESP_LOGI(TAG, "SC_STATUS_LINK");
	            wifi_config_t *wifi_config = pdata;
	            ESP_LOGI(TAG, "SSID:%s", wifi_config->sta.ssid);
	            ESP_LOGI(TAG, "PASSWORD:%s", wifi_config->sta.password);

	            ESP_ERROR_CHECK( esp_wifi_disconnect() );
	            ESP_ERROR_CHECK(esp_wifi_set_auto_connect(truebit));
	            ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_config) );

	            ESP_ERROR_CHECK( esp_wifi_connect() );
	            break;
	        case SC_STATUS_LINK_OVER:
	            ESP_LOGI(TAG, "SC_STATUS_LINK_OVER");
	            if (pdata != NULL) {
	                uint8_t phone_ip[4] = { 0 };
	                memcpy(phone_ip, (uint8_t* )pdata, 4);
	                ESP_LOGI(TAG, "Phone ip: %d.%d.%d.%d\n", phone_ip[0], phone_ip[1], phone_ip[2], phone_ip[3]);
	            }
	            xEventGroupSetBits(wifi_event_group, ESPTOUCH_DONE_BIT);
	            break;
	        default:
	            break;
	    }
}

//wifi_sta_init

void smartconfig_example_task(void * pvParameters)
{
    EventBits_t uxBits;
    smart_config_flag = 1;
    ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS) );
    ESP_ERROR_CHECK( esp_smartconfig_start(sc_callback) );
    while (1) {
        uxBits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
        if(uxBits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi Connected to ap");
        }
        if(uxBits & ESPTOUCH_DONE_BIT) {
            ESP_LOGI(TAG, "smartconfig over");
            xEventGroupClearBits(wifi_event_group, WIFI_REQUEST_STA_CONNECT_BIT);
            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        	xEventGroupClearBits(wifi_event_group, WIFI_DISCONNECTED_BIT);
        	smart_config_flag = 0;
            esp_smartconfig_stop();

            vTaskDelete(NULL);
        }
    }
}

// APP wifi init
/*void wifi_init_sta(void)

{

    wifi_event_group = xEventGroupCreate();



    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL) );



    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {

        .sta = {

            .ssid = "ETOR-2.4G",

            .password = "Admin2018"

        },

    };



    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );

    ESP_ERROR_CHECK(esp_wifi_start() );



    ESP_LOGI(TAG, "wifi_init_sta finished.");

    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s",

             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);

}*/
void initialise_wifi(void)
{
	tcpip_adapter_init();
	wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(wifi_event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );


    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );

/*    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "ETOR-2.4G",
            .password = "Admin2018"
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );*/

    esp_wifi_set_auto_connect(truebit) ;
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );
     esp_wifi_connect() ;
}

