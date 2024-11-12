#include <RF24.h>
#include <RF24Network.h>
#include <SPI.h>

RF24 radio(10, 9); // CE, CSN
RF24Network network(radio);

#define NODE_ID 2

struct Payload {
  char message[32];
};

void setup() {
  Serial.begin(9600);
  Serial.println("Запуск устройства 2...");
  
  radio.begin();
  network.begin(NODE_ID);

  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(90);
  radio.setDataRate(RF24_250KBPS);
  
  Serial.println("Устройство 2 инициализировано");
}

void loop() {
  network.update();  // Обновление сети для получения данных

  while (network.available()) {
    RF24NetworkHeader header;
    Payload payload;
    network.read(header, &payload, sizeof(payload));
    
    Serial.print("Получено сообщение: ");
    Serial.println(payload.message);
  }
  
  delay(1000);  // Задержка для стабильной работы сети
}
