#include <WiFi.h>
#include <esp_now.h>

// Receiver MAC address
uint8_t receiverMAC[] = {0xf0, 0xf5, 0xbd, 0x05, 0xce, 0x60};

// Pin definitions
#define BTN_LEFT 3
#define BTN_UP 4
#define BTN_DOWN 5
#define BTN_RIGHT 6

struct PadState {
  bool left;
  bool right;
  bool up;
  bool down;
};

PadState readPads() {
  PadState pads;
  pads.left  = !digitalRead(BTN_LEFT);
  pads.up    = !digitalRead(BTN_UP);
  pads.down  = !digitalRead(BTN_DOWN);
  pads.right = !digitalRead(BTN_RIGHT);
  return pads;
}

void setup() {
  Serial.begin(115200);

  // Configurar pins dos Pads
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);

  // Iniciar Wifi
  WiFi.mode(WIFI_STA);
  if(esp_now_init() != ESP_OK){
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Conectar
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, receiverMAC, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);
}

void loop() {
  PadState pads = readPads();

  // Enviar estado dos Pads
  esp_err_t result = esp_now_send(receiverMAC, (uint8_t*)&pads, sizeof(pads));
  if(result == ESP_OK){
    Serial.print("Sent: L=");
    Serial.print(pads.left);
    Serial.print(" U=");
    Serial.print(pads.up);
    Serial.print(" D=");
    Serial.print(pads.down);
    Serial.print(" R=");
    Serial.println(pads.right);
  }

  delay(10);
}
