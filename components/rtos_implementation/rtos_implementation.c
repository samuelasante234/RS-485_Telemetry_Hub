#include "rtos_implementation.h"
#include "oled_config.h"
#include "crc_check.h"
#include "max_chip_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_system.h"

static QueueHandle_t queue_for_actual_data =NULL;

void vTaskRS485(void *pvParameters);

void vTaskRS485(void *pvParameters) {
    if (!queue_for_actual_data) {
        queue_for_actual_data=xQueueCreate(0,sizeof(ExtractedDataStruct));
        if (!queue_for_actual_data) {
            printf("Queue for actual data couldn't be created!");
            fflush(stdout);
            esp_restart();
        }
    }
    Queue_Struct_UART struct_receive = {0};
    for (;;) {
        xQueueReceive(uart_event_queue,&struct_receive,portMAX_DELAY);
        if ((struct_receive.type == UART_DATA) && struct_receive.size != 6) {
            flush_uart_rx_fifo();
            data_packet_to_tx.byte_0.full_8_bits=0x00;
            data_packet_to_tx.byte_1=0xFF;
            data_packet_to_tx.byte_2_5=crc32(data_packet_to_tx.byte_1);
            push_to_uart_tx_fifo(&data_packet_to_tx);
            continue;
        }
        if (struct_receive.type == UART_BUFFER_FULL || struct_receive.type==UART_BREAK || struct_receive.type==UART_PARITY_ERR || struct_receive.type==UART_DATA_BREAK || struct_receive.type == UART_EVENT_MAX) {
            flush_uart_rx_fifo();
            continue;
        }
        if (struct_receive.type == UART_FIFO_OVF || struct_receive.type == UART_FRAME_ERR) {
            flush_uart_rx_fifo();
            data_packet_to_tx.byte_0.full_8_bits=0x00;
            data_packet_to_tx.byte_1=0xFF;
            data_packet_to_tx.byte_2_5=crc32(data_packet_to_tx.byte_1);
            push_to_uart_tx_fifo(&data_packet_to_tx);
            continue;
        }
        read_from_uart_rx_fifo(&data_packet_from_rx);
        if (!is_crc_passed(data_packet_from_rx.byte_1,data_packet_from_rx.byte_2_5))
            data_packet_from_rx.byte_0.bit1=1;
        ExtractedDataStruct extracted_struct_data ={
            .byte_0=data_packet_from_rx.byte_0.full_8_bits,
            .byte1=data_packet_from_rx.byte_1,
        };
        portBASE_TYPE send_status;
        if ((send_status = xQueueSend(queue_for_actual_data,&extracted_struct_data,portMAX_DELAY)) !=pdPASS) {
            printf("Could not send to extracted queue!");
            fflush(stdout);
            esp_restart();
        }
    }
}