//IMPORTANT: The color format seems RGB565.. but for some reason the colors aren't the ones expected...

#include <TFT.h> // Arduino LCD library
#include <SPI.h>
// pin definition for the Uno
#define cs 10
#define dc 8
#define rst 9
// Arrows stuff
#define ARROW_WIDTH 15
#define ARROW_HEIGHT 20

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
  ArrowLeft(125, 50, ARROW_WIDTH, ARROW_HEIGHT, 0x037D);
  ArrowUp(95, 50, ARROW_WIDTH, ARROW_HEIGHT, 0xFEE0);
  ArrowDown(75, 50, ARROW_WIDTH, ARROW_HEIGHT, 0x1425);
  ArrowRight(45, 50, ARROW_WIDTH, ARROW_HEIGHT, 0x0B17);

  //int a = TFTscreen.width();
  //Serial.print(String(a));
  //TFTscreen.stroke(255, 0, 0);
  //TFTscreen.line(0, 0, a/2, 0);
}

void ArrowDown(int x, int y, int width, int height, uint32_t color) {
  //Nota: As coordenadas x, y correspondem à ponta da seta (ponta do triangulo)

  //Proporções
  int rectheight = height * 2 / 3;  //2/3 da altura total é a altura do retangulo
  int triheight = height * 1/3;  //1/3 da altura total é a altura do triangulo

  //Desenho da seta
  TFTscreen.fillRect(x - width/4, y + triheight, width/2, rectheight, color);
  TFTscreen.fillTriangle(x, y ,x - width/2, y + triheight, x + width/2, y + triheight, color);
  
  /*  Pontos de interesse (canto do retangulo e vertices do triangulo) [PARA DEBUG]
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.point(x, y);
  TFTscreen.stroke(0, 255, 0);
  TFTscreen.point(x - width/4, y + triheight);
  TFTscreen.point(x - width/2, y + triheight);
  TFTscreen.point(x + width/2, y + triheight);
  */
}

void ArrowUp(int x, int y, int width, int height, uint32_t color) {
  //Nota: As coordenadas x, y correspondem à ponta da seta (ponta do triangulo)

  //Proporções
  int rectheight = height * 2 / 3;  //2/3 da altura total é a altura do retangulo
  int triheight = height * 1/3;  //1/3 da altura total é a altura do triangulo

  //Desenho da seta
  TFTscreen.fillRect(x - width/4, y - triheight-rectheight, width/2, rectheight, color);
  TFTscreen.fillTriangle(x, y ,x - width/2, y - triheight, x + width/2, y - triheight, color);

  /*  Pontos de interesse (canto do retangulo e vertices do triangulo) [PARA DEBUG]
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.point(x, y);
  TFTscreen.stroke(0, 255, 0);
  TFTscreen.point(x - width/4, y - triheight);
  TFTscreen.point(x - width/2, y - triheight);
  TFTscreen.point(x + width/2, y - triheight);
  */
}

void ArrowLeft(int x, int y, int width, int height, uint32_t color) {
  //Nota: As coordenadas x, y correspondem à ponta da seta (ponta do triangulo)

  //Proporções
  int rectheight = height * 2 / 3;  //2/3 da altura total é a altura do retangulo
  int triheight = height * 1/3;  //1/3 da altura total é a altura do triangulo

  //Desenho da seta
  TFTscreen.fillRect(x - triheight-rectheight, y - width/4, rectheight, width/2, color);
  TFTscreen.fillTriangle(x, y ,x - triheight, y + width/2, x - triheight, y - width/2, color);
  
  /*  Pontos de interesse (canto do retangulo e vertices do triangulo) [PARA DEBUG]
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.point(x, y);
  TFTscreen.stroke(0, 255, 0);
  TFTscreen.point(x - triheight, y + width/2);
  TFTscreen.point(x - triheight, y - width/2);
  TFTscreen.point(x - triheight, y - width/4);
  */
}

void ArrowRight(int x, int y, int width, int height, uint32_t color) {
  //Nota: As coordenadas x, y correspondem à ponta da seta (ponta do triangulo)

  //Proporções
  int rectheight = height * 2 / 3;  //2/3 da altura total é a altura do retangulo
  int triheight = height * 1/3;  //1/3 da altura total é a altura do triangulo

  //Desenho da seta
  TFTscreen.fillRect(x + triheight, y - width/4, rectheight, width/2, color);
  TFTscreen.fillTriangle(x, y ,x + triheight, y + width/2, x + triheight, y - width/2, color);
  
  /*  Pontos de interesse (canto do retangulo e vertices do triangulo) [PARA DEBUG]
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.point(x, y);
  TFTscreen.stroke(0, 255, 0);
  TFTscreen.point(x + triheight, y + width/2);
  TFTscreen.point(x + triheight, y - width/2);
  TFTscreen.point(x + triheight, y - width/4);
  */
}

void ArrowatTime(int type, long absolute_time){

}

int y=50;
void loop() {
  /* Funciona.. mas é preciso generalizzar, e talvez dê jeito utilizar o tempo (absoluto) de quandoa  seta deve pressionada
  int arrowHeight = 30;
  ArrowDown(50, y, 20, 30, 0x0000);
  
  y--;

  if (y < 0 - arrowHeight) y = TFTscreen.height(); // loop back
  ArrowDown(50, y, 20, 30, 0x000F); //background color previous arrow (so it doesn't leave a trail)
  delay(50);
  */
}
