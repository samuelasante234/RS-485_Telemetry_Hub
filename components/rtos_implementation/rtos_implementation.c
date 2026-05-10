#include "rtos_implementation.h"
#include "oled_config.h"
#include "crc_check.h"
#include "max_chip_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "lookup_table.h"
#include "freertos/semphr.h"

static QueueHandle_t queue_for_actual_data =NULL;
static QueueHandle_t queue_for_display_data=NULL;
static RS485_Packet to_hold_recently_sent_packet={0};

void vTaskRS485(void *pvParameters);
void vTaskActuator(void *pvParameters);

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
            xSemaphoreTake(xMutex,portMAX_DELAY);
            data_packet_to_tx.byte_0.full_8_bits=0x00;
            data_packet_to_tx.byte_1=0xFF;
            data_packet_to_tx.byte_2_5=crc32(data_packet_to_tx.byte_1);
            to_hold_recently_sent_packet=data_packet_to_tx;
            push_to_uart_tx_fifo(&data_packet_to_tx);
            xSemaphoreGive(xMutex);
            continue;
        }
        if (struct_receive.type == UART_BUFFER_FULL || struct_receive.type==UART_BREAK || struct_receive.type==UART_PARITY_ERR || struct_receive.type==UART_DATA_BREAK || struct_receive.type == UART_EVENT_MAX) {
            flush_uart_rx_fifo();
            continue;
        }
        if (struct_receive.type == UART_FIFO_OVF || struct_receive.type == UART_FRAME_ERR) {
            flush_uart_rx_fifo();
            xSemaphoreTake(xMutex,portMAX_DELAY);
            data_packet_to_tx.byte_0.full_8_bits=0x00;
            data_packet_to_tx.byte_1=0xFF;
            data_packet_to_tx.byte_2_5=crc32(data_packet_to_tx.byte_1);
            to_hold_recently_sent_packet=data_packet_to_tx;
            push_to_uart_tx_fifo(&data_packet_to_tx);
            xSemaphoreGive(xMutex);
            continue;
        }
        read_from_uart_rx_fifo(&data_packet_from_rx);
        if (!is_crc_passed(data_packet_from_rx.byte_1,data_packet_from_rx.byte_2_5))
            data_packet_from_rx.byte_0.bit1=1;
        ExtractedDataStruct extracted_struct_data ={
            .byte_0_extracted.full_bits=data_packet_from_rx.byte_0.full_8_bits,
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
void vTaskActuator(void *pvParameters) {
    ExtractedDataStruct extracted_struct_data={0};
    for (;;) {
        xQueueReceive(queue_for_actual_data,&extracted_struct_data,portMAX_DELAY);
        uint8_t comb_of_lsb_3;
        comb_of_lsb_3=(extracted_struct_data.byte_0_extracted.bit0) |((extracted_struct_data.byte_0_extracted.bit1)<<1)|((extracted_struct_data.byte_0_extracted.bit2)<<2);
        switch (comb_of_lsb_3) {
            case 0b00000000:
                if (lookup_table[extracted_struct_data.byte1]==NULL) {
                    xSemaphoreTake(xMutex,portMAX_DELAY);
                    data_packet_to_tx.byte_0.full_8_bits=0b0001001;
                    data_packet_to_tx.byte_1=0xFF;
                    data_packet_to_tx.byte_2_5=crc32(data_packet_to_tx.byte_1);
                    to_hold_recently_sent_packet=data_packet_to_tx;
                    push_to_uart_tx_fifo(&data_packet_to_tx);
                    xSemaphoreGive(xMutex);
                }
                else {
                    if (!queue_for_display_data) {
                        queue_for_display_data=xQueueCreate(0,sizeof(extracted_struct_data.byte1));
                        if (!queue_for_display_data) {
                            printf("Couldn't create queue for display data!");
                            fflush(stdout);
                            esp_restart();
                        }
                    }
                    portBASE_TYPE send_status;
                    if ((send_status = xQueueSend(queue_for_display_data,&extracted_struct_data.byte1,portMAX_DELAY)) !=pdPASS) {
                        printf("Could not send to display queue!");
                        fflush(stdout);
                        esp_restart();
                    }
                    StructForDisplay *temp = lookup_table[extracted_struct_data.byte1];
                    if (temp->is_long) {
                        xSemaphoreTake(xMutex,portMAX_DELAY);
                        data_packet_to_tx.byte_0.full_8_bits=0b0010001;
                        data_packet_to_tx.byte_1=0xFF;
                        data_packet_to_tx.byte_2_5=crc32(data_packet_to_tx.byte_1);
                        to_hold_recently_sent_packet=data_packet_to_tx;
                        push_to_uart_tx_fifo(&data_packet_to_tx);
                        xSemaphoreGive(xMutex);
                    }
                    xSemaphoreTake(xBinarySemaphore,portMAX_DELAY);
                    vTaskDelayUntil(portMAX_DELAY,portMAX_DELAY);
                    xSemaphoreTake(xMutex,portMAX_DELAY);
                        data_packet_to_tx.byte_0.full_8_bits=0b00000001;
                        data_packet_to_tx.byte_1=0xFF;
                        data_packet_to_tx.byte_2_5=crc32(data_packet_to_tx.byte_1);
                        to_hold_recently_sent_packet=data_packet_to_tx;
                        push_to_uart_tx_fifo(&data_packet_to_tx);
                        xSemaphoreGive(xMutex);
                }
                break;
            case 0b00000001:
                break;
            case 0b00000010:
                break;
            case 0b00000011:
                xSemaphoreTake(xMutex,portMAX_DELAY);
                push_to_uart_tx_fifo(&to_hold_recently_sent_packet);
                xSemaphoreGive(xMutex);
                break;
            case 0b00000100:
                xSemaphoreTake(xMutex,portMAX_DELAY);
                data_packet_to_tx.byte_0.full_8_bits=0x00000011;
                data_packet_to_tx.byte_1=0xFF;
                data_packet_to_tx.byte_2_5=crc32(data_packet_to_tx.byte_1);
                push_to_uart_tx_fifo(&data_packet_to_tx);
                xSemaphoreGive(xMutex);
                break;
            case 0b00000101:
                xSemaphoreTake(xMutex,portMAX_DELAY);
                data_packet_to_tx.byte_0.full_8_bits=0x00000011;
                data_packet_to_tx.byte_1=0xFF;
                data_packet_to_tx.byte_2_5=crc32(data_packet_to_tx.byte_1);
                push_to_uart_tx_fifo(&data_packet_to_tx);
                xSemaphoreGive(xMutex);
                break;
            case 0b00000110:
                xSemaphoreTake(xMutex,portMAX_DELAY);
                data_packet_to_tx.byte_0.full_8_bits=0x00000011;
                data_packet_to_tx.byte_1=0xFF;
                data_packet_to_tx.byte_2_5=crc32(data_packet_to_tx.byte_1);
                push_to_uart_tx_fifo(&data_packet_to_tx);
                xSemaphoreGive(xMutex);
                break;
            case 0b00000111:
                xSemaphoreTake(xMutex,portMAX_DELAY);
                data_packet_to_tx.byte_0.full_8_bits=0x00000011;
                data_packet_to_tx.byte_1=0xFF;
                data_packet_to_tx.byte_2_5=crc32(data_packet_to_tx.byte_1);
                push_to_uart_tx_fifo(&data_packet_to_tx);
                xSemaphoreGive(xMutex);
                break;
            default:
                break;
        }
    }
}