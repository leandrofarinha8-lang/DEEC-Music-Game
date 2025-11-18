//!!STILL TESTING!! The images format is 'invalid' from 20 onwards for some reason :(

#include <SD.h> //Stupid error but SD.h must be included before TFT.h to use PImage
#include <TFT.h>
#include <SPI.h>

#define cs  10
#define dc   8
#define rst  9
#define sd    4

TFT TFTscreen = TFT(cs, dc, rst);

PImage logo;

bool ImageToScreen(int x, int y, char * name){
  //Assumes TFT.begin() was done

  if (!SD.begin(sd)) { //problems with the sd
    Serial.println(F("failed sd!"));
    return false;
  }

  PImage img = TFTscreen.loadImage(name); //name ex: "DEEC.bmp"
  
  if (!img.isValid()) { //Problems with image format
    Serial.println(F("failed image format!"));
    return false; 
  }
  TFTscreen.image(img, x, y);

  return true;
}
void setup() {
  // initialize the GLCD and show a message
  // asking the user to open the serial line
  TFTscreen.begin();
  TFTscreen.background(255, 255, 255);
  TFTscreen.setRotation(2);
  Serial.begin(9600);
  /*
  for(int i=0; i<34; i++){
    Serial.println(i);
    String test = String("DEECDAY/")+String(i)+String(".bmp");
    ImageToScreen(0, 0, test.c_str());
  }
  */
  ImageToScreen(0, 0, "DEECDAY/30.bmp");
}

void loop() {

}
