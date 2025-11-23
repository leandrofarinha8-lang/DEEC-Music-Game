#include <esp_now.h>
#include <WiFi.h>

struct PadState {
  bool left;
  bool right;
  bool up;
  bool down;
};

PadState pads;

void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  memcpy(&pads, data, sizeof(pads));

  Serial.print("L:");
  Serial.print(pads.left);
  Serial.print("  R:");
  Serial.print(pads.right);
  Serial.print("  U:");
  Serial.print(pads.up);
  Serial.print("  D:");
  Serial.println(pads.down);
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_recv_cb(onReceive);
}

void loop() {}
