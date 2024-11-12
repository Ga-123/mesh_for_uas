#include <RF24.h>
#include <RF24Network.h>
#include <SPI.h>

// Настройки для NRF24L01
RF24 radio(10, 9); // CE, CSN
RF24Network network(radio);

#define NODE_ID 5        // Уникальный ID узла от 1 до 5 для каждого устройства
#define TARGET_NODE 5    // ID целевого узла, куда будут направляться сообщения
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
  delay(100);
  radio.begin();
  network.begin(NODE_ID);  // Установка сетевого уровня и ID узла
  radio.setPALevel(RF24_PA_LOW);

  // Настройки канала и скорости передачи
  radio.setChannel(90);
  radio.setDataRate(RF24_250KBPS);

  // Включаем режим прослушивания
  radio.startListening();

  Serial.print("Устройство с NODE_ID ");
  Serial.println(NODE_ID);
  delay(2000);
}

void loop() {
  network.update();   // Обновление сети, получение сообщений

  // Отправка сообщения с узла-отправителя
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

    // Возвращаемся в режим приема после отправки
    radio.startListening();
    delay(5000);  // Пауза перед повторной отправкой
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
      radio.stopListening();  // Отключаем прием для пересылки
      network.write(forwardHeader, &payload, sizeof(payload));
      radio.startListening();  // Включаем прием обратно
    }
  }

  delay(100);  // Задержка для стабилизации работы сети
}
