/*
 * tcp.h
 *
 *  Created on: Dec 19, 2017
 *      Author: markin
 */

#ifndef MAIN_TCP_H_
#define MAIN_TCP_H_
/* tcp_perf Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/




#ifdef __cplusplus
extern "C" {
#endif


/*test options*/
#define EXAMPLE_ESP_WIFI_MODE_AP false //TRUE:AP FALSE:STA
#define EXAMPLE_ESP_TCP_MODE_SERVER true //TRUE:server FALSE:client
#define EXAMPLE_ESP_TCP_PERF_TX false //TRUE:send FALSE:receive
#define EXAMPLE_ESP_TCP_DELAY_INFO true //TRUE:show delay time info

/*AP info and tcp_server info*/
#define EXAMPLE_DEFAULT_SSID CONFIG_TCP_PERF_WIFI_SSID
#define EXAMPLE_DEFAULT_PWD CONFIG_TCP_PERF_WIFI_PASSWORD
#define EXAMPLE_DEFAULT_PORT (10001)
#define EXAMPLE_DEFAULT_PKTSIZE (1460)
#define EXAMPLE_MAX_STA_CONN 1 //how many sta can be connected(AP mode)

#define EXAMPLE_DEFAULT_SERVER_IP "192.168.4.1"


#define SERVER_PORT (10101)
#define EXAMPLE_PACK_BYTE_IS 97 //'a'
//#define TAG "tcp_perf:"

/* FreeRTOS event group to signal when we are connected to wifi*/
/*
extern EventGroupHandle_t tcp_event_group;
#define WIFI_CONNECTED_BIT BIT0
*/

typedef enum {
	XTASK1 = 6,
	XTASK2,
	XTASK3,
	XTASK4,
	XTASK5,
	XTASK6,
	XTASK7,
	XTASK8,
	XTASK9,
	XTASK10,

}TASK_YX;

extern int  g_total_data;
extern bool g_rxtx_need_restart;



extern void vTasktxIRDA(void * pvParameters);
extern void vTaskIRDA(void * pvParameters);
extern void vTaskIRDA1(void * pvParameters);
extern void vTaskIRDA2(void * pvParameters);
extern void vTaskIRDA_XF(void * pvParameters);
extern void vTaskIRDA_XM(void * pvParameters);
extern void vTaskIRDA_LS(void * pvParameters);
extern void vTaskIRDA_Learn(void * pvParameters);
extern void vTasklearnIRDA1(void * pvParameters);
extern uint8_t *irda_data_tcp;
extern uint8_t learn_flag;
extern uint8_t irda_data[2048];
extern uint8_t learndata[232];
extern int irtxlen;
extern void *tcp_recv;
extern xQueueHandle TcpQueue;
extern xQueueHandle IrQueue;
extern TaskHandle_t TCP_LearnIR;
extern int txirData_len;
extern int Learn_Task_flag ;
struct IrLearnMessage
{
	uint8_t LearnData[232];
	uint8_t len;
}IrMessage;
//create a tcp server socket. return ESP_OK:success ESP_FAIL:error
esp_err_t create_tcp_server(int port);
//create a tcp client socket. return ESP_OK:success ESP_FAIL:error
esp_err_t create_tcp_client();

//send data task
void send_data(void *pvParameters);
//receive data task
void recv_data(void *pvParameters);

//close all socket
void close_socket();

//get socket error code. return: error code
int get_tcp_socket_error_code(int socket);

//show socket error code. return: error code
int show_tcp_socket_error_reason(const char* str, int socket);

//check working socket
int check_tcp_working_socket();


void socket_server_tcp(void *pvParameters);
void socket_client_tcp(void *pvParameters);

void socket_server_learn(void *pvParameters);
#ifdef __cplusplus
}
#endif




#endif /* MAIN_TCP_H_ */
