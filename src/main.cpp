
// EKSAMEN EMBEDDED SYSTEMS
/////////////////////////////////////////////////////////////////////////////////
// TODO[ ] Change some statics (or all) to global variables.                   //
// TODO[x] Use highscore temp to illustrate adding highscores to rtc memory.   //
// TODO[ ] Make the highscore table stored in RTC memory.                      //
/////////////////////////////////////////////////////////////////////////////////
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
//#include <TMRpcm.h>
/*
<<<<<<<<<<<<<<<<<<<< DS3231 >>>>>>>>>>>>>>>>>>>>
*/
#include <Wire.h>
#include <RtcDS3231.h>

///////////////////////////////////////////////////Objects/Inits///////////////////////////////////////////////////
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
//TMRpcm tmrpcm;
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
    // digitalWrite(BUZZER_PIN, LOW);


    // Initiate REAL TIME CLOCK (DS3231).
    Rtc.Begin();

    // Load the high scores from rtc memory.
    ReadEntriesFromRtcMemory();

    // TFT
    TftInitiate();

    //PCM
    // PcmInitiate();

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
        case ENTER_HIGHSCORE:
            EnterHighscore();
            break;
        case PRINT_HIGHSCORE:
            PrintHighScoreTable();
            break;
    }
}

// TODO StopPlayback in stateChanged if statment in every function
void Idle() {

    if (stateChanged) {
        //tmrpcm.play("test.wav");              // TODO NEED TO CHANGE THIS SOUND
        currentPos = 11;
        PrintStartMenu();
        stateChanged = false;
    }
/*
    if (!tmrpcm.isPlaying()) {
        tmrpcm.stopPlayback();
    }
*/
    byte okBtnPressed = CheckButton(BTN_OK_PIN);
    byte upBtnPressed = CheckButton(BTN_UP_PIN);
    byte downBtnPressed = CheckButton(BTN_DOWN_PIN);

    // Ok button pressed -> Do what user have chosen.
    if (okBtnPressed) {

        // Start game.
        if (currentPos == 11) {
            //tmrpcm.stopPlayback();
            currentState = GAME;
            stateChanged = true;

            // Change difficulty
        } else if (currentPos == 41) {
            //tmrpcm.stopPlayback();
            currentState = GAME_DIFFICULTY;
            stateChanged = true;

            // Print high-score.
        } else if (currentPos == 71) {
            //tmrpcm.stopPlayback();
            currentState = PRINT_HIGHSCORE;
            stateChanged = true;
        }

    }
    if (upBtnPressed) {
        MoveUpInMenu();
    }
    if (downBtnPressed) {
        MoveDownInMenu();
    }
}

void GameDifficulty() {

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
        MoveUpInMenu();
    }
    if (downBtnPressed) {
        MoveDownInMenu();
    }
}

/*
  Game is running.
  -> If wire is touched buzz sound will come and state will change to game over.
*/
void Game() {

    if (stateChanged) {
        //tmrpcm.play("start.wav");
        DrawText("GO GO GO!!", ST77XX_GREEN, DEFAULT_TEXT_SIZE, 35, 75, true);
        DrawText("No time to loose!", ST77XX_BLUE, 2, 25, 105, false);
        playerLives = difficulty;
        textColor = ST77XX_BLUE;
        previousTime = GAME_DURATION; // Initial set it to duration of game.
        startTime = Rtc.GetDateTime();
        stateChanged = false;
    }
    if (1 == 1) {
        //tmrpcm.stopPlayback();
        currentTime = Rtc.GetDateTime();
        timeGoneBy = currentTime.TotalSeconds() - startTime.TotalSeconds();
        timeLeft = GAME_DURATION - timeGoneBy;
        snprintf(gameBuffer, ArraySize(gameBuffer), "%02lu", previousTime);

        if (timeLeft == 0) {
            //tmrpcm.stopPlayback();
            currentState = GAME_OVER;
            stateChanged = true;
        } else if (previousTime > timeLeft) {
            // tmrpcm.stopPlayback();
            // Change color on timer when the time left decreases.
            if (timeLeft <= (GAME_DURATION / 3)) {
                textColor = ST77XX_RED;
            } else if (timeLeft <= (GAME_DURATION / 3) * 2) {
                textColor = ST77XX_YELLOW;
            }
            DrawText(gameBuffer, ST77XX_BLACK, 6, 80, 15, false);
            snprintf(gameBuffer, ArraySize(gameBuffer), "%02d", timeLeft);
            DrawText(gameBuffer, textColor, 6, 80, 15, false);
            previousTime = timeLeft;
        }
    }

    byte wireState = digitalRead(WIRE_PIN);
    byte goalPinState = digitalRead(GOAL_PIN);
    if (wireState == LOW) {
        playerLives--;
        if (playerLives == 0) {
            //  tmrpcm.stopPlayback();
            currentState = GAME_OVER;
            stateChanged = true;
        } else {
            //   tmrpcm.play("over.wav");
            delay(200);
            // tmrpcm.stopPlayback();
        }

    }
    if (goalPinState == LOW) {
        //tmrpcm.stopPlayback();
        currentState = GAME_COMPLETE;
        stateChanged = true;
    }
}

/*

*/
void GameOver() {
    if (stateChanged) {
        //  tmrpcm.play("over.wav");
        DrawText("GAME OVER!", ST77XX_RED, DEFAULT_TEXT_SIZE, 25, 55, true);
        stateChanged = false;
    }
    /*
    if (!tmrpcm.isPlaying()) {
        tmrpcm.stopPlayback();
    }
*/
    byte buttonPressed = CheckButton(BTN_OK_PIN);
    if (buttonPressed) {
        //tmrpcm.stopPlayback();
        currentState = IDLE;
        stateChanged = true;
    }
}

void GameComplete() {

    if (stateChanged) {
        //tmrpcm.play("complete.wav");
        DrawText("SUCCESS!!", ST77XX_GREEN, DEFAULT_TEXT_SIZE, 35, 35, true);
        snprintf(gameBuffer, ArraySize(gameBuffer), "Your time:%02d.sec", GAME_DURATION - timeLeft);
        DrawText(gameBuffer, ST77XX_BLUE, 2, 20, 70, false);
        DrawText("Press OK to try again.", ST77XX_BLUE, 1, 45, 100, false);
        DrawText("Press DOWN to enter high-score.", ST77XX_BLUE, 1, 45, 110, false);
        stateChanged = false;
    }
    /*if (!tmrpcm.isPlaying()) {
        tmrpcm.stopPlayback();
    }
*/
    byte buttonPressed = CheckButton(BTN_OK_PIN);
    byte buttonDownPressed = CheckButton(BTN_DOWN_PIN);
    if (buttonPressed) {
        //tmrpcm.stopPlayback();
        currentState = IDLE;
        stateChanged = true;
    } else if (buttonDownPressed) {
        //tmrpcm.stopPlayback();
        currentState = ENTER_HIGHSCORE;
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

/*
void PcmInitiate() {
    tmrpcm.speakerPin = BUZZER_PIN;
    if (!SD.begin(SD_CS)) {
        Serial.println("SD FAILED TO INIT..");
        return;
    }
    tmrpcm.setVolume(5);
    tmrpcm.quality(true);
}
*/
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
               ArraySize(datestring),
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

void EnterHighscore() {

    int iReturnCode = EnterInitials();
    if (iReturnCode == DONE) {
        // tmrpcm.stopPlayback();
        UpdateHighScore(gameBuffer, timeGoneBy);
        currentState = IDLE;
        stateChanged = true;
    }
}

int EnterInitials() {
    static char letterBuffer[2];
    static char firstLetter;
    static char secondLetter;
    static char thirdLetter;
    if (stateChanged) {
        memset(gameBuffer, 0, ArraySize(gameBuffer)); // reset buffer incase of old values.
        currentPos = 15;
        letterBuffer[0] = 0x41;
        letterBuffer[1] = '\0';
        DrawText("A", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 15, 50, true);
        DrawText("A", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 35, 50, false);
        DrawText("A", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 55, 50, false);
        stateChanged = false;
    }
    byte okBtnPressed = CheckButton(BTN_OK_PIN);
    byte upBtnPressed = CheckButton(BTN_UP_PIN);
    byte downBtnPressed = CheckButton(BTN_DOWN_PIN);

    if (okBtnPressed) {
        switch (currentPos) {
            case 15:
                firstLetter = letterBuffer[0];
                letterBuffer[0] = 0x41;
                break;
            case 35:
                secondLetter = letterBuffer[0];
                letterBuffer[0] = 0x41;
                break;
            case 55:
                thirdLetter = letterBuffer[0];
                break;
            default:
                Serial.println("Something went wrong!");
                break;
        }
        currentPos = currentPos + 20;
        if (currentPos > 55) {
            gameBuffer[0] = firstLetter;
            gameBuffer[1] = secondLetter;
            gameBuffer[2] = thirdLetter;
            gameBuffer[3] = '\0';
            DrawText("Successfully entered", ST77XX_GREEN, DEFAULT_TEXT_SIZE, 10, 15, true);
            DrawText(gameBuffer, ST77XX_BLUE, DEFAULT_TEXT_SIZE, 75, 80, false);
            delay(2000);
            return DONE;
        }

    } else if (upBtnPressed) {
        DrawText(letterBuffer, ST77XX_BLACK, DEFAULT_TEXT_SIZE, currentPos, 50, false);
        letterBuffer[0]++;
        if (letterBuffer[0] > 0x5A) {
            letterBuffer[0] = 0x41;
        }
        DrawText(letterBuffer, ST77XX_BLUE, DEFAULT_TEXT_SIZE, currentPos, 50, false);

    } else if (downBtnPressed) {
        DrawText(letterBuffer, ST77XX_BLACK, DEFAULT_TEXT_SIZE, currentPos, 50, false);
        letterBuffer[0]--;
        if (letterBuffer[0] < 0x41) {
            letterBuffer[0] = 0x5A;
        }
        DrawText(letterBuffer, ST77XX_BLUE, DEFAULT_TEXT_SIZE, currentPos, 50, false);
    }
    return NOT_DONE;
}

void UpdateHighScore(char *initials, byte time) {
    // Add First Entry.
    if (highScoreEntriesCount == 0) {
        AddHighScoreEntry(0, initials, time);

        // Entry has worse time than current last place, but table is not full. add to last place.
    } else if (time > highScoreEntries[highScoreEntriesCount - 1].time) {
        if (highScoreEntriesCount != HIGH_SCORE_TABLE_SIZE) {
            AddHighScoreEntry(highScoreEntriesCount, initials, time);
        }

        // Add entries sorted by time.
    } else {
        for (int i = 0; i < highScoreEntriesCount; i++) {
            // Better time than entry at [i].
            if (time < highScoreEntries[i].time) {
                // Make space for entry.
                MakeSpaceForHighScoreEntry(i);
                // Add entry at index [i].
                AddHighScoreEntry(i, initials, time);
                break;
            }
        }
    }
    // Update the highscore list in rtc memory.
    WriteEntriesToRtcMemory();
}

// Adds entry with values (initials and time) on the given index of the high-score table.
void AddHighScoreEntry(int index, char *initials, byte time) {
    strncpy(highScoreEntries[index].initials, initials, 4);
    highScoreEntries[index].time = time;
    if (highScoreEntriesCount < HIGH_SCORE_TABLE_SIZE) {
        highScoreEntriesCount++;
    }
}

// Push all entries after the index you want to replace one index up.(down on the high-score table)
// Start from last entry to the entry you want to replace.
void MakeSpaceForHighScoreEntry(int indexToReplace) {
    for (int j = highScoreEntriesCount - 1; j >= indexToReplace; j--) {
        // list is full, jump one index up to avoid adding entry out of bounds.
        // Last entry will be replaced by the one over it( "Pushed" out of highscore list).
        if (j == HIGH_SCORE_TABLE_SIZE - 1) {
            continue;
            // Push entry one index up (down on the high-score list).
        } else {
            //highScoreEntries[j] = highScoreEntries[j+1];
            strncpy(highScoreEntries[j + 1].initials, highScoreEntries[j].initials, 4);
            highScoreEntries[j + 1].time = highScoreEntries[j].time;
        }
    }
}

void PrintHighScoreTable() {
    if (stateChanged) {
        uint16_t color;
        byte xCorr;
        byte yCorr;
        stateChanged = false;

        DrawText("HIGH SCORES", ST77XX_GREEN, DEFAULT_TEXT_SIZE, 20, 5, true);
        for (int i = 0; i < highScoreEntriesCount; i++) {
            ResetGameBuffer();
            snprintf(gameBuffer, ArraySize(gameBuffer), "%d Place:", i + 1);

            switch (i) {
                case 0:
                    color = ST77XX_BLUE;
                    xCorr = 15;
                    yCorr = 35;
                    break;
                case 1:
                    color = ST77XX_YELLOW;
                    xCorr = 15;
                    yCorr = 70;
                    break;
                case 2:
                    color = ST77XX_RED;
                    xCorr = 15;
                    yCorr = 105;
                    break;
                default:
                    color = ST77XX_BLUE;
                    xCorr = 0;
                    yCorr = 0;
            }
            DrawText(gameBuffer, color, DEFAULT_TEXT_SIZE - 1, xCorr, yCorr, false);

            ResetGameBuffer();
            snprintf(gameBuffer, ArraySize(gameBuffer), "%s %02d.sec", highScoreEntries[i].initials,
                     highScoreEntries[i].time);

            DrawText(gameBuffer, color, DEFAULT_TEXT_SIZE - 1, xCorr + 15, yCorr + 17, false);
        }
    }
    byte okBtnPressed = CheckButton(BTN_OK_PIN);
    byte upBtnPressed = CheckButton(BTN_UP_PIN);
    byte downBtnPressed = CheckButton(BTN_DOWN_PIN);
    if (okBtnPressed || upBtnPressed || downBtnPressed) {
        // tmrpcm.stopPlayback();
        currentState = IDLE;
        stateChanged = true;
    }

}

void PrintStartMenu() {
    DrawText("Start game.", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 25, 10, true);
    DrawText(pzAsciArrow, ST77XX_BLUE, DEFAULT_TEXT_SIZE, 0, 11, false);
    DrawText("Difficulty.", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 25, 40, false);
    DrawText("High-score.", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 25, 70, false);
}

void PrintDifficultyMenu() {
    DrawText("Easy.", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 40, 10, true);
    DrawText("->", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 0, 11, false);
    DrawText("Medium.", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 40, 40, false);
    DrawText("Hard.", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 40, 70, false);
}

// Writes the high-score entries to the RTC memory in the format listed below.
// [x] = 1 byte.
// [entryCount][inital][inital][inital][ZeroTerminator][time]
void WriteEntriesToRtcMemory() {
    Wire.beginTransmission(0x57);
    Wire.write(0x00);
    Wire.write(0x00);
    Wire.write(highScoreEntriesCount);
    for (int i = 0; i < highScoreEntriesCount; i++) {
        for (int j = 0; j < 4; j++) {
            Wire.write(highScoreEntries[i].initials[j]);
        }
        Wire.write(highScoreEntries[i].time);
    }
    byte error = Wire.endTransmission();
    Serial.print("Return Code RtcMemory Write: ");
    Serial.println(error);
}

// Get high-score entries from RTC memory and update the highScoreEntries array accordingly.
void ReadEntriesFromRtcMemory() {
    byte entry = 0;
    byte initialsIndex = 0;

    // Memory address to read from.
    Wire.beginTransmission(0x57);
    Wire.write(0x00);
    Wire.write(0x00);
    Wire.endTransmission();
    // Request.
    Wire.requestFrom(0x57, 1);
    byte countTemp = Wire.read();

    // Check for the first byte is not ovcer the maximum size of highScoreEntries array.(Should not happen)
    if (countTemp <= HIGH_SCORE_TABLE_SIZE) {
        highScoreEntriesCount = countTemp;
    } else {
        highScoreEntriesCount = 0;
    }

    // Delay to before starting a new conversation.
    delay(100);

    // Memory address to read from.
    Wire.beginTransmission(0x57);
    Wire.write(0x00);
    Wire.write(0x01);
    Wire.endTransmission();
    // Request.
    Wire.requestFrom(0x57, highScoreEntriesCount * 5); // amount of entries * size of each entry.

    while (Wire.available()) {

        // Check if memory has more entries than expressed by count (first byte in rtc memory).
        // It will result in problems in the rest of the code
        // and if highScoreEntries = HIGH_SCORE_TABLE_SIZE it will result in buffer overflow.
        if (entry > highScoreEntriesCount - 1) {
            Serial.println(" Something went wrong trying to read Rtc memory");
            return;
        }
        // Read the initials and the zero terminator.
        if (initialsIndex < 4) {
            highScoreEntries[entry].initials[initialsIndex] = (char) Wire.read();
            initialsIndex++;
            // Read the time of the entry.
        } else {
            initialsIndex = 0;
            highScoreEntries[entry].time = Wire.read();
            entry++;
        }
    }
}

void ResetGameBuffer() {
    memset(gameBuffer, 0, ArraySize(gameBuffer)); // "Reset" buffer.
}

void MoveUpInMenu() {
    // Highest position in menu screen is y = 11.
    if (currentPos > 11) {
        DrawText(pzAsciArrow, ST77XX_BLACK, DEFAULT_TEXT_SIZE, 0, currentPos, false);
        currentPos -= 30;
        DrawText(pzAsciArrow, ST77XX_BLUE, DEFAULT_TEXT_SIZE, 0, currentPos, false);
    }
}

void MoveDownInMenu() {
    // Lowest position in menu screen is y = 71.
    if (currentPos < 71) {
        DrawText(pzAsciArrow, ST77XX_BLACK, DEFAULT_TEXT_SIZE, 0, currentPos, false);
        currentPos += 30;
        DrawText(pzAsciArrow, ST77XX_BLUE, DEFAULT_TEXT_SIZE, 0, currentPos, false);
    }
}