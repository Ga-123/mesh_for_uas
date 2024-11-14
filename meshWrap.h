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
void sendMessage();
void receiveMessagesMaster();

void setupMesh(uint8_t nodeID, int ledPin);
void updateMesh();
void sendScheduledMessage();
void receiveMessages();

#endif // MESH_WRAP_H
