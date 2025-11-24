#define LDR A0

bool LDRVal(){
  int val;
  val=analogRead(LDR);
  if (val<999){
    return(true);
  }
  else if (val>=999||val<1024){
    return(false);
  }


}

void setup() {
  Serial.begin(9600);

}

void loop() {
  
  Serial.println(LDRVal());
  Serial.println(analogRead(LDR));
  

}
