#include "RF24Network.h"
#include "RF24.h"
#include "RF24Mesh.h"
#include <SPI.h>


RF24 radio(10, 9);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

const int ledPin = 2;

struct payload_t {
  unsigned long ms;
  unsigned long counter;
};

uint32_t ctr = 0;
uint32_t displayTimer = 0;


void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  pinMode(ledPin, OUTPUT);

  mesh.setNodeID(0);
  Serial.println(mesh.getNodeID());

  if (!mesh.begin()) {
    Serial.println(F("Radio hardware not responding."));
    while (1) {}
  }
}

void loop() {
  mesh.update();
  mesh.DHCP();

  if (network.available()) {
    RF24NetworkHeader header;
    payload_t payload;
    network.read(header, &payload, sizeof(payload));

    Serial.print("Received from node ");
    Serial.print(header.from_node);
    Serial.print(": counter = ");
    Serial.print(payload.counter);
    Serial.print(", ms = ");
    Serial.println(payload.ms);

    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);
  }

  if (millis() - displayTimer > 5000) {
    ctr++;
    payload_t payload = { millis(), ctr };

    for (int i = 0; i < mesh.addrListTop; i++) {
      if (mesh.addrList[i].nodeID != 0) {
        RF24NetworkHeader header(mesh.addrList[i].address, 'M');
        bool success = network.write(header, &payload, sizeof(payload));

        digitalWrite(ledPin, success ? HIGH : LOW);
        delay(200);
        digitalWrite(ledPin, LOW);

        Serial.println(success ? F("Send OK") : F("Send Fail"));
      }
    }
    displayTimer = millis();
  }
}
