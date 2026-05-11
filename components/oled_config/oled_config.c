#include "oled_config.h"
#include "driver/i2c.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define I2C_MODE I2C_MODE_MASTER
#define SDA_PIN 4
#define SCL_PIN 5
#define CLOCK_SPEED 1000000
#define I2C_CHANNEL I2C_NUM_0
#define SIZE_OF_COMMAND_LIST_IN_BYTES 20
#define NUMBER_OF_OPERATIONS 28
#define OLED_SLAVE_ADDRESS 0x7C
#define CONTROL_BYTE_FOR_COMMANDS 0x00
#define CONTROL_BYTE_FOR_PIXELS 0x40

void oled_init();
void oled_screen_init();
static uint8_t transmit_buffer_for_init[SIZE_OF_COMMAND_LIST_IN_BYTES*NUMBER_OF_OPERATIONS];
static uint8_t init_buffer[23]={
    0xAE,0xD5,0x80,0xA8,0xFF,0xD3,0x00,0x40,0x8D,0x14,0x20,
    0xFC,0xA0,0xC0,0xDA,0x12,0x81,0x7F,0xD9,0x22,0xA5,0xA6,0xAF
};

void oled_init() {
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
        return;
    }
    result=i2c_driver_install(I2C_CHANNEL,I2C_MODE,0,0,ESP_INTR_FLAG_LOWMED);
    if (result != ESP_OK) {
        printf("Cannot install driver! Error: %s\n",esp_err_to_name(result));
        fflush(stdout);
        return;
    }
}
void oled_screen_init() {
    i2c_cmd_handle_t oled_handle;
    oled_handle = i2c_cmd_link_create_static(transmit_buffer_for_init,SIZE_OF_COMMAND_LIST_IN_BYTES*NUMBER_OF_OPERATIONS);
    if (oled_handle == NULL) {
        printf("Size of transmit buffer not enough!\n");
        esp_restart();
    }
    esp_err_t result;
    result = i2c_master_start(oled_handle);
    if (result != ESP_OK) {
        printf("Couldn't queue start bit! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return;
    }
    result = i2c_master_write_byte(oled_handle,OLED_SLAVE_ADDRESS,true);
    if (result != ESP_OK) {
        printf("Couldn't queue slave address! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return;
    }
    result = i2c_master_write_byte(oled_handle,CONTROL_BYTE_FOR_COMMANDS,true);
    if (result != ESP_OK) {
        printf("Couldn't queue control byte for commands! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return;
    }
    result=i2c_master_write(oled_handle,init_buffer,sizeof(init_buffer),true);
    if (result != ESP_OK) {
        printf("Couldn't queue initialization commands! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return;
    }
    result = i2c_master_stop(oled_handle);
    if (result != ESP_OK) {
        printf("Couldn't queue stop bit! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return;
    }
    result=i2c_master_cmd_begin(I2C_CHANNEL,oled_handle,portMAX_DELAY);
    if (result != ESP_OK) {
        printf("Couldn't send queued commands! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return;
    }
    i2c_cmd_link_delete_static(oled_handle);
}