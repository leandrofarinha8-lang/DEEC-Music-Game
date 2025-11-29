#include <esp_now.h>
#include <WiFi.h>

//Pinos dos Pads
#define LEFT_SEND 3
#define UP_SEND 4
#define DOWN_SEND 5
#define RIGHT_SEND 6

struct PadState {
  bool left;
  bool right;
  bool up;
  bool down;
};

PadState pads;

void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) { //Função a executar quando recebe informação do sender
  memcpy(&pads, data, sizeof(pads));

  if(pads.left){
    digitalWrite(LEFT_SEND, HIGH);
  }
  else{
    digitalWrite(LEFT_SEND, LOW);
  }

  if(pads.right){
    digitalWrite(RIGHT_SEND, HIGH);  
  }
  else{
    digitalWrite(RIGHT_SEND, LOW);
  }

  if(pads.up){
    digitalWrite(UP_SEND, HIGH);  
  }
  else{
    digitalWrite(UP_SEND, LOW);
  }

  if(pads.down){
    digitalWrite(DOWN_SEND, HIGH);
  }
  else{
    digitalWrite(DOWN_SEND, LOW);
  }

  Serial.print("LEFT: ");Serial.print(pads.left);Serial.print("  UP: ");Serial.print(pads.up); Serial.print("  DOWN: ");Serial.print(pads.down); Serial.print("  RIGHT: ");Serial.println(pads.right);
}

void setup() {
  Serial.begin(115200);

  pinMode(LEFT_SEND, OUTPUT);
  pinMode(RIGHT_SEND, OUTPUT);
  pinMode(UP_SEND, OUTPUT);
  pinMode(DOWN_SEND, OUTPUT);

  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_recv_cb(onReceive);
}

void loop() {}
