#include <WiFi.h>
#include <PubSubClient.h>
#include <PZEM004Tv30.h>

// Pines UART2
#define PZEM_RX 16  // GPIO16 ← TX del PZEM
#define PZEM_TX 17  // GPIO17 → RX del PZEM

// Crear instancia del PZEM por UART2
PZEM004Tv30 pzem(Serial2, PZEM_RX, PZEM_TX);

// WiFi
const char* ssid = "MEGACABLE-2.4G-2AA6";
const char* password = "password";

// MQTT
const char* mqtt_server = "192.168.100.91";
const int mqtt_port = 1883;
const char* mqtt_user = "admin";
const char* mqtt_pass = "public";

WiFiClient espClient;
PubSubClient client(espClient);

void reconnectMQTT() {
  while (!client.connected()) {
    if (client.connect("ESP32PZEM", mqtt_user, mqtt_pass)) {
      Serial.println("MQTT conectado.");
    } else {
      Serial.print("Fallo MQTT: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(9600);

  // Conectar WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi conectado.");

  client.setServer(mqtt_server, mqtt_port);
  Serial.print("Dirección del PZEM detectada: 0x");
  Serial.println(pzem.readAddress(), HEX);

}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  // Leer datos del PZEM
  float voltaje = pzem.voltage();
  float corriente = pzem.current();
  float potencia = pzem.power();
  float energia = pzem.energy();

  Serial.println("---- DATOS PZEM ENVIADOS ----");

  if (!isnan(voltaje)) {
    Serial.printf("Voltaje: %.2f V\n", voltaje);
    client.publish("energia/voltaje", String(voltaje, 2).c_str());
  } else {
    Serial.println("Voltaje: ERROR");
  }

  if (!isnan(corriente)) {
    Serial.printf("Corriente: %.2f A\n", corriente);
    client.publish("energia/corriente", String(corriente, 2).c_str());
  } else {
    Serial.println("Corriente: ERROR");
  }

  if (!isnan(potencia)) {
    Serial.printf("Potencia: %.2f W\n", potencia);
    client.publish("energia/potencia", String(potencia, 2).c_str());
  } else {
    Serial.println("Potencia: ERROR");
  }

  if (!isnan(energia)) {
    Serial.printf("Energía acumulada: %.4f kWh\n", energia);
    client.publish("energia/kwh", String(energia, 4).c_str());
  } else {
    Serial.println("Energía: ERROR");
  }

  Serial.println("------------------------------\n");

  delay(5000); // Cada 5 segundos
}