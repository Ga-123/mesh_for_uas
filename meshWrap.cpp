#include "meshWrap.h"
#include <SPI.h>


//Настройка CE и CSN
RF24 radio(10, 9);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

const int retryDelay = 200;  // Задержка для повторной отправки сообщений
static int ledPin;


void setupMeshMaster(int pin) {
  ledPin = pin;
  pinMode(ledPin, OUTPUT);

  mesh.setNodeID(0);  // Устанавливаем ID узла (0 - главный узел)
  Serial.println(mesh.getNodeID()); 

  if (!mesh.begin()) {
    Serial.println(F("Radio hardware not responding."));
    while (1) {}
  }
}

void updateMeshMaster() {
  mesh.update();
  mesh.DHCP();
}

// Обработка входящих сообщений
void receiveMessagesMaster() {
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


void setupMesh(uint8_t nodeID, int pin) {
  ledPin = pin;
  pinMode(ledPin, OUTPUT);

  mesh.setNodeID(nodeID);
  radio.begin();
  radio.setPALevel(RF24_PA_MIN);

  Serial.println(F("Connecting to the mesh..."));
  if (!mesh.begin()) {          // Инициализация сети
    handleConnectionFailure();  // Если не удается подключиться, выполняем попытку восстановления
  }
}

void updateMesh() {
  mesh.update();
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
