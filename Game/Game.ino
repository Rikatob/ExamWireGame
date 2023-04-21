// EKSAMEN EMBEDDED SYSTEMS

/*
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
  GAME_OVER,

}
#define WIRE_PIN 8
#define BUZZER_PIN 7


// VARIABLES
State currentState;

void setup() {
  // state should start as idle.
  currentState = State.IDLE;

  Serial.begin(9600);
  // Use pullup to get less components on the board,
  // estimated 20 - 50 K resistor in arduino,
  // if i want to have more control later on i can use pulldown and and add the chosen resistor.
  pinMode(WIRE_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
}

void loop() {

  switch(currentState){
    case State.IDLE:
      // run function idle.
      break;
    case State.GAME:
      // run Game function.
      break;
    case State.GAME_OVER:
      // Run game over function.
      break;
  }
  int wireState = digitalRead(WIRE_PIN);
  int buzzerState = !wireState;
  digitalWrite(BUZZER_PIN, buzzerState);
  /*
  if (wireState == LOW) {
    digitalWrite(BUZZER_PIN, HIGH);
    Serial.print("Inside: ");
    Serial.println(wireState);
  }
  else {
    digitalWrite(BUZZER_PIN, LOW);
  }
  */
  Serial.println(wireState);
}
