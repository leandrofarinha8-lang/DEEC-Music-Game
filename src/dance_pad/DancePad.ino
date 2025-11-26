#include <WiFi.h>
#include <esp_now.h>

// Receiver MAC address (replace with your receiver ESP MAC)
uint8_t receiverMAC[] = {0xf0, 0xf5, 0xbd, 0x05, 0xce, 0x60};

// Pin definitions
#define BTN_LEFT 3
#define BTN_UP 4
#define BTN_DOWN 5
#define BTN_RIGHT 6

enum {
  LEFT,
  UP,
  DOWN,
  RIGHT
};

// Struct for sending pad state
struct PadState {
  bool left;
  bool right;
  bool up;
  bool down;
};

unsigned long lastPressTime[4] = {0, 0, 0, 0};
bool lastState[4] = {LOW, LOW, LOW, LOW};

bool debouncedInput(int type){
  int btn;
  int index;

  switch(type){
    case LEFT:  btn = BTN_LEFT;  index = 0; break;
    case UP:    btn = BTN_UP;    index = 1; break;
    case DOWN:  btn = BTN_DOWN;  index = 2; break;
    case RIGHT: btn = BTN_RIGHT; index = 3; break;
  }

  bool currentState = digitalRead(btn);
  unsigned long currentTime = millis();

  if(lastState[index] != currentState && (currentTime - lastPressTime[index]) > 50){ //Manda um sinal ENQUANTO PRESSIONADO
      lastPressTime[index] = currentTime;
      lastState[index] = currentState;
      return true;
  }

  lastState[index] = currentState;
  return false;
}

// Read all pads and return state
PadState readPads() {
  PadState pads;
  pads.left  = debouncedInput(LEFT);
  pads.up    = debouncedInput(UP);
  pads.down  = debouncedInput(DOWN);
  pads.right = debouncedInput(RIGHT);
  return pads;
}

void setup() {
  Serial.begin(115200);

  // Configure pins
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);

  // Initialize WiFi and ESP-NOW
  WiFi.mode(WIFI_STA);
  if(esp_now_init() != ESP_OK){
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Add peer
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, receiverMAC, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);
}

void loop() {
  PadState pads = readPads();

  // Send pad state
  esp_err_t result = esp_now_send(receiverMAC, (uint8_t*)&pads, sizeof(pads));
  if(result == ESP_OK){ //Pode ser comentado posteriormente
    Serial.print("Sent: L=");
    Serial.print(pads.left);
    Serial.print(" U=");
    Serial.print(pads.up);
    Serial.print(" D=");
    Serial.print(pads.down);
    Serial.print(" R=");
    Serial.println(pads.right);
  }

  delay(10); // small delay
}


