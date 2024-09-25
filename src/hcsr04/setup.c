#include "setup.h"

void setup_hcsr04(int trig, int echo) {
    gpio_set_direction(trig, GPIO_MODE_OUTPUT);
    gpio_set_direction(echo, GPIO_MODE_INPUT);
}

int read_hcsr04(int trig, int echo) {
    gpio_set_level(echo, 0);
    gpio_set_level(trig, 1);
    esp_rom_delay_us(10);

    uint64_t start_time_out = esp_timer_get_time();
    while(gpio_get_level(echo) == 0) {
        uint64_t time_now = esp_timer_get_time();
        if((time_now - start_time_out) >= 25000) 
            return -1; 
    }

    double start_echo_time = esp_timer_get_time();
    while(gpio_get_level(echo) == 1);

    double end_echo_time = esp_timer_get_time();

    end_echo_time = (double)end_echo_time/1000000.0;
    start_echo_time = (double)start_echo_time/1000000.0;

    int distance = (int)(((end_echo_time - start_echo_time) * 34300) / 2);
    return distance;
}
