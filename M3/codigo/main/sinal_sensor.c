#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"

#define SENSOR1_PIN GPIO_NUM_25
#define SENSOR2_PIN GPIO_NUM_33
#define BUFFER_SIZE 30

SemaphoreHandle_t sensor1_semaphore;
SemaphoreHandle_t sensor2_semaphore;
int sensor1_values[BUFFER_SIZE];
int sensor2_values[BUFFER_SIZE];
int sensor1_index = 0;
int sensor2_index = 0;

void sensor1_task(void *pvParameters) {
    while (1) {
        if (xSemaphoreTake(sensor1_semaphore, portMAX_DELAY)) {
            int valorSensor1 = gpio_get_level(SENSOR1_PIN);
            sensor1_values[sensor1_index] = valorSensor1;
            sensor1_index = (sensor1_index + 1) % BUFFER_SIZE;
            xSemaphoreGive(sensor1_semaphore);
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void sensor2_task(void *pvParameters) {
    while (1) {
        if (xSemaphoreTake(sensor2_semaphore, portMAX_DELAY)) {
            int valorSensor2 = gpio_get_level(SENSOR2_PIN);
            sensor2_values[sensor2_index] = valorSensor2;
            sensor2_index = (sensor2_index + 1) % BUFFER_SIZE;
            xSemaphoreGive(sensor2_semaphore);
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void plotter_task(void *pvParameters) {
    // Limpar o terminal
        printf("\e[1;1H\e[2J");

    while (1) {
        // Posicionar o cursor na linha 1 do terminal
        printf("\e[1;1H");

        // Imprimir o gráfico na linha 1
        printf("Sensor 1: ");
        for (int i = 0; i < BUFFER_SIZE; ++i) {
            printf("%c ", sensor1_values[(sensor1_index + i) % BUFFER_SIZE] ? '|' : '_');
        }
        printf("\n");

        // Imprimir o gráfico na linha 2
        printf("Sensor 2: ");
        for (int i = 0; i < BUFFER_SIZE; ++i) {
            printf("%c ", sensor2_values[(sensor2_index + i) % BUFFER_SIZE] ? '|' : '_');
        }
        printf("\n");

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void app_main() {
    sensor1_semaphore = xSemaphoreCreateBinary();
    sensor2_semaphore = xSemaphoreCreateBinary();

    gpio_config_t io_config = {
        .pin_bit_mask = (1ULL << SENSOR1_PIN) | (1ULL << SENSOR2_PIN),
        .mode = GPIO_MODE_INPUT,
    };
    gpio_config(&io_config);

    xTaskCreate(sensor1_task, "Sensor 1 Task", 2048, NULL, 10, NULL);
    xTaskCreate(sensor2_task, "Sensor 2 Task", 2048, NULL, 10, NULL);
    xTaskCreate(plotter_task, "Plotter Task", 2048, NULL, 5, NULL);

    xSemaphoreGive(sensor1_semaphore);
    xSemaphoreGive(sensor2_semaphore);
}
