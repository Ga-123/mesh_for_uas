#include "meshWrap.h"

const int ledPin = 2;      // Пин для светодиода
const uint8_t nodeID = 2;  // Уникальный ID узла (для каждого устройства меняем ID)

static unsigned long ctr = 0;

void setup() {
  Serial.begin(115200);
  setupMesh(nodeID, ledPin);  // Инициализация сети с уникальным ID и светодиодом
}

void loop() {
  updateMesh();             // Обновление состояния сети

  payload_t data_to_send;
  data_to_send.ms = millis();
  data_to_send.counter = ctr;

  sendScheduledMessage(data_to_send);   // Отправка сообщений

  RF24NetworkHeader header;
  payload_t payload;
  if (receiveMessages(header, payload)) {
    // Вывод информации о полученных данных
    Serial.print("Received packet #");
    Serial.print(payload.counter);
    Serial.print(" at ");
    Serial.println(payload.ms);
  }
}
