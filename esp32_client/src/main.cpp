#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "P601";
const char* password = "66668888t6";

// MQTT Broker settings
const char* mqtt_server = "192.168.1.52";
const int mqtt_port = 1883;
const char* mqtt_user = "thang";
const char* mqtt_password = "123456";

// Define pins for devices
struct Device {
    const char* name;
    int pin;
    bool state;
};

// Array of devices
Device devices[] = {
    {"led", 2, false},      // Built-in LED
    {"relay1", 4, false},   // Relay 1
    {"relay2", 5, false},   // Relay 2
    {"relay3", 18, false},  // Relay 3
    {"relay4", 19, false}   // Relay 4
};

const int NUM_DEVICES = sizeof(devices) / sizeof(Device);

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
    delay(10);
    Serial.println("Connecting to WiFi...");
    
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void publishState() {
    StaticJsonDocument<200> doc;
    for (int i = 0; i < NUM_DEVICES; i++) {
        doc[devices[i].name] = devices[i].state;
    }
    
    char buffer[200];
    serializeJson(doc, buffer);
    client.publish("devices/status", buffer);
}

void handleCommand(const char* device, const char* command) {
    for (int i = 0; i < NUM_DEVICES; i++) {
        if (strcmp(devices[i].name, device) == 0) {
            if (strcmp(command, "ON") == 0) {
                digitalWrite(devices[i].pin, HIGH);
                devices[i].state = true;
            } else if (strcmp(command, "OFF") == 0) {
                digitalWrite(devices[i].pin, LOW);
                devices[i].state = false;
            }
            publishState();
            break;
        }
    }
}

void callback(char* topic, byte* payload, unsigned int length) {
    // Create a buffer for the payload
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';
    
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        Serial.println("Failed to parse JSON");
        return;
    }
    
    // Handle both single device and multiple device commands
    if (doc.is<JsonObject>()) {
        for (JsonPair pair : doc.as<JsonObject>()) {
            const char* device = pair.key().c_str();
            const char* command = pair.value().as<const char*>();
            handleCommand(device, command);
        }
    }
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        String clientId = "ESP32Client-" + String(random(0xffff), HEX);
        
        if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
            Serial.println("connected");
            client.subscribe("devices/control");
            // Publish initial state
            publishState();
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    
    // Initialize device pins
    for (int i = 0; i < NUM_DEVICES; i++) {
        pinMode(devices[i].pin, OUTPUT);
        digitalWrite(devices[i].pin, LOW);
    }
    
    setup_wifi();
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
}