#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <TM1637Display.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define PIR_PIN 17
#define DHT_PIN 19
#define BUTTON_PIN 4
#define DHT_TYPE DHT22
#define CLK 13
#define DIO 12

// Configurações de WiFi
const char *ssid = "Wokwi-GUEST";
const char *password = "";  // Substitua pela sua senha
const char *mqtt_server = "46.17.108.113";
const int mqtt_port = 1883;
const char *mqtt_client_id = "ESP32Client";
const char *mqtt_topic_temp = "/TEF/DreamWatch/attrs/Média de Temperatura";
const char *mqtt_topic_humidity = "/TEF/DreamWatch/attrs/Média de Umidade";
const char *mqtt_topic_motion = "/TEF/DreamWatch/attrs/Número de Movimentos";

DHT dht(DHT_PIN, DHT_TYPE);
TM1637Display display(CLK, DIO);
WiFiClient espClient;
PubSubClient client(espClient);

bool isSleeping = false;
bool buttonPressed = false;
unsigned long sleepStartTime = 0;
unsigned long lastMovementTime = 0;
unsigned long movementRecordingStartTime = 0;
const int movementThreshold = 5 * 1000;
const int monitoringDelay = 15 * 60 * 1000;
const int monitoringInterval = 2 * 60 * 1000;
const int maxMovementsThreshold = 3;

float totalTemperature = 0.0;
float totalHumidity = 0.0;
int readingsCount = 0;
int movementsCount = 0;
float minTemperature = 100.0;
float maxTemperature = -100.0;
float minHumidity = 100.0;
float maxHumidity = 0.0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando à rede WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Conectado à rede WiFi");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao servidor MQTT...");
    if (client.connect(mqtt_client_id)) {
      Serial.println("Conectado ao servidor MQTT");
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void sendSensorData(float temperature, float humidity, int movementsCount) {
  char tempBuffer[10];
  dtostrf(temperature, 4, 2, tempBuffer);
  client.publish(mqtt_topic_temp, tempBuffer);

  char humidityBuffer[10];
  dtostrf(humidity, 4, 2, humidityBuffer);
  client.publish(mqtt_topic_humidity, humidityBuffer);

  char movementsBuffer[5];
  sprintf(movementsBuffer, "%d", movementsCount);
  client.publish(mqtt_topic_motion, movementsBuffer);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  dht.begin();
  display.setBrightness(0x0a);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int motionValue = digitalRead(PIR_PIN);
  buttonPressed = (digitalRead(BUTTON_PIN) == LOW);

  if ((motionValue == HIGH || buttonPressed) && !isSleeping) {
    isSleeping = true;
    sleepStartTime = millis();
    lastMovementTime = millis();
    Serial.println("Início do sono");
    display.setBrightness(0x0a);
  } else if (isSleeping && motionValue == LOW && !buttonPressed) {
    float currentTemperature = dht.readTemperature();
    float currentHumidity = dht.readHumidity();

    if (!isnan(currentTemperature) && !isnan(currentHumidity)) {
      totalTemperature += currentTemperature;
      totalHumidity += currentHumidity;

      readingsCount++;

      minTemperature = min(minTemperature, currentTemperature);
      maxTemperature = max(maxTemperature, currentTemperature);
      minHumidity = min(minHumidity, currentHumidity);
      maxHumidity = max(maxHumidity, currentHumidity);

      displayElapsedTime();

      if (millis() - lastMovementTime > movementThreshold) {
        Serial.println("Pessoa se mexeu durante o sono!");
        movementsCount++;

        if (millis() - sleepStartTime > monitoringDelay && movementsCount == 1) {
          movementRecordingStartTime = millis();
          Serial.println("Início do registro de movimentos");
        }
      }
    }
  } else if (isSleeping && buttonPressed) {
    isSleeping = false;
    unsigned long sleepDuration = millis() - sleepStartTime;
    Serial.print("Fim do sono. Duração: ");
    printTime(sleepDuration / 1000);
    Serial.println();

    float avgTemperature = totalTemperature / readingsCount;
    float avgHumidity = totalHumidity / readingsCount;

    Serial.print("Média de Temperatura durante o sono: ");
    Serial.println(avgTemperature);
    Serial.print("Média de Umidade durante o sono: ");
    Serial.println(avgHumidity);

    Serial.print("Temperatura Mínima durante o sono: ");
    Serial.println(minTemperature);
    Serial.print("Temperatura Máxima durante o sono: ");
    Serial.println(maxTemperature);

    Serial.print("Umidade Mínima durante o sono: ");
    Serial.println(minHumidity);
    Serial.print("Umidade Máxima durante o sono: ");
    Serial.println(maxHumidity);

    displayElapsedTime();

    Serial.print("Número de Movimentos durante o sono: ");
    Serial.println(movementsCount);

    if (movementsCount > 0) {
      Serial.print("Duração do Registro de Movimentos: ");
      printTime((millis() - movementRecordingStartTime) / 1000);
      Serial.println();
    }

    sendSensorData(avgTemperature, avgHumidity, movementsCount);

    display.setBrightness(0x00);

    totalTemperature = 0.0;
    totalHumidity = 0.0;
    readingsCount = 0;
    movementsCount = 0;
    minTemperature = 100.0;
    maxTemperature = -100.0;
    minHumidity = 100.0;
    maxHumidity = 0.0;
  }

  delay(1000);
}

void printTime(unsigned long seconds) {
  int hours = seconds / 3600;
  int minutes = (seconds % 3600) / 60;
  int remainingSeconds = seconds % 60;

  if (hours > 0) {
    Serial.print(hours);
    Serial.print("h ");
  }

  if (minutes > 0 || hours > 0) {
    Serial.print(minutes);
    Serial.print("m ");
  }

  Serial.print(remainingSeconds);
  Serial.print("s");
}

void displayElapsedTime() {
  unsigned long elapsedSeconds = (millis() - sleepStartTime) / 1000;
  int hours = elapsedSeconds / 3600;
  int minutes = (elapsedSeconds % 3600) / 60;

  display.showNumberDecEx(hours * 100 + minutes, 0b11100000, true);
  delay(1000);
}
