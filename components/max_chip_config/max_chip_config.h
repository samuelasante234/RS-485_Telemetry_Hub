#include "driver/uart.h"

extern QueueHandle_t uart_event_queue;
typedef struct {
    uart_event_type_t type;
    size_t size;
    bool timeout_flag;
}Queue_Struct_UART;
typedef struct {
    union {
        uint8_t check_info_full;
        struct __attribute__((packed)) {
            uint8_t is_message_previous:1;
            uint8_t is_corrupted_by_receiver:1;
            uint8_t is_corrupted_by_sender:1;
            uint8_t is_data_valid:1;
            uint8_t is_done_processing:1;
            uint8_t node_number:3;
        };
    }check_info;
    uint8_t data;
    uint32_t checksum;
} __attribute__((packed)) RS485_Packet;
extern RS485_Packet data_packet_from_rx;
extern RS485_Packet data_packet_to_tx;

void max3485_init();
void push_to_uart_tx_fifo(RS485_Packet* packet);
void read_from_uart_rx_fifo(RS485_Packet* packet);
void flush_uart_rx_fifo();