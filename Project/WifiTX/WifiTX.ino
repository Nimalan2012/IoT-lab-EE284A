#include <WiFi.h>
#include <WiFiUdp.h>

const char *ssid = "Nimalan iphone";
const char *password = "Nimz1307";

WiFiUDP udp;

void setup() {
  Serial.begin(115200);
  delay(10);

  // Connect to phone hotspot
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Print the channel to ensure it matches the Rx node's environment
  Serial.print("Operating on Wi-Fi Channel: ");
  Serial.println(WiFi.channel());
  
  Serial.println("Tx Node Ready. Broadcasting UDP Pings...");
}

void loop() {
  // Broadcast packets for CSI extraction
  udp.beginPacket("255.255.255.255", 1234);
  udp.print("PING");
  udp.endPacket();
  
  delay(5); 
}