// EKSAMEN EMBEDDED SYSTEMS
#define WIRE_PIN 8
#define BUZZER_PIN 7
/*
        ENUM
IDLE 
-> Play idle music.
-> check if startGame button is pressed.

GAME 
-> Start timer.
-> Check wire. 

GAME_OVER 
-> Show score.
-> play game over music.
*/
enum State {
  IDLE,
  GAME,
  GAME_OVER
};


// VARIABLES
enum State currentState;

void setup() {
  // state should start as idle.
  currentState = IDLE;

  Serial.begin(9600);
  // Use pullup to get less components on the board,
  // estimated 20 - 50 K resistor in arduino,
  // if i want to have more control later on i can use pulldown and and add the chosen resistor.
  pinMode(WIRE_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  currentState = GAME;
}

void loop() {

  switch(currentState){
    case IDLE:
      Idle();
      break;
    case GAME:
      Game();
      break;
    case GAME_OVER:
      GameOver();
      break;
  }
  
}

void Idle(){

}

void Game(){
  int wireState = digitalRead(WIRE_PIN); 
  
  if (wireState == LOW) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);
    currentState = GAME_OVER;
  }
  else {
    digitalWrite(BUZZER_PIN, LOW);
  }
  
}
void GameOver(){
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("GAME OVER!");
}