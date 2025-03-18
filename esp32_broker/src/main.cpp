#include <PicoMQTT.h>

#define WIFI_SSID "MAS"
#define WIFI_PASSWORD "abc123123"


class MQTT: public PicoMQTT::Server {
    protected:
        PicoMQTT::ConnectReturnCode auth(const char * client_id, const char * username, const char * password) override {
            // only accept client IDs which are 3 chars or longer
            if (String(client_id).length() < 3) {    // client_id is never NULL
                return PicoMQTT::CRC_IDENTIFIER_REJECTED;
            }


            // only accept connections if username and password are provided
            if (!username || !password) {  // username and password can be NULL
                // no username or password supplied
                return PicoMQTT::CRC_NOT_AUTHORIZED;
            }


            // accept two user/password combinations
            if (
                ((String(username) == "thang") && (String(password) == "123456"))
                || ((String(username) == "bob") && (String(password) == "password"))) {
                return PicoMQTT::CRC_ACCEPTED;
            }


            // reject all other credentials
            return PicoMQTT::CRC_BAD_USERNAME_OR_PASSWORD;
        }
} mqtt;


void setup() {
  // Setup serial
  Serial.begin(115200);

  // Connect to WiFi
  Serial.printf("Connecting to WiFi %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(1000); }
  Serial.printf("WiFi connected, IP: %s\n", WiFi.localIP().toString().c_str());

  // Subscribe to a topic pattern and attach a callback
  mqtt.subscribe("#", [](const char* topic, const char* payload) {
    Serial.printf("Received message in topic '%s': %s\n", topic, payload);
  });
  mqtt.begin();
}


void loop() {
  mqtt.loop();
  if (random(1000) == 0){
    mqtt.publish("picomqtt/welcome", "Hello from PicoMQTT!");
  }
}