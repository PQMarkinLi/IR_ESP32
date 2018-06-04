
#include "freertos/FreeRTOS.h"

#include "freertos/task.h"

#include "esp_system.h"

#include "esp_log.h"

#include "driver/uart.h"

#include "soc/uart_struct.h"

#include "string.h"
#include "testUART1.h"
#ifndef size_t
#define size_t unsigned int
#endif
#define BUF_SIZE (1024)
#define RX_BUF_SIZE (1024)
//uart1
#define UART1_TXD  (GPIO_NUM_22)
#define UART1_RXD  (GPIO_NUM_23)
#define UART1_RTS  (UART_PIN_NO_CHANGE)
#define UART1_CTS  (UART_PIN_NO_CHANGE)
//uart2
#define UART2_TXD  (GPIO_NUM_16)
#define UART2_RXD  (GPIO_NUM_17)

#define UART2_RTS  (UART_PIN_NO_CHANGE)
#define UART2_CTS  (UART_PIN_NO_CHANGE)



void initialise_uart1()
{
    int uart_num = UART_NUM_1;
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(uart_num,&uart_config);
    uart_set_pin(uart_num, UART1_TXD, UART1_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(uart_num,RX_BUF_SIZE * 2,0,0,NULL,0);
}
void initialise_uart2()
{
    int uart_num = UART_NUM_2;
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(uart_num,&uart_config);
    uart_set_pin(uart_num, UART2_TXD, UART2_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(uart_num,RX_BUF_SIZE * 2,0,0,NULL,0);
}
