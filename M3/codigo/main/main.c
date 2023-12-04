#include <stdio.h>
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"

#define SENSOR1_PIN GPIO_NUM_34
#define SENSOR2_PIN GPIO_NUM_33
#define SENSOR3_PIN GPIO_NUM_32
#define SENSOR4_PIN GPIO_NUM_35

#define LUZ GPIO_NUM_2
int var = 2;
bool sensor1 = false;
bool sensor2 = false;
bool sensor3 = false;
bool sensor4 = false;
int quarto1 = 0;

static void sensor1_task(void *pvParameters) {
    while (1){
        sensor1 = !gpio_get_level(SENSOR1_PIN);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    
}

static void sensor2_task(void *pvParameters) {
    
    while (1){
        sensor2 = !gpio_get_level(SENSOR2_PIN);
        vTaskDelay(pdMS_TO_TICKS(10));
    }

}

static void logic_task(void *pvParameters) {
    bool anterior1 = false;
    bool anterior2 = false;
    int timer = var;
    bool arc = false;
    while (1){
        if(arc){
            if(!sensor1&&!sensor2){
                arc = false;
            }
        }else if(anterior1){
            if(timer <= 0){
                anterior1 =false;
                timer = var;
            }else if(sensor2){
                anterior1 =false;
                quarto1 -= 1;
                arc = true;
            }
            timer -= 1;
        }else if(anterior2){
            if(timer <= 0){
                anterior2 =false;
                timer = var;
            }else if(sensor1){
                anterior2 =false;
                quarto1 += 1;
                arc = true;
            }
            timer -= 1;
        }else if (sensor1){ // saida de pessoas logica
            if(quarto1 > 0){
                anterior1 = true;
            }
        }else if (sensor2){ // entra de pessoas logica
            anterior2 = true;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
}

static void luzer_tesk(void *pvParameters){
    printf("\033[2J\033[H");
    while (1){
        if(quarto1>0){
            gpio_set_level(LUZ, 1);
        }else{
            gpio_set_level(LUZ, 0);
        }
        printf("\e[1;1H  %d \n", quarto1);
         printf(" %d \n", sensor1);
         printf(" %d \n", sensor2);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void app_main() {

    // Configurar os pinos GPIO
    esp_rom_gpio_pad_select_gpio(SENSOR1_PIN);
    gpio_set_direction(SENSOR1_PIN, GPIO_MODE_INPUT);
    esp_rom_gpio_pad_select_gpio(SENSOR2_PIN);
    gpio_set_direction(SENSOR2_PIN, GPIO_MODE_INPUT);
    esp_rom_gpio_pad_select_gpio(SENSOR3_PIN);
    gpio_set_direction(SENSOR3_PIN, GPIO_MODE_INPUT);
    esp_rom_gpio_pad_select_gpio(SENSOR4_PIN);
    gpio_set_direction(SENSOR4_PIN, GPIO_MODE_INPUT);
    esp_rom_gpio_pad_select_gpio(LUZ);
    gpio_set_direction(LUZ,  GPIO_MODE_OUTPUT);

    xTaskCreate(sensor1_task, "Sensor 1 Task", 2048, NULL, 5, NULL);
    xTaskCreate(sensor2_task, "Sensor 2 Task", 2048, NULL, 5, NULL);
    xTaskCreate(logic_task, "Plotter Task", 2048, NULL, 5, NULL);
    xTaskCreate( luzer_tesk, "luz Task", 2048, NULL, 5, NULL);
}
