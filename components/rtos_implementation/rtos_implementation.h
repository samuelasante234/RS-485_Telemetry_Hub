#include "freertos/FreeRTOS.h"

typedef struct {
    uint8_t byte_0;
    uint8_t byte1;
} ExtractedDataStruct;

void vTaskRS485(void *pvParameters);