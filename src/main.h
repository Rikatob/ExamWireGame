

#ifndef EXAMEMBEDDED_MAIN_H
#define EXAMEMBEDDED_MAIN_H

#include <Arduino.h>
#include <RtcDS3231.h>

//////////////////////////////////////////////////////Defines//////////////////////////////////////////////////////

// Game settings
#define GAME_DURATION 30
#define HIGH_SCORE_TABLE_SIZE 3
//#define RESET_HIGHSCORE                          // Uncomment this to reset the highscore list in RTC EPROM.

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
    SPLASH_SCREEN,
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
unsigned long previousTime;
enum State currentState;
enum Difficulty difficulty;
bool stateChanged;
char gameBuffer[20];
char pzAsciArrow[2];
uint16_t textColor;
byte debounceDuration;
byte playerLives;
byte highScoreEntriesCount;
byte currentPos;
byte timeGoneBy;
byte timeLeft;


//////////////////////////////////////////////////Function prototypes//////////////////////////////////////////////////

/************************* Game logic.*************************/
void SplashScreen();

void Idle();

void Game();

void GameOver();

void GameComplete();

void EnterHighscore();

void GameDifficulty();

void ResetGameBuffer();

byte CheckButton(byte buttonPin);


/************************* TFT DISPLAY.*************************/
void TftInitiate();

void PrintGameCompleteSetup();

void PrintStartMenu();

void PrintDifficultyMenu();

void PrintHighScoreTable();

int EnterInitials();

void DrawText(const char *text, uint16_t color, byte size, byte x, byte y, bool clearScreen);

void MoveUpInMenu();

void MoveDownInMenu();


/************************* REAL TIME CLOCK.*************************/
void CalibrateRtc();

void WriteEntriesToRtcMemory();

void ReadEntriesFromRtcMemory();

void ResetHighScoreInRtc();



/*********************** High score functions.************************/
void UpdateHighScore(char *initials, byte time);

void AddHighScoreEntry(int index, char *initials, byte time);

void MakeSpaceForHighScoreEntry(int indexToReplace);


#endif //EXAMEMBEDDED_MAIN_H
