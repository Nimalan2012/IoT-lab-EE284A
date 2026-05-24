#include <WiFi.h>
#include <PubSubClient.h>
#include "esp_wifi.h"
#include "freertos/queue.h"

// Use phone hotspot to connect to wifi
const char* ssid = "Nimalan iphone";           
const char* password = "Nimz1307";     

// public broker settings
const char* mqtt_server = "broker.emqx.io"; 
const char* mqtt_topic = "ee284a/csi/data"; 

#define WINDOW_SIZE 50
#define FALL_THRESHOLD 150.0 
#define COOLDOWN_PACKETS 150 

float window_buffer[WINDOW_SIZE];
int buffer_index = 0;
bool buffer_full = false;
int cooldown_counter = 0;

typedef struct {
    float variance;
    bool fall_detected;
} mqtt_msg_t;

QueueHandle_t mqtt_queue;
WiFiClient espClient;
PubSubClient client(espClient);

// calculate variance which would determine if there is sudden movements in the environment
float calculate_variance() {
    float sum = 0.0, mean = 0.0, variance_sum = 0.0;
    for (int i = 0; i < WINDOW_SIZE; i++) sum += window_buffer[i];
    mean = sum / WINDOW_SIZE;
    for (int i = 0; i < WINDOW_SIZE; i++) {
        float diff = window_buffer[i] - mean;
        variance_sum += (diff * diff);
    }
    return variance_sum / WINDOW_SIZE;
}

// CSI callback
void wifi_csi_rx_cb(void *ctx, wifi_csi_info_t *info) {
    if (!info || !info->buf) return;

    int8_t *csi_data = (int8_t *)info->buf;
    float packet_sum = 0.0;
    int valid_subcarriers = 0;

    for (int i = 6; i < 58; i++) {
        if (i == 32) continue; 
        int8_t imag = csi_data[i * 2];
        int8_t real = csi_data[(i * 2) + 1];
        float amplitude = sqrtf((float)(imag * imag + real * real));
        packet_sum += amplitude;
        valid_subcarriers++;
    }

    window_buffer[buffer_index] = packet_sum / valid_subcarriers;
    buffer_index++;
    if (buffer_index >= WINDOW_SIZE) {
        buffer_index = 0;
        buffer_full = true;
    }

    if (cooldown_counter > 0) {
        cooldown_counter--;
        return;
    }

    if (buffer_full) {
        float current_variance = calculate_variance();
        bool fall_detected = (current_variance > FALL_THRESHOLD);

        static int publish_throttle = 0;
        publish_throttle++;

        if (fall_detected || publish_throttle > 10) {
            publish_throttle = 0;
            mqtt_msg_t msg;
            msg.variance = current_variance;
            msg.fall_detected = fall_detected;
            xQueueSend(mqtt_queue, &msg, 0);

            if (fall_detected) cooldown_counter = COOLDOWN_PACKETS;
        }
    }
}

// Reconnect to Public MQTT
void mqttReconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Create a random Client ID
        String clientId = "ESP32Client-";
        clientId += String(random(0xffff), HEX);
        
        // Connect WITHOUT password
        if (client.connect(clientId.c_str())) {
            Serial.println("Connected to EMQX!");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    mqtt_queue = xQueueCreate(10, sizeof(mqtt_msg_t));

    WiFi.mode(WIFI_STA);
    WiFi.disconnect(); // Clear any old saved connections
    delay(100);

    // Connect to WiFi using Arduino
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { 
        delay(500); 
        Serial.print("."); 
    }
    Serial.println("\nRx Node WiFi connected.");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP()); 

    // set MQTT server
    client.setServer(mqtt_server, 1883);

    // Configure and enable CSI
    wifi_csi_config_t csi_config = {
        .lltf_en = true,
        .htltf_en = true,
        .stbc_htltf2_en = true,
        .ltf_merge_en = true,
        .channel_filter_en = true,
        .manu_scale = false,
        .shift = false
    };

    // Apply CSI settings onto existing WiFi connection
    esp_wifi_set_csi_config(&csi_config);
    esp_wifi_set_csi_rx_cb(&wifi_csi_rx_cb, NULL);
    esp_wifi_set_csi(true);

    Serial.println("CSI Enabled! Listening for Tx pings...");
}

void loop() {
    if (!client.connected()) mqttReconnect();
    client.loop(); 

    mqtt_msg_t received_msg;
    if (xQueueReceive(mqtt_queue, &received_msg, 0)) {
        char jsonPayload[100];
        snprintf(jsonPayload, 100, "{\"variance\": %.2f, \"fall_detected\": %s}", 
                 received_msg.variance, received_msg.fall_detected ? "true" : "false");
        
        client.publish(mqtt_topic, jsonPayload);

        if (received_msg.fall_detected) Serial.print("FALL DETECTED! ");
        Serial.println(jsonPayload);
    }
}