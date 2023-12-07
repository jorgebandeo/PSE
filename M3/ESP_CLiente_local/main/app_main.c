/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "driver/gpio.h"
#include <driver/adc.h>
#include <esp_adc_cal.h>

#define SENSOR1_PIN GPIO_NUM_5
#define SENSOR2_PIN GPIO_NUM_18

#define LUZ GPIO_NUM_2

#define ADC_CHANNEL ADC1_CHANNEL_4 // GPIO 32


/*
#define PINO_D13 13
#define PINO_D12 12
#define PINO_D14 14
#define PINO_D27 27
#define PINO_D26 26
#define PINO_D25 25
#define PINO_D33 33
#define PINO_D32 32
#define PINO_D35 23

static int pinos[] = {PINO_D13, PINO_D12, PINO_D14, PINO_D27, PINO_D26, PINO_D25, PINO_D33, PINO_D32,PINO_D35 };
*/
static const char *TAG = "mqtt";

bool sensor1 = false;
bool sensor2 = false;
bool sensor3 = false;
bool sensor4 = false;
bool anterior1 = false;
bool anterior2 = false;
int quarto1 = 0;
int LDR_voltag = 0;

esp_mqtt_client_handle_t client;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_BROKER_URL,
    };
#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.broker.address.uri, "FROM_STDIN") == 0) {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128) {
            int c = fgetc(stdin);
            if (c == '\n') {
                line[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt_cfg.broker.address.uri = line;
        printf("Broker url: %s\n", line);
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

    client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}


static void sensor1_task(void *pvParameters) {
    while (1){
        sensor1 = !gpio_get_level(SENSOR1_PIN);
        //sensor1 = !gpio_get_level(pinos[0]);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    
}

static void sensor2_task(void *pvParameters) {
    while (1){
        sensor2 = !gpio_get_level(SENSOR2_PIN);
        //sensor2 = !gpio_get_level(pinos[1]);
        vTaskDelay(pdMS_TO_TICKS(10));
    }

}




        


/*
void configurar_pullups() {
    gpio_config_t io_conf;

    // Lista de pinos a serem configurados com pull-up
    int num_pinos = sizeof(pinos) / sizeof(pinos[0]);

    // Configuração dos pinos como entradas com pull-up
    for (int i = 0; i < num_pinos; i++) {
        io_conf.pin_bit_mask = (1ULL << pinos[i]);
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        gpio_config(&io_conf);
   }
}
*/

static void logic_task(void *pvParameters) {
    
    bool arco = false;
    while (1){
        if(arco){
            if(!sensor1&&!sensor2){
                arco = false;
                //msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "OFF", 0, 0, 0);
                //ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            }
        }else if (((!anterior1&&anterior2)&&sensor1)||(anterior2&&(sensor1&&!sensor2))){ // saida de pessoas logica
            if(quarto1 > 0){
                quarto1 -=1;
            }
            arco = true;
        }else if (((anterior1&&!anterior2)&&sensor2)||((anterior1&&!sensor1)&&sensor2)){ // entra de pessoas logica
            quarto1 += 1;
            arco = true;
            //msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "ON", 0, 0, 0);
            //ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        }

        anterior1 = sensor1;
        anterior2 = sensor2;

        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
}

static void luzer_tesk(void *pvParameters){
        // Configure ADC
	adc1_config_width(ADC_WIDTH_BIT_12);
	adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_11); // @suppress("Symbol is not resolved")

    int msg_id;
    int len;
    int last_quarto1_state = quarto1;
    printf("\033[2J\033[H");
    while (1){
        
            //if (quarto1 != last_quarto1_state) {
            // O estado de quarto1 mudou, publicar no MQTT
            //last_quarto1_state = quarto1;
            LDR_voltag = adc1_get_raw(ADC_CHANNEL);
        
            if(LDR_voltag == -1){
                ESP_LOGI("LDR", "There is a error in LDR");
                if (quarto1 > 0) {
                gpio_set_level(LUZ, 1);
                msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "ONN", 0, 0, 0);
                ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
                } else {
                    gpio_set_level(LUZ, 0);
                    msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "OFF", 0, 0, 0);
                    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
                }
            } else {
                LDR_voltag = (LDR_voltag*100)/4096;
                printf("LDR Value: %d \n", LDR_voltag);
                char str[6]; // Enough space for 6 digits plus a null terminator
                sprintf(str, "%d", LDR_voltag);

                len = strlen(str); // Set the length to the actual length of the string
                
                char data[len];
                for (int j = 0; j < len; j++) {
                    data[j] = str[j];
                }

                
            

                if (quarto1 > 0) {
                    gpio_set_level(LUZ, 1);
                    char message[6]; // Ajuste o tamanho conforme necessário
                    sprintf(message, "ONN%s", data);
                    msg_id = esp_mqtt_client_publish(client, "/topic/qos0", message, 0, 0, 0);
                    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
                } else {
                    gpio_set_level(LUZ, 0);
                    char message[6]; // Ajuste o tamanho conforme necessário
                    sprintf(message, "OFF%s", data);
                    msg_id = esp_mqtt_client_publish(client, "/topic/qos0", message, 0, 0, 0);
                    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
                }
            
        }

        
        printf("\e[1;1H  %d \n", quarto1);
        printf("%d \n", LDR_voltag);
        printf(" %d \n", sensor1);
        printf(" %d \n", sensor2);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    //configurar_pullups();
    
        //Configure ADC


    //Characterize ADC






    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
    esp_log_level_set("mqtt_example", ESP_LOG_VERBOSE);
    esp_log_level_set("transport_base", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("transport", ESP_LOG_VERBOSE);
    esp_log_level_set("outbox", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    mqtt_app_start();

    // Configurar os pinos GPIO
    
    esp_rom_gpio_pad_select_gpio(SENSOR1_PIN);
    gpio_set_direction(SENSOR1_PIN, GPIO_MODE_INPUT);
    esp_rom_gpio_pad_select_gpio(SENSOR2_PIN);
    gpio_set_direction(SENSOR2_PIN, GPIO_MODE_INPUT);
    
    esp_rom_gpio_pad_select_gpio(LUZ);
    gpio_set_direction(LUZ,  GPIO_MODE_OUTPUT);

    xTaskCreate(sensor1_task, "Sensor 1 Task", 2048, NULL, 5, NULL);
    xTaskCreate(sensor2_task, "Sensor 2 Task", 2048, NULL, 5, NULL);
    xTaskCreate(logic_task, "Plotter Task", 2048, NULL, 5, NULL);
    xTaskCreate(luzer_tesk, "luz Task", 2048, NULL, 5, NULL);

}
