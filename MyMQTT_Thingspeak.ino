/*
11сен25
Подключил подписку. В основном для создания скетча для управления реле.

12авг25
ВНИМАНИЕ!
Библиотека EspMQTTClient не поддерживает работу с шифрованием, 
защита поддерживается только в виде авторизации. [github.com]
Плюс мало информации и примеров.
ПРИНИМАЮ РЕШЕНИЕ: Отувз от библиотеки EspMQTTClient!
Перехожу на использование библиотеки PubSubClient

27июл25
Подключаю датчик BME280 [NodeMCU_BME280_ThingSpeaktmp.ino]

https://www.mathworks.com/help/thingspeak/use-arduino-client-to-publish-to-a-channel.html
Publish and Subscribe to a ThingSpeak Channel Using Secure MQTT

https://microkontroller.ru/esp8266-projects/podklyuchenie-nodemcu-esp8266-k-mqtt-brokeru-s-pomoshhyu-arduino-ide/?ysclid=me5l1h4d1d841493685
Подключение NodeMCU ESP8266 к MQTT брокеру с помощью Arduino IDE

https://kotyara12.ru/iot/esp8266mqtt/
Телеметрия на ESP8266 + MQTT. Пошаговое руководство по созданию DIY-проекта с 
удаленным управлением

https://ipc2u.by/articles/2024/rol-mqtt-protokola-i-urovney-qos-v-iiot-klyuchevye-osobennosti-i-preimushchestva/
Роль MQTT протокола и уровней QoS в IIoT: ключевые особенности и преимущества
Поддержка различных уровней качества обслуживания (QoS): Протокол предлагает три уровня QoS, которые позволяют контролировать надёжность доставки сообщений в зависимости от требований приложения:

    QoS 0 (At most once): Сообщение доставляется только один раз, без подтверждения доставки.
    QoS 1 (At least once): Сообщение гарантированно доставляется как минимум один раз, но возможны дублирования.
    QoS 2 (Exactly once): Сообщение доставляется ровно один раз, что исключает вероятность дублирования.
	
QoS пока отложил, можно ещё посмотреть:
https://forum.arduino.ru/t/class-rugaetsya-na-funkcziyu/13169?ysclid=me8g6sug1c58746536

https://aiu.susu.ru/iot/summer/quick_start
Интернет вещей с ESP8266 – быстрый старт

https://www.bizkit.ru/2019/12/08/15499/?ysclid=macfreji7v287326097
Настройка MQTT на ThingSpeak и CloudMQTT для работы с IoT устройствами

15июн25
https://itcomnetworks.github.io/ru/tech/2022/03/25/using-MQTT-protocol-with-Thingspeak.html
IoT-Ep6 | Использование протокола MQTT с Thingspeak
https://github.com/itcomnetworks/IoT-Series
Скетчи

файл <MyMQTT_Thingspeak.ino>
*/
//#include "EspMQTTClient.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "mqtt_secrets.h"
//#include "DHT.h"

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
/*
#define mqttclientID SECRET_MQTT_CLIENT_ID
#define mqttUserName SECRET_MQTT_USERNAME
#define mqttPass SECRET_MQTT_PASSWORD
*/
#define ssid SECRET_WIFI_NAME
#define pass SECRET_WIFI_PASSWORD

#define LED D3
//или
//int LED = D3;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

const char* mqttServer = "mqtt3.thingspeak.com";  // совершенно равнозначно "mqtt.thingspeak.com"
const int mqttPort = 1883;

/*
EspMQTTClient mqttClient(
  SECRET_WIFI_NAME,
  SECRET_WIFI_PASSWORD,
  "mqtt3.thingspeak.com",  // MQTT Broker server ip
  SECRET_MQTT_USERNAME,   // Can be omitted if not needed
  SECRET_MQTT_PASSWORD,   // Can be omitted if not needed
  SECRET_MQTT_CLIENT_ID      // Client name that uniquely identify your device
);
*/
// DHT information
/*
#define DHTPIN D2
#define DHTTYPE DHT22       // or set DHT11 type if you use DHT11
DHT dht(DHTPIN, DHTTYPE);
*/

Adafruit_BME280 bme;
float temperature, humidity, pressure, altitude;

//String TWat = "Повышенная температура = ";
//String TWatZ;

int C;  //Upravlenie znacheniem avarijnoy signalizacii, Жарко
int CN;  //Нормальная температура

String t;	//Температура
int r1 = 0;	//Включение реле обогрева
int r2 = 0;	//Включение реле охлаждения

/*Временные задержки для посылки в ThingSpeak*/
unsigned long lastTime = 0;
unsigned long delayTime = 20000; //интервал равен 20-ти секундам

//char MY_TOPIC[50];  // Буфер для топика
String MY_TOPIC = "channels/" + String(CHANNEL_ID) + "/publish";

void ConnectWifi();
void ConnectMQTT();
void publishData();
void mqttSubscribe();

byte tries = 20;  // Попыткок подключения к точке доступа

void setup()  // => =====================================================
{
Serial.begin(115200);
while (!Serial)	// Рекомендуется для Leonardo
delay(10);  
//=======================================================================
/*Моя стандартная вставка для индикации названия скетча при запуске*/
/* Если не используется в программе
Serial.begin(115200);
while (!Serial)	// Рекомендуется для Leonardo
delay(10);
*/
byte tries = 20;
delay(5000);
//Serial.println("Вставка для индикации названия скетча при запуске: <My_Scetch.ino>");
  Serial.println("Использование протокола MQTT с Thingspeak: <MyMQTT_Thingspeak.ino>");
  while (--tries)
	{
		delay(500);               // Пауза 500 мкс
        Serial.print(".");        // Печать "."
		Serial.print(tries);	  // Печать ожидания пуска программы
	}
  Serial.println();
  Serial.println();
  
//  Serial.end();      // закрыть порт UART, если не используется в программе
//=======================================================================
  
	pinMode(LED, OUTPUT);
    C = 26; //Аварийное значение температуры
	CN = 20; //Нормальная температура
  
    bme.begin(0x76); 
	
	ConnectWifi();
	delay(2000);	//2-секундная задержка

// Configure the MQTT client
  mqttClient.setServer(mqttServer, mqttPort);
  
// Форматируем топик
// Для безопасности используйте snprintf():
//  snprintf(MY_TOPIC, sizeof(MY_TOPIC), "channels/%d/publish", CHANNEL_ID);
  
 /*  
  ВНИМАНИЕ!
  snprintf НЕ РАБОТАЕТ с Thingspeak MQTT (не выяснил, почему)  
 */   
// Установите функцию обработчика сообщений MQTT
  mqttClient.setCallback(mqttSubscriptionCallback);
  /* Установите буфер для обработки возвращаемого JSON. ПРИМЕЧАНИЕ: Переполнение 
буфера сообщений приведет к тому, что ваш обратный вызов не будет вызван */
  mqttClient.setBufferSize( 2048 );

/* -------------------------------------------------- */

Serial.print("ssid: ");
Serial.println(SECRET_WIFI_NAME);

Serial.print("pass: ");
Serial.println(SECRET_WIFI_PASSWORD);

Serial.print("channelID: ");
Serial.println(CHANNEL_ID);

Serial.print("mqttUserName: ");
Serial.println(SECRET_MQTT_USERNAME);

Serial.print("mqttPass: ");
Serial.println(SECRET_MQTT_PASSWORD);

Serial.print("mqttclientID: ");
Serial.println(SECRET_MQTT_CLIENT_ID);

Serial.print("apiKey: ");
Serial.println(apiKey);

Serial.println();
Serial.println();

ConnectMQTT();

//Подписываемся на топики управления реле
//	mqttSubscribe(CHANNEL_ID, apiKey);
	mqttSubscribe(CHANNEL_ID);

delay(10000);
}
/* <= setup ================================================= */
/*
void onConnectionEstablished() {
  Serial.println("MQTT Client is connected to Thingspeak!");
}
*/
void loop()   // => ==========================================
{
	// Reconnect to Wi-Fi
    if(WiFi.status() != WL_CONNECTED)
		ConnectWifi();
	
	// Подключитесь, если клиент MQTT не подключен, и повторно подпишитесь на обновления канала
	if (!mqttClient.connected()) 
	{
     ConnectMQTT();
	 
//	 mqttSubscribe(CHANNEL_ID, apiKey);
	 mqttSubscribe(CHANNEL_ID);
	}
	
	mqttClient.loop();

  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F * 0.75;

 // TWatZ = TWat+temperature;
/*	
	{
      digitalWrite(LED, HIGH);
      unsigned long currentMillis = millis();
	  
	  if (currentMillis - previousMillis >= interval) 
	  {
		  previousMillis = currentMillis;
		  TW = 0;;
		  }

    if(!TW)
	{
    //  sendMessage("Повышенная температура!");
      sendMessage(TWatZ);
      TW = 1;
      }
    }
*/
 
/*	
	else
	{
		Serial.println( "Нормальная температура" );
      digitalWrite(LED, LOW);
      TW = 0;
      }
  Serial.print("TW = ");
  Serial.println(TW);
*/
	if (mqttClient.connected() && ((millis() - lastTime )> delayTime)) 
//	if ((millis() - lastTime) > delayTime)	
  {	
		publishData();
		
		if(temperature > C)
		{
			Serial.print( "Повышенная температура= " );
		    Serial.println(temperature);
		}
		else if(temperature > CN) 
		{
			Serial.print( "Нормальная температура= " );
			Serial.println(temperature);
		}
		else
		{
			Serial.print( "Низкая температура= ");
			Serial.println(temperature);
		}
	/*
	    if(r1==0)	//для проверки,взял из файла MyMQTT_Th_Deep.ino
			r1=1;
		else
			r1=0;
	*/
	    lastTime = millis();
  }
} 
 /* <= loop ========================================================== */

/*Подпрограммы => ==================================================== */

 // Функция обработки сообщений из подписки MQTT
//https://www.mathworks.com/help/thingspeak/use-arduino-client-to-publish-to-a-channel.html
void mqttSubscriptionCallback(char* myTopic, byte* payload, unsigned int length)
{
	// Print the details of the message that was received to the serial monitor.
  Serial.print("\nMessage arrived [");
  Serial.print(myTopic);
  Serial.println("] ");
  
  String message;  
  for (int i = 0; i < length; i++) 
    message += (char)payload[i];
  
 Serial.println("message= "); 
 Serial.println(message);
 Serial.println();
/*
 char *F1 = strstr(myTopic, "field1");
  if(F1)
    {
        // вычисляем позицию подстроки в строке
        long position = F1 - myTopic;
        temperature = myTopic[position].toFloat();
		Serial.print("Temperature: ");
		Serial.println(temperature);
    }
*/
// Преобразуем payload в строку
 // char message[length + 1];
//  memcpy(message, payload, length);
  //о функции memcpy: https://ru.linux-console.net/?p=39263
//  message[length] = '\0';  // Добавляем нуль-терминатор

  // Проверяем, на какое поле пришло сообщение
  if (strstr(myTopic, "field1") != NULL) 
  {
	temperature = message.toFloat();  // Преобразуем строку в float
    Serial.print("Temperature: ");
    Serial.println(temperature);
  }
  
  if (strstr(myTopic, "field5") != NULL) 
  {
	r1 = message.toInt();  // Преобразуем строку в Int
    Serial.print("R1: ");
    Serial.println(r1);
  }
  else if (strstr(myTopic, "field6") != NULL) 
  {
	r2 = message.toInt();  // Преобразуем строку в Int
    Serial.print("R2: ");
    Serial.println(r2);
  }
  
  // Печать подробностей сообщения, которое было получено в серийный монитор
  Serial.print("Message arrived [");
  Serial.print(myTopic);
  Serial.println("] ");

  Serial.print("Включить охлаждение = ");
  Serial.println(r1);
  Serial.print("Включить обогрев = ");
  Serial.println(r2);
 
 if(r1 == 1)
 {
	 digitalWrite(LED, HIGH);
	 Serial.println("Включение охлаждения");
 }
 else if(r2 == 1)
 {
	 Serial.println("Включение обогрева");
	 digitalWrite(LED, LOW);
 }
 else
 {
	 Serial.println("Температура в норме");
	 digitalWrite(LED, LOW);
 }
 /*
  if (message == "on") 
  {
    digitalWrite(LED, HIGH);
  }
  else if (message == "off") 
  {
    digitalWrite(LED, LOW);
  }
 */
  Serial.println();
  Serial.println("-----------------------");  
}

/* ------------------------------------------------------------ */
// Subscribe to ThingSpeak channel for updates.
//void mqttSubscribe(String ChannelID, String API_Key)
void mqttSubscribe(String ChannelID)
{
//String subscribeTermo = "channels/" + String(ChannelID) + "/subscribe";
String subscribeTermo = "channels/" + String(ChannelID) + "/subscribe" + "/fields" + "/field1";
String subscribeZhara = "channels/" + String(ChannelID) + "/subscribe" + "/fields" + "/field5";
String subscribeKholod = "channels/" + String(ChannelID) + "/subscribe" + "/fields" + "/field6";

mqttClient.subscribe(subscribeTermo.c_str());
mqttClient.subscribe(subscribeZhara.c_str());
mqttClient.subscribe(subscribeKholod.c_str());
}

/* ------------------------------------------------------------ */
void publishData() 
{
 // if (mqttClient.isConnected() && (millis() - lastTime > delayTime)) 
 // {
	  
/*	Можно так [esp_mgtt2.ino]:
if (pause == 0)
  {
	  ******; // что-то
	  pause = 3000; // пауза около 3 секунд
  }
pause--;	  
*/	  
    // Check if any reads failed and exit early (to try again).
    if (temperature > C) 
	{
		t = "Zhara!";
		//char* r1 = "ReleCoolON!";
		r1 = 1;
		//char* r2 = "ReleHeatOFF!";
		r2 = 0;
		
		Serial.print(F("Zhara!= "));
		/*
		Serial.println(temperature);
		Serial.print(F("Влажность= "));
		Serial.println(humidity);
		Serial.print(F("Давление= "));
		Serial.println(pressure);
		*/
	}
	//	else if(temperature <= C && temperature > CN)
		else if(temperature > CN)
		{
			t = "T_Norma";
		//	char* r1 = "ReleCoolOFF!";
			r1 = 0;
		//	char* r2 = "ReleHeatOFF!";
			r2 = 0;
			Serial.println(F("T_Norma"));
		}
		else
		{
			t = "Kholod";
		//	char* r1 = "ReleCoolOFF!";
			r1 = 0;
		//	char* r2 = "ReleHeatON!";
			r2 = 1;
			Serial.println(F("Kholod"));
		}

	//	Serial.print(F(Температура= "));
		Serial.println(temperature);
	//	Serial.print(F("Влажность= "));
		Serial.println(humidity);
	//	Serial.print(F("Давление= "));
		Serial.println(pressure);
		
    Serial.println(F("\nPublising data to Thingspeak"));
	
//	Serial.println(t);
//	Serial.println(F(String(t)));
//	Serial.println(String(t));

  String payload="field1=";
  //payload += String(temperature);
  payload+=temperature;	//можно и так, и так
  payload += "&field2=";
  //payload += String(humidity);
  payload+=humidity;
  payload += "&field3=";
  //payload += String(pressure);
  payload += pressure;
  payload += "&field4=";
  payload += t;
  payload += "&field5=";
  payload += r1;
  payload += "&field6=";
  payload += r2;  
  payload += "&status=MQTTPUBLISH";

  if (mqttClient.connected())
  {
    Serial.print("Sending payload: ");
    Serial.println(payload);
  }

 // mqttClient.publish(MY_TOPIC.c_str(), payload.c_str());
  
  if (mqttClient.publish(MY_TOPIC.c_str(), (char*) payload.c_str())) 
    {
	  Serial.println("Publish ok");
	}
    else
	{
      Serial.println("Publish failed");
    }
   // Serial.println("Data published");
	Serial.println();
//    lastTime = millis();
// }// if (mqttClient.isConnected() && (millis() - lastTime > delayTime))
 }
 
/* ------------------------------------------------------------ */
// Publish messages to a ThingSpeak channel.
/*
void mqttPublish(long pubChannelID, String message) 
{
  String topicString ="channels/" + String( pubChannelID ) + "/publish";
  mqttClient.publish( topicString.c_str(), message.c_str() );
}
*/
/* --------------------------------------------------------------------- */
void ConnectWifi()
{   Serial.print("Connecting to ");             // Печать "Подключение к:"
    Serial.println(ssid);                       // Печать "Название Вашей WiFi сети"
	WiFi.mode(WIFI_STA); // nodemcu as station
	WiFi.begin(ssid, pass);                 // Подключение к WiFi Сети
//	Serial.print ("connecting to wifi");
	while (--tries && WiFi.status() != WL_CONNECTED)
	{
		delay(500);                               // Пауза 500 мкс
        Serial.print(".");                        // Печать "."
		Serial.print(tries);
	}
	
	if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println();
    Serial.println("Non Connecting to WiFi..");
    tries=20;
  }
  else
  {
    // Иначе удалось подключиться отправляем сообщение
    // о подключении и выводим адрес IP
    Serial.println("");
    Serial.println("WiFi connected");
   	Serial.print("IP Address:");
	Serial.println(WiFi.localIP());
	Serial.print("MacAddress:");
	Serial.println(WiFi .macAddress());
  }
}
/* ------------------------------------------------------------ */
// подключаемся к MQTT серверу
void ConnectMQTT()
{
	while (!mqttClient.connected()) 
	{
		 Serial.println(F("Attempting MQTT connection..."));
         Serial.print(F("mqttClient ID:"));
         Serial.println(SECRET_MQTT_CLIENT_ID);
         Serial.print(F("mqtt_login "));
         Serial.println(SECRET_MQTT_USERNAME);
         Serial.print(F("mqtt_pass "));
         Serial.println(SECRET_MQTT_PASSWORD);
         Serial.println();

		// Connect to the MQTT broker
		Serial.println("Connecting to MQTT...");		
		if (mqttClient.connect(SECRET_MQTT_CLIENT_ID, SECRET_MQTT_USERNAME, SECRET_MQTT_PASSWORD )) 
		{
			Serial.println("connected");  
	  // Или так (см. https://www.mathworks.com/help/thingspeak/mqtt-publish-and-subscribe-with-esp8266.html): 
	     //   Serial.println( "Connected with Client ID:  " + String( SECRET_MQTT_CLIENT_ID ) + " User "+ String( SECRET_MQTT_USERNAME ) + " Pwd "+String( SECRET_MQTT_PASSWORD ) );
			Serial.println("Connected with Client ID:  " + String( SECRET_MQTT_CLIENT_ID ));
			Serial.println(" User "+ String( SECRET_MQTT_USERNAME ));
			Serial.println(" Pwd "+String( SECRET_MQTT_PASSWORD ));
			Serial.println();

 //Или так (см. https://www.mathworks.com/help/thingspeak/use-arduino-client-to-publish-to-a-channel.html):
 
			 Serial.print( "MQTT to " );
			 Serial.println( mqttServer );
			 Serial.print (" at port ");
			 Serial.println( mqttPort );
			 Serial.println( " successful." );
        } 
		else 
		{
			Serial.print("failed with state ");
			Serial.print(mqttClient.state());
			Serial.println( " Попробуй ещё раз через несколько секунд" );
			delay(2000);
	  
	  // Или так (см. https://www.mathworks.com/help/thingspeak/mqtt-publish-and-subscribe-with-esp8266.html): 
	        Serial.print( "failed, rc = " );
      // See https://pubsubclient.knolleary.net/api.html#state for the failure code explanation.
            Serial.print(mqttClient.state() );
            Serial.println( " Will try again in 5 seconds" );
            delay( 5000 );
        }
    }  
}
/* <= Подпрограммы ================================================ */
 