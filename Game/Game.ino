// EKSAMEN EMBEDDED SYSTEMS
#define WIRE_PIN 2
#define BUZZER_PIN 3
#define GAME_BUTTON_PIN 4
/*
        STATE MACHINE
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
// Set it to true so the first time idle runs the "setup" for idle state.
bool stateChanged = true;

void setup() {
  // state should start as idle.
  currentState = IDLE;

  Serial.begin(9600);
  // Use pullup to get less components on the board,
  // estimated 20 - 50 K resistor in arduino,
  // if i want to have more control later on i can use pulldown and and add the chosen resistor.
  pinMode(WIRE_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(GAME_BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(BUZZER_PIN, LOW);
}

void loop() {

  switch (currentState) {
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

void Idle() {
  if (stateChanged) {
    Serial.println("GAME IN IDLE MODE");
    stateChanged = false;
  }

  int buttonState = digitalRead(GAME_BUTTON_PIN);

  // Game button pressed, then start game.
  if (!buttonState) {
    currentState = GAME;
    stateChanged = true;
    delay(100);
  }
}

/*
  Game is running.
  -> If wire is touched buzz sound will come and state will change to game over.
*/
void Game() {
  if (stateChanged) {
    Serial.println("GAME STARTED!");
    stateChanged = false;
  }
  int wireState = digitalRead(WIRE_PIN);

  if (wireState == LOW) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);
    currentState = GAME_OVER;
    stateChanged = true;
  } else {
    //digitalWrite(BUZZER_PIN, LOW);
  }
}

/*

*/
void GameOver() {
  if (stateChanged) {
    Serial.println("GAME OVER!");
    digitalWrite(BUZZER_PIN, LOW);
    stateChanged = false;
  }

  int buttonState = digitalRead(GAME_BUTTON_PIN);
  if (!buttonState) {
    currentState = IDLE;
    stateChanged = true;
    delay(100);
  }
}