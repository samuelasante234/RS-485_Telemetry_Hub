#include "driver/uart.h"

extern QueueHandle_t uart_event_queue;
typedef struct {
    uart_event_type_t type;
    size_t size;
    bool timeout_flag;
}Queue_Struct_UART;
typedef struct {
    union {
        uint8_t full_8_bits;
        struct __attribute__((packed)) {
            uint8_t bit0: 1;
            uint8_t bit1: 1;
            uint8_t bit2: 1;
            uint8_t bit3: 1;
            uint8_t msbs: 4;
        };
    }byte_0;
    uint8_t byte_1;
    uint32_t byte_2_5;
} __attribute__((packed)) RS485_Packet;
extern RS485_Packet data_packet_from_rx;
extern RS485_Packet data_packet_to_tx;

void max3485_init();
void push_to_uart_tx_fifo(RS485_Packet* packet);
void read_from_uart_rx_fifo(RS485_Packet* packet);
void flush_uart_rx_fifo();