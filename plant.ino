#include <ESP8266WiFi.h>
#include <Adafruit_MQTT_Client.h>
#include <DHT.h>

/************************* WiFi Credentials *************************/
#define WIFI_SSID "Redmi"
#define WIFI_PASSWORD "12345678"

/************************* Adafruit IO Setup *************************/
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME "Adarshdk"
#define AIO_KEY "aio_PdWr225aHeyMdF7rT0TB7OeWPlD6"

/************************* WiFi and MQTT Client *************************/
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Publish waterlevel = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/waterlevel");
Adafruit_MQTT_Subscribe relayControl = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/relay-control");


/************************* DHT Sensor Setup *************************/
#define DHT_PIN 2  // Pin connected to the DHT sensor
#define DHT_TYPE DHT11  // Change to DHT22 if you're using a DHT22 sensor
DHT dht(DHT_PIN, DHT_TYPE);

/************************* WiFi Connection Setup *************************/
/************************* Relay Setup *************************/
#define RELAY_PIN 4 // Pin connected to the relay
bool relayState = false;

void setupWiFi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

/************************* MQTT Connection Setup *************************/
void setupMQTT() {
  mqtt.connect();

  if (!mqtt.connected()) {
    Serial.println("Failed to connect to Adafruit IO");
    while (1);
  }

  Serial.println("Connected to Adafruit IO");
  mqtt.subscribe(&relayControl);
}

void toggleRelay(bool state) {
  relayState = state;
  digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
  Serial.print("Relay State: ");
  Serial.println(relayState ? "ON" : "OFF");
}

/************************* MQTT Callback Function *************************/
void MQTTcallback(char* topic, byte* payload, unsigned int length) {
  // Convert the payload to a string
  String payloadString = "";
  for (int i = 0; i < length; i++) {
    payloadString += (char)payload[i];
  }

  // Check if the relay control feed received a message
  if (String(topic) == AIO_USERNAME "/feeds/relay-control") {
    if (payloadString == "ON") {
      toggleRelay(true);
    } else if (payloadString == "OFF") {
      toggleRelay(false);
    }
  }
}

/************************* Main Setup *************************/
void setup() {
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);

  setupWiFi();
  setupMQTT();

  dht.begin();
}

/************************* Main Loop *************************/
void loop() {
  // Read temperature and humidity from DHT sensor
  float temperatureValue = dht.readTemperature();
  float humidityValue = dht.readHumidity();
  int water = digitalRead(0);
//  Serial.println(water);

  // Publish temperature and humidity values to Adafruit IO
  temperature.publish(temperatureValue);
  humidity.publish(humidityValue);
  waterlevel.publish(water);

  // Process MQTT packets and handle incoming messages
  mqtt.processPackets(1000);

  // Check if the toggle switch is ON
  if (relayState) {
    // Perform actions when relay is ON
    // ...
  }
  


  // Other code or logic here
}
