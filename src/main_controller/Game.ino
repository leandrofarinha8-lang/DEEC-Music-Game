//IMPORTANT NOTE: Most functions don't have error handling (mainly the ones related to the files) since every byte is valuable on arduino

#include <SD.h> //O SD.h tem que ser incluido antes do TFT.h para poder usar PImage
#include <TFT.h> // Arduino LCD library
#include <SPI.h>

//--TFT Pins--
#define cs 10
#define dc 8
#define rst 9
#define sd 4
//------------------------------

//----Arrows stuff-----
#define ARROW_WIDTH 15
#define ARROW_HEIGHT 20

//Posição horizonntal das setas
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

#define MAX_ARROWS 3 // Já não é o numero maximo de setas por mapa mas sim o numero maximo de setas EM SIMULTANEO no ecra
// Removi MAX_SONG (apenas usava isto quando carregava a musica toda de memoria, agora carrego gradualmente)

//Pads
#define PAD_LEFT A0 //é o 3 do Receiver
#define PAD_UP A1 //é o 4 do Receiver
#define PAD_DOWN A2 //é o 5 do Receiver
#define PAD_RIGHT A3 //é o 6 do Receiver

//Pinos para o sensor de distancia
#define echo 5
#define trigger 6
#define DistanceON_Value 10 //Distancia para a qual o sensor de distancia se 'ativa'

//Pinos do LDR
#define LDR A4

//Pino do buzzer
#define BUZZER 7

//----Para carregar nomes de mapas para uma array (não abusar na memoria se não as imagens podem falhar)-----
#define MAX_MAPS 2
#define MAX_NAME_LENGTH 13 //O nº de chars permitidos no SD é de 8 para o nome e 3 pra extensão (13 é suficiente para 8 chars de nome + '.' + 3 chars de extensão + '/0')

char MapNames[MAX_MAPS][MAX_NAME_LENGTH];
int MapsCount = 0;

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
  //Neste ponto todos os (nomes dos) mapas lidos estão na array global, depois no menu é apenas usar os botões pra alterarem o index e o sensor de distancia pra escolher o mapa
  dir.close();
}

//------------------------------------------------------------

TFT TFTscreen = TFT(cs, dc, rst);
bool day = true; //É usado posteriormente para implementar o Dark Mode

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

//-------Pad debounce (apenas responde a TRANSIÇÕES)----
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

  if(currentState == HIGH && lastPadState[index] != currentState && (currentTime - lastPadPressTime[index]) > 10){ //Vê a transição para HIGH
      lastPadPressTime[index] = currentTime;
      lastPadState[index] = currentState;
      return true;
  }

  lastPadState[index] = currentState;
  return false;
}
//Nota: não vou juntar com a outra função porque os 'botões' em cima nnão têm PULL_UP (a logica é normal) e os de baixo tem PULL_UP (Logica invertida)
//-----------Menu Buttons (nnão está junto com os Pads porque aqui usamos INTPUT PULL_UP)---------------
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

    if (currentState == LOW && lastMenuState[index] != currentState && (currentTime - lastMenuPressTime[index]) > 10) {
        lastMenuPressTime[index] = currentTime;
        lastMenuState[index] = currentState;
        return true;
    }

    lastMenuState[index] = currentState;
    return false;
}
//--------------------------------------
//----TFT Related functions--------------
bool ImageToScreen(int x, int y, char * name){
  //Assumes TFT.begin() was done

  PImage img = TFTscreen.loadImage(name); //name ex: "DEEC.bmp"
  
  if (!img.isValid()) { //Problems with image format
    Serial.println(F("failed image format!"));
    return false; 
  }
  TFTscreen.image(img, x, y);

  img.close(); //FINALMENTE RESOLVIDO (isto evita uma memory leak que causava com que as imagem parecem de carregar passado algumas iterações)
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

//Função pro LDR (substitui por void em vez de bool porque não retorna nada)
void LDRVal(){
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
Arrow Permanent(LEFT, 0, 0);

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
  int TotalArrows; // Total de setas no mapa
  int score = 0;
  int combo = 0;
  char FilePath[20];
  File songFile;
  File mapFile;
  unsigned long lastNoteTime = 0;
  int noteFreq = 0, noteDur = 0;
  int currentArrowIndex = 0; //Indice da proxima seta a carregar

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

    // ---Name--- (não estou a usar name pra nada ainda mas vou deixar por agora)
    name = file.readStringUntil('\n');
    name.trim();
    Serial.println(name);

    // ---Duration---
    String durationStr = file.readStringUntil('\n');
    durationStr.trim();
    duration = durationStr.toInt();

    // ---Arrow Num---
    String arrowNumStr = file.readStringUntil('\n');
    arrowNumStr.trim();
    TotalArrows = arrowNumStr.toInt();
    
    ArrowNum = 0; // Começa com 0 setas carregadas
    score = 0;
    combo = 0;
    currentArrowIndex = 0;
    
    file.close();
  }

  
  void begin() {
    //---Criar ponto de acesso ao ficheiro da musica---
    char songPath[16];
    strcpy(songPath, FilePath);
    strcat(songPath, "/SONG");

    songFile = SD.open(songPath);
    
    // Abrir arquivo do mapa para leitura progressiva
    char mapPath[20];
    strcpy(mapPath, FilePath);
    strcat(mapPath, "/MAP");
    mapFile = SD.open(mapPath, FILE_READ);
    
    // Saltar informação que não correspnde a setas (já foi carregada no mapLoad)
    mapFile.readStringUntil('\n'); // nome
    mapFile.readStringUntil('\n'); // duração
    mapFile.readStringUntil('\n'); // número total de setas
    
    lastNoteTime = millis();
    noteFreq = 0;
    noteDur = 0;
    noTone(BUZZER); //O buzzer as vezes dava erro, isto já resolve isso

    LastUpdate = 0;
    playing = true;
    CurrentTime = 0;

    //Obter background do nivel dependendo se é de dia ou noite
    char filename[20];
    strcpy(filename, FilePath);

    if(day){
      strcat(filename, "/D.bmp");
    }
    else{
      strcat(filename, "/N.bmp");
    }

    //Carregar background do nivel
    ImageToScreen(0, 0, filename);

    //Antes de começar fazer uma contagem decrescente
    TFTscreen.setRotation(0);
    TFTscreen.setTextSize(2);
    TFTscreen.stroke(255,255,255);
    for(int i=3; i>0; i--){
      TFTscreen.text((String(i)).c_str(), 60, 55);
      delay(1000);
      TFTscreen.fillRect(50, 0, 20, 100, 0x0000); //Apagar texto
    }
    TFTscreen.setRotation(2);

    //Obter o timestamp do começo do mapa
    StartTime = millis();
  }

  void loadNextArrows() {
    // Carrega novas setas (apenas se houver espaço pra isso)
    while (ArrowNum < MAX_ARROWS && currentArrowIndex < TotalArrows && mapFile.available()) {
      String line = mapFile.readStringUntil('\n');
      line.trim();

      if (line.length() == 0) continue;

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

      layout[ArrowNum] = Arrow(type, startTime, endTime);
      ArrowNum++;
      currentArrowIndex++;
    }
  }

  void removeArrow(int index) {
    // Remove seta do array e move as que sobram deixando o indice 0 livre
    for (int i = index; i < ArrowNum - 1; i++) {
      layout[i] = layout[i + 1];
    }
    ArrowNum--;
  }

  void play(){
   unsigned long now = millis();
    if (!playing) return;

    //Ler valores dos Pads entre frames e usar OR pra preservar o valor se houvver pelo menos um true (usar ground comum entre o esp e o arduino pra reduzir ruido)
    Pads.left  |= (debouncedPadInput(PAD_LEFT));
    Pads.up    |= (debouncedPadInput(PAD_UP));
    Pads.down  |= (debouncedPadInput(PAD_DOWN));
    Pads.right |= (debouncedPadInput(PAD_RIGHT)); 

  //-----Musica-----
  if (now - lastNoteTime >= (unsigned long)noteDur) {
    noTone(BUZZER);

    int c;
    int value = 0;

    // Teste com uma 'Lambda' para declarar uma função aqui dentro e poder usar as variaveis c e value dentro da função (é meio esquisito mas funciona bem)
    // ver [https://en.cppreference.com/w/cpp/language/lambda.html ]
    auto readNum = [&]() { //Tive que lever valores à 'moda antiga' para poupar algum espaço (c lê o caracter, value termina com o valor numerico)
        value = 0;
        // Ver se o byte é um numero (todos os numeros são menos que o valor ASCII de '0')
        while (songFile.available() && ( (c = songFile.read()) < '0' )) {}
        // acumular número
        value = c - '0';
        while (songFile.available() && ( (c = songFile.read()) >= '0' ))
            value = value * 10 + (c - '0');
        return value;
    };

    if (!songFile.available()) { //Loop em caso de chegar ao fim antes do final da duração do mapa
        songFile.seek(0);
    }

    //Le os valores ATUAIS de frequencia e duração
    noteFreq = readNum();
    noteDur  = readNum();

      if (noteFreq > 0) { //pausa caso freq==0
        tone(BUZZER, noteFreq);
      } 
      else {
        noTone(BUZZER); 
      }
      lastNoteTime = now;
    } 
    //-------------------------------

    if(duration <= CurrentTime){ //Verificação de final de jogo
        playing = false;
        songFile.close();
        mapFile.close();
        return;
    }

    //-------------Frame------------
    if(millis() - LastUpdate > 25){
      LastUpdate = millis();
      CurrentTime = millis() - StartTime;

      // Carregar novas setas se necessário
      loadNextArrows();

      // Atualiza todas as setas (carregadas)
      for (int i = 0; i < ArrowNum; i++) {
          Arrow &currentA = layout[i];

          // Atualiza posição
          if (CurrentTime >= currentA .startTime && CurrentTime <= currentA .endTime) {
              currentA.update(CurrentTime);
          }

          //Se a seta passou da zona de acerto e ainda está visível é um MISS
          if (!currentA.missed && currentA.visible && currentA.y > HIT_Y + 15 && currentA.y > 10) { //A ultima condição é para evitar um bug
              currentA.eraseCurrent(); //ERASE APENAS É POSSIVEL QUANDO A SETA ESTÁ VISIVEL!!! (daí isto aparecer logo no inicio)
              currentA.missed = true;
              currentA.visible = false;
              Serial.println("MISS");
              combo = 0; // resetar combo
              Serial.print("Score: "); 
              Serial.println(score);
              Serial.print("Combo: "); 
              Serial.println(combo);
              
              // Se a seta foi falhada ela é removida da array
              removeArrow(i);
              i--; // Ajustar indice 
          }
      }

      //Desenha as setas permanentes no sitio dos acertos
      Permanent.type = LEFT;
      Permanent.x = ARROW_LEFT_X;
      Permanent.drawColor(0xFFFF,HIT_Y);
      Permanent.type = RIGHT;
      Permanent.x = ARROW_RIGHT_X;
      Permanent.drawColor(0xFFFF,HIT_Y);
      Permanent.type = UP;
      Permanent.x = ARROW_UP_X;
      Permanent.drawColor(0xFFFF,HIT_Y);
      Permanent.type = DOWN;
      Permanent.x = ARROW_DOWN_X;
      Permanent.drawColor(0xFFFF,HIT_Y);

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

  // Função para processar os inputs dos Pads
  char processInput(char key) {
    Arrow* closestArrow = nullptr;
    int closestDist = 9999;
    int closestIndex = -1;

    for (int i = 0; i < ArrowNum; i++){ //encontrar a seta mais proxima do HIT_Y
      if (layout[i].visible && !layout[i].missed && layout[i].type == key){ //apenas vê setas visiveis (e do tipo pressionado)
         int diff = abs(layout[i].y - HIT_Y);
          if (diff < closestDist){
            closestDist = diff;
            closestArrow = &layout[i];
            closestIndex = i;
          }
      }
    }

    if (closestArrow){ //Encontrando a seta mais proxima do tipo correspondente temos que ver se foi acerto, erro, etc
      closestArrow->eraseCurrent();
      closestArrow->visible = false;
      closestArrow->missed = true;//not missed but this stops updates (é como se a seta contasse como 'já acertada')

      // Remove seta acertada (ou falhada) porque já não está a fazer nada na array
      if (closestIndex != -1) {
        removeArrow(closestIndex);
      }

      if (closestDist <= HIT_EXCELLENT) return HIT_EXCELLENT; //preciso de meter o combo ao barulho...
      else if (closestDist <= HIT_GOOD) return HIT_GOOD;
      else return HIT_MISS;
    }

    return HIT_MISS; // Se não há nenhuma seta do tipo correto conta como miss há mesma
  }

  void ScoresHandle(char result) { //Calculo do score e atualização do combo
    //Prints para debug (e talvez até deixemos assim por causa do requisito de serial communication)
    switch(result) {
        case HIT_EXCELLENT:
            score += 100 + (combo*5);
            combo++;
            Serial.println("EXCELLENT!");
            break;
        case HIT_GOOD:
            score += 50 + (combo*5);
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
  pinMode(echo, INPUT);
  pinMode(trigger, OUTPUT);
  pinMode(LDR, INPUT);
  pinMode(BUZZER, OUTPUT);

  //Configuração inicial do ecra
  TFTscreen.begin();
  TFTscreen.background(0, 0, 0);
  TFTscreen.setRotation(2);

  //Ligar o SD
  SD.begin(sd);
  
  //Carregar nome dos mapas disponiveis
  loadMapNames();
}

void blockTillInput(){ //Uso duas vezes isto, uma função acaba por poupar espaço e ser mais legivel
  do{delay(10);}while( !(debouncedMenuInput(PUSH_LEFT) || debouncedMenuInput(PUSH_RIGHT) || DistanceSensorOn()) );
}

void loop(){
    //NOTA PARA OTIMIZAÇÃO: Substituir ints que assumem apenas valores pequenos por bytes!
    //Já não é necessário pois as imagens já têm espaço pra carregar (se não houver erros tá terminado!)
    LDRVal();

    //----MENU/INTRO----------
    //Substituido por chars em vez de arrays pra poupar espaço
    char filename[13] = "DEECDAY/0.bmp";
    const char* prefix_day = "DEECDAY/";
    const char* prefix_night = "DEECN/";

    for(byte i=0; i<5; i++){
      // Copy appropriate prefix
      const char* prefix = day ? prefix_day : prefix_night;
      byte prefix_len = day ? 8 : 7;
      memcpy(filename, prefix, prefix_len);
    
      // Add number
      filename[prefix_len] = '0' + i;
    
      // Add extension
      strcpy(filename + prefix_len + 1, ".bmp");
    
      ImageToScreen(0, 0, filename);
  }
    
    //Esperar por um botão para prosseguir o codigo (criar uma função depois)
    blockTillInput();
    
    //-----SELEÇÃO DE MAPA-----
    byte i=0;
    TFTscreen.background(0, 0, 0);
    TFTscreen.setRotation(0);
    TFTscreen.stroke(255,255,255);
    TFTscreen.setTextSize(2);
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
    
    i=1; //PARA TESTE!!!!!! REMOVER QUANDO ESTIVER MONTADO
    char map_path[15] = "/MAPS/";
    strcat(map_path, MapNames[i]);          // concatena

    //----JOGO (carregar o mapa escolhido, inicializar mapa e jogar mapa)---------
    CurrentMap.loadFromFile(map_path);
    //After selecting the map need to map.begin() and then on a loop do map.play()

    CurrentMap.begin();

    while(CurrentMap.playing){
      CurrentMap.play();
    }
    noTone(BUZZER); //Força o buzzer a desligar
    
    //---END SCREEN---- (score e assim)
    TFTscreen.background(0, 0, 0);
    TFTscreen.setRotation(0);
    TFTscreen.stroke(255,255,255);
    TFTscreen.setTextSize(2);
    TFTscreen.text("GAME OVER!", 7, 5);
    TFTscreen.setTextSize(1);
    TFTscreen.text(("SCORE: " + String(CurrentMap.score)).c_str(), 45, 55);
    TFTscreen.setRotation(2);
    
    //Esperar por um botão para prosseguir o codigo
    blockTillInput();
}
