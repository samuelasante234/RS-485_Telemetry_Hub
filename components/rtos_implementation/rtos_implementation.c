#include "rtos_implementation.h"
#include "oled_config.h"
#include "crc_check.h"
#include "max_chip_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "lookup_table.h"
#include "freertos/semphr.h"

#define STATE_NEW_MESSAGE_NOT_CORRUPTED                                     (0b000)
#define STATE_PREVIOUS_MESSAGE_NOT_CORRUPTED                                (0b001)
#define STATE_NEW_MESSAGE_CORRUPTED_BY_RECEIVER_ONLY                        (0b010)
#define STATE_PREVIOUS_MESSAGE_CORRUPTED_BY_RECEIVER_ONLY                   (0b011)
#define STATE_NEW_MESSAGE_CORRUPTED_BY_SENDER_ONLY                          (0b100)
#define STATE_PREVIOUS_MESSAGE_CORRUPTED_BY_SENDER_ONLY                     (0b101)
#define STATE_NEW_MESSAGE_CORRUPTED_BY_SENDER_CORRUPTED_BY_RECEIVER         (0b110)
#define STATE_PREVIOUS_MESSAGE_CORRUPTED_BY_SENDER_CORRUPTED_BY_RECEIVER    (0b111)

static QueueHandle_t queue_for_actual_data =NULL;
static QueueHandle_t queue_for_display_data=NULL;
static RS485_Packet to_hold_recently_sent_packet={0};


void vTaskRS485(void *pvParameters);
void vTaskActuator(void *pvParameters);
void vTaskDisplay(void *pvParameters);

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
            data_packet_to_tx.check_info.check_info_full=0x00;
            data_packet_to_tx.data=0xFF;
            data_packet_to_tx.checksum=crc32(data_packet_to_tx.data<<8 | data_packet_to_tx.check_info.check_info_full);
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
            data_packet_to_tx.check_info.check_info_full=0x00;
            data_packet_to_tx.data=0xFF;
            data_packet_to_tx.checksum=crc32(data_packet_to_tx.data<<8 | data_packet_to_tx.check_info.check_info_full);
            to_hold_recently_sent_packet=data_packet_to_tx;
            push_to_uart_tx_fifo(&data_packet_to_tx);
            xSemaphoreGive(xMutex);
            continue;
        }
        read_from_uart_rx_fifo(&data_packet_from_rx); 
        ExtractedDataStruct extracted_struct_data ={
            .check_info.check_info_full=data_packet_from_rx.check_info.check_info_full,
            .data=data_packet_from_rx.data,
        };
        if (!is_crc_passed(data_packet_from_rx.data<<8 | data_packet_from_rx.check_info.check_info_full, data_packet_from_rx.checksum))
            extracted_struct_data.check_info.is_corrupted_by_sender=1;
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
        comb_of_lsb_3=(extracted_struct_data.check_info.is_message_previous) |((extracted_struct_data.check_info.is_corrupted_by_receiver)<<1)|((extracted_struct_data.check_info.is_corrupted_by_sender)<<2);
        switch (comb_of_lsb_3) {
            case STATE_NEW_MESSAGE_NOT_CORRUPTED:
                if (lookup_table[extracted_struct_data.data]==NULL) {
                    xSemaphoreTake(xMutex,portMAX_DELAY);
                    data_packet_to_tx.check_info.check_info_full=0b00001001;
                    data_packet_to_tx.data=0xFF;
                    data_packet_to_tx.checksum=crc32(data_packet_to_tx.data<<8 | data_packet_to_tx.check_info.check_info_full);
                    to_hold_recently_sent_packet=data_packet_to_tx;
                    push_to_uart_tx_fifo(&data_packet_to_tx);
                    xSemaphoreGive(xMutex);
                }
                else {
                    if (!queue_for_display_data) {
                        queue_for_display_data=xQueueCreate(0,sizeof(extracted_struct_data.data));
                        if (!queue_for_display_data) {
                            printf("Couldn't create queue for display data!");
                            fflush(stdout);
                            esp_restart();
                        }
                    }
                    portBASE_TYPE send_status;
                    if ((send_status = xQueueSend(queue_for_display_data,&extracted_struct_data.data,portMAX_DELAY)) !=pdPASS) {
                        printf("Could not send to display queue!");
                        fflush(stdout);
                        esp_restart();
                    }
                    StructForDisplay *temp = lookup_table[extracted_struct_data.data];
                    if (temp->is_long) {
                        xSemaphoreTake(xMutex,portMAX_DELAY);
                        data_packet_to_tx.check_info.check_info_full=0b00010001;
                        data_packet_to_tx.data=0xFF;
                        data_packet_to_tx.checksum=crc32(data_packet_to_tx.data<<8 | data_packet_to_tx.check_info.check_info_full);
                        to_hold_recently_sent_packet=data_packet_to_tx;
                        push_to_uart_tx_fifo(&data_packet_to_tx);
                        xSemaphoreGive(xMutex);
                    }
                    xSemaphoreTake(xBinarySemaphore,portMAX_DELAY);
                    vTaskDelayUntil(0,portMAX_DELAY);
                    xSemaphoreTake(xMutex,portMAX_DELAY);
                        data_packet_to_tx.check_info.check_info_full=0b00000001;
                        data_packet_to_tx.data=0xFF;
                        data_packet_to_tx.checksum=crc32(data_packet_to_tx.data<<8 | data_packet_to_tx.check_info.check_info_full);
                        to_hold_recently_sent_packet=data_packet_to_tx;
                        push_to_uart_tx_fifo(&data_packet_to_tx);
                    xSemaphoreGive(xMutex);
                }
                break;
            case STATE_PREVIOUS_MESSAGE_NOT_CORRUPTED:  //can do nothing: slave can do nothing
                break;
            case STATE_NEW_MESSAGE_CORRUPTED_BY_RECEIVER_ONLY:
                break;
            case STATE_PREVIOUS_MESSAGE_CORRUPTED_BY_RECEIVER_ONLY:
                xSemaphoreTake(xMutex,portMAX_DELAY);
                push_to_uart_tx_fifo(&to_hold_recently_sent_packet);
                xSemaphoreGive(xMutex);
                break;
            case STATE_NEW_MESSAGE_CORRUPTED_BY_SENDER_ONLY:
                xSemaphoreTake(xMutex,portMAX_DELAY);
                data_packet_to_tx.check_info.check_info_full=0b00000011;
                data_packet_to_tx.data=0xFF;
                data_packet_to_tx.checksum=crc32(data_packet_to_tx.data<<8 | data_packet_to_tx.check_info.check_info_full);
                push_to_uart_tx_fifo(&data_packet_to_tx);
                xSemaphoreGive(xMutex);
                break;
            case STATE_PREVIOUS_MESSAGE_CORRUPTED_BY_SENDER_ONLY:
                xSemaphoreTake(xMutex,portMAX_DELAY);
                data_packet_to_tx.check_info.check_info_full=0b00000011;
                data_packet_to_tx.data=0xFF;
                data_packet_to_tx.checksum=crc32(data_packet_to_tx.data<<8 | data_packet_to_tx.check_info.check_info_full);
                push_to_uart_tx_fifo(&data_packet_to_tx);
                xSemaphoreGive(xMutex);
                break;
            case STATE_NEW_MESSAGE_CORRUPTED_BY_SENDER_CORRUPTED_BY_RECEIVER:
                xSemaphoreTake(xMutex,portMAX_DELAY);
                data_packet_to_tx.check_info.check_info_full=0b00000011;
                data_packet_to_tx.data=0xFF;
                data_packet_to_tx.checksum=crc32(data_packet_to_tx.data<<8 | data_packet_to_tx.check_info.check_info_full);
                push_to_uart_tx_fifo(&data_packet_to_tx);
                xSemaphoreGive(xMutex);
                break;
            case STATE_PREVIOUS_MESSAGE_CORRUPTED_BY_SENDER_CORRUPTED_BY_RECEIVER:
                xSemaphoreTake(xMutex,portMAX_DELAY);
                data_packet_to_tx.check_info.check_info_full=0b00000011;
                data_packet_to_tx.data=0xFF;
                data_packet_to_tx.checksum=crc32(data_packet_to_tx.data<<8 | data_packet_to_tx.check_info.check_info_full);
                push_to_uart_tx_fifo(&data_packet_to_tx);
                xSemaphoreGive(xMutex);
                break;
            default:
                break;
        }
    }
}
void vTaskDisplay(void *pvParameters) {
    uint8_t struct_key;
    for (;;) {
        xQueueReceive(queue_for_display_data,&struct_key,portMAX_DELAY);
        to_hand_control_to_oled(lookup_table[struct_key]);
        xSemaphoreGive(xBinarySemaphore);
    }
}