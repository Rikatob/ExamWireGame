//
// Created by rikat on 4/24/2023.
//

#ifndef EXAMEMBEDDED_MAIN_H
#define EXAMEMBEDDED_MAIN_H

#include <Arduino.h>
#include <RtcDS3231.h>
/*
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <SD.h>
#include <TMRpcm.h>
*/
//////////////////////////////////////////////////////Defines//////////////////////////////////////////////////////
#define DONE 2
#define NOT_DONE 3

// GamePins
#define WIRE_PIN 5
#define BUZZER_PIN 12
#define BTN_OK_PIN 6
#define BTN_UP_PIN 9
#define BTN_DOWN_PIN 10

#define GOAL_PIN 11
// TFT
#define SD_CS 16
#define TFT_CS 8
#define TFT_RST 14
#define TFT_DC 15
#define DEFAULT_TEXT_SIZE 3
#define GAME_DURATION 30
#define HIGH_SCORE_TABLE_SIZE 3
/*Macro for length of array in printDateTime*/
#define ArraySize(a) (sizeof(a) / sizeof(a[0]))

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
    GAME_COMPLETE,
    GAME_DIFFICULTY,
    ENTER_HIGHSCORE,
    PRINT_HIGHSCORE
};

enum Difficulty {
    EASY = 3,
    MEDIUM = 2,
    HARD = 1
};

typedef struct _HighScoreEntry {
    char initials[4]; // "AAA" + zero terminator.
    byte time;
} HighScoreEntry;

enum State currentState = IDLE; // Default state as idle.
enum Difficulty difficulty = EASY; // default difficulty easy.
bool stateChanged = true; // Set it to true so the first time idle runs the "setup" for idle state.
byte debounceDuration = 150; // Used to handle the "bouncing" effect of button.
byte playerLives = 0;
byte highScoreEntriesCount = 0;
byte currentPos = 0;
unsigned long lastTimeButtonWasPressed = 0;
unsigned long previousTime = 0;
byte timeGoneBy = 0;
byte timeLeft = 0;
uint16_t textColor = 0;
char gameBuffer[20] = {0};
char pzAsciArrow[2] = {(char) -81, '\0'};
HighScoreEntry highScoreEntries[HIGH_SCORE_TABLE_SIZE];
//unsigned long currentTime = 0;
int gameDuration = 20;

//////////////////////////////////////////////////Function prototypes//////////////////////////////////////////////////

void Idle();

void Game();

void GameOver();

void GameComplete();

byte CheckButton(byte buttonPin);

void TftInitiate();

void CalibrateRtc();

void DrawText(const char *text, uint16_t color, byte size, byte x, byte y, bool clearScreen);

void printDateTime(const RtcDateTime &dt, uint8_t cursorX, uint8_t cursorY);

void PrintStartMenu();

void PrintDifficultyMenu();

void GameDifficulty();

void EnterHighscore();

int EnterInitials();

void UpdateHighScore(char *initials, byte time);

void AddHighScoreEntry(int index, char *initials, byte time);

void MakeSpaceForHighScoreEntry(int indexToReplace);

void PrintHighScoreTable();

void WriteEntriesToRtcMemory();

void ReadEntriesFromRtcMemory();

void ResetGameBuffer();

void MoveUpInMenu();

void MoveDownInMenu();

#endif //EXAMEMBEDDED_MAIN_H
