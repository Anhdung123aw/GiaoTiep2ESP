#include <WiFi.h>
#include <esp_now.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// MAC của ESP32 nhận
uint8_t peerMac[] = {0xEC, 0xE3, 0x34, 0x14, 0x79, 0x84};

struct Payload {
  float temp;
  float humi;
};

void onSent(const uint8_t*, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sent OK" : "Sent FAIL");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  dht.begin();

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_send_cb(onSent);

  esp_now_peer_info_t peer{};
  memcpy(peer.peer_addr, peerMac, 6);
  peer.channel = 0;
  peer.encrypt = false;
  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("DHT read failed!");
    delay(2000);
    return;
  }

  Payload p{t, h};
  esp_err_t result = esp_now_send(peerMac, (uint8_t*)&p, sizeof(p));

  if (result == ESP_OK) {
    Serial.printf("Sent -> Temp: %.1f °C, Humi: %.1f %%\n", t, h);
  } else {
    Serial.printf("Error sending data: %d\n", result);
  }

  delay(3000);
}
