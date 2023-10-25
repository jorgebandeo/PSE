#include <DHT.h>
#define LED 2
#define LDR 14
#define DHTPIN 5     // Pino ao qual o sensor DHT11 está conectado
#define DHTTYPE DHT11 // Tipo de sensor DHT (DHT11 neste caso)
#define pinRelay  4 
#define pinRelay2  12
#define WATER_SENSOR_PIN 13 // Pino ao qual o sensor de nível de água está conectado

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(WATER_SENSOR_PIN, INPUT);
  pinMode(LDR, INPUT);
  pinMode(pinRelay, OUTPUT); // Configura o pino do relé como saída
  pinMode(pinRelay2, OUTPUT); // Configura o pino do relé como saída
  pinMode(LED, OUTPUT);
}

void loop() {

  delay(2000); // Aguarda 2 segundos entre leituras

  float humidity = dht.readHumidity(); // Lê a umidade do sensor
  float temperature = dht.readTemperature(); // Lê a temperatura em Celsius (por padrão)
  int waterLevel = analogRead(WATER_SENSOR_PIN); // Lê o estado do sensor de nível de água
  int ldr = analogRead(LDR);
  // Verifica se a leitura do sensor foi bem-sucedida
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Falha ao ler o sensor DHT!");
    return;
  }

  // Exibe os valores lidos no monitor serial
  Serial.print("Umidade: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperatura: ");
  Serial.print(temperature);
  Serial.print(" °C\t");
  Serial.print("luz: ");
  Serial.print(ldr );
  Serial.print(" Nível de água: ");
  Serial.print(waterLevel );
  Serial.print("aquecedor :");  
  if (temperature > 25){
    digitalWrite(pinRelay, LOW); // Desliga o relé (desaciona o dispositivo)
    Serial.println("desligado");
    digitalWrite(pinRelay2, LOW);
    
  }
    
  else if(temperature <= 25){
    digitalWrite(pinRelay, HIGH); // Liga o relé (aciona o dispositivo)
    Serial.println("ligado");
    digitalWrite(pinRelay2, HIGH);
    
  }else{
    Serial.println("desligado");
  }
  
  if (ldr < 600){
    digitalWrite(LED, HIGH);
  }else{
    digitalWrite(LED, LOW);
  }

  
}
