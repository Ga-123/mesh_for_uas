#include "meshWrap.h"

const int ledPin = 2;      // Пин для светодиода
const uint8_t nodeID = 2;  // Уникальный ID релейного узла (для каждого устройства меняем ID)

void setup() {
  Serial.begin(115200);
  setupMesh(nodeID, ledPin);  // Инициализация сети с уникальным ID и светодиодом
}

void loop() {
  updateMesh();             // Обновление состояния сети
  sendScheduledMessage();   // Отправка сообщений
  receiveMessages();        // Обработка входящих сообщений
}
