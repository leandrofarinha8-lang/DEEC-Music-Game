//IMPORTANT NOTE: Most functions don't have error handling (mainly the ones related to the files) since every byte is valuable on arduino

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
#define HIT_EXCELLENT 5  
#define HIT_GOOD 10
#define HIT_MISS 15

//Butões pro menu
#define PUSH_LEFT 2
#define PUSH_RIGHT 3

#define MAX_ARROWS 5
#define MAX_SONG 1 //Numero maximo de notas e durações para uma musica

//Pads (o sinal será enviado por um esp e recebidl nestes pinos, srá nos pinos A)
#define PAD_LEFT A0
#define PAD_UP A1
#define PAD_DOWN A2
#define PAD_RIGHT A3

//----PINOS POR DEFINIR------
//Pinos para o sensor de distancia
#define echo 5
#define trigger 6
#define DistanceON_Value 10 //Distancia para a qual o sensor de distancia se 'ativa'

//Pinos do LDR
#define LDR A4

//Pino do buzzer
#define BUZZER 7

//----Para carregar nomes de mapas para uma array (não abusar na memoria se não as imagens podem falahr)-----
#define MAX_MAPS 2
#define MAX_NAME_LENGTH 13 //O nº de chars permitidos no SD é de 8 para o nome e 3 pra extensão (13 é suficiente para 8 chars de nome + '.' + 3 chars de extensão + '/0')

char MapNames[MAX_MAPS][MAX_NAME_LENGTH];
int MapsCount = 0;

/*
void playMelody(const char* filename, bool reset = false) {
  static File melodyFile;
  static unsigned long previousTime = 0;
  static int currentFrequency = 0;
  static int currentDuration = 0;
  static bool fileInitialized = false;

  unsigned long currentTime = millis();

  //Resetar (quando troca de nivel vou precisar)
  if (reset && fileInitialized) {
    melodyFile.seek(0);      //Volta ao inicio
    previousTime = currentTime;
    currentFrequency = 0;
    currentDuration = 0;
    noTone(BUZZER);         
    return;
  }

  //Se o ficheiro já está inicializado...
  if (!fileInitialized) {

    melodyFile = SD.open(filename);
    if (!melodyFile) {
      Serial.println("Failed to open melody file!");
      return;
    }

    previousTime = currentTime;
    fileInitialized = true;
  }

  // Check if it's time to play the next note
  if (currentTime - previousTime >= currentDuration) {
    noTone(BUZZER); // Stop previous note

    if (melodyFile.available()) {
      currentFrequency = melodyFile.parseInt();
      currentDuration = melodyFile.parseInt();

      Serial.print("Frequency: "); Serial.println(currentFrequency);
      Serial.print("Duration: "); Serial.println(currentDuration);
      if (currentFrequency > 0) {
        tone(BUZZER, currentFrequency, currentDuration);
      }

      previousTime = currentTime;
    } else {
      // End of file: loop back to start
      melodyFile.seek(0);
    }
  }
}
*/

void loadMapNames(){
  MapsCount = 0;

  File dir = SD.open("/MAPS");

  // Ler tudo na pasta
  while (true) {
    File newFile = dir.openNextFile();
    if (!newFile) break;

    if (newFile.isDirectory()){ //Os mapas seram apenas diretórios
      if (MapsCount < MAX_MAPS) {
        strncpy(MapNames[MapsCount], newFile.name(), MAX_NAME_LENGTH - 1);
        MapNames[MapsCount][MAX_NAME_LENGTH - 1] = '\0'; //O problema de trabalhar com c_strings :(
        MapsCount++;
      }
    }

    newFile.close();
  }
  //Neste ponto todos os mapas lidos estão na array global, depois no menu é apenas usar os botões pra alterarem o index e o sensor de distancia pra escolher o mapa
  dir.close();
}

int choose_map(int choice){
  if(PUSH_RIGHT){ //depois falta meter o input debounce..
    choice = (choice == MapsCount-1)? MapsCount-1 : choice+1;
  }
  if(PUSH_LEFT){
    choice = (choice == 0)? 0 : choice+1;
  }

  return choice; //returna apenas o indice do mapa escolhido
}
//------------------------------------------------------------

TFT TFTscreen = TFT(cs, dc, rst);
bool day = true;

enum{ //Para a direção das setas
  LEFT,
  UP, 
  DOWN,
  RIGHT
};

struct PadState{
  bool left;
  bool right;
  bool up;
  bool down;
};

PadState Pads = {false, false, false, false};

//-------Pad debounce and check for transition----
unsigned long lastPadPressTime[4] = {0, 0, 0, 0};
bool lastPadState[4] = {LOW, LOW, LOW, LOW};

bool debouncedPadInput(int btn){
  int index;

  switch(btn){
    case PAD_LEFT:  index = 0; break;
    case PAD_UP:    index = 1; break;
    case PAD_DOWN:  index = 2; break;
    case PAD_RIGHT: index = 3; break;
  }

  bool currentState = digitalRead(btn);
  unsigned long currentTime = millis();

  if(currentState == HIGH && lastPadState[index] != currentState && (currentTime - lastPadPressTime[index]) > 50){ //Vê a transição para HIGH
      lastPadPressTime[index] = currentTime;
      lastPadState[index] = currentState;
      return true;
  }

  lastPadState[index] = currentState;
  return false;
}
//Nota: não vou juntar com a outra função porque os 'botões' em cima nnão têm PULL_UP (a logica é normal) e os de baixo tem PULL_UP (Logica invertida)
//-----------Menu Buttons---------------
unsigned long lastMenuPressTime[2] = {0, 0};
bool lastMenuState[2] = {LOW, LOW};

bool debouncedMenuInput(int btn) {
    int index;
    
    switch(btn){
        case PUSH_LEFT: index = 0; break;
        case PUSH_RIGHT: index = 1; break;
        //podia meter um default pra segurança mas a gente conhece o codigo minimamente bem (e precisamos de espaço)
    }

    bool currentState = digitalRead(btn);
    unsigned long currentTime = millis();

    if (currentState == LOW && lastMenuState[index] != currentState && (currentTime - lastMenuPressTime[index]) > 50) {
        lastMenuPressTime[index] = currentTime;
        lastMenuState[index] = currentState;
        return true;
    }

    lastMenuState[index] = currentState;
    return false;
}
//--------------------------------------
//----TFT Related function--------------
bool ImageToScreen(int x, int y, char * name){
  //Assumes TFT.begin() was done

  PImage img = TFTscreen.loadImage(name); //name ex: "DEEC.bmp"
  
  if (!img.isValid()) { //Problems with image format
    Serial.println(F("failed image format!"));
    return false; 
  }
  TFTscreen.image(img, x, y);

  img.close(); //FINALMENTE RESOLVIDO
  return true;
}
//----------------------------------------

//---Funções para o sensor de distancia---
long measureDistance() { 
  long duration, distance;
  //Send pulse
  digitalWrite(trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  //Wait for echo and measure time until it happens
  duration = pulseIn(echo, HIGH);
  
  //Compute distance
  distance = duration/58;
  return distance;
}

bool DistanceSensorOn(){
  long distance = 0;
  for(int i=0; i<3; i++){ //3 repetições para evitar flutuações
    distance += measureDistance();
  }

  if(distance/3 <= DistanceON_Value)
    return true;

  return false;
}
//----------------------------------------

//Função pro LDR
bool LDRVal(){
  int val;
  val=analogRead(LDR);

  if (val<999){
    day = true;
  }
  else if (val>=999){
    day = false;
  }
}
//----------------------------------------
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
  int score = 0;
  int combo = 0;
  char FilePath[20];

  GameMap(){layout = new Arrow[MAX_ARROWS];}  // construtor vazio

  void loadFromFile(char* path) {
    // Create a buffer for the full filename
    char filename[20];
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
    CurrentTime = 0;

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
    /*
    strcat(FilePath, "/SONG"); //Modifies file path! (we don't need the old one since evrything was loaded anad sincne this is an attribute we can use it in play() )
    playMelody(FilePath, true); //não esquecer de resetar a musica!
    */

    //Antes de começar fazer uam contagem decrescente
    TFTscreen.setRotation(0);
    TFTscreen.setTextSize(2);
    TFTscreen.stroke(255,255,255);
    for(int i=3; i>0; i--){
      TFTscreen.text((String(i)).c_str(), 60, 55);
      delay(1000);
      TFTscreen.fillRect(50, 0, 20, 100, 0x0000); //Apagar texto
    }
    TFTscreen.setRotation(2);
    StartTime = millis();
  }

  void play(){
     if (!playing) return; 

    //playMelody(FilePath); //Reset é falso por default;

    if(duration <= CurrentTime){
        playing = false;
        return;
    }
      
    //Ler valores dos Pads entre frames e usar OR pra preservar o valor se houvver pelo menos um true (usar ground comum entre o esp e o arduino pra reduzir ruido)
    Pads.left  |= (debouncedPadInput(PAD_LEFT));
    Pads.up    |= (debouncedPadInput(PAD_UP));
    Pads.down  |= (debouncedPadInput(PAD_DOWN));
    Pads.right |= (debouncedPadInput(PAD_RIGHT)); 


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

      //Ler os Inputs do jogador (Tenho que usar debounce porque o loop vai executar rapidamente)

      if (Pads.left) {
          char result = processInput(LEFT);
          ScoresHandle(result);
      }
      if (Pads.up) {
          char result = processInput(UP);
          ScoresHandle(result);
      }
      if (Pads.down) {
          char result = processInput(DOWN);
          ScoresHandle(result);
      }
      if (Pads.right) {
          char result = processInput(RIGHT);
          ScoresHandle(result);
      }
    
      // Resetar valores dos Pads (no final do frame)
      Pads.left  = false;
      Pads.up    = false;
      Pads.down  = false;
      Pads.right = false;
    
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
    switch(result) {
        case HIT_EXCELLENT:
            score += 100 + (combo*0.05);
            combo++;
            Serial.println("EXCELLENT!");
            break;
        case HIT_GOOD:
            score += 50 + (combo*0.05);
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

//----Misc Functions (Mainly for code encapsulation)-------
bool DetectUserInput(){


}

void Menu(){ 
  for(int i=0; i<5; i++){
    String test = (day == true) ? String("DEECDAY/")+String(i)+String(".bmp") : String("DEECN/")+String(i)+String(".bmp");
    ImageToScreen(0, 0, test.c_str());
  }
}

int MapSelector(){ //retorna o indice do nome do mapa escolhido
  int current = 0;


}



GameMap CurrentMap;

void setup(){
  Serial.begin(9600);

  //----Inicializar pinos---
  pinMode(PUSH_LEFT, INPUT_PULLUP);
  pinMode(PUSH_RIGHT, INPUT_PULLUP);
  pinMode(PAD_LEFT, INPUT);
  pinMode(PAD_UP, INPUT);
  pinMode(PAD_DOWN, INPUT);
  pinMode(PAD_RIGHT, INPUT);
  
  //Configuração inicial do ecra
  TFTscreen.begin();
  TFTscreen.background(0, 0, 0);
  TFTscreen.setRotation(2);

  //Ligar o SD
  SD.begin(sd);
  
  //Carregar nome dos mapas disponiveis
  loadMapNames();
}

void blockTillInput(){
  do{delay(10);}while( !(debouncedMenuInput(PUSH_LEFT) || debouncedMenuInput(PUSH_RIGHT) || DistanceSensorOn()) );
}

void loop(){
    LDRVal();

    //----MENU/INTRO----------
    Menu();
    
    //Esperar por um botão para prosseguir o codigo (criar uma função depois)
    blockTillInput();

    //-----SELEÇÃO DE MAPA-----

    TFTscreen.background(0, 0, 0);
    TFTscreen.setRotation(0);
    TFTscreen.stroke(255,255,255);
    TFTscreen.setTextSize(2);
    int i = 0;
    do{
      TFTscreen.text(MapNames[i], 40, 55);
      TFTscreen.fillRect(0, 0, 180, 128, 0x0000); //Apagar texto
      if(debouncedMenuInput(PUSH_RIGHT)){ //depois falta meter o input debounce..
        i = (i == MapsCount-1)? MapsCount-1 : i+1;
      }
      if(debouncedMenuInput(PUSH_LEFT)){
        i = (i == 0)? 0 : i+1;
      }
    }while(!DistanceSensorOn());
    TFTscreen.setRotation(2);
    i=1;
    char map_path[15] = "/MAPS/";
    strcat(map_path, MapNames[i]);          // concatena

    //----JOGO (carregar o mapa escolhido, inicializar mapa e jogar mapa)---------
    CurrentMap.loadFromFile(map_path);
    //After selecting the map need to map.begin() and then on a loop do map.play()

    CurrentMap.begin();

    //Talvez incluir uma contagem de 3, 2, 1 ... GO (no fimd e carregar a imagem e as setas branca [no begin])

    while(CurrentMap.playing){
      CurrentMap.play();
    }
    
    //---END SCREEN---- (score e assim)
    
    TFTscreen.background(0, 0, 0);
    TFTscreen.setRotation(0);
    TFTscreen.stroke(255,255,255);
    TFTscreen.setTextSize(2);
    TFTscreen.text("GAME OVER!", 7, 5);
    TFTscreen.setTextSize(1);
    TFTscreen.text(("SCORE: " + String(CurrentMap.score)).c_str(), 45, 55);
    TFTscreen.setRotation(2);
    
    
    //Esperar por um botão para prosseguir o codigo (criar uma função depois)
    blockTillInput();
}
