/*
 * udp.h
 *
 *  Created on: Mar 14, 2018
 *      Author: markin
 */

#ifndef MAIN_UDP_H_
#define MAIN_UDP_H_

#ifdef __cplusplus
extern "C"{
#endif






//#define EXAMPLE_DEFAULT_SERVER_IP (192.168.1.1)
//#define EXAMPLE_DEFAULT_PORT (8081)
#define UDP_DEFAULT_PKTSIZE (1024)

#define UDP_BROADCAST_PORT (40400)
extern int total_data;
extern int success_pack;
extern char *esp32_id;
//create a udp client socket. return ESP_OK:success ESP_FAIL:error
esp_err_t create_udp_server();
//create a udp client socket. return ESP_OK:success ESP_FAIL:error
esp_err_t create_udp_client(char* server_ip,int server_port);
//get socket error code. return: error code
int get_udp_socket_error_code(int socket);

//show socket error code. return: error code
int show_udp_socket_error_reason( int socket);

//check connected socket. return: error code
int check_udp_connected_socket();

//close all socket
void close_udp_socket();

void udp_device_discovery(void *pvParameters);
void send_recv_data(void *pvParameters);
void udp_send_ip(void *pvParameters);
void udp_Device_Management(void *pvParameters);
void udp_Send_Learn(void *pvParameters);
#ifdef __cplusplus
}
#endif
#endif /* MAIN_UDP_H_ */
