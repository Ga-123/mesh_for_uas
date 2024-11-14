#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>


//Настройка CE и CSN
RF24 radio(10, 9);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

const int ledPin = 2;        // Пин для светодиода

const uint8_t nodeID = 2;    // ID текущего узла (можно изменить на 2, 3, 4 или 5)

struct payload_t {
  unsigned long ms;
  unsigned long counter;
};

uint32_t displayTimer = 0;


// Функция обработки подключения при неудаче
void handleConnectionFailure() {
  if (radio.isChipConnected()) {  // Проверка наличия радиомодуля
    do {
      Serial.println(F("Could not connect to network.\nConnecting to the mesh..."));
    } while (mesh.renewAddress() == MESH_DEFAULT_ADDRESS); // Пробуем обновить адрес
  } else {
    Serial.println(F("Radio hardware not responding."));
    while (1) {} // Бесконечный цикл при отсутствии радиомодуля
  }
}

// Отправка сообщения по расписанию
void sendScheduledMessage() {
  if (millis() - displayTimer >= 1000) {
    displayTimer = millis();

    // Пытаемся отправить сообщение
    if (!mesh.write(&displayTimer, 'M', sizeof(displayTimer))) {
      // Проверка подключения при неудаче отправки
      if (!mesh.checkConnection()) {
        Serial.println("Renewing Address");
        if (mesh.renewAddress() == MESH_DEFAULT_ADDRESS) {
          mesh.begin(); // Переинициализация сети
        }
      } else {
        Serial.println("Send fail, Test OK");
      }
    } else {
      Serial.print("Send OK: ");
      Serial.println(displayTimer);
    }
  }
}

// Обработка полученных сообщений
void receiveMessages() {
  if (network.available()) {
    RF24NetworkHeader header;
    payload_t payload;
    network.read(header, &payload, sizeof(payload));

    // Вывод информации о полученных данных
    Serial.print("Received packet #");
    Serial.print(payload.counter);
    Serial.print(" at ");
    Serial.println(payload.ms);

    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);

    // Перенаправляем сообщение другим узлам в сети
    forwardMessage(payload);
  }
}

// Перенаправление полученного сообщения другим узлам
void forwardMessage(const payload_t &payload) {
  for (int i = 0; i < mesh.addrListTop; i++) {
    if (mesh.addrList[i].nodeID != 0 && mesh.addrList[i].nodeID != nodeID) {
      RF24NetworkHeader newHeader(mesh.addrList[i].address, 'M');
      network.write(newHeader, &payload, sizeof(payload));

      // Выводим информацию о пересылке
      Serial.print("Forwarded packet #");
      Serial.print(payload.counter);
      Serial.print(" to node ");
      Serial.println(mesh.addrList[i].nodeID);
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  pinMode(ledPin, OUTPUT);

  mesh.setNodeID(nodeID);     // Устанавливаем ID узла

  radio.begin();
  radio.setPALevel(RF24_PA_MIN); // Устанавливаем минимальный уровень мощности для экономии энергии

  Serial.println(F("Connecting to the mesh..."));
  if (!mesh.begin()) {        // Инициализация сети
    handleConnectionFailure(); // Если не удается подключиться, выполняем попытку восстановления
  }
}

void loop() {
  mesh.update();        // Обновляем состояние сети

  sendScheduledMessage(); // Отправка сообщения по расписанию
  receiveMessages();      // Обработка полученных сообщений
}
