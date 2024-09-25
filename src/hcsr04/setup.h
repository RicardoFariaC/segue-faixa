#ifndef hcsr04
#define hcsr04

#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void setup_hcsr04(int trig, int echo);
int read_hcsr04(int trig, int echo);

#endif