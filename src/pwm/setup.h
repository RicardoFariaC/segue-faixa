#ifndef pwm
#define pwm
#include "driver/ledc.h"


void setup_ledc(int motor_pin, int ledc_freq, int channel);

#endif