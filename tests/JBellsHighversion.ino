#define BUZZER 8

// Frequências da melodia (0 = pausa)
/*Notas: case 'c': tone=1915; break;
 case 'd': tone=1700; break;
 case 'e': tone=1519; break;
 case 'f': tone=1432; break;
 case 'g': tone=1275; break;
 case 'a': tone=1136; break;
 case 'b': tone=1014; break;
 case 'C': tone= 956; break;

 E,E,E2,E,E,E2,E,G,C2,D,E4,F,F,F,F,F,E,E,E0.5,E0.5,E,D,D,E,D2
 G2,E,E,E2,E,E,E2,E,G,C2,D,E4,F,F,F,F,F,E,E,E0.5,E0.5,G,G,F,D,C4
*/

int crazy_frog_melody[] = {
    1432,1432,1432,1432,
    1432, 1432, 1432, 1700, 1136, 1275, 1432,
    1519, 1519, 1519, 1519, 1519, 1432, 1432,
    1432, 1432, 1432, 1275, 1275, 1432, 1275,1700,
    1432, 1432,
    1432, 1432, 1432, 1432, 1432, 1700,1136,
    1275, 1432, 1519, 1519, 1519, 1519, 1519,
    1432,1432,1432,1432, 1700, 1700, 1519, 1275,
   	1136
};

// Duração de cada nota em ms
/*E,E,E2,E,  ---4
  E,E2,E,G,C2,D,E4, ----7
  F,F,F,F,F,E,E, ----7
  E0.5,E0.5,E,D,D,E,D2,G2,--8
  E,E, --2
  E2,E,E,E2,E,G,C2, --7
  D,E4,F,F,F,F,F, ---7
  E,E,E0.5,E0.5,G,G,F,D, --8
  C4--1
*/
int crazy_frog_durations[] = {
    300 ,300,600,300, //----4
    300, 600, 300, 300, 600, 300, 1000,//---7
    300, 300, 300, 300, 300, 300, 300,//---7
    150, 150, 300, 300, 300, 300, 600, 600,//---8
    300, 300,//---2

    600, 300, 300, 600, 300, 300, 600,//---7
    300, 1000, 300, 300, 300, 300, 300,//---7C 
    300, 300, 150, 150, 300, 300, 300, 300,//---8
    1000//---2


};


// Variáveis para controlo de timing
unsigned long previousTime = 0;
int currentNote = 0;
int totalNotes;

void setup() {
  pinMode(BUZZER, OUTPUT);
  totalNotes = sizeof(crazy_frog_melody) / sizeof(crazy_frog_melody[0]);
}

void loop() {
  unsigned long currentTime = millis();
  
  // Verifica se é hora de passar para a próxima nota
  if (currentTime - previousTime >= crazy_frog_durations[currentNote]) {
    // Para a nota atual antes de passar para a próxima
    noTone(BUZZER);
    
    currentNote++;
    
    // Verifica se chegou ao final da melodia
    if (currentNote >= totalNotes) {
      currentNote = 0;
      delay(1000); // Pequena pausa antes de repetir (não bloqueante por muito tempo)
    }
    
    // Toca a próxima nota (se não for pausa)
    if (crazy_frog_melody[currentNote] != 0) {
      tone(BUZZER, crazy_frog_melody[currentNote], crazy_frog_durations[currentNote]);
    }
    
    previousTime = currentTime;
  }
}