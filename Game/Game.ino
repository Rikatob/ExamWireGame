// EKSAMEN EMBEDDED SYSTEMS

/*
<<<<<<<<SPI>>>>>>>>>
CS    -> 10 ( CHIP SELECT TFT)
CS    -> 6 (CHIP SELECT RFID) 
MOSI  -> 11 ( MASTER IN SLAVE OUT)
MISO  -> 12 ( MASTER IN SLAVE OUT)
SCK   -> 13 ( CLOCK SPEED)
*/

/////////////////////////////////////////////////////Libraries/////////////////////////////////////////////////////
/*
<<<<<<<<<<<<<<<<<<<< ST7789 >>>>>>>>>>>>>>>>>>>>
*/
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

//////////////////////////////////////////////////////Defines//////////////////////////////////////////////////////
#define WIRE_PIN 2
#define BUZZER_PIN 3
#define GAME_BUTTON_PIN 4
#define GOAL_PIN 5

// TFT
#define TFT_CS 10
#define TFT_RST 9
#define TFT_DC 8
#define DEFAULT_TEXT_SIZE 3

///////////////////////////////////////////////////Objects/Inits///////////////////////////////////////////////////
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

/////////////////////////////////////////////////////Variables/////////////////////////////////////////////////////
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
  GAME_OVER,
  GAME_COMPLETE
};

enum State currentState;
// Set it to true so the first time idle runs the "setup" for idle state.
bool stateChanged = true;

// Used to handle the "bouncing" effect of button.
unsigned long debounceDuration = 150;
unsigned long lastTimeButtonWasPressed = 0;



void setup() {
  // State should start as idle.
  currentState = IDLE;
  Serial.begin(9600);

  //<BASE>
  // Use pullup to get less components on the board,
  // estimated 20 - 50 K resistor in arduino,
  // if i want to have more control later on i can use pulldown and and add the chosen resistor.
  pinMode(WIRE_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(GAME_BUTTON_PIN, INPUT_PULLUP);
  pinMode(GOAL_PIN, INPUT_PULLUP);
  digitalWrite(BUZZER_PIN, LOW);
  //</BASE>
  TftInitiate();
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
    case GAME_COMPLETE:
      GameComplete();
      break;
  }
}

void Idle() {
  if (stateChanged) {
    DrawText("GAME IN IDLE MODE!", ST77XX_RED, DEFAULT_TEXT_SIZE, 15, 40, true);
    //Serial.println("GAME IN IDLE MODE");
    stateChanged = false;
  }
  byte buttonPressed = CheckButton(GAME_BUTTON_PIN);

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
    DrawText("GAME STARTED!", ST77XX_RED, DEFAULT_TEXT_SIZE, 5, 55, true);
    //Serial.println("GAME STARTED!");
    stateChanged = false;
  }
  int wireState = digitalRead(WIRE_PIN);
  int goalPinState = digitalRead(GOAL_PIN);
  if (wireState == LOW) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);
    currentState = GAME_OVER;
    stateChanged = true;
  }
  if (goalPinState == LOW) {
    currentState = GAME_COMPLETE;
    stateChanged = true;
  }
}

/*

*/
void GameOver() {
  if (stateChanged) {
    DrawText("GAME OVER!", ST77XX_RED, DEFAULT_TEXT_SIZE, 25, 55, true);
    //Serial.println("GAME OVER!");
    digitalWrite(BUZZER_PIN, LOW);
    stateChanged = false;
  }

  byte buttonPressed = CheckButton(GAME_BUTTON_PIN);
  if (buttonPressed) {
    currentState = IDLE;
    stateChanged = true;
  }
}

// REEEEEEEEEEEEEEEEEEEEEEEEEEWORK NEEEEEEEEEEEDED!!!!
void GameComplete() {
  if (stateChanged) {
    DrawText("GAME WON!", ST77XX_RED, DEFAULT_TEXT_SIZE, 25, 55, true);
    stateChanged = false;
  }


  delay(2000);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  digitalWrite(BUZZER_PIN, LOW);

  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  digitalWrite(BUZZER_PIN, LOW);

  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN, LOW);


  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  digitalWrite(BUZZER_PIN, LOW);


  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  digitalWrite(BUZZER_PIN, LOW);
}

int CheckButton(int buttonPin) {
  if (millis() - lastTimeButtonWasPressed > debounceDuration) {
    byte buttonState = digitalRead(buttonPin);
    if (!buttonState) {
      lastTimeButtonWasPressed = millis();
      return true;
    }
  }
  return false;
}


void TftInitiate() {
  // Reset the TFT to get a "clean" start.
  digitalWrite(TFT_RST, HIGH);
  digitalWrite(TFT_RST, LOW);

  // Initiate ST7789 1.14" 240x135.
  tft.init(135, 240);
  // Set start point to black screen.
  tft.fillScreen(ST77XX_BLACK);
  // Rotate screen to show widesreen where 0,0 is top 135 line and left 240 line.
  tft.setRotation(1);
}

// Print text to the screen in a certain color('RGB565 16bit' "ST77XX_<color>" pre-defined from lib), size, position(x=COLUMN, y=ROW) and clearing screen.
void DrawText(char* text, uint16_t color, uint8_t size, uint8_t x, uint8_t y, bool clearScreen) {
  if (clearScreen) {
    tft.fillScreen(ST77XX_BLACK);
  }
  tft.setCursor(x, y);
  tft.setTextColor(color);
  tft.setTextSize(size);
  tft.setTextWrap(true);
  tft.print(text);
}
