#include "meshWrap.h"

const int ledPin = 2;      // Пин для светодиода

void setup() {
  Serial.begin(115200);
  setupMeshMaster(ledPin);  // Инициализация сети
}

void loop() {
  updateMeshMaster();       // Обновление состояния сети
  receiveMessagesMaster();  // Обработка входящих сообщений
  sendMessages();           // Отправка сообщений
}
