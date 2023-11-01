#include <DHT.h>
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT; // Inicializa a biblioteca BluetoothSerial para comunicação Bluetooth
char valorRecebido; // Variável para armazenar o valor recebido via Bluetooth
float temperature_maxima; // Variável para armazenar a temperatura máxima
float temperatura_minima; // Variável para armazenar a temperatura mínima

#define LED 2 // Define o pino do LED
#define LDR 14 // Define o pino do sensor de luz (LDR)
#define DHTPIN 5     // Pino ao qual o sensor DHT11 está conectado
#define DHTTYPE DHT11 // Tipo de sensor DHT (DHT11 neste caso)
#define pinRelay  4   // Pino ao qual o relé 1 está conectado
#define pinRelay2  12  // Pino ao qual o relé 2 está conectado
#define WATER_SENSOR_PIN 13 // Pino ao qual o sensor de nível de água está conectado

DHT dht(DHTPIN, DHTTYPE); // Inicializa o sensor DHT com o pino especificado e o tipo de sensor

void limparTerminal() {
  SerialBT.print("\033[H\033[J"); // Limpa o terminal Bluetooth
}

void setup() {
  Serial.begin(115200); // Inicializa a comunicação serial via USB
  SerialBT.begin("JORGE BANDEO"); // Inicializa a comunicação Bluetooth com o nome "JORGE BANDEO"
  Serial.println("O dispositivo já pode ser pareado!"); // Mensagem de inicialização
  dht.begin(); // Inicializa o sensor DHT
  pinMode(WATER_SENSOR_PIN, INPUT); // Configura o pino do sensor de nível de água como entrada
  pinMode(LDR, INPUT); // Configura o pino do sensor de luz como entrada
  pinMode(pinRelay, OUTPUT); // Configura o pino do relé 1 como saída
  pinMode(pinRelay2, OUTPUT); // Configura o pino do relé 2 como saída
  pinMode(LED, OUTPUT); // Configura o pino do LED como saída
}

bool COMD = false; // Variável para armazenar o estado do comando geral
bool led_comd = false; // Variável para armazenar o estado do comando do LED
bool bomb_comd = false; // Variável para armazenar o estado do comando da bomba
bool temp_comd = false; // Variável para armazenar o estado do comando do aquecedor

void loop() {
  valorRecebido = (char)SerialBT.read(); // Lê o valor recebido via Bluetooth
  
  if (Serial.available()) {
    SerialBT.write(Serial.read()); // Repassa os dados recebidos via USB para o Bluetooth
  }

  if (SerialBT.available()) {
    // Verifica o valor recebido via Bluetooth e atualiza os estados dos comandos correspondentes
    if(valorRecebido == 'c') {
      COMD = !COMD; // Inverte o estado do comando geral
    }
    if(valorRecebido == 'l') {
      led_comd = !led_comd; // Inverte o estado do comando do LED
    }    
    if(valorRecebido == 'b') {
      bomb_comd = !bomb_comd; // Inverte o estado do comando da bomba
    }
    if(valorRecebido == 'a') {
      temp_comd = !temp_comd; // Inverte o estado do comando do aquecedor
    }
  }
  delay(30); // Aguarda 30 milissegundos

  delay(2000); // Aguarda 2 segundos entre leituras

  float temperature = dht.readTemperature(); // Lê a temperatura do sensor DHT
  int waterLevel = analogRead(WATER_SENSOR_PIN); // Lê o valor do sensor de nível de água
  int ldr = analogRead(LDR); // Lê o valor do sensor de luz (LDR)
  
  // Verifica se a leitura do sensor de temperatura foi bem-sucedida
  if (isnan(temperature)) {
    SerialBT.println("Falha ao ler o sensor DHT!"); // Mensagem de erro se a leitura do sensor falhar
    return;
  }

  // Exibe os valores lidos no monitor SerialBT
  SerialBT.print("comando: ");
  SerialBT.print(COMD ? " Ativo": "Desativo"); // Exibe se o comando geral está ativado ou desativado
  SerialBT.println();
  SerialBT.print("Temperatura: ");
  SerialBT.print(temperature); // Exibe a temperatura lida pelo sensor DHT
  SerialBT.print(" °C\t");
  SerialBT.print("luz: ");
  SerialBT.print(ldr); // Exibe o valor lido pelo sensor de luz (LDR)
  SerialBT.print(" Nível de água: ");
  SerialBT.print(waterLevel); // Exibe o valor lido pelo sensor de nível de água
  SerialBT.println();
  SerialBT.print("aquecedor : ");  

  bool over = false; // Variável para verificar se uma condição de desligamento foi alcançada

  // Controle do aquecedor com base na temperatura e nos comandos recebidos
  if (!temp_comd && COMD) {
    digitalWrite(pinRelay, LOW); // Desliga o aquecedor se o comando do aquecedor estiver desativado
    SerialBT.print("desligado ");
    over = true;
  } else if (temperature < 25 || (temp_comd && COMD)) {
    digitalWrite(pinRelay, HIGH); // Liga o aquecedor se a temperatura for inferior a 25°C ou se o comando do aquecedor estiver ativado
    SerialBT.print("ligado ");
  } else {
    digitalWrite(pinRelay, LOW); // Desliga o aquecedor se a temperatura for superior a 25°C
    SerialBT.print("desligado ");
    over = true;
  }
  
  // Controle da bomba com base nas condições de desligamento e nos comandos recebidos
  SerialBT.print("bomba : "); 
  if (!bomb_comd && COMD) {
    digitalWrite(pinRelay2, HIGH); // Desliga a bomba se o comando da bomba estiver desativado
    SerialBT.print("desligado ");
  } else if (over || (bomb_comd && COMD)) {
    digitalWrite(pinRelay2, LOW); // Liga a bomba se uma condição de desligamento for alcançada ou se o comando da bomba estiver ativado
    SerialBT.print("ligado ");
  } else {
    digitalWrite(pinRelay2, HIGH); // Desliga a bomba se nenhuma condição de desligamento for alcançada
    SerialBT.print("desligado ");
  }
  
  // Controle do LED com base nas condições de desligamento e nos comandos recebidos
  SerialBT.print("LUZ : "); 
  if (!led_comd && COMD) {
    digitalWrite(LED, LOW); // Desliga o LED se o comando do LED estiver desativado
    SerialBT.print("desligado ");
  } else if (ldr < 600 || (led_comd && COMD)) {
    digitalWrite(LED, HIGH); // Liga o LED se o valor do sensor de luz for inferior a 600 ou se o comando do LED estiver ativado
    SerialBT.print("ligado ");
  } else {
    digitalWrite(LED, LOW); // Desliga o LED se o valor do sensor de luz for superior a 600
    SerialBT.print("desligado ");
  }
  SerialBT.println(); // Pula uma linha no monitor SerialBT
}