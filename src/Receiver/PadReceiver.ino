#include <esp_now.h>
#include <WiFi.h>

//Pinos por definir
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

void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  memcpy(&pads, data, sizeof(pads));

  if(pads.left){
    digitalWrite(LEFT_SEND, HIGH);
  }
  else if(pads.right){
    digitalWrite(RIGHT_SEND, HIGH);  
  }
  else if(pads.up){
    digitalWrite(UP_SEND, HIGH);  
  }
  else if(pads.down){
    digitalWrite(DOWN_SEND, HIGH);
  }
  
  delay(1); //Para dar algum tempo de leitura (a leitra será feita entre frames no main_controller) (talvez tenha que muda pra 2)

  //Resetar valores
  digitalWrite(LEFT_SEND, LOW);
  digitalWrite(RIGHT_SEND, LOW); 
  digitalWrite(UP_SEND, LOW); 
  digitalWrite(DOWN_SEND, LOW);  
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_recv_cb(onReceive);
}

void loop() {}
