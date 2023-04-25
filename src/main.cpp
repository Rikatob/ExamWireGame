
// EKSAMEN EMBEDDED SYSTEMS

/*
<<<<<<<<SPI>>>>>>>>>
CS    -> 10 ( CHIP SELECT TFT)
CS    -> 7 (CHIP SELECT SD)
MOSI  -> 11 ( MASTER OUT SLAVE IN)
MISO  -> 12 ( MASTER IN SLAVE OUT)
SCK   -> 13 ( CLOCK SPEED)


<<<<<<<<I2C>>>>>>>>>
SDA -> A4 (DATA LINE)
SCL -> A5 (CLOCK LINE)
*/

/////////////////////////////////////////////////////Libraries/////////////////////////////////////////////////////
#include "main.h"
#include <Arduino.h>
/*
<<<<<<<<<<<<<<<<<<<< ST7789 >>>>>>>>>>>>>>>>>>>>
*/
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
/*
<<<<<<<<<<<<<<<<<<<< SD-CARD >>>>>>>>>>>>>>>>>>>>
*/
#include <SD.h>
#include <TMRpcm.h>
/*
<<<<<<<<<<<<<<<<<<<< DS3231 >>>>>>>>>>>>>>>>>>>>
*/
#include <Wire.h>
#include <RtcDS3231.h>

///////////////////////////////////////////////////Objects/Inits///////////////////////////////////////////////////
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
TMRpcm tmrpcm;
RtcDS3231<TwoWire> Rtc(Wire);
RtcDateTime startTime;
RtcDateTime currentTime;

void setup() {
    Serial.begin(9600);

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

    // TFT
    TftInitiate();
    //PCM
    PcmInitiate();
    // Initiate REAL TIME CLOCK (DS3231).
    Rtc.Begin();
    // CalibrateRtc();   // TODO THIS FUCKS UP THE BUZZER

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
        case GAME_DIFFICULTY:
            GameDifficulty();
            break;
    }


}

// TODO StopPlayback in stateChanged if statment in every function
void Idle() {

    static int currentPos;

    if (stateChanged) {
        currentPos = 11;
        PrintStartMenu();
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

            // Change difficulty
        } else if (currentPos == 41) {
            tmrpcm.stopPlayback();
            currentState = GAME_DIFFICULTY;
            stateChanged = true;

            // Print high-score.
        } else if (currentPos == 71) {
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

void GameDifficulty() {
    static int currentPos;
    if (stateChanged) {
        currentPos = 11;
        PrintDifficultyMenu();
        stateChanged = false;
    }


    byte okBtnPressed = CheckButton(BTN_OK_PIN);
    byte upBtnPressed = CheckButton(BTN_UP_PIN);
    byte downBtnPressed = CheckButton(BTN_DOWN_PIN);
    // Ok button pressed -> Do what user have chosen.
    if (okBtnPressed) {

        // User chose Easy.
        if (currentPos == 11) {
            difficulty = EASY;
            currentState = IDLE;
            stateChanged = true;

            // User chose Medium.
        } else if (currentPos == 41) {
            difficulty = MEDIUM;
            currentState = IDLE;
            stateChanged = true;

            // User chose Hard.
        } else if (currentPos == 71) {
            difficulty = HARD;
            currentState = IDLE;
            stateChanged = true;
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
        DrawText("GO GO GO!!", ST77XX_GREEN, DEFAULT_TEXT_SIZE, 35, 55, true);
        DrawText("No time to loose!", ST77XX_BLUE, 2, 25, 85, false);
        if (!tmrpcm.isPlaying()) {
            tmrpcm.play("start.wav");
        }
        playerLives = difficulty;
        textColor = ST77XX_BLUE;
        previousTime = GAME_DURATION; // Initial set it to duration of game.
        startTime = Rtc.GetDateTime();
        stateChanged = false;
    }
    if (!tmrpcm.isPlaying()) {
        currentTime = Rtc.GetDateTime();
        timeGoneBy = currentTime.TotalSeconds() - startTime.TotalSeconds();
        timeLeft = GAME_DURATION - timeGoneBy;
        snprintf(gameBuffer, countof(gameBuffer), "%02lu", previousTime);

        if (timeLeft == 0) {
            tmrpcm.stopPlayback();
            currentState = GAME_OVER;
            stateChanged = true;
        } else if (previousTime > timeLeft) {
            tmrpcm.stopPlayback();
            // Change color on time when the time left decreases.
            if (timeLeft <= 10) {
                textColor = ST77XX_RED;
            } else if (timeLeft <= 20) {
                textColor = ST77XX_YELLOW;
            }
            DrawText(gameBuffer, ST77XX_BLACK, DEFAULT_TEXT_SIZE, 100, 15, false);
            snprintf(gameBuffer, countof(gameBuffer), "%02lu", timeLeft);
            DrawText(gameBuffer, textColor, DEFAULT_TEXT_SIZE, 100, 15, false);
            previousTime = timeLeft;
        }
    }

    byte wireState = digitalRead(WIRE_PIN);
    byte goalPinState = digitalRead(GOAL_PIN);
    if (wireState == LOW) {
        playerLives--;
        if (playerLives == 0) {
            tmrpcm.stopPlayback();
            currentState = GAME_OVER;
            stateChanged = true;
        } else{
            tmrpcm.play("over.wav");
            delay(200);
            tmrpcm.stopPlayback();
        }

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


void GameComplete() {

    if (stateChanged) {
        DrawText("SUCCESS!!", ST77XX_GREEN, DEFAULT_TEXT_SIZE, 35, 35, true);
        snprintf(gameBuffer, countof(gameBuffer), "Your time:%02lu.sec", GAME_DURATION - timeLeft);
        DrawText(gameBuffer, ST77XX_BLUE, 2, 20, 70, false);
        DrawText("Press OK to try again.", ST77XX_BLUE, 1, 45, 100, false);
        if (!tmrpcm.isPlaying()) {
            tmrpcm.play("complete.wav");
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


void PcmInitiate() {
    tmrpcm.speakerPin = BUZZER_PIN;
    if (!SD.begin(SD_CS)) {
        Serial.println("SD FAILED TO INIT..");
        return;
    }
    tmrpcm.setVolume(5);
    tmrpcm.quality(true);
}

void CalibrateRtc() {
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) {
        Serial.println("RTC is older than compile time, updating DateTime");
        Rtc.SetDateTime(compiled);
    } else if (now > compiled) {
        Serial.println("RTC is newer than compile time, this is expected");
    } else if (now == compiled) {
        Serial.println("RTC is the same as compile time, while not expected all is still fine");
    }

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
// TODO USE THIS PRINT DATE TIME ????????????????

// print out the time and date from DS3231 to a certain place in the TFT.
void printDateTime(const RtcDateTime &dt, uint8_t cursorX, uint8_t cursorY) {
    char datestring[20];

    snprintf_P(datestring,
               countof(datestring),
               PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
               dt.Month(),
               dt.Day(),
               dt.Year(),
               dt.Hour(),
               dt.Minute(),
               dt.Second());
    //tft.setTextSize(2);
    DrawText(datestring, ST77XX_BLUE, DEFAULT_TEXT_SIZE, cursorX, cursorY, false);
}


void PrintStartMenu() {
    DrawText("Start game.", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 40, 10, true);
    DrawText("->", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 0, 11, false);
    DrawText("Difficulty.", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 40, 40, false);
    DrawText("High-score.", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 40, 70, false);
}

void PrintDifficultyMenu() {
    DrawText("Easy.", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 40, 10, true);
    DrawText("->", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 0, 11, false);
    DrawText("Medium.", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 40, 40, false);
    DrawText("Hard.", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 40, 70, false);
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