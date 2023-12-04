#include <stdio.h>
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"
#include "freertos/semphr.h"
#define SENSOR1_PIN GPIO_NUM_34
#define SENSOR2_PIN GPIO_NUM_33
#define SENSOR3_PIN GPIO_NUM_32
#define SENSOR4_PIN GPIO_NUM_35

#define LUZ GPIO_NUM_2

bool sensor1 = false;
bool sensor2 = false;
bool sensor3 = false;
bool sensor4 = false;
bool anterior1 = false;
bool anterior2 = false;
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

    bool arco = false;
    while (1){
        if(arco){
            if(!sensor1&&!sensor2){
                arco = false;
            }
        }else if (((!anterior1&&anterior2)&&sensor1)||(anterior2&&(sensor1&&!sensor2))){ // saida de pessoas logica
            if(quarto1 > 0){
                quarto1 -=1;
            }
            arco = true;
        }else if (((anterior1&&!anterior2)&&sensor2)||((anterior1&&!sensor1)&&sensor2)){ // entra de pessoas logica
            quarto1 += 1;
            arco = true;
        }

        anterior1 = sensor1;
        anterior2 = sensor2;
        vTaskDelay(pdMS_TO_TICKS(15));
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
        vTaskDelay(100 / portTICK_PERIOD_MS);
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
