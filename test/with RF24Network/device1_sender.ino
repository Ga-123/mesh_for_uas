#include <RF24.h>
#include <RF24Network.h>
#include <SPI.h>

RF24 radio(10, 9); // CE, CSN
RF24Network network(radio);

#define NODE_ID 1
#define TARGET_NODE 2

struct Payload {
  char message[32];
};

void setup() {
  Serial.begin(9600);
  Serial.println("Запуск устройства 1...");
  
  radio.begin();
  network.begin(NODE_ID);
  
  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(90);
  radio.setDataRate(RF24_250KBPS);

  delay(100); // Пауза для стабильного запуска
  Serial.println("Устройство 1 инициализировано");
}

void loop() {
  Payload payload = { "Hello from Node 1" };
  RF24NetworkHeader header(TARGET_NODE);

  Serial.println("Попытка отправки...");
  if (network.write(header, &payload, sizeof(payload))) {
    Serial.println("Сообщение успешно отправлено");
  } else {
    Serial.println("Ошибка при отправке");
  }
  
  delay(5000);  // Пауза перед повторной отправкой
}
