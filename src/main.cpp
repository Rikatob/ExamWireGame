
// EKSAMEN EMBEDDED SYSTEMS

/*
<<<<<<<<SPI>>>>>>>>>
CS    -> 10 ( CHIP SELECT TFT)
CS    -> 7 (CHIP SELECT SD)
MOSI  -> 11 ( MASTER OUT SLAVE IN)
MISO  -> 12 ( MASTER IN SLAVE OUT)
SCK   -> 13 ( CLOCK SPEED)
*/

/////////////////////////////////////////////////////Libraries/////////////////////////////////////////////////////
/*
<<<<<<<<<<<<<<<<<<<< ST7789 >>>>>>>>>>>>>>>>>>>>
*/
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <SD.h>
#include <TMRpcm.h>
#include "main.h"


void setup() {
    // State should start as idle.
    currentState = IDLE;
    Serial.begin(9600);
    currentTime = millis();
    //<BASE>
    // Use pullup to get less components on the board,
    // estimated 20 - 50 K resistor in arduino,
    // if i want to have more control later on i can use pulldown and and add the chosen resistor.
    pinMode(WIRE_PIN, INPUT_PULLUP);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(BTN_OK_PIN, INPUT_PULLUP);
    pinMode(BTN_UP_PIN, INPUT_PULLUP);
    pinMode(BTN_DOWN_PIN, INPUT_PULLUP);
    pinMode(GOAL_PIN, INPUT_PULLUP);
    digitalWrite(BUZZER_PIN, LOW);
    //</BASE>
    TftInitiate();
    //PMC
    tmrpcm.speakerPin = BUZZER_PIN;
    if (!SD.begin(SD_CS)) {
        Serial.println("SD FAILED TO INIT..");
        return;
    }
    tmrpcm.setVolume(5);
    tmrpcm.quality(true);
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

    static int currentPos;

    if (stateChanged) {
        currentPos = 11;
        StartMenu();
        stateChanged = false;
    }

    if (!tmrpcm.isPlaying()) {
        //tmrpcm.play("test.wav");              // TODO NEED TO CHANGE THIS SOUND
    }

    byte okBtnPressed = CheckButton(BTN_OK_PIN);
    byte upBtnPressed = CheckButton(BTN_UP_PIN);
    byte downBtnPressed = CheckButton(BTN_DOWN_PIN);
    // Ok button pressed -> Do what user have chosen.
    if (okBtnPressed) {

        // Start game.
        if (currentPos == 11) {
            tmrpcm.stopPlayback();
            currentState = GAME;
            stateChanged = true;

            // Print high-score.
        } else if (currentPos == 41) {
            DrawText("HIGHSCORE TABLE WILL BE HERE!", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 5, 0, true);
        }

    }
    if (upBtnPressed) {
        MoveUpInMenu(&currentPos);
    }
    if (downBtnPressed) {
        MoveDownInMenu(&currentPos);
    }
}

/*
  Game is running.
  -> If wire is touched buzz sound will come and state will change to game over.
*/
void Game() {
    if (stateChanged) {
        DrawText("GAME STARTED!", ST77XX_RED, DEFAULT_TEXT_SIZE, 5, 55, true);
        if (!tmrpcm.isPlaying()) {
            tmrpcm.play("start.wav");
        }

        stateChanged = false;
    }
    byte wireState = digitalRead(WIRE_PIN);
    byte goalPinState = digitalRead(GOAL_PIN);
    if (wireState == LOW) {
        tmrpcm.stopPlayback();
        currentState = GAME_OVER;
        stateChanged = true;
    }
    if (goalPinState == LOW) {
        tmrpcm.stopPlayback();
        currentState = GAME_COMPLETE;
        stateChanged = true;
    }
}

/*

*/
void GameOver() {
    if (stateChanged) {
        DrawText("GAME OVER!", ST77XX_RED, DEFAULT_TEXT_SIZE, 25, 55, true);
        if (!tmrpcm.isPlaying()) {
            tmrpcm.play("over.wav");
        }
        stateChanged = false;
    }

    byte buttonPressed = CheckButton(BTN_OK_PIN);
    if (buttonPressed) {
        tmrpcm.stopPlayback();
        currentState = IDLE;
        stateChanged = true;
    }
}

// REEEEEEEEEEEEEEEEEEEEEEEEEEWORK NEEEEEEEEEEEDED!!!!
void GameComplete() {

    if (stateChanged) {
        DrawText("GAME WON!", ST77XX_RED, DEFAULT_TEXT_SIZE, 25, 55, true);
        if (!tmrpcm.isPlaying()) {
            tmrpcm.play("complete.wav");
        }
        stateChanged = false;
    }


}

byte CheckButton(byte buttonPin) {
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
// 240x135 x and y goes up in a unsigned 8 bit int (byte macro)
void DrawText(const char *text, uint16_t color, byte size, byte x, byte y, bool clearScreen) {
    if (clearScreen) {
        tft.fillScreen(ST77XX_BLACK);
    }
    tft.setCursor(x, y);
    tft.setTextColor(color);
    tft.setTextSize(size);
    tft.setTextWrap(true);
    tft.print(text);
}


void StartMenu() {
    DrawText("Start game.", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 40, 10, true);
    DrawText("->", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 0, 11, false);
    DrawText("High-score.", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 40, 40, false);
}

void MoveUpInMenu(int *currentPos) {
    // If currentPos is 10, then it cant go up.
    if (*currentPos != 11) {
        DrawText("->", ST77XX_BLACK, DEFAULT_TEXT_SIZE, 0, *currentPos, false);
        *currentPos -= 30;
        DrawText("->", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 0, *currentPos, false);
    }
}

void MoveDownInMenu(int *currentPos) {

    DrawText("->", ST77XX_BLACK, DEFAULT_TEXT_SIZE, 0, *currentPos, false);
    *currentPos += 30;
    DrawText("->", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 0, *currentPos, false);
}