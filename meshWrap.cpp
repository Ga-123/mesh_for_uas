#include "meshWrap.h"
#include <SPI.h>


//Настройка CE и CSN
RF24 radio(10, 9);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

static int ledPin;
static unsigned long displayTimer = 0;
static unsigned long ctr = 0;
static uint8_t nodeID;


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

void setupMeshMaster(int pin) {
  ledPin = pin;
  pinMode(ledPin, OUTPUT);

  mesh.setNodeID(0);  // Устанавливаем ID узла (0 - главный узел)

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
bool receiveMessagesMaster(RF24NetworkHeader &header, payload_t &payload) {
  if (network.available()) {
    // Чтение данных из сети
    network.read(header, &payload, sizeof(payload));

    return true; // Успешно получили сообщение
  }

  return false; // Сообщений нет
}


// Отправка сообщений на все узлы в сети
void sendMessages(payload_t &payload) {
  if (millis() - displayTimer > 5000) {
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

  mesh.setNodeID(nodeID);  // Уникальный ID релейного узла (для каждого устройства меняем ID)
  radio.begin();
  radio.setPALevel(RF24_PA_MIN);

  Serial.println(F("Connecting to the mesh..."));
  if (!mesh.begin()) {
    handleConnectionFailure();  // Если не удается подключиться, выполняем попытку восстановления
  }
}

void updateMesh() {
  mesh.update();
}

// Отправка сообщения по расписанию
void sendScheduledMessage(payload_t &payload) {
  if (millis() - displayTimer >= 1000) {
    displayTimer = millis();

    // Пытаемся отправить сообщение
    if (!mesh.write(&payload, 'M', sizeof(payload))) {
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
bool receiveMessages(RF24NetworkHeader &header, payload_t &payload) {
  if (network.available()) {
    network.read(header, &payload, sizeof(payload));

    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);

    // Перенаправляем сообщение другим узлам в сети
    forwardMessage(payload);

    return true; // Успешно получили сообщение
  }

  return false; // Сообщений нет
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
