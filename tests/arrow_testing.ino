#include <TFT.h> // Arduino LCD library
#include <SPI.h>
// pin definition for the Uno
#define cs 10
#define dc 8
#define rst 9
// Arrows stuff
#define ARROW_WIDTH 15
#define ARROW_HEIGHT 20
#define ARROW_LEFT_X 95
#define ARROW_UP_X 75
#define ARROW_DOWN_X 55
#define ARROW_RIGHT_X 35
#define ARROW_START_HEIGHT 10
#define ARROW_END_HEIGHT 140

TFT TFTscreen = TFT(cs, dc, rst);
// position of the line on screen
int xPos = 0;

void setup() {
  // initialize the serial port
  Serial.begin(9600);
  // initialize the display
  TFTscreen.begin();
  // clear the screen with a pretty color
  TFTscreen.background(0, 0, 0);
  TFTscreen.setRotation(2);

  //Check colors at: https://rgbcolorpicker.com/565
  ArrowLeft(ARROW_LEFT_X, ARROW_START_HEIGHT, ARROW_WIDTH, ARROW_HEIGHT, 0x28FF);
  ArrowUp(ARROW_UP_X, ARROW_START_HEIGHT, ARROW_WIDTH, ARROW_HEIGHT, 0x07FF);
  ArrowDown(ARROW_DOWN_X, ARROW_START_HEIGHT, ARROW_WIDTH, ARROW_HEIGHT, 0x0640);
  ArrowRight(ARROW_RIGHT_X, ARROW_START_HEIGHT, ARROW_WIDTH, ARROW_HEIGHT, 0xF800);
 
  ArrowLeft(ARROW_LEFT_X, ARROW_END_HEIGHT, ARROW_WIDTH, ARROW_HEIGHT, 0xC618);
  ArrowUp(ARROW_UP_X, ARROW_END_HEIGHT, ARROW_WIDTH, ARROW_HEIGHT, 0xC618);
  ArrowDown(ARROW_DOWN_X, ARROW_END_HEIGHT, ARROW_WIDTH, ARROW_HEIGHT, 0xC618);
  ArrowRight(ARROW_RIGHT_X, ARROW_END_HEIGHT, ARROW_WIDTH, ARROW_HEIGHT, 0xC618);
 
  //int a = TFTscreen.width();
  //Serial.print(String(a));
  //TFTscreen.stroke(255, 0, 0);
  //TFTscreen.line(0, 0, a/2, 0);
}

void ArrowDown(int x1, int y1, int width, int height, uint16_t color) {
  int x = x1;
  int y = y1 - height/2;
  //Nota: As coordenadas x, y correspondem à ponta da seta (ponta do triangulo), enqaunto que as x1 e y1 devem corresponder ao centro (vertical) da seta

  //Proporções
  int rectheight = height * 2 / 3;  //2/3 da altura total é a altura do retangulo
  int triheight = height * 1/3;  //1/3 da altura total é a altura do triangulo

  //Desenho da seta
  TFTscreen.fillRect(x - width/4, y + triheight, width/2, rectheight, color);
  TFTscreen.fillTriangle(x, y ,x - width/2, y + triheight, x + width/2, y + triheight, color);
  
  /*  Pontos de interesse (canto do retangulo e vertices do triangulo) [PARA DEBUG]
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.point(x1, y1);
  TFTscreen.stroke(0, 255, 0);
  TFTscreen.point(x - width/4, y + triheight);
  TFTscreen.point(x - width/2, y + triheight);
  TFTscreen.point(x + width/2, y + triheight);
  */
}

void ArrowUp(int x1, int y1, int width, int height, uint16_t color) {
//Nota: As coordenadas x, y correspondem à ponta da seta (ponta do triangulo), enqaunto que as x1 e y1 devem corresponder ao centro (vertical) da seta

  int x = x1;
  int y = y1 + height/2;
  //Proporções
  int rectheight = height * 2 / 3;  //2/3 da altura total é a altura do retangulo
  int triheight = height * 1/3;  //1/3 da altura total é a altura do triangulo

  //Desenho da seta
  TFTscreen.fillRect(x - width/4, y - triheight-rectheight, width/2, rectheight, color);
  TFTscreen.fillTriangle(x, y ,x - width/2, y - triheight, x + width/2, y - triheight, color);

  /*  Pontos de interesse (canto do retangulo e vertices do triangulo) [PARA DEBUG]
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.point(x1, y1);
  TFTscreen.stroke(0, 255, 0);
  TFTscreen.point(x - width/4, y - triheight);
  TFTscreen.point(x - width/2, y - triheight);
  TFTscreen.point(x + width/2, y - triheight);
  */
}

void ArrowLeft(int x1, int y1, int width, int height, uint16_t color) {
  //Nota: As coordenadas x, y correspondem à ponta da seta (ponta do triangulo), enqaunto que as x1 e y1 devem corresponder ao centro (vertical) da seta
  int x = x1 + height/2;
  int y = y1;
  //Proporções
  int rectheight = height * 2 / 3;  //2/3 da altura total é a altura do retangulo
  int triheight = height * 1/3;  //1/3 da altura total é a altura do triangulo

  //Desenho da seta
  TFTscreen.fillRect(x - triheight-rectheight, y - width/4, rectheight, width/2, color);
  TFTscreen.fillTriangle(x, y ,x - triheight, y + width/2, x - triheight, y - width/2, color);
  
  /*  Pontos de interesse (canto do retangulo e vertices do triangulo) [PARA DEBUG]
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.point(x1, y1);
  TFTscreen.stroke(0, 255, 0);
  TFTscreen.point(x - triheight, y + width/2);
  TFTscreen.point(x - triheight, y - width/2);
  TFTscreen.point(x - triheight, y - width/4);
  */
}

void ArrowRight(int x1, int y1, int width, int height, uint16_t color) {
//Nota: As coordenadas x, y correspondem à ponta da seta (ponta do triangulo), enqaunto que as x1 e y1 devem corresponder ao centro (vertical) da seta
  int x = x1 - height/2;
  int y = y1;
  //Proporções
  int rectheight = height * 2 / 3;  //2/3 da altura total é a altura do retangulo
  int triheight = height * 1/3;  //1/3 da altura total é a altura do triangulo

  //Desenho da seta
  TFTscreen.fillRect(x + triheight, y - width/4, rectheight, width/2, color);
  TFTscreen.fillTriangle(x, y ,x + triheight, y + width/2, x + triheight, y - width/2, color);

  /*  Pontos de interesse (canto do retangulo e vertices do triangulo) [PARA DEBUG]
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.point(x1, y1);
  TFTscreen.stroke(0, 255, 0);
  TFTscreen.point(x + triheight, y + width/2);
  TFTscreen.point(x + triheight, y - width/2);
  TFTscreen.point(x + triheight, y - width/4);
  */
}

int Posanim(unsigned long startTime, unsigned long endTime){
  //No final era bom ficar com uma função tipo ArrowatTime(startTime, endTime) que já tenham um loop (ou switch) em si
  unsigned long now = millis();
    
  if (now <= startTime) return 0;
  if (now >= endTime) return 180;

  return map(now, startTime, endTime, 0, 180);
}

int y=50;
void loop() {
  int posY = Posanim(10000, 15000);
  ArrowLeft(ARROW_LEFT_X, posY, ARROW_WIDTH, ARROW_HEIGHT, 0x28FF);
  ArrowLeft(ARROW_LEFT_X, posY-1, ARROW_WIDTH, ARROW_HEIGHT, 0x0000);
}
