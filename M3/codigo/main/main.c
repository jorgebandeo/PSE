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
        // Ler os sensores digitais
        
        

        // Verificar a sequência de ativação
        if (ACO <0){
            ACO ++;
        }else{
            ACO = 0;
            int sensorValue1 = gpio_get_level(SENSOR_PIN_1);
            int sensorValue2 = gpio_get_level(SENSOR_PIN_2);
            if(!sensorValue1) {
                bool ativo = true;
                while(ativo == true){
                    sensorValue1 = gpio_get_level(SENSOR_PIN_1);
                    if(sensorValue1){
                        int ativo2 = 5;
                        while(ativo2 > 0 ){
                            sensorValue2 = gpio_get_level(SENSOR_PIN_2);
                            ativo2--;
                            if(!sensorValue2){
                                pessoasDentro++; // Alguém entrou
                                ativo2 = 0;
                                ativo = false;
                                break;
                            }
                        }

                    }
                    
                }
            } else if(!sensorValue2) {
                if(pessoasDentro>0){
                    bool ativo = true;
                    while(ativo == true){
                        sensorValue2 = gpio_get_level(SENSOR_PIN_2);
                        if(sensorValue2){
                            int ativo2 = 5;
                            while(ativo2 > 0 ){
                                sensorValue1 = gpio_get_level(SENSOR_PIN_1);
                                ativo2--;
                                if(!sensorValue1){
                                    pessoasDentro--; // Alguém saiu
                                    ativo2 = 0;
                                    ativo = false;
                                    break;
                                }
                            }

                        }
                        
                    }
                }
            }
        
        }
        // Limpar a tela
        printf("\033[2J\033[H");

        // Imprimir o número de pessoas
        printf("Pessoas dentro: %d\n", pessoasDentro);

        // Atualizar os últimos valores dos sensores
        


        //vTaskDelay(pdMS_TO_TICKS(100)); // atraso de 0,001 segundo
    }
    
}
