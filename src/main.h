//
// Created by rikat on 4/24/2023.
//

#ifndef EXAMEMBEDDED_MAIN_H
#define EXAMEMBEDDED_MAIN_H

#include <Arduino.h>
#include <RtcDS3231.h>

//////////////////////////////////////////////////////Defines//////////////////////////////////////////////////////

// Game settings
#define GAME_DURATION 30
#define HIGH_SCORE_TABLE_SIZE 3

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

// Return codes
#define DONE 2
#define NOT_DONE 3

/*Macro for length of array in printDateTime*/
#define ArraySize(a) (sizeof(a) / sizeof(a[0]))

/////////////////////////////////////////////////////ENUMS/STRUCTS/////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////Variables/////////////////////////////////////////////////////
unsigned long lastTimeButtonWasPressed = 0;
unsigned long previousTime = 0;
enum State currentState = IDLE; // Default state as idle.
enum Difficulty difficulty = EASY; // default difficulty easy.
bool stateChanged = true; // Set it to true so the first time idle runs the "setup" for idle state.
char gameBuffer[20] = {0};
char pzAsciArrow[2] = {(char) -81, '\0'};
uint16_t textColor = 0;
byte debounceDuration = 150; // Used to handle the "bouncing" effect of button.
byte playerLives = 0;
byte highScoreEntriesCount = 0;
byte currentPos = 0;
byte timeGoneBy = 0;
byte timeLeft = 0;
HighScoreEntry highScoreEntries[HIGH_SCORE_TABLE_SIZE];

/**RTTTL format tunes REF: http://arcadetones.emuunlim.com/*/
const char *tetris = "tetris:d=4,o=5,b=160:e6,8b,8c6,8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6,8g6,8f6,e6,8e6,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,a";
const char *tetris2 = "tetris2:d=4,o=5,b=63:d#6,b,c#6,a#,16b,16g#,16a#,16b,16b,16g#,16a#,16b,c#6,g,d#6,16p,16g#,16a#,16b,c#6,16p,16b,16a#,g#,g,g#,16f,16g,16g#,16a#,8d#.6,32d#6,32p,32d#6,32p,32d#6,32p,16d6,16d#6,8f.6,16d6,8a#,8p,8f#6,8d#6,8f#,8g#,a#.,16p,16a#,8d#.6,16f6,16f#6,16f6,16d#6,16a#,8g#.,16b,8d#6,16f6,16d#6,8a#.,16b,16a#,16g#,16f,16f#,d#";
const char *mario = "mario:d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g,8p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,16p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16c7,16p,16c7,16c7,p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16d#6,8p,16d6,8p,16c6";
const char *giana = "giana:d=4,o=5,b=125:8g,8g,8a,8g,8a#,8g,8a,8g,8g,8g,8c6,8g,8a#,8g,8a,8g,8g,8f,8a,8f,8a#,8f,8c6,8f,8d6,8f,8c6,8f,8a#,8f,8a,8f,8g,8d#,8a,8d#,8a#,8d#,8a,8d#,8g,8d#,8c6,8d#,8a#,8d#,8a,8d#,8f#,8d,8g,8d,8a,8d,8a#,8d,8c6,8d,8d6,8d,8a#,8d,8a,8d";
const char *indy = "Indy:d=4,o=5,b=250:e,8p,8f,8g,8p,1c6,8p.,d,8p,8e,1f,p.,g,8p,8a,8b,8p,1f6,p,a,8p,8b,2c6,2d6,2e6,e,8p,8f,8g,8p,1c6,p,d6,8p,8e6,1f.6,g,8p,8g,e.6,8p,d6,8p,8g,e.6,8p,d6,8p,8g,f.6,8p,e6,8p,8d6,2c6";
const char *death = "Death March:d=4,o=5,b=125:c.,c,8c,c.,d#,8d,d,8c,c,8c,2c.";


//////////////////////////////////////////////////Function prototypes//////////////////////////////////////////////////

void Idle();

void Game();

void GameOver();

void GameComplete();

void PrintGameCompleteSetup();

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
