# RF24 Mesh Network
Этот проект реализует mesh-сеть из 5 узлов с использованием библиотеки RF24Mesh для обмена данными через радиомодули NRF24L01. Главный узел (устройство 1) периодически отправляет данные на все остальные узлы,
которые могут их перенаправлять дальше для гарантированной доставки в сеть. На каждом узле реализовано светодиодное оповещение об успешной отправке, а также о получении сообщения.


## Зависимости
Для работы кода необходимо установить следующие библиотеки:

- [RF24](https://github.com/nRF24/RF24): библиотека для работы с радиомодулями NRF24L01.
- [RF24Network](https://github.com/nRF24/RF24Network): надстройка для создания сети на основе радиомодулей NRF24L01.
- [RF24Mesh](https://github.com/nRF24/RF24Mesh): надстройка для автоматической конфигурации сети (mesh) на основе RF24Network.

Для установки библиотек:

1. Откройте Arduino IDE.
2. Перейдите в Sketch > Include Library > Manage Libraries.
3. Введите название библиотеки и установите ее.


## Список используемых компонентов
1. Кабель с разъемом JST 20 см. (M+F) (папа + мама) 5 пар. **Количество:** 2
2. Набор силиконовых многожильных проводов 24AWG 5 цветов по 10 метров. **Количество:** 1
3. Повышающий DC-DC преобразователь из 3.7В в 5В/8В/9В/12В (CKCS BS01) (5 штук). **Количество:** 1
4. Аккумуляторная батарейка, 3.7В, 2500 мАч. **Количество:** 5
5. Контроллер Arduino NANO V3.0 TYPE-C USB (CH340)/Плата Ардуино Нано V3.0, пины для пайки. **Количество:** 5
6. Радио модуль NRF24L01+PA+LNA 1100м. **Количество:** 5
7. Напечатанный на 3D принтере корпус. **Количество:** 5


## Схема подключения
![Схема подключения](/wiring_diagram.jpg "Схема подключения")


## Использование

### Описание проекта
В проекте участвуют 5 устройств, работающих в сети mesh. Каждое устройство имеет уникальный ID узла и выполняет свои функции в зависимости от роли:

- Устройство 1 (Главный узел):
  - Периодически отправляет данные на все узлы в сети.
  - Использует светодиод для сигнализации об успешной отправке (горит 200 мс) и о неуспешной (светодиод не загорается).
- Устройства 2-5 (Релейные узлы):
  - Получают сообщения, проверяют адрес назначения и перенаправляют их другим узлам при необходимости.
  - Используют светодиод для сигнализации при получении сообщения (горит 200 мс).
  - Каждый узел отправляет тестовое сообщение раз в секунду, если в сети нет других активных сообщений.
 
### Функции кода
1. **Отправка данных от главного узла (`sendMessages()`).** Главный узел отправляет данные на все узлы сети, проверяя успешность передачи:
   ```
   // Цикл для отправки данных на все узлы в сети, кроме самого себя
   for (int i = 0; i < mesh.addrListTop; i++) {
     if (mesh.addrList[i].nodeID != 0) {
       RF24NetworkHeader header(mesh.addrList[i].address, 'M');
       bool success = network.write(header, &payload, sizeof(payload));

       digitalWrite(ledPin, success ? HIGH : LOW);
        
       Serial.println(success ? F("Send OK") : F("Send Fail"));
     }
   }
   ```
   Если передача успешна, светодиод на главном узле загорается на короткое время.

2. **Обработка входящих сообщений в главном узле (`receiveMessagesMaster()`).** Функция помогает главному узлу получать и обрабатывать сообщения, поступающие от других узлов в сети, чтобы отслеживать активность и корректность передачи данных в сети.
   ```
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
   ```

3. **Отправка сообщений от релейных узлов (`sendScheduledMessage()`).** Функция используется для того, чтобы релейные узлы периодически отправляли сообщения в сеть. Это позволяет проверять их активность и корректность подключения к сети, а также помогает поддерживать mesh-сеть активной, так как каждый узел участвует в обмене данными.
   ```
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
   ```

4. **Обработка входящих сообщений в релейных узлах (`receiveMessages()`).** Функция в релейных узлах отвечает за получение сообщений от других узлов в сети, обработку и сигнализацию о получении, а также за перенаправление сообщения другим узлам. Это помогает обеспечивать бесперебойную передачу данных между узлами и поддерживать mesh-сеть активной и связной.
   ```
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
   ```

5. **Перенаправление сообщений (`forwardMessage(const payload_t &payload)`).** Функция обеспечивает надежную передачу данных по сети, распространяя полученные сообщения между узлами. Это помогает гарантировать, что данные достигнут узлов, которые находятся вне зоны прямой связи с передающим узлом, поддерживая связность и надежность mesh-сети.
   ```
   if (mesh.addrList[i].nodeID != 0 && mesh.addrList[i].nodeID != nodeID) {
     RF24NetworkHeader newHeader(mesh.addrList[i].address, 'M');
     network.write(newHeader, &payload, sizeof(payload));

     // Выводим информацию о пересылке
     Serial.print("Forwarded packet #");
     Serial.print(payload.counter);
     Serial.print(" to node ");
     Serial.println(mesh.addrList[i].nodeID);
   }
   ```

6. Сигнализация через светодиод:
   - При получении сообщения светодиод загорается на короткое время.
   - При успешной отправке светодиод загорается на 200 мс.
   - При неудачной отправке светодиод не загорается.

7. **Обработка подключения (`handleConnectionFailure()`).** В случае потери связи с сетью узлы 2-5 пытаются обновить адрес и повторно подключиться.
   ```
   if (radio.isChipConnected()) {  // Проверка наличия радиомодуля
     do {
       Serial.println(F("Could not connect to network.\nConnecting to the mesh..."));
     } while (mesh.renewAddress() == MESH_DEFAULT_ADDRESS); // Пробуем обновить адрес
   } else {
     Serial.println(F("Radio hardware not responding."));
     while (1) {} // Бесконечный цикл при отсутствии радиомодуля
   }
   ```

### Начало работы
1. Настройка пинов и ID: В каждой программе необходимо правильно настроить CE и CSN для модуля NRF24L01, пин для светодиода, а также ID узла:
   ```
   RF24 radio(10, 9);  // CE и CSN пины
   const int ledPin = 2;  // Пин для светодиода
   const uint8_t nodeID = 2;  // Пример ID для устройства 2
   ```

2. Компиляция и загрузка кода:
   - Скопируйте и вставьте код для главного узла (device_master.ino) и загрузите на одно из устройств.
   - Скопируйте и вставьте код для релейных узлов (deviceXXX.ino) и загрузите на остальные устройства, изменив `nodeID` на уникальный для каждого устройства (от 2 до 5).

3. Запуск и проверка:
   - После загрузки кода устройства будут автоматически подключаться к сети и обмениваться сообщениями.
   - Проверяйте успешность передачи данных по сигналам на светодиодах и выводу в Serial Monitor.
