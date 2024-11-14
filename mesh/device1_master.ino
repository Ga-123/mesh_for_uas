#include "RF24Network.h"
#include "RF24.h"
#include "RF24Mesh.h"
#include <SPI.h>


//Настройка CE и CSN
RF24 radio(10, 9);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

const int ledPin = 2;  // Пин для светодиода

// Структура данных для передачи в сети
struct payload_t {
  unsigned long ms;
  unsigned long counter;
};

uint32_t ctr = 0;
uint32_t displayTimer = 0;


// Обработка входящих сообщений
void receiveMessages() {
  if (network.available()) {
    RF24NetworkHeader header;
    payload_t payload;
    network.read(header, &payload, sizeof(payload));

    // Выводим информацию о полученных данных
    Serial.print("Received from node ");
    Serial.print(header.from_node);
    Serial.print(": counter = ");
    Serial.print(payload.counter);
    Serial.print(", ms = ");
    Serial.println(payload.ms);

    delay(200);
  }
}

// Отправка сообщений на все узлы в сети
void sendMessages() {
  if (millis() - displayTimer > 5000) {
    ctr++;
    payload_t payload = { millis(), ctr };

    // Цикл для отправки данных на все узлы в сети, кроме самого себя
    for (int i = 0; i < mesh.addrListTop; i++) {
      if (mesh.addrList[i].nodeID != 0) {
        RF24NetworkHeader header(mesh.addrList[i].address, 'M');
        bool success = network.write(header, &payload, sizeof(payload));

        digitalWrite(ledPin, success ? HIGH : LOW);
        
        Serial.println(success ? F("Send OK") : F("Send Fail"));
      }
    }
    delay(200);
    digitalWrite(ledPin, LOW);
    displayTimer = millis();
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  pinMode(ledPin, OUTPUT);

  mesh.setNodeID(0);  // Устанавливаем ID узла (0 - главный узел)
  Serial.println(mesh.getNodeID());

  // Инициализируем сеть, если не удается, то выводим ошибку
  if (!mesh.begin()) {
    Serial.println(F("Radio hardware not responding."));
    while (1) {}
  }
}

void loop() {
  mesh.update();   // Обновляем состояние сети
  mesh.DHCP();     // Управляем распределением адресов узлов в сети

  receiveMessages();  // Обработка входящих сообщений
  sendMessages();     // Отправка сообщений
}
