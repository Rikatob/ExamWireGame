//
// Created by rikat on 4/24/2023.
//

#ifndef EXAMEMBEDDED_MAIN_H
#define EXAMEMBEDDED_MAIN_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <SD.h>
#include <TMRpcm.h>

//////////////////////////////////////////////////////Defines//////////////////////////////////////////////////////
#define WIRE_PIN 2
#define BUZZER_PIN 9
#define BTN_OK_PIN 4
#define BTN_UP_PIN A1
#define BTN_DOWN_PIN A0

#define GOAL_PIN 5
// TFT
#define SD_CS 7
#define TFT_CS 10
#define TFT_RST 3
#define TFT_DC 8
#define DEFAULT_TEXT_SIZE 3

///////////////////////////////////////////////////Objects/Inits///////////////////////////////////////////////////
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
TMRpcm tmrpcm;
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
byte debounceDuration = 150;
unsigned long lastTimeButtonWasPressed = 0;
unsigned long currentTime = 0;


//////////////////////////////////////////////////Function prototypes//////////////////////////////////////////////////

void Idle();
void Game();
void GameOver();
void GameComplete();
byte CheckButton(byte buttonPin);
void TftInitiate();
void DrawText(const char* text, uint16_t color, byte size, byte x, byte y, bool clearScreen);
void StartMenu();
void MoveUpInMenu(int *currentPos);
void MoveDownInMenu(int *currentPos);
#endif //EXAMEMBEDDED_MAIN_H
