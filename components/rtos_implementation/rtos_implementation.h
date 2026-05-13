#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

typedef struct {
    union {
        struct __attribute__((packed)){
            uint8_t bit0:1;
            uint8_t bit1:1;
            uint8_t bit2:1;
            uint8_t bit3:1;
            uint8_t bit4:1;
            uint8_t msbs:3;
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