#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>


RF24 radio(10, 9);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

const int ledPin = 2;

const uint8_t nodeID = 2; // change it to 2, 3, 4 or 5

struct payload_t {
  unsigned long ms;
  unsigned long counter;
};

uint32_t displayTimer = 0;


void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  pinMode(ledPin, OUTPUT);

  mesh.setNodeID(nodeID);

  radio.begin();
  radio.setPALevel(RF24_PA_MIN);

  Serial.println(F("Connecting to the mesh..."));
  if (!mesh.begin()) {
    if (radio.isChipConnected()) {
      do {
        Serial.println(F("Could not connect to network.\nConnecting to the mesh..."));
      } while (mesh.renewAddress() == MESH_DEFAULT_ADDRESS);
    } else {
      Serial.println(F("Radio hardware not responding."));
      while (1) {}
    }
  }
}

void loop() {
  mesh.update();

  if (millis() - displayTimer >= 1000) {
    displayTimer = millis();

    if (!mesh.write(&displayTimer, 'M', sizeof(displayTimer))) {
      if (!mesh.checkConnection()) {
        Serial.println("Renewing Address");
        if (mesh.renewAddress() == MESH_DEFAULT_ADDRESS) {
          mesh.begin();
        }
      } else {
        Serial.println("Send fail, Test OK");
      }
    } else {
      Serial.print("Send OK: ");
      Serial.println(displayTimer);
    }
  }

  if (network.available()) {
    RF24NetworkHeader header;
    payload_t payload;
    network.read(header, &payload, sizeof(payload));

    Serial.print("Received packet #");
    Serial.print(payload.counter);
    Serial.print(" at ");
    Serial.println(payload.ms);

    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);

    for (int i = 0; i < mesh.addrListTop; i++) {
      if (mesh.addrList[i].nodeID != 0 && mesh.addrList[i].nodeID != nodeID) {
        RF24NetworkHeader newHeader(mesh.addrList[i].address, 'M');
        network.write(newHeader, &payload, sizeof(payload));

        Serial.print("Forwarded packet #");
        Serial.print(payload.counter);
        Serial.print(" to node ");
        Serial.println(mesh.addrList[i].nodeID);
      }
    }
  }
}
