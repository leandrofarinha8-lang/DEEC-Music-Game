// Not this relevant, basically just added a GAME() function which will handle the whole game (menu, gmae and so on)
// There is an error on the GAME(), for some reason if I uncomment the TFT.text calls it skips the map.play() ??????

#include <SD.h>
#include <TFT.h> // Arduino LCD library
#include <SPI.h>
//--pin definition for the Uno--
#define cs 10
#define dc 8
#define rst 9
#define sd 4
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
#define HIT_EXCELLENT 3  
#define HIT_GOOD 5
#define HIT_MISS 10
//-------------------------------

//Butões (a certo ponto seram pressure-pads mas é indiferente) [SÃO NUMEROS ALEATORIOS NÃO SÃO JÁ OS PINOS]
#define BTN_LEFT 3
#define BTN_UP 3
#define BTN_DOWN 3
#define BTN_RIGHT 3

//Butões pro menu
#define PUSH_LEFT 2
#define PUSH_RIGHT 5

#define MAX_ARROWS 7

TFT TFTscreen = TFT(cs, dc, rst);
bool day = true;

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

enum{
  LEFT,
  UP, 
  DOWN,
  RIGHT,
  MENU_LEFT,
  MENU_RIGHT
};

unsigned long lastPressTime[6] = {0, 0, 0, 0, 0, 0};
bool lastState[6] = {LOW, LOW, LOW, LOW, LOW, LOW};
bool debouncedInput(int type){
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
    break;
    case MENU_LEFT:
      btn = PUSH_LEFT;
      index = 4;
    break;
    case MENU_RIGHT:
      btn = PUSH_RIGHT;
      index = 5;
  }

  bool currentState = digitalRead(btn);
  unsigned long currentTime = millis();

  if (currentState == LOW && lastState[index] != currentState && (currentTime - lastPressTime[index]) > 50) { //O estado mudou, e passou pelo menos 'x' tempo desde a ultima mudança
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

  Arrow(){};
  
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
    if (missed) return; //para a seta não dar mais updates quando é errada
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
//Setas permanentes no sitio dos acertos
Arrow left(LEFT, 0, 0);
Arrow right(RIGHT, 0, 0);
Arrow up(UP, 0, 0);
Arrow down(DOWN, 0, 0);

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
  char FilePath[20];


  GameMap(Arrow *arrowArray, int count) { //no futuro devera construir a partir de um ficheiro guardado no sd
        layout = arrowArray;
        ArrowNum = count;
        score = 0;
        combo = 0;
  }

  GameMap(){layout = new Arrow[MAX_ARROWS];}  // construtor vazio

  void loadFromFile(char* path) {
    if (!SD.begin(sd)) {
      return;
    }
    
    // Create a buffer for the full filename
    char filename[15];
    strcpy(filename, path);
    strcpy(FilePath, filename);
    strcat(filename, "/MAP"); // append "MAP"

    File file = SD.open(filename, FILE_READ);
    if (!file) {
      return;
    }

    // ---Name---
    name = file.readStringUntil('\n');
    name.trim();
    Serial.println(name);

    // ---Duration---
    String durationStr = file.readStringUntil('\n');
    durationStr.trim();
    int duration1 = durationStr.toInt();

    // ---Arrow Num---
    String arrowNumStr = file.readStringUntil('\n');
    arrowNumStr.trim();
    int ArrowNum1 = arrowNumStr.toInt();
    if (ArrowNum1 > MAX_ARROWS) ArrowNum1 = MAX_ARROWS;

    // ---Parse arrows---
    for (int i = 0; i < ArrowNum1; i++) {
      if (!file.available()) break;

      String line = file.readStringUntil('\n');
      line.trim();

      char typeChar;
      unsigned long startTime;
      unsigned long endTime;
      sscanf(line.c_str(), "%c,%lu,%lu", &typeChar, &startTime, &endTime);

      int type;
      switch(typeChar) {
        case 'L': type = LEFT; break;
        case 'U': type = UP; break;
        case 'D': type = DOWN; break;
        case 'R': type = RIGHT; break;
      }

      layout[i] = Arrow(type, startTime, endTime);
      duration = duration1;
      ArrowNum = ArrowNum1;
      score = 0;
      combo = 0;
      }
    file.close();
  }

  
  void begin() {
    //Carregar imagem e desenhar setas brancas (sitios a acertar)
    //Start time e end time são irrelevante pq estas setas não vao ser atualizadas

    //Pequena pausa para dar tempo de desenhar
    LastUpdate = 0;
    playing = true;

    char filename[20];
    strcpy(filename, FilePath);

    if(day){
      strcat(filename, "/D.bmp");
    }
    else{
      strcat(filename, "/N.bmp");
    }
    //Carregar background do nivel (vai ter que ser obtido numa variavvel no futuro)
    ImageToScreen(0, 0, filename);
    StartTime = millis();
  }

  void play() {
      if(duration <= CurrentTime){
        playing = false;
        return;
      }
      
      if (!playing) return; 

      if(millis() - LastUpdate > 25){
      LastUpdate = millis();
      CurrentTime = millis() - StartTime;

      // Atualiza todas as setas
      for (int i = 0; i < ArrowNum; i++) {
          Arrow &currentA = layout[i];

          // Atualiza posição
          if (CurrentTime >= currentA .startTime && CurrentTime <= currentA .endTime) {
              currentA.update(CurrentTime);
          }

          //Se a seta passou da zona de acerto e ainda está visível é um MISS
          if (!currentA.missed && currentA.visible && currentA.y > HIT_Y + 15 && currentA.y > 10) { //A ultima condição é para evitar um bug
              currentA.eraseCurrent(); //ERASE APENAS É POSSIVEL QUANDO A SETA ESTÁ VISIVEL!!! (erro de 30 mins btw). Vou deixar a class Arrow como tá pq só faz sentido apagar algo visivel
              currentA.missed = true;
              currentA.visible = false;
              Serial.println("MISS");
              combo = 0; // resetar combo
              Serial.print("Score: "); 
              Serial.println(score);
              Serial.print("Combo: "); 
              Serial.println(combo);
          }
      }

      //Desenha as setas permanentes no sitio dos acertos
      left.drawColor(0xFFFF,HIT_Y);
      right.drawColor(0xFFFF,HIT_Y);
      up.drawColor(0xFFFF,HIT_Y);
      down.drawColor(0xFFFF,HIT_Y);

      /*Text testing (need to show score and combo)
      TFTscreen.stroke(0xFFFF);
      TFTscreen.setTextSize(1);
      TFTscreen.setRotation(1);
      TFTscreen.text("Hello", 50, 120);
      TFTscreen.setRotation(2);
      */

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
      closestArrow->eraseCurrent();
      closestArrow->visible = false;
      closestArrow->missed = true;//not missed but this stops updates (é como se a seta contasse como 'já acertada')

      if (closestDist <= HIT_EXCELLENT) return HIT_EXCELLENT; //preciso de meter o combo ao barulho...
      else if (closestDist <= HIT_GOOD) return HIT_GOOD;
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
    //Arrow(UP, 1500, 3500),
    //Arrow(DOWN, 1800, 4200),
    //Arrow(RIGHT, 2200, 5000),
    Arrow(LEFT, 2600, 5600),
    //Arrow(UP, 3000, 6200),
    //Arrow(DOWN, 3400, 6800),
    Arrow(RIGHT, 3800, 7400),
    Arrow(LEFT, 4200, 8000),
    //Arrow(UP, 4600, 8600),
    //Arrow(DOWN, 5000, 9200),
    Arrow(RIGHT, 5400, 9800),
    Arrow(LEFT, 5800, 10400),
    //Arrow(UP, 6200, 11000),
    //Arrow(DOWN, 6600, 11600),
};

#define NUM_ARROWS (sizeof(arrows)/sizeof(arrows[0]))
GameMap map1(arrows, NUM_ARROWS);
GameMap mapFromFile; // global, mas vazio

void GAME(){
  //Also need to check the LDR to see if it's night or day
  int i=0;
  //First of all need the menu (with animation)
  while(true){
    i++;
    Serial.println("TIME: " + String(i));
    //Also need to check the LDR to see if it's night or day

    //First of all need the menu (without animation)

    //After the menu animation need to select the map (depends if it's day or night)

    //After that use the distance sensor to begin the map
    mapFromFile.loadFromFile("/MAPS/CFrog");
    //After selecting the map need to map.begin() and then on a loop do map.play()
    mapFromFile.begin();
    delay(100); //some time to loadd
    while(mapFromFile.playing){ 
      //NOTA: Enquanto o jogo roda tenho que receber informações do padsender
      mapFromFile.play();
    }
    //After that I need to show the score (endscreen)
    TFTscreen.background(0, 0, 0);
    Serial.println("GAME ENDED");
    Serial.println(mapFromFile.score);
    TFTscreen.stroke(255,255,255);
    TFTscreen.setTextSize(1);

    //TFTscreen.setRotation(0);
    //String scoreText = "Score: " + String(mapFromFile.score);
    //TFTscreen.text(scoreText.c_str(),5,50);
    //TFTscreen.text("Game Ended!",5,10);
    //TFTscreen.text("Press a push button\nto return!",5,90);
    //TFTscreen.setRotation(2);
    break; //for testing
    //After a button is pressed the game should return to the menu (without doing the menu animation again)
  }
  return;
}
void setup() {
  // initialize the serial port
  Serial.begin(9600);
  pinMode(PUSH_LEFT, INPUT_PULLUP);
  pinMode(PUSH_RIGHT, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  // initialize the display
  TFTscreen.begin();
  // clear the screen with a pretty color
  TFTscreen.background(0, 0, 0);
  TFTscreen.setRotation(2);

  //int a = TFTscreen.width();
  //Serial.print(String(a));
  //TFTscreen.stroke(255, 0, 0);
  //TFTscreen.line(0, 0, a/2, 0);
  //map1.begin();
  //mapFromFile.loadFromFile("/MAPS/CFrog/MAP");  // parse do arquivo
  //mapFromFile.begin();
  GAME();
}


void loop() {
  //mapFromFile.play();
}
