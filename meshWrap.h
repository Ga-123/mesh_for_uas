#ifndef MESH_WRAP_H
#define MESH_WRAP_H

#include <RF24Network.h>
#include <RF24.h>
#include <RF24Mesh.h>

struct payload_t {
  unsigned long ms;
  unsigned long counter;
};

// Объявления функций
void setupMeshMaster(int ledPin);
void updateMeshMaster();
void sendMessages();
void receiveMessagesMaster();

void setupMesh(uint8_t nodeID, int ledPin);
void updateMesh();
void sendScheduledMessage();
void receiveMessages();
void handleConnectionFailure();
void forwardMessage(const payload_t &payload);

#endif // MESH_WRAP_H
