#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "hcsr04/setup.h"
#include "pwm/setup.h"
#include "driver/ledc.h"

// --- HCSR04 SETUP ---
#define TRIGGER 1
#define ECHO 2
// --- END SETUP ---

// --- LEDC SETUP ---
#define MOTOR_LDIR_0 26
#define MOTOR_LDIR_1 27

#define MOTOR_RDIR_0 33
#define MOTOR_RDIR_1 25

#define MOTOR_PIN_L 13                  
#define MOTOR_PIN_R 14                   
#define LEDC_CHANNEL_L LEDC_CHANNEL_0   // Canal PWM utilizado (ESP32 possui 16 canais: de 0 a 15).
#define LEDC_CHANNEL_R LEDC_CHANNEL_1   // Canal PWM utilizado (ESP32 possui 16 canais: de 0 a 15).
#define LEDC_TIMER LEDC_TIMER_0       // Temporizador associado ao canal PWM (ESP32 possui 4 temporizadores: de 0 a 3).
#define LEDC_MODE LEDC_LOW_SPEED_MODE // Modo de operação do PWM. 'LEDC_LOW_SPEED_MODE' é geralmente usado para frequências abaixo de 80 MHz.
#define LEDC_FREQUENCY 1000           // Frequência do PWM em Hz. Para motores DC, frequências típicas variam de 1 kHz a 20 kHz.
#define LEDC_RESOLUTION LEDC_TIMER_8_BIT // Resolução do PWM em bits. 8 bits permitem 256 níveis de controle de velocidade (0 a 255).


// --- END SETUP ---

// --- IR SETUP ---
#define GPIO_SENSOR_R 18
#define GPIO_SENSOR_L 19

volatile bool r_line = true;
volatile bool l_line = true;
// --- END SETUP ---


void delay(int ms);
void r_handle_interrupt(void *arg);
void l_handle_interrupt(void *arg);
void setup_line_sensor();
void app_main(); 
void setup(); 
void loop();

void delay(int ms) {
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

void r_handle_interrupt(void *arg) {
    r_line = false;
}

void l_handle_interrupt(void *arg) {
    l_line = false;
}

void setup_line_sensor() {
    gpio_install_isr_service(0);
    gpio_set_direction(GPIO_SENSOR_R, GPIO_MODE_INPUT);
    gpio_set_direction(GPIO_SENSOR_L, GPIO_MODE_INPUT);

    gpio_pullup_en(GPIO_SENSOR_R);
    gpio_pullup_en(GPIO_SENSOR_L);

    gpio_set_intr_type(GPIO_SENSOR_R, GPIO_INTR_POSEDGE);
    gpio_isr_handler_add(GPIO_SENSOR_R, r_handle_interrupt, NULL);

    gpio_set_intr_type(GPIO_SENSOR_L, GPIO_INTR_POSEDGE);
    gpio_isr_handler_add(GPIO_SENSOR_L, l_handle_interrupt, NULL);
}

void setup() {
    // L MOTOR SETUP

    setup_line_sensor();
    gpio_set_direction(MOTOR_LDIR_0, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTOR_LDIR_1, GPIO_MODE_OUTPUT);
    
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,            // Define o modo de operação (baixa velocidade).
        .timer_num = LEDC_TIMER,            // Define o temporizador que será usado (temporizador 0).
        .duty_resolution = LEDC_RESOLUTION, // Define a resolução do duty cycle (8 bits = 256 níveis).
        .freq_hz = LEDC_FREQUENCY,          // Define a frequência do PWM (comum 1000 Hz para controle de motor DC). verificar manual motor para frequencias maiores
        .clk_cfg = LEDC_AUTO_CLK            // Define o tipo de clock. LEDC_AUTO_CLK permite que o sistema escolha o clock apropriado.
    };
    ledc_timer_config(&ledc_timer); // Aplica as configurações do temporizador.

    // Configuração do canal LEDC para o pino do motor DC
    ledc_channel_config_t ledc_channel = {
        .gpio_num = MOTOR_PIN_L,          // Define o pino GPIO onde o motor DC está conectado.
        .speed_mode = LEDC_MODE,        // Define o modo de operação (baixa velocidade).
        .channel = LEDC_CHANNEL_L,        // Define o canal PWM (canal 0).
        .intr_type = LEDC_INTR_DISABLE, // Desabilita interrupções para este canal.
        .timer_sel = LEDC_TIMER,        // Associa o canal PWM ao temporizador configurado (temporizador 0).
        .duty = 0,                      // Define o ciclo de trabalho inicial (0% = motor desligado).
        .hpoint = 0                     
    };
    ledc_channel_config(&ledc_channel);
    
    // setup_ledc(MOTOR_PIN_L, LEDC_FREQUENCY, LEDC_CHANNEL_L);
    // END SETUP

    // R MOTOR SETUP
    gpio_set_direction(MOTOR_RDIR_0, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTOR_RDIR_1, GPIO_MODE_OUTPUT);


    ledc_timer_config_t ledc_timer_r = {
        .speed_mode = LEDC_MODE,            // Define o modo de operação (baixa velocidade).
        .timer_num = LEDC_TIMER_1,            // Define o temporizador que será usado (temporizador 0).
        .duty_resolution = LEDC_RESOLUTION, // Define a resolução do duty cycle (8 bits = 256 níveis).
        .freq_hz = LEDC_FREQUENCY,          // Define a frequência do PWM (comum 1000 Hz para controle de motor DC). verificar manual motor para frequencias maiores
        .clk_cfg = LEDC_AUTO_CLK            // Define o tipo de clock. LEDC_AUTO_CLK permite que o sistema escolha o clock apropriado.
    };
    ledc_timer_config(&ledc_timer_r); // Aplica as configurações do temporizador.

    // Configuração do canal LEDC para o pino do motor DC
    ledc_channel_config_t ledc_channel_r = {
        .gpio_num = MOTOR_PIN_R,          // Define o pino GPIO onde o motor DC está conectado.
        .speed_mode = LEDC_MODE,        // Define o modo de operação (baixa velocidade).
        .channel = LEDC_CHANNEL_R,        // Define o canal PWM (canal 0).
        .intr_type = LEDC_INTR_DISABLE, // Desabilita interrupções para este canal.
        .timer_sel = LEDC_TIMER_1,        // Associa o canal PWM ao temporizador configurado (temporizador 0)
        .duty = 0,                      // Define o ciclo de trabalho inicial (0% = motor desligado).
        .hpoint = 0                     
    };
    ledc_channel_config(&ledc_channel_r);

    // setup_ledc(MOTOR_PIN_R, LEDC_FREQUENCY, LEDC_CHANNEL_R);
    // END SETUP


    // setup_ledc(MOTOR_PIN_L, LEDC_FREQUENCY, MOTOR_L_CHANNEL);
    // setup_ledc(MOTOR_PIN_R, LEDC_FREQUENCY);
    // setup_hcsr04(TRIGGER, ECHO);
}

void loop() {
    int l_pwm = 200;
    int r_pwm = 200;
    while(1) {
        // if(l_line && r_line) {
        //     l_pwm = 0;
        //     r_pwm = 0;
        // } else if(l_line && !r_line) {
        //     l_pwm = 0;
        //     r_pwm = 255;
        // } else if(!l_line && r_line) {
        //     l_pwm = 255;
        //     r_pwm = 0;
        // } else {
        //     l_pwm = 255;
        //     r_pwm = 255;
        // }

        if(r_line && l_line) { // FRENTE
            gpio_set_level(MOTOR_LDIR_0, 0);
            gpio_set_level(MOTOR_LDIR_1, 1);

            gpio_set_level(MOTOR_RDIR_0, 0);
            gpio_set_level(MOTOR_RDIR_1, 1);
        } else if(!r_line && l_line) { // VIRA A 
            gpio_set_level(MOTOR_LDIR_0, 0);
            gpio_set_level(MOTOR_LDIR_1, 1);

            gpio_set_level(MOTOR_RDIR_0, 1);
            gpio_set_level(MOTOR_RDIR_1, 0);            
        } else if(r_line && !l_line) {
            gpio_set_level(MOTOR_LDIR_0, 1);
            gpio_set_level(MOTOR_LDIR_1, 0);

            gpio_set_level(MOTOR_RDIR_0, 0);
            gpio_set_level(MOTOR_RDIR_1, 1);
        } else { // FREIO
            gpio_set_level(MOTOR_LDIR_0, 0);
            gpio_set_level(MOTOR_LDIR_1, 0);

            gpio_set_level(MOTOR_RDIR_0, 0);
            gpio_set_level(MOTOR_RDIR_1, 0); 
        }

        printf("RLINHA: %d\n", r_line);
        printf("LLINHA: %d\n", l_line);

        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_L, l_pwm);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_L);
 
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_R, r_pwm);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_R);
        delay(0);

        r_line = true;
        l_line = true;
    
        delay(100);
    }
}

void app_main() {
    vTaskDelay(pdMS_TO_TICKS(5000));
    setup();
    loop();
}
