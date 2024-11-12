#include <RF24.h>
#include <SPI.h>

RF24 radio(10, 9); // CE, CSN
const byte address[6] = "00001";

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(90);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(address);
  radio.stopListening();
}

void loop() {
  const char text[] = "Hello from Node 1";
  bool success = radio.write(&text, sizeof(text));

  if (success) {
    Serial.println("Сообщение успешно отправлено.");
  } else {
    Serial.println("Ошибка при отправке.");
  }
  delay(1000);
}
