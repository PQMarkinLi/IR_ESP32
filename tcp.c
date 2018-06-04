/*
 * tcp.c
 *
 *  Created on: Dec 19, 2017
 *      Author: markin
 */

/* tcp_perf Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "errno.h"
#include "port/arpa/inet.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>

#include "hxd019_drv.h"
#include "tcp.h"
#include "wifi.h"


static const char *TAG = "TCP:";
/* FreeRTOS event group to signal when we are connected to wifi */
//EventGroupHandle_t tcp_event_group;

/*socket*/

static int server_socket = 0;
static struct sockaddr_in server_addr;
static struct sockaddr_in client_addr;
static int connect_socket = 0;
bool g_rxtx_need_restart = false;
static unsigned int socklen = sizeof(client_addr);
int g_total_data = 0;






//use this esp32 as a tcp server. return ESP_OK:success ESP_FAIL:error
esp_err_t create_tcp_server(int port)
{
	static const char *TAG = "create_tcp_server:";
    ESP_LOGI(TAG, "server socket....port=%d\n", port);
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        show_tcp_socket_error_reason("create_server", server_socket);
        return ESP_FAIL;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        show_tcp_socket_error_reason("bind_server", server_socket);
        close(server_socket);
        return ESP_FAIL;
    }
    if (listen(server_socket, 5) < 0) {
        show_tcp_socket_error_reason("listen_server", server_socket);
        close(server_socket);
        return ESP_FAIL;
    }
    connect_socket = accept(server_socket, (struct sockaddr *)&client_addr, &socklen);
    if (connect_socket < 0) {
        show_tcp_socket_error_reason("accept_server", connect_socket);
        close(server_socket);
        return ESP_FAIL;
    }
    /*connection established，now can send/recv*/
    ESP_LOGI(TAG, "tcp connection established!");
    return ESP_OK;
}
//use this esp32 as a tcp client. return ESP_OK:success ESP_FAIL:error
esp_err_t create_tcp_client()
{
    ESP_LOGI(TAG, "client socket....serverip:port=%s:%d\n",
             EXAMPLE_DEFAULT_SERVER_IP, EXAMPLE_DEFAULT_PORT);
    connect_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (connect_socket < 0) {
        show_tcp_socket_error_reason("create client", connect_socket);
        return ESP_FAIL;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(EXAMPLE_DEFAULT_PORT);
    server_addr.sin_addr.s_addr = inet_addr(EXAMPLE_DEFAULT_SERVER_IP);
    ESP_LOGI(TAG, "connecting to server...");
    if (connect(connect_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        show_tcp_socket_error_reason("client connect", connect_socket);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "connect to server success!");
    return ESP_OK;
}



int get_tcp_socket_error_code(int socket)
{
    int result;
    uint32_t optlen = sizeof(int);
    int err = getsockopt(socket, SOL_SOCKET, SO_ERROR, &result, &optlen);
    if (err == -1) {
        ESP_LOGE(TAG, "getsockopt failed:%s", strerror(err));
        return -1;
    }
    return result;
}

int show_tcp_socket_error_reason(const char *str, int socket)
{
    int err = get_tcp_socket_error_code(socket);

    if (err != 0) {
        ESP_LOGW(TAG, "%s socket error %d %s", str, err, strerror(err));
    }

    return err;
}

int check_tcp_working_socket()
{
    int ret;
#if EXAMPLE_ESP_TCP_MODE_SERVER
    ESP_LOGD(TAG, "check server_socket");
    ret = get_tcp_socket_error_code(server_socket);
    if (ret != 0) {
        ESP_LOGW(TAG, "server socket error %d %s", ret, strerror(ret));
    }
    if (ret == ECONNRESET) {
        return ret;
    }
#endif
    ESP_LOGD(TAG, "check connect_socket");
    ret = get_tcp_socket_error_code(connect_socket);
    if (ret != 0) {
        ESP_LOGW(TAG, "connect socket error %d %s", ret, strerror(ret));
    }
    if (ret != 0) {
        return ret;
    }
    return 0;
}

void close_tcp_socket()
{
    close(connect_socket);
    close(server_socket);
}



void socket_client_tcp(void *pvParameters)
{
	static const char *TAG = "socket_client_tcp:";
	ESP_LOGD(TAG, "socket_client_tcp ");
	char *data = (char *)pvParameters;
	int sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	inet_pton(AF_INET, "192.168.1.186", &serverAddress.sin_addr.s_addr);
	serverAddress.sin_port = htons(9999);
	int rc = connect(sock, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr_in));
	ESP_LOGD(TAG, "connect rc: %d", rc);

	rc = send(sock, data, strlen(data), 0);
	ESP_LOGD(TAG, "send: rc: %d", rc);
	rc = close(sock);
	ESP_LOGD(TAG, "close: rc: %d", rc);
	vTaskDelete(NULL);
}
void socket_server_tcp(void *pvParameters)
{
	static const char *TAG = "socket_server_tcp:";
	xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT,false, true, portMAX_DELAY);
	struct IrLearnMessage * senddata;
	struct sockaddr_in serverAddress;
	char databuf[232];
	//create a socket that we will listen upon
	int sock=socket(AF_INET,SOCK_STREAM,0);
	if (sock<0)
	{
		ESP_LOGE(TAG,"socket: %d %s",sock,strerror(errno));
		goto END;
	}
	bzero(&serverAddress,sizeof(serverAddress));
	//bind our server socket to a port
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(SERVER_PORT);
	int rc = bind(sock, (struct sockaddr *)&serverAddress,sizeof(serverAddress));
	if(rc<0)
	{
		ESP_LOGE(TAG,"bind: %d %s",rc,strerror(errno));
		goto END;
	}
	ESP_LOGI(TAG, "bind");
	//flag the socket as listening for new connections
	rc = listen(sock,5);
	if(rc<0)
	{
		ESP_LOGE(TAG,"listen: %d %s",rc,strerror(errno));
		goto END;
	}
	ESP_LOGI(TAG, "listen");
	//receive data
	int clientSock;
	fd_set rfd;//描述符集 这个将用来测试有没有一个可用的连接
	struct timeval timeout;
	timeout.tv_sec = 3; //等下select用到这个
	timeout.tv_usec = 0;
	struct sockaddr_in clientAddress;

	while(1){
		//listen for a new client connection
		clientSock = accept(sock,(struct sockaddr *)&clientAddress,&socklen);
		if(clientSock<0){
			ESP_LOGE(TAG,"accept: %d %s",clientSock,strerror(errno));
			close(sock);
			goto END;
		}
		ESP_LOGI(TAG, "accept");
		//we now have a new client...
		char buf_ip[255];
		memset(buf_ip, 0, sizeof(buf_ip));
//		inet_ntop(AF_INET, &clientAddress.sin_addr, buf_ip, sizeof(buf_ip));
		//loop reading data
		int len;
		while(1)
		{
			char buf[256];
			uint8_t s_buf[256];
			char learn_fail[4]={0xfd,0xa0,0x00,0x00};
			memset(buf,0,sizeof(buf));
			FD_ZERO(&rfd); //总是这样先清空一个描述符集
			FD_SET(clientSock,&rfd);
			if (send(clientSock,NULL,0,0) != 0)
			{
				ESP_LOGI(TAG,"close clientSock");
				close(clientSock);
				break;
			}
			switch(select(clientSock+1,&rfd,NULL,NULL,&timeout))
			{
			case 0:
				if(learn_flag == 1)
							{
//								printf("learn_flag=1 : %d \r\n",learn_flag);
								char learn_succ[236] = { 0xfd,0xa1,0x00,0xe8};
								learn_succ[3] = sizeof(learndata);
								for(int i=0;i<learn_succ[3];i++)
								{
									learn_succ[i+4] = learndata[i];
//									printf("0x%02X, ",learn_succ[i+4]);
								}
//								send(clientSock,learn_succ ,sizeof(learn_succ),0);
								if(learn_succ[5] != 0x00)
									send(clientSock,learn_succ ,sizeof(learn_succ),0);
								else
									send(clientSock,learn_fail ,sizeof(learn_fail),0);
								memset(learndata,0,sizeof(learndata));
								memset(learn_succ,0,sizeof(learn_succ));
								learn_flag = 0;
							}
							else if(learn_flag == 0x00)
							{
//								printf("learn_flag=0 : %d \r\n",learn_flag);
							}
				break;
			case -1:
				break;
			default:
				if(FD_ISSET(clientSock,&rfd))
				{
					len=read(clientSock,buf,sizeof(buf));
					ESP_LOGI(TAG, "read");
					ESP_LOGI(TAG, "len %d",len);
					ESP_LOGI(TAG, "recv buf %s",buf);
					if(len <= 0)
						goto DISCONNECT;
//			receive from ETOR IR code ,接收来自中控的数据，判断数据头 sd%
					if(buf[0]==0xfd)
					{
						switch(buf[1])
						{
//				 SEND IR CODE
						case 0xbf:
							irtxlen = buf[2]*(0xff)+buf[3];
							int i=0;
							for(i=0;i<irtxlen;i++)
							{
								irda_data[i] = buf[i+4];
							}
							txirData_len = irtxlen;
							ESP_LOGI(TAG, "irda_data %s",irda_data);
							xTaskCreate(&vTaskIRDA1, "vTaskIRDA1", 2048 ,NULL , XTASK8, NULL);
							break;
//					learn IR code
						case 0xaf:
							if(Learn_Task_flag == 0)
								Learn_Task_flag = 1;
							ESP_LOGI(TAG, "xTaskCreate vTasklearnIRDA1 ");
								xTaskCreate(&vTasklearnIRDA1, "vTasklearnIRDA1", 2048 ,NULL , XTASK10, &TCP_LearnIR);
							break;
						case 0xae:
							ESP_LOGI(TAG, "vTaskDelete(TCP_LearnIR) ");
							Learn_Task_flag = 0;
							break;
						default:
							break;
						}
					}
				}//end if
				break;
			}//end switch
			//receive client buffer
		}//END WHILE
	DISCONNECT:
		ESP_LOGI(TAG, "close(clientSock)");
		close(clientSock);
	}
	ESP_LOGI(TAG, "close(sock)");
	close(sock);
	END:
	ESP_LOGI(TAG, "task tcp_conn vTaskDelete .");
	vTaskDelete(NULL);
}
