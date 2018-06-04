/*
 * udp.c
 *
 *  Created on: Mar 15, 2018
 *      Author: markin
 */


/*
 * udp.c
 *
 *  Created on: Mar 14, 2018
 *      Author: markin
 */
#include <cJSON.h>
#include <string.h>
#include <sys/socket.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"


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

#include "lwip/ip_addr.h"

#include "udp.h"
#include "tcp.h"
#include "wifi.h"
/* FreeRTOS event group to signal when we are connected to WiFi and ready to start UDP test*/

#define EXAMPLE_DEFAULT_PORT 40000
static int udpsocket;

static struct sockaddr_in remote_addr;
static unsigned int socklen;

int total_data = 0;
int success_pack = 0;

//create a udp server socket. return ESP_OK:success ESP_FAIL:error
esp_err_t create_udp_server()
{
	static const char *TAG = "udp";
    ESP_LOGI(TAG, "create_udp_server() port:%d", EXAMPLE_DEFAULT_PORT);
    udpsocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpsocket < 0) {
    	//show_socket_error_reason(udpsocket);
	return ESP_FAIL;
    }
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(EXAMPLE_DEFAULT_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(udpsocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    	show_udp_socket_error_reason(udpsocket);
	close(udpsocket);
	return ESP_FAIL;
    }
    return ESP_OK;
}

//create a udp client socket. return ESP_OK:success ESP_FAIL:error
esp_err_t create_udp_client(char* server_ip,int server_port)
{
	static const char *TAG = "UDP";
    ESP_LOGI(TAG, "create_udp_client()");
    ESP_LOGI(TAG, "connecting to %s:%d",
    		server_ip, server_port);
    udpsocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpsocket < 0) {
    	show_udp_socket_error_reason(udpsocket);
	return ESP_FAIL;
    }
    /*for client remote_addr is also server_addr*/
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(server_port);
    remote_addr.sin_addr.s_addr = inet_addr(server_ip);


    return ESP_OK;
}
int get_udp_socket_error_code(int socket)
{
	static const char *TAG = "udp";
    int result;
    u32_t optlen = sizeof(int);
    if(getsockopt(socket, SOL_SOCKET, SO_ERROR, &result, &optlen) == -1) {
	ESP_LOGE(TAG, "getsockopt failed");
	return -1;
    }
    return result;
}

int show_udp_socket_error_reason( int socket)
{
	static const char *TAG = "udp";
    int err = get_udp_socket_error_code(socket);
    ESP_LOGW(TAG, "socket error %d %s", err, strerror(err));
    return err;
}

int check_udp_connected_socket()
{
	static const char *TAG = "udp";
    int ret;
    ESP_LOGD(TAG, "check connect_socket");
    ret = get_udp_socket_error_code(udpsocket);
    if(ret != 0) {
    	ESP_LOGW(TAG, "socket error %d %s", ret, strerror(ret));
    }
    return ret;
}
void close_udp_socket()
{
	close(udpsocket);
}


//send or recv data task

void send_recv_data(void *pvParameters)
{
	static const char *TAG = "udp";
    ESP_LOGI(TAG, "task send_recv_data start!\n");

    int len;
    char databuff[UDP_DEFAULT_PKTSIZE];

    /*send&receive first packet*/
    socklen = sizeof(remote_addr);
//    memset(databuff, EXAMPLE_PACK_BYTE_IS, UDP_DEFAULT_PKTSIZE);
#if EXAMPLE_ESP_UDP_MODE_SERVER
    ESP_LOGI(TAG, "first recvfrom:");
    len = recvfrom(udpsocket, databuff, UDP_DEFAULT_PKTSIZE, 0, (struct sockaddr *)&remote_addr, &socklen);
#else
    ESP_LOGI(TAG, "first sendto:");
    len = sendto(udpsocket, databuff, UDP_DEFAULT_PKTSIZE, 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
#endif

    if (len > 0) {
	ESP_LOGI(TAG, "transfer data with %s:%u\n",
		inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port));
	xEventGroupSetBits(wifi_event_group, UDP_CONNCETED_SUCCESS);
    } else {

	close(udpsocket);
	vTaskDelete(NULL);
    } /*if (len > 0)*/

#if EXAMPLE_ESP_UDP_PERF_TX
    vTaskDelay(500 / portTICK_RATE_MS);
#endif
    ESP_LOGI(TAG, "start count!\n");
    while(1) {
#if EXAMPLE_ESP_UDP_PERF_TX
	len = sendto(udpsocket, databuff, UDP_DEFAULT_PKTSIZE, 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
#else
	len = recvfrom(udpsocket, databuff, UDP_DEFAULT_PKTSIZE, 0, (struct sockaddr *)&remote_addr, &socklen);
#endif
	if (len > 0) {
	    total_data += len;
	    success_pack++;
	} else {
	    if (LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG) {
	    }
	} /*if (len > 0)*/
    } /*while(1)*/
}
// device discovery

//send UDP
void udp_send_ip(void *pvParameters)
{
	static const char *TAG = "udp_send_ip";
	int udpsocket_ip;
    ESP_LOGI(TAG, "create_udp_server() port:%d", EXAMPLE_DEFAULT_PORT);
    udpsocket_ip = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpsocket_ip < 0) {
    	//show_socket_error_reason(udpsocket);
	return ESP_FAIL;
    }
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(EXAMPLE_DEFAULT_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(udpsocket_ip, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    	show_udp_socket_error_reason(udpsocket);
	close(udpsocket_ip);
    }
	/*send&receive first packet*/
	socklen = sizeof( server_addr);
	int i=0;
	fd_set rfd;//描述符集 这个将用来测试有没有一个可用的连接
	struct timeval timeout;
	timeout.tv_sec = 60; //等下select用到这个
	timeout.tv_usec = 0;
	char databuff[1024]={0};
	cJSON *udprx;
	while(1)
	{
		FD_ZERO(&rfd); //总是这样先清空一个描述符集
		FD_SET(udpsocket_ip,&rfd);
		switch(select(udpsocket_ip+1,&rfd,NULL,NULL,&timeout))
		{
		case -1:
			break;
		case 0:
			break;
		default:
			if(FD_ISSET(udpsocket_ip,&rfd))
			{
				int len = recvfrom(udpsocket_ip, databuff, UDP_DEFAULT_PKTSIZE, 0, (struct sockaddr *)&remote_addr, &socklen);
				udprx = cJSON_Parse(databuff);
//				printf("%s \r\n",databuff);
				cJSON * item = cJSON_GetObjectItem(udprx,"Action");
				char *Action = "SeekDevice";
//				printf("%s \r\n",item->valuestring);
				if(strcmp(item->valuestring,Action)==0)
				{
					cJSON *root,*data_body;
					char name[24]="IRTransmitter";
					uint32_t addr=ipaddr_addr(ipaddr);
					uint8_t addr8=(addr>>24);
					char achar[10];
					sprintf(achar,"%d",addr8);
					strcat(name,achar);
//					ESP_LOGI(TAG, "achar %s \n",achar);
					char *out="";
					out = (char*) malloc(256 * sizeof(char));
//					ESP_LOGI(TAG, "wifi ip %s \n",ipaddr);
//					ESP_LOGI(TAG, "ip %s \n",name);
					root = cJSON_CreateObject();
					data_body=cJSON_CreateObject();
					cJSON_AddStringToObject(root,"Action","Alive");
					cJSON_AddStringToObject(root,"DeviceType","IRTransmitter");
					cJSON_AddStringToObject(root,"MACID",irmac);
					cJSON_AddStringToObject(root,"IP",ipaddr);
					cJSON_AddStringToObject(root,"Name",name);
					cJSON_AddStringToObject(root,"Version","1.0.0");
					cJSON_AddStringToObject(root,"Activated","");
					cJSON_AddStringToObject(root,"ExpiryDate","");
		//			tcp server port : 10101
					cJSON_AddStringToObject(data_body,"Port","10101");
					cJSON_AddItemToObject(root,"Data",data_body);
					out = cJSON_Print(root);
//					ESP_LOGI(TAG, " %s \n",out);
					for(int j=3;j>0;j--)
					{
						sendto(udpsocket_ip, out, strlen(out), 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
						vTaskDelay(2000 / portTICK_RATE_MS);
					}
					i++;
//					ESP_LOGI(TAG, "send_udp_ip %d times.\n",i);
					free(out);
					cJSON_Delete(root);
				}
				cJSON_Delete(udprx);
			}//end if
			break;
		}//end switch


	}// end while
	vTaskDelete(NULL);

}

void udp_Device_Management(void *pvParameters)
{
	static const char *TAG = "udp_Device_Management";
	static int udpsocket_dm;
	char* server_ip="192.168.1.255";
	int server_port=40000;
	xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT,false, true, portMAX_DELAY);
	ESP_LOGI(TAG, "task udp_Device_Management start!\n");
	xEventGroupWaitBits(wifi_event_group, WIFI_STA_GOT_IP_BIT,false, true, portMAX_DELAY);
	/*create udp socket*/
	ESP_LOGI(TAG, "create udp client after 3s...");
	vTaskDelay(3000 / portTICK_RATE_MS);


    ESP_LOGI(TAG, "create_udp_client()");
    ESP_LOGI(TAG, "connecting to %s:%d",
    		server_ip, server_port);
    udpsocket_dm = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpsocket_dm < 0) {
    	show_udp_socket_error_reason(udpsocket_dm);
	return ESP_FAIL;
    }
    static struct sockaddr_in remote_addr_dm;
    /*for client remote_addr is also server_addr*/
    remote_addr_dm.sin_family = AF_INET;
    remote_addr_dm.sin_port = htons(server_port);
    remote_addr_dm.sin_addr.s_addr = inet_addr(server_ip);
//	int socket_ret_DM = create_udp_client("192.168.1.255",40000);
	/*send&receive first packet*/
	socklen = sizeof( remote_addr_dm);
	while(1)
	{
		cJSON *root,*data_body;
		char name[24]="IRTransmitter";
		uint32_t addr=ipaddr_addr(ipaddr);
		uint8_t addr8=(addr>>24);
		char achar[10];
		sprintf(achar,"%d",addr8);
		strcat(name,achar);
		char *out="";
		out = (char*) malloc(256 * sizeof(char));
		root = cJSON_CreateObject();
		cJSON_AddStringToObject(root,"Company","ETOR");
		cJSON_AddStringToObject(root,"PartNumber","IF-3001R");
		cJSON_AddStringToObject(root,"MACID",irmac);
		cJSON_AddStringToObject(root,"BatchNumber","20180507");
		cJSON_AddStringToObject(root,"SoftwareVersion","V1.0.0");
		cJSON_AddStringToObject(root,"IP",ipaddr);
		cJSON_AddStringToObject(root,"Port","40000");
		out = cJSON_Print(root);
		//ESP_LOGI(TAG, " %s \n",out);
		sendto(udpsocket_dm, out, strlen(out), 0, (struct sockaddr *)&remote_addr_dm, sizeof(remote_addr_dm));
		vTaskDelay(1000 / portTICK_RATE_MS);
		free(out);
		cJSON_Delete(root);
	}
	vTaskDelete(NULL);
}

