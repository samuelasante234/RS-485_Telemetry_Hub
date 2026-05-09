#include "max_chip_config.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define BAUD_RATE 0
#define TRANSACTION_BITS_NO 0
#define RX_TIMEOUT 0
#define BYTES_FOR_RXFIFO 6
#define BYTES_FOR_TXFIFO 0
#define MAX_NO_OF_ERROR_EVENTS 0
#define UART_CHANNEL UART_NUM_1
#define UART_TX_PIN 17
#define UART_RX_PIN 18
/*UART INT_ENA REGISTER BITFIELD*/
#define UART_RXFIFO_FULL_INT_ENA 1<<0
#define UART_TXFIFO_EMPTY_INT_ENA 1<<1
#define UART_PARITY_ERR_INT_ENA 1<<2
#define UART_FRM_ERR_INT_ENA 1<<3
#define UART_RXFIFO_OVF_INT_ENA 1<<4
#define UART_DSR_CHG_INT_ENA 1<<5
#define UART_CTS_CHG_INT_ENA 1<<6
#define UART_BRK_DET_INT_ENA 1<<7
#define UART_RXFIFO_TOUT_INT_ENA 1<<8
#define UART_SW_XON_INT_ENA 1<<9
#define UART_SW_XOFF_INT_ENA 1<<10
#define UART_GLITCH_DET_INT_ENA 1<<11
#define UART_TX_BRK_DONE_INT_ENA 1<<12
#define UART_TX_BRK_IDLE_DONE_INT_ENA 1<<13
#define UART_TX_DONE_INT_ENA 1<<14
#define UART_RS485_PARITY_ERR_INT_ENA 1<<15
#define UART_RS485_FRM_ERR_INT_ENA 1<<16
#define UART_RS485_CLASH_INT_ENA 1<<17
#define UART_AT_CMD_CHAR_DET_INT_ENA 1<<18
#define UART_WAKEUP_INT_ENA 1<<19


void max3485_init();
void read_from_uart_rx_fifo(RS485_Packet* packet);
void push_to_uart_tx_fifo(RS485_Packet* packet);
void flush_uart_rx_fifo();
RS485_Packet data_packet_from_rx = {0};
RS485_Packet data_packet_to_tx = {0};
QueueHandle_t uart_event_queue = NULL;

void flush_uart_rx_fifo() {
    esp_err_t result;
    result = uart_flush_input(UART_CHANNEL);
    if (result != ESP_OK) {
        printf("Couldn't clear RX FIFO! Error: %s",esp_err_to_name(result));
        fflush(stdout);
        return;
    }
}
void read_from_uart_rx_fifo(RS485_Packet* packet) {
    uart_read_bytes(UART_CHANNEL,(void *) packet, BYTES_FOR_RXFIFO,portMAX_DELAY);
}
void push_to_uart_tx_fifo(RS485_Packet* packet) {
    uart_write_bytes(UART_CHANNEL,(void *)packet,sizeof(RS485_Packet));
    esp_err_t result;
    result = uart_wait_tx_done(UART_CHANNEL,portMAX_DELAY);
    if (result !=ESP_OK) {
        printf("Couldn't transmit bytes! Error: %s",esp_err_to_name(result));
        fflush(stdout);
        return;
    }
}
void max3485_init() {
    uart_config_t max3485_config = {
        .baud_rate=BAUD_RATE,
        .data_bits=TRANSACTION_BITS_NO,
        .flow_ctrl=UART_HW_FLOWCTRL_DISABLE,
        .parity=0,
        .rx_flow_ctrl_thresh=0,
        .source_clk=UART_SCLK_APB,
        .stop_bits=UART_STOP_BITS_1,
    };
    uart_intr_config_t max3485_config_intr = {
        .intr_enable_mask= UART_RXFIFO_FULL_INT_ENA|UART_TXFIFO_EMPTY_INT_ENA|UART_RXFIFO_TOUT_INT_ENA,
        .rx_timeout_thresh=RX_TIMEOUT,
        .rxfifo_full_thresh=BYTES_FOR_RXFIFO,
        .txfifo_empty_intr_thresh=BYTES_FOR_TXFIFO,
    };
    esp_err_t result;
    result = uart_param_config(UART_CHANNEL,&max3485_config);
    if (result != ESP_OK) {
        printf("Could not configure UART! Error: %s\n",esp_err_to_name(result));
        fflush(stdout);
        return;
    }
    result = uart_intr_config(UART_CHANNEL,&max3485_config_intr);
    if (result != ESP_OK) {
        printf("Could not configure UART interrupts! Error: %s\n",esp_err_to_name(result));
        fflush(stdout);
        return;
    }
    result = uart_driver_install(UART_CHANNEL,0,0,MAX_NO_OF_ERROR_EVENTS,&uart_event_queue,ESP_INTR_FLAG_LOWMED);
    if (result != ESP_OK) {
        printf("Could not install UART driver! Error: %s\n",esp_err_to_name(result));
        fflush(stdout);
        return;
    }
    result = uart_set_pin(UART_CHANNEL,UART_TX_PIN,UART_RX_PIN,-1,-1);
    if (result != ESP_OK) {
        printf("Could not route UART pins! Error: %s\n",esp_err_to_name(result));
        fflush(stdout);
        return;
    }
}