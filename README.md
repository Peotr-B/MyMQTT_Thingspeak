# Scriptor42
 11сен25
Подключил подписку. В основном для создания скетча для управления реле.

12авг25
ВНИМАНИЕ!
Библиотека EspMQTTClient не поддерживает работу с шифрованием, 
защита поддерживается только в виде авторизации. [github.com]
Плюс мало информации и примеров.
ПРИНИМАЮ РЕШЕНИЕ: Отувз от библиотеки EspMQTTClient!
Перехожу на использование библиотеки PubSubClient

 11авг25
Задал вопрос DeepSeek:
Покажи, как подписаться на топик по протоколу MQTT в ThinkSpeak
Имеется публикация вида:
String MY_TOPIC = "channels/" + String(CHANNEL_ID) + "/publish";
String payload = "field1=" + String(temperature) + "&field2=" + String(humidity) + "&field3=" + String(pressure) + "&field4=" + String(t) + "&field5=" + r1 + "&field6=" + r2;
mqttClient.publish(MY_TOPIC, payload);
Нужно создать подписку на поля field1=String(temperature), field5=r1 и field6=r2
вида
mqttClient.subscribe();

DeepSeek:
Подписка на топики MQTT в ThingSpeak
Для подписки на топики в ThingSpeak по протоколу MQTT вам нужно использовать другой топик, отличный от топика публикации. В ThingSpeak подписка работает через топики вида channels/<CHANNEL_ID>/subscribe/fields/<FIELD>/<API_KEY>.

Вот как можно подписаться на нужные поля:

cpp
String subscribeTopic1 = "channels/" + String(CHANNEL_ID) + "/subscribe/fields/field1/" + String(API_KEY);
String subscribeTopic5 = "channels/" + String(CHANNEL_ID) + "/subscribe/fields/field5/" + String(API_KEY);
String subscribeTopic6 = "channels/" + String(CHANNEL_ID) + "/subscribe/fields/field6/" + String(API_KEY);

mqttClient.subscribe(subscribeTopic1.c_str());
mqttClient.subscribe(subscribeTopic5.c_str());
mqttClient.subscribe(subscribeTopic6.c_str());
Где:

CHANNEL_ID - ID вашего канала ThingSpeak

API_KEY - ваш API ключ для чтения (Read API Key)

Если вы хотите подписаться на все поля одновременно, можно использовать:

cpp
String subscribeAll = "channels/" + String(CHANNEL_ID) + "/subscribe/json/" + String(API_KEY);
mqttClient.subscribe(subscribeAll.c_str());
Не забудьте также реализовать callback-функцию для обработки входящих сообщений:

cpp
void callback(char* topic, byte* payload, unsigned int length) {
  // Обработка полученных данных
}

// В setup():
mqttClient.setCallback(callback);
Примечание: Убедитесь, что ваш API ключ имеет права на чтение (Read Access), и что вы используете правильный MQTT-брокер ThingSpeak (mqtt3.thingspeak.com).
