#include "oled_config.h"
#include "driver/i2c.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "oled_font.h"

#define I2C_MODE                                    I2C_MODE_MASTER
#define SDA_PIN                                     4
#define SCL_PIN                                     5
#define CLOCK_SPEED                                 1000000
#define I2C_CHANNEL                                 I2C_NUM_0
#define SIZE_OF_COMMAND_LIST_IN_BYTES               20
#define NUMBER_OF_OPERATIONS_FOR_INIT                6
#define OLED_SLAVE_ADDRESS                          0x7C
#define CONTROL_BYTE_FOR_COMMANDS                   0x00
#define CONTROL_BYTE_FOR_PIXELS                     0x40
#define NUMBER_OF_OPERATIONS_FOR_PIXELS_FONT        5
#define NUMBER_OF_OPERATIONS_FOR_PIXELS_COMMANDS    5

#define ESP_RETURN_IF_FAILED(error_code,string_to_print) do { if((error_code)!= ESP_OK) {printf(string_to_print "%s\n",esp_err_to_name(error_code));fflush(stdout);return;}} while(0)

void oled_init();
void oled_screen_init();
void oled_screen_print(char* message);
static bool is_next_stream_a_word(char* message, int current_index, int column_checker);
static void copy_characters(char* original_message, char* new_message);
static uint8_t transmit_buffer_for_init[SIZE_OF_COMMAND_LIST_IN_BYTES*NUMBER_OF_OPERATIONS_FOR_INIT];
const static uint8_t init_buffer[23]={
    0xAE,0xD5,0x80,0xA8,0xFF,0xD3,0x00,0x40,0x8D,0x14,0x20,
    0xFC,0xA0,0xC0,0xDA,0x12,0x81,0x7F,0xD9,0x22,0xA4,0xA6,0xAF
};
static char pixels_buffer[1025]={0};
static uint8_t pixels_font[1024*8]={0};
static uint8_t transmit_buffer_for_pixels_font[SIZE_OF_COMMAND_LIST_IN_BYTES*NUMBER_OF_OPERATIONS_FOR_PIXELS_FONT];
static uint8_t transmit_buffer_for_pixels_for_commands[SIZE_OF_COMMAND_LIST_IN_BYTES*NUMBER_OF_OPERATIONS_FOR_PIXELS_COMMANDS];
static uint8_t commands_before_data[6]={
    0x21,0x00,0x7F,0x22,0x00,0x00
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
    ESP_RETURN_IF_FAILED(result,"Couldn't configure I2C channel! Error: ");
    result=i2c_driver_install(I2C_CHANNEL,I2C_MODE,0,0,ESP_INTR_FLAG_LOWMED);
    ESP_RETURN_IF_FAILED(result,"Cannot install driver! Error: ");
    
}
void oled_screen_init() {
    i2c_cmd_handle_t oled_handle;
    oled_handle = i2c_cmd_link_create_static(transmit_buffer_for_init,SIZE_OF_COMMAND_LIST_IN_BYTES*NUMBER_OF_OPERATIONS_FOR_INIT);
    if (oled_handle == NULL) {
        printf("Size of transmit buffer not enough!\n");
        esp_restart();
    }
    esp_err_t result;
    result = i2c_master_start(oled_handle);
    ESP_RETURN_IF_FAILED(result,"Couldn't queue start bit! Error: ");
    result = i2c_master_write_byte(oled_handle,OLED_SLAVE_ADDRESS,true);
    ESP_RETURN_IF_FAILED(result,"Couldn't queue slave address! Error: ");
    result = i2c_master_write_byte(oled_handle,CONTROL_BYTE_FOR_COMMANDS,true);
    ESP_RETURN_IF_FAILED(result,"Couldn't queue control byte for commands! Error: ");
    result=i2c_master_write(oled_handle,init_buffer,sizeof(init_buffer),true);
    ESP_RETURN_IF_FAILED(result,"Couldn't queue initialization commands! Error: ");
    result = i2c_master_stop(oled_handle);
    ESP_RETURN_IF_FAILED(result,"Couldn't queue stop bit! Error: ");
    result=i2c_master_cmd_begin(I2C_CHANNEL,oled_handle,portMAX_DELAY);
    ESP_RETURN_IF_FAILED(result,"Couldn't send queued commands! Error: ");
    i2c_cmd_link_delete_static(oled_handle);
}
void oled_screen_print(char* message) {
    copy_characters(message,pixels_buffer);
    uint8_t page_count=0, length_of_formatted=strlen(pixels_buffer),dummy=length_of_formatted;
    while ((dummy-16)>=0) {
        page_count+=1,dummy-=16;
    }
    page_count = dummy > 0? page_count+1 : page_count;
    commands_before_data[5]=page_count>0? page_count-1:page_count;
    for (uint16_t i=0;i<length_of_formatted;i++) {
        for (uint16_t j=0;j<8;j++) {
            pixels_font[(8*i)+j]=font8x8[pixels_buffer[i]-32][j];
        }
    }
    i2c_cmd_handle_t oled_handle;
    oled_handle=i2c_cmd_link_create_static(transmit_buffer_for_pixels_for_commands,sizeof(transmit_buffer_for_pixels_for_commands));
    if (oled_handle == NULL) {
        printf("Size of transmit buffer not enough!\n");
        esp_restart();
    }
    esp_err_t result;
    result = i2c_master_start(oled_handle);
    ESP_RETURN_IF_FAILED(result, "Couldn't queue start bit! Error: ");
    result = i2c_master_write_byte(oled_handle,OLED_SLAVE_ADDRESS,true);
    ESP_RETURN_IF_FAILED(result,"Couldn't queue slave address! Error: ");
    result = i2c_master_write_byte(oled_handle,CONTROL_BYTE_FOR_COMMANDS,true);
    ESP_RETURN_IF_FAILED(result,"Couldn't queue control byte for commands! Error: ");
    result=i2c_master_write(oled_handle,commands_before_data,sizeof(commands_before_data),true);
    ESP_RETURN_IF_FAILED(result, "Couldn't queue initialization commands! Error:");
    result = i2c_master_stop(oled_handle);
    ESP_RETURN_IF_FAILED(result,"Couldn't queue stop bit! Error: ");
    result=i2c_master_cmd_begin(I2C_CHANNEL,oled_handle,portMAX_DELAY);
    ESP_RETURN_IF_FAILED(result,"Couldn't send queued commands! Error: ");
    i2c_cmd_link_delete_static(oled_handle);
    oled_handle=i2c_cmd_link_create_static(transmit_buffer_for_pixels_font,sizeof(transmit_buffer_for_pixels_font));
    if (oled_handle == NULL) {
        printf("Size of transmit buffer not enough!\n");
        esp_restart();
    }
    result = i2c_master_start(oled_handle);
    ESP_RETURN_IF_FAILED(result, "Couldn't queue start bit! Error: ");
    result = i2c_master_write_byte(oled_handle,OLED_SLAVE_ADDRESS,true);
    ESP_RETURN_IF_FAILED(result,"Couldn't queue slave address! Error: ");
    result = i2c_master_write_byte(oled_handle,CONTROL_BYTE_FOR_PIXELS,true);
    ESP_RETURN_IF_FAILED(result,"Couldn't queue control byte for commands! Error: ");
    result=i2c_master_write(oled_handle,pixels_font,length_of_formatted*8,true);
    ESP_RETURN_IF_FAILED(result,"Couldn't queue pixel bytes! Error: ");
    result = i2c_master_stop(oled_handle);
    ESP_RETURN_IF_FAILED(result,"Couldn't queue stop bit! Error: ");
    result=i2c_master_cmd_begin(I2C_CHANNEL,oled_handle,portMAX_DELAY);
    ESP_RETURN_IF_FAILED(result,"Couldn't send queued commands! Error: ");
    i2c_cmd_link_delete_static(oled_handle);
}
static bool is_next_stream_a_word(char* message, int current_index, int column_checker) {
    int i=1;
    while (message[i+current_index] != '\0') {
        if (message[i+current_index] ==' ') break;
        else i++;
    }
    return ((i+column_checker)<16);
}
static void copy_characters(char* original_message, char* new_message) {
    int length_of_message=strlen(original_message);
    int i=0,j=0,k=0;
    for (;i<length_of_message;) {
        if (original_message[i] == ' ') {
            if (!is_next_stream_a_word(original_message,i,j)) {
                for (;j<16;j++,k++) new_message[k+i]=' ';
                j%=16,i++;
                continue;
            }
            else new_message[k+i]=original_message[i];
        }
        else {
            new_message[k+i]=original_message[i];
        }
        i++,j++;
        j%=16;
    }
    new_message[k+i]='\0';
}