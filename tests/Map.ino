//This is basically an integration between the class ARROW and a new class called GameMap (hopefully it'll have a constructor from a file someday)
//The code may contain some erros since it is in a early stage of development (aka I startted today :\)
//Also important to note that I modified the ARROW class a bit (I needed some atributes to make it easier to work with on the new class)

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

//Acertos
#define HIT_Y 140

//Nota que está em pixeis (ou seja usamos a distancia)
#define HIT_EXCELLENT 10  
#define HIT_GOOD 20
#define HIT_MISS 30
//-------------------------------

//Butões (a certo ponto seram pressure-pads mas é indiferente) [SÃO NUMEROS ALEATORIOS NÃO SÃO JÁ OS PINOS]
#define BTN_LEFT 3
#define BTN_UP 3
#define BTN_DOWN 3
#define BTN_RIGHT 3

TFT TFTscreen = TFT(cs, dc, rst);

enum{
  LEFT,
  UP, 
  DOWN,
  RIGHT
};

unsigned long lastPressTime[4] = {0, 0, 0, 0};
bool lastState[4] = {LOW, LOW, LOW, LOW};
bool debouncedInput(int type){
  //NOTA: Ainda tenho que ver se vamos usar PULL_UP
  int btn;
  int index;

  switch(type){
    case LEFT:
      btn = BTN_LEFT;
      index = 0;
    break;
    case UP:
      btn = BTN_UP;
      index = 1;
    break;
    case DOWN:
      btn = BTN_DOWN;
      index = 2;
    break;
    case RIGHT:
      btn = BTN_RIGHT;
      index = 3;
  }

  bool currentState = digitalRead(btn);
  unsigned long currentTime = millis();

  if (currentState == HIGH && lastState[index] == LOW && (currentTime - lastPressTime[index]) > 50) { //O estado mudou, e passou pelo menos 'x' tempo desde a ultima mudança
      lastPressTime[index] = currentTime;
      lastState[index] = currentState;
      return true;
  }
  lastState[index] = currentState;
  return false;
}

class Arrow{
  public:

  int x, y, width, height;
  uint16_t color;
  char type;
  unsigned long startTime;
  unsigned long endTime;
  bool visible = false;
  int previousY;
  bool missed = false;
  
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

  void erasePrevious(){ //Fazer uma função dedicada em que apaga apenas os pixeis necessários (para reduzir flickering)
    if(visible)
      drawColor(0x0000, previousY); //Apaga na posição ANTERIOR (Para isso é preciso que essa posição exixsta, ou seja, que a seta esteja visivel)
    
    //Pra agora o background é sempre preto.. talvez tenha que alterar mais tarde
  }

  void eraseCurrent(){ //Fazer uma função dedicada em que apaga apenas os pixeis necessários (para reduzir flickering)
    if(visible)
      drawColor(0x0000, y); //Apaga na posição ANTERIOR (Para isso é preciso que essa posição exixsta, ou seja, que a seta esteja visivel)
    
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
      erasePrevious();

      //Desenha a seta (na nova posição)
      draw();

      previousY = y; //A posição atual passa a ser a antiga (para a proxima vez que for feito o update)
    }
  }
};

class GameMap{ //Tem que ter: Logica do jogo, socre, combo, ver acertos e falhas, desenhar elementos permanentes (setas de acerto, background, ...)
  public:

  unsigned long duration;
  unsigned long StartTime;
  unsigned long CurrentTime;
  unsigned long LastUpdate;
  bool playing = false;
  String name;
  Arrow * layout;
  int ArrowNum;
  int score;
  int combo;


  GameMap(Arrow *arrowArray, int count) { //no futuro devera construir a partir de um ficheiro guardado no sd
        layout = arrowArray;
        ArrowNum = count;
        score = 0;
        combo = 0;
  }
  
  void begin() {
    //Carregar imagem e desenhar setas brancas (sitios a acertar)
    //Start time e end time são irrelevante pq estas setas não vao ser atualizadas

    //Pequena pausa para dar tempo de desenhar
    StartTime = millis();
    LastUpdate = 0;
    playing = true;
  }

  void play() {
      if (!playing) return;
      

      if(millis() - LastUpdate > 25){
      LastUpdate = millis();
      unsigned long currentTime = millis() - StartTime;

      // Atualiza todas as setas
      for (int i = 0; i < ArrowNum; i++) {
          Arrow &currentA = layout[i];

          // Atualiza posição
          if (currentTime >= currentA .startTime && currentTime <= currentA .endTime) {
              currentA.update(currentTime);
          }

          //Se a seta passou da zona de acerto e ainda está visível é um MISS
          if (!currentA.missed && currentA.visible && currentA.y > HIT_Y + 15 && currentA.y > 10) { //A ultima condição é para evitar um bug
              currentA.eraseCurrent(); //ERASE APENAS É POSSIVEL QUANDO A SETA ESTÁ VISIVEL!!! (erro de 30 mins btw). Vou deixar a class Arrow como tá pq só faz sentido apagar algo visivel
              currentA.missed = true;
              currentA.visible = false;
              Serial.println("MISS");
              combo = 0; // resetar combo
          }
      }

      //Setas permanentes no sitio dos acertos
      Arrow left(LEFT, 0, 0);
      Arrow right(RIGHT, 0, 0);
      Arrow up(UP, 0, 0);
      Arrow down(DOWN, 0, 0);
      left.drawColor(0xFFFF,HIT_Y);
      right.drawColor(0xFFFF,HIT_Y);
      up.drawColor(0xFFFF,HIT_Y);
      down.drawColor(0xFFFF,HIT_Y);

      //Ler os Inputs do jogador (Tenho que usar debounce porque o loop vai executar rapidamente)

      if (debouncedInput(LEFT)) {
          char result = processInput(LEFT);
          ScoresHandle(result);
      }
      if (debouncedInput(UP)) {
          char result = processInput(UP);
          ScoresHandle(result);
      }
      if (debouncedInput(DOWN)) {
          char result = processInput(DOWN);
          ScoresHandle(result);
      }
      if (debouncedInput(RIGHT)) {
          char result = processInput(RIGHT);
          ScoresHandle(result);
      }
      }
  }

  // Função para processar a tecla apertada
  char processInput(char key) {
    Arrow* closestArrow = nullptr;
    int closestDist = 9999;

    for (int i = 0; i < ArrowNum; i++){ //encontrar a seta mais proxima do HIT_Y
      if (layout[i].visible && !layout[i].missed && layout[i].type == key){ //apenas vê setas visiveis (e do tipo pressionado)
         int diff = abs(layout[i].y - HIT_Y);
          if (diff < closestDist){
            closestDist = diff;
            closestArrow = &layout[i];
          }
      }
    }

    if (closestArrow){
        closestArrow->visible = false;
        closestArrow->eraseCurrent();

        if (closestDist <= 10) return HIT_EXCELLENT; //preciso de meter o combo ao barulho...
        else if (closestDist <= 20) return HIT_GOOD;
        else return HIT_MISS;
    }

    return HIT_MISS; // nenhuma seta do tipo correto
  }

  void ScoresHandle(char result) {
    //Prints para debug (e talvvez até deixemos assim)
    float combo_bonus = 1.0 + (combo * 0.05); // +5% de bonus
    switch(result) {
        case HIT_EXCELLENT:
            score += (int)100*combo_bonus;
            combo++;
            Serial.println("EXCELLENT!");
            break;
        case HIT_GOOD:
            score += (int)50*combo_bonus;
            combo++;
            Serial.println("GOOD!");
            break;
        case HIT_MISS:
            combo = 0;
            Serial.println("MISS!");
            break;
    }
    Serial.print("Score: "); 
    Serial.println(score);
    Serial.print("Combo: "); 
    Serial.println(combo);
  }
};

Arrow arrows[] = {
    Arrow(LEFT, 1000, 3000),
    Arrow(UP, 1500, 3500),
    Arrow(DOWN, 1800, 4200),
    Arrow(RIGHT, 2200, 5000),
};

#define NUM_ARROWS (sizeof(arrows)/sizeof(arrows[0]))
GameMap map1(arrows, NUM_ARROWS);
void setup() {
  // initialize the serial port
  Serial.begin(9600);
  // initialize the display
  TFTscreen.begin();
  // clear the screen with a pretty color
  TFTscreen.background(0, 0, 0);
  TFTscreen.setRotation(2);

  //int a = TFTscreen.width();
  //Serial.print(String(a));
  //TFTscreen.stroke(255, 0, 0);
  //TFTscreen.line(0, 0, a/2, 0);
  map1.begin();
}


void loop() {
  map1.play();
}


