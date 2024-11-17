#include "meshWrap.h"

const int ledPin = 2;      // Пин для светодиода

static unsigned long ctr = 0;

void setup() {
  Serial.begin(115200);
  setupMeshMaster(ledPin);  // Инициализация сети
}

void loop() {
  updateMeshMaster();       // Обновление состояния сети

  RF24NetworkHeader header;
  payload_t payload;

  if (receiveMessagesMaster(header, payload)) {
    // Выводим информацию о полученных данных
    Serial.print("Received from node ");
    Serial.print(header.from_node);
    Serial.print(": counter = ");
    Serial.print(payload.counter);
    Serial.print(", ms = ");
    Serial.println(payload.ms);
  }

  delay(200);

  payload_t data_to_send;
  data_to_send.ms = millis();
  data_to_send.counter = ctr;
  ctr++;

  sendMessages(data_to_send);           // Отправка сообщений
}
