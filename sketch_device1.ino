#include <RF24.h>
#include <RF24Network.h>
#include <SPI.h>

// Настройки для NRF24L01
RF24 radio(10, 9); // CE, CSN
RF24Network network(radio);

#define NODE_ID 1  // Уникальный ID узла от 1 до 5 для каждого устройства
#define TARGET_NODE 5 // ID целевого узла, куда будут направляться сообщения
#define SENDER 1

const uint16_t targetNode = TARGET_NODE;

// Настройка пина для светодиода
const int ledPin = 2;

struct Payload {
  uint16_t from;      // ID отправителя
  uint16_t to;        // ID получателя
  char message[32];   // Сообщение
};

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);

  radio.begin();
  network.begin(NODE_ID);
  radio.setPALevel(RF24_PA_LOW);

  radio.setChannel(90);
  radio.setDataRate(RF24_250KBPS);

  radio.startListening();

  Serial.print("Устройство с NODE_ID ");
  Serial.println(NODE_ID);
}

void loop() {
  network.update();
  
  // Отправка сообщения
  if (NODE_ID == SENDER) {
    radio.stopListening(); 
    
    Payload payload = { NODE_ID, targetNode, "Hello from Node 1"};
    RF24NetworkHeader header(targetNode);

    if (network.write(header, &payload, sizeof(payload))) {
      // Успешная отправка
      digitalWrite(ledPin, HIGH);
      delay(3000); // Светодиод горит 3 секунды
      digitalWrite(ledPin, LOW);
    } else {
      // Ошибка при отправке
      for (int i = 0; i < 10; i++) {
        digitalWrite(ledPin, HIGH);
        delay(300); // Мигает раз в 0.3 секунды
        digitalWrite(ledPin, LOW);
        delay(300);
      }
    }
    radio.startListening();
    delay(5000); // Пауза перед повторной отправкой
  }

  // Прием сообщений
  while (network.available()) {
    Serial.println("Данные доступны для чтения");
    RF24NetworkHeader header;
    Payload payload;
    network.read(header, &payload, sizeof(payload));

    if (payload.to == NODE_ID) {
      Serial.println("Сообщение получено для этого узла.");
      // Получено сообщение для этого узла
      for (int i = 0; i < 5; i++) {
        digitalWrite(ledPin, HIGH);
        delay(1000); // Мигает раз в 1 секунду
        digitalWrite(ledPin, LOW);
        delay(1000);
      }
      Serial.print("Получено сообщение: ");
      Serial.println(payload.message);
    } else {
      // Пересылка сообщения
      RF24NetworkHeader forwardHeader(payload.to);
      radio.stopListening();
      network.write(forwardHeader, &payload, sizeof(payload));
      radio.startListening();
    }
  }

  delay(100);
}