#include "oled_config.h"
#include "driver/i2c.h"
#include "stdbool.h"
#include "string.h"
#include "stdlib.h"

#define I2C_MODE I2C_MODE_MASTER
#define SDA_PIN 4
#define SCL_PIN 5
#define CLOCK_SPEED 1000000
#define I2C_CHANNEL I2C_NUM_0
#define SIZE_OF_COMMAND_LIST_IN_BYTES 0

i2c_cmd_handle_t oled_init();
static uint8_t transmit_buffer[SIZE_OF_COMMAND_LIST_IN_BYTES];

i2c_cmd_handle_t oled_init() {
    i2c_config_t oled_config={
        .mode= I2C_MODE,
        .sda_io_num=SDA_PIN,
        .scl_io_num=SCL_PIN,
        .sda_pullup_en=false,
        .scl_pullup_en=false,
        .master.clk_speed=CLOCK_SPEED,
        .clk_flags=I2C_SCLK_SRC_FLAG_FOR_NOMAL,
    };
    esp_err_t result;
    result=i2c_param_config(I2C_CHANNEL,&oled_config);
    if (result != ESP_OK) {
        printf("Cannot configure I2C channel! Error: %s\n",esp_err_to_name(result));
        fflush(stdout);
        return NULL;
    }
    result=i2c_driver_install(I2C_CHANNEL,I2C_MODE,0,0,ESP_INTR_FLAG_LOWMED);
    if (result != ESP_OK) {
        printf("Cannot install driver! Error: %s\n",esp_err_to_name(result));
        fflush(stdout);
        return NULL;
    }
    i2c_cmd_handle_t oled_handle;
    oled_handle = i2c_cmd_link_create_static(transmit_buffer,SIZE_OF_COMMAND_LIST_IN_BYTES);
    if (oled_handle == NULL) {
        printf("Size of transmit buffer not enough!\n");
    }
    return oled_handle;
}