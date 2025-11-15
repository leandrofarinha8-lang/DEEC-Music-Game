#include <TFT.h> // Arduino LCD library
#include <SPI.h>
//--pin definition for the Uno--
#define cs 10
#define dc 8
#define rst 9
//------------------------------

//----Arrows stuff-----
#define ARROW_WIDTH 15
#define ARROW_HEIGHT 20

//Posição horizonntal
#define ARROW_LEFT_X 95
#define ARROW_UP_X 75
#define ARROW_DOWN_X 55
#define ARROW_RIGHT_X 35

//Cores
#define ARROW_LEFT_COLOR 0x28FF
#define ARROW_UP_COLOR 0x07FF
#define ARROW_DOWN_COLOR 0x0640
#define ARROW_RIGHT_COLOR 0xF800

//Alturas (para animação)
#define ARROW_START_HEIGHT 10
#define ARROW_END_HEIGHT 200 //era 140, mas vou meter um valor de modo a que as setas passem o topo
//-------------------------------

TFT TFTscreen = TFT(cs, dc, rst);

enum{
  LEFT,
  UP, 
  DOWN,
  RIGHT
};

class Arrow{
  public:

  int x, y, width, height;
  uint16_t color;
  char type;
  unsigned long startTime;
  unsigned long endTime;
  bool visible = false;
  int previousY;
  
  Arrow(char t, unsigned long st, unsigned long et){ //Construtor

    width= ARROW_WIDTH;
    height= ARROW_HEIGHT;

    startTime = st;
    endTime = et;

    type = t;
    y = ARROW_START_HEIGHT;

    switch(t){
      case LEFT: 
        x = ARROW_LEFT_X; 
        color = ARROW_LEFT_COLOR;
      break;
      case UP: 
        x = ARROW_UP_X; 
        color = ARROW_UP_COLOR;
      break;
      case DOWN: 
        x = ARROW_DOWN_X; 
        color = ARROW_DOWN_COLOR;
      break;
      case RIGHT: 
        x = ARROW_RIGHT_X; 
        color = ARROW_RIGHT_COLOR;
      break;
    }

    previousY = y;
  }

  void drawColor(uint16_t c, int Y){ //vai dar mais jeito pra depois fazer o erase
    //Nota: As coordenadas x, y correspondem à ponta da seta (ponta do triangulo), enqaunto que as x1 e y1 devem corresponder ao centro (vertical) da seta
    int rectheight = height * 2 / 3;  //2/3 da altura total é a altura do retangulo
    int triheight = height * 1/3;  //1/3 da altura total é a altura do triangulo
    
    int x, y;

    switch(type){
      case LEFT:
        x = this->x + height/2;
        y = Y;

        //Desenho da seta
        TFTscreen.fillRect(x - triheight-rectheight, y - width/4, rectheight, width/2, c);
        TFTscreen.fillTriangle(x, y ,x - triheight, y + width/2, x - triheight, y - width/2, c);
      break;
      case UP:
        x = this->x;
        y = Y + height/2;

        //Desenho da seta
        TFTscreen.fillRect(x - width/4, y - triheight-rectheight, width/2, rectheight, c);
        TFTscreen.fillTriangle(x, y ,x - width/2, y - triheight, x + width/2, y - triheight, c);

      break;
      case DOWN:
        x = this->x;
        y = Y - height/2;

        //Desenho da seta
        TFTscreen.fillRect(x - width/4, y + triheight, width/2, rectheight, c);
        TFTscreen.fillTriangle(x, y ,x - width/2, y + triheight, x + width/2, y + triheight, c);
      break;
      case RIGHT:
        x = this->x - height/2;
        y = Y;

        //Desenho da seta
        TFTscreen.fillRect(x + triheight, y - width/4, rectheight, width/2, c);
        TFTscreen.fillTriangle(x, y ,x + triheight, y + width/2, x + triheight, y - width/2, c);
      break;
    }
  }

  void draw(){ //Apenas vai ser chamada quando a seta aparecer (o current_time >= start_time)
    drawColor(color, y); //Desenha na posição ATUAL
    visible = true;
  }

  void erase(){ //Fazer uma função dedicada em que apaga apenas os pixeis necessários (para reduzir flickering)
    if(visible)
      drawColor(0x0000, previousY); //Apaga na posição ANTERIOR (Para isso é preciso que essa posição exixsta, ou seja, que a seta esteja visivel)
    
    //Pra agora o background é sempre preto.. talvez tenha que alterar mais tarde
  }

  void update(unsigned long current_time){ //Atualiza atributos da seta baseando-se no tempo
    
    if (current_time < startTime || current_time > endTime) return; //Ainda não apareceu ou já saiu da tela (não há nada para atualizar)

    //Se já apareceu é preciso atualizar a posição atual (y)

    if (current_time <= startTime) //Ainda não apareceu
      y = ARROW_START_HEIGHT;  
    else if (current_time >= endTime){  //Já apareceu e saiu do ecra (já não é mais visivel)
      y = ARROW_END_HEIGHT;
      visible = false;
    }
    else{    //Já apareceu e ainda não percorreu o ecra todo
      y = map(current_time, startTime, endTime, ARROW_START_HEIGHT, ARROW_END_HEIGHT);
    }

    if (y != previousY) { //Tentativa de reduzir o 'flicker' no ecra (apenas redesenha se a posição mudou)
      //Apaga (a posição anterior)
      erase();

      //Desenha a seta (na nova posição)
      draw();

      previousY = y; //A posição atual passa a ser a antiga (para a proxima vez que for feito o update)
    }
  }
};
unsigned long startTime;
void setup() {
  // initialize the serial port
  Serial.begin(9600);
  // initialize the display
  TFTscreen.begin();
  // clear the screen with a pretty color
  TFTscreen.background(0, 0, 0);
  TFTscreen.setRotation(2);

  startTime = millis();
  //int a = TFTscreen.width();
  //Serial.print(String(a));
  //TFTscreen.stroke(255, 0, 0);
  //TFTscreen.line(0, 0, a/2, 0);
}

Arrow arrows[] = {
    Arrow(LEFT,   1000,  3000),
    Arrow(UP,     1500,  3500),
    Arrow(DOWN,   1800,  4200),
    Arrow(RIGHT,  2200,  5000),
};

#define NUM_ARROWS (sizeof(arrows)/sizeof(arrows[0]))
unsigned long LastUpdate = 0;
void loop() {
  unsigned long GameTime = millis() - startTime;
  if(millis() - LastUpdate >= 15){ //desenha apenas a cada 15ms (para tentar reduzir flickering)
    for (int i = 0; i < NUM_ARROWS; i++) {
        arrows[i].update(GameTime);
    }

    LastUpdate = GameTime;
  }
}
