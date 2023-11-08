#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "sdkconfig.h"

#define SENSOR_PIN_1 GPIO_NUM_25
#define SENSOR_PIN_2 GPIO_NUM_33

void app_main(void)
{
    // Configurar os pinos GPIO
    esp_rom_gpio_pad_select_gpio(SENSOR_PIN_1);
    gpio_set_direction(SENSOR_PIN_1, GPIO_MODE_INPUT);
    esp_rom_gpio_pad_select_gpio(SENSOR_PIN_2);
    gpio_set_direction(SENSOR_PIN_2, GPIO_MODE_INPUT);


    int pessoasDentro = 0; // Variável para contar pessoas
    int ACO = 0;
    while(1) {
        //le o sinal do sensor invertido pelo ja que o sensor e normalmente ativo
        //se for 1 tem obstaculo se for 0 ncao tem
        int sensorValue1 = !gpio_get_level(SENSOR_PIN_1);
        int sensorValue2 = !gpio_get_level(SENSOR_PIN_2);
        /*logica 
            o sensor 1 aciona primeiro e posteriomente o sensor 2, onse o sensor 1 pode ou nao permasecer ativo 
            entrada
            caso 1:
            __|¯¯¯¯|______________
            ___________|¯¯¯¯|_____
            nesse caso vai ser apliado o sinal ate o senal 2 ser ativo 
            __|¯¯¯¯¯¯¯¯|__________
            ___________|¯¯¯¯|_____
            caso 2:
            __|¯¯¯¯¯¯¯|___________
            ______|¯¯¯¯¯¯¯|_______
            caso 3: 
            __|¯¯¯¯¯¯¯|___________
            __|¯¯¯¯¯¯¯¯¯|_________
            
            a analogo considerase como saida 
        */
        if (sensorValue1){ //se o senor um acionado 
            
        }
    }
    // Limpar a tela
    printf("\033[2J\033[H");

    // Imprimir o número de pessoas
    printf("Pessoas dentro: %d\n", pessoasDentro);

    // Atualizar os últimos valores dos sensores
        


    //vTaskDelay(pdMS_TO_TICKS(100)); // atraso de 0,001 segund
}
