#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

typedef struct {
    union {
        struct __attribute__((packed)){
            uint8_t is_message_previous:1;
            uint8_t is_corrupted_by_receiver:1;
            uint8_t is_corrupted_by_sender:1;
            uint8_t is_data_valid:1;
            uint8_t is_done_processing:1;
            uint8_t node_number:3;
        };
        uint8_t full_bits;
    } byte_0_extracted;
    uint8_t byte1;
}__attribute__((packed)) ExtractedDataStruct;
extern SemaphoreHandle_t xMutex;
extern SemaphoreHandle_t xBinarySemaphore;


void vTaskRS485(void *pvParameters);
void vTaskActuator(void *pvParameters);
void vTaskDisplay(void *pvParameters);