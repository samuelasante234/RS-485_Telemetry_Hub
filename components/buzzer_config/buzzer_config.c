#include "buzzer_config.h"
#include "driver/ledc.h"

#define BUZZER_NUM 5
#define BUZZER_CHAN 0
#define LEDC_SPEED_MODE LEDC_LOW_SPEED_MODE
#define TIMER_NUM 0
#define ON_FREQUENCY 5000
#define BUZZER_DUTY_COUNT_FOR_ON 127
#define BUZZER_DUTY_COUNT_FOR_PAUSE 0
#define HPOINT 0
#define COUNTER_RESOLUTION LEDC_TIMER_8_BIT

void buzzer_init();
void buzzer_resume();
void buzzer_pause();

void buzzer_init() {
    ledc_timer_config_t timer_conf ={
        .clk_cfg=LEDC_APB_CLK,
        .duty_resolution=COUNTER_RESOLUTION,
        .freq_hz=8000,
        .timer_num=TIMER_NUM,
        .speed_mode = LEDC_SPEED_MODE
    };
    esp_err_t result=ledc_timer_config(&timer_conf);
    if (result != ESP_OK) {
        printf("Couldn't configure timer! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return;
    }
    ledc_channel_config_t channel_conf ={
        .channel=BUZZER_CHAN,
        .duty=BUZZER_DUTY_COUNT_FOR_PAUSE,
        .gpio_num = BUZZER_NUM,
        .hpoint=HPOINT,
        .intr_type=0,
        .timer_sel=TIMER_NUM,
        .speed_mode = LEDC_SPEED_MODE,
    };
    result = ledc_channel_config(&channel_conf);
    ledc_fade_func_install(0);
    if (result != ESP_OK) {
        printf("Couldn't configure channel! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return;
    }
    
}
void buzzer_pause() {
    esp_err_t result = ledc_set_duty_and_update(LEDC_SPEED_MODE,BUZZER_CHAN,BUZZER_DUTY_COUNT_FOR_PAUSE,HPOINT);
    if (result != ESP_OK) {
        printf("Couldn't pause timer! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return;
    }
}
void buzzer_resume() {
    esp_err_t result = ledc_set_freq(LEDC_SPEED_MODE, TIMER_NUM,ON_FREQUENCY);
    if (result != ESP_OK) {
        printf("Couldn't set frequency! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return;
    };
    result = ledc_set_duty(LEDC_SPEED_MODE, BUZZER_CHAN, BUZZER_DUTY_COUNT_FOR_ON);
    if (result != ESP_OK) {
        printf("Couldn't set duty! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return;
    };
    result = ledc_update_duty(LEDC_SPEED_MODE, BUZZER_CHAN);
    if (result != ESP_OK) {
        printf("Couldn't update duty! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return;
    };
}