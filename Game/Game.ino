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

// Used to handle the "bouncing" effect of button.
unsigned long debounceDuration = 150;
unsigned long lastTimeButtonWasPressed = 0;

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

  byte buttonPressed = CheckButton();

  // Game button pressed, then start game.
  if (buttonPressed) {
    currentState = GAME;
    stateChanged = true;
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

  byte buttonPressed = CheckButton();
  if (buttonPressed) {
    currentState = IDLE;
    stateChanged = true;
  }
}


int CheckButton() {
  if (millis() - lastTimeButtonWasPressed > debounceDuration) {
    byte buttonState = digitalRead(GAME_BUTTON_PIN);
    if (!buttonState) {
      lastTimeButtonWasPressed = millis();
      return true;
    }
  }
  return false;
}