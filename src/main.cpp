
// EKSAMEN EMBEDDED SYSTEMS
/////////////////////////////////////////////////////////////////////////////////
// TODO[x] Change some statics (or all) to global variables.                   //
// TODO[x] Use highscore temp to illustrate adding highscores to rtc memory.   //
// TODO[x] Make the highscore table stored in RTC memory.                      //
// TODO[ ] Use lightDiode to enforce the blue, yellow, red timer countdown     //
// TODO[ ] Oragnize code into Classes or new .c , .h files.                    //
/////////////////////////////////////////////////////////////////////////////////
/**
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
/**
<<<<<<<<<<<<<<<<<<<< ST7789 >>>>>>>>>>>>>>>>>>>>
*/
#include <Adafruit_ST7789.h>
#include <Adafruit_ImageReader.h>
#include <SPI.h>
/**
<<<<<<<<<<<<<<<<<<<< SD-CARD/TONES >>>>>>>>>>>>>>>>>>>>
*/
#include <SdFat.h>
#include <NonBlockingRtttl.h>
#include "tunes.h"
/**
<<<<<<<<<<<<<<<<<<<< DS3231 >>>>>>>>>>>>>>>>>>>>
*/
#include <Wire.h>
#include <RtcDS3231.h>

///////////////////////////////////////////////////Objects/Inits///////////////////////////////////////////////////
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
RtcDS3231<TwoWire> Rtc(Wire);
RtcDateTime startTime;
RtcDateTime currentTime;
HighScoreEntry highScoreEntries[HIGH_SCORE_TABLE_SIZE];

SdFat SD;
Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys
Adafruit_Image img;        // An image loaded into RAM
int32_t width = 0, height = 0; // BMP image x y.


void setup() {
    pinMode(WIRE_PIN, INPUT_PULLUP);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(BTN_OK_PIN, INPUT_PULLUP);
    pinMode(BTN_UP_PIN, INPUT_PULLUP);
    pinMode(BTN_DOWN_PIN, INPUT_PULLUP);
    pinMode(GOAL_PIN, INPUT_PULLUP);

    Serial.begin(115200);

    /** Initiate REAL TIME CLOCK (DS3231).*/
    Rtc.Begin();
    CalibrateRtc();

    /** Load the high scores from rtc memory.*/
#if defined(RESET_HIGHSCORE)
    ResetHighScoreInRtc();
#endif
    ReadEntriesFromRtcMemory();
    /** TFT */
    TftInitiate();
    if (!SD.begin(SD_CS, SD_SCK_MHZ(10))) {
        Serial.println(F("SD begin() failed"));
    }

    /** Init the variables with values.*/
    // Used to handle the "bouncing" effect of button.
    debounceDuration = 150;
    // Array to hold the ascii value of the arrow used in menu, with zero terminator as default.
    pzAsciArrow[0] = (char) -81;
    pzAsciArrow[1] = '\0';
    // Default state as SPALSH_SCREEN.
    currentState = SPLASH_SCREEN;
    // Default difficulty easy.
    difficulty = EASY;
    // Set it to true so the first time Splash screen runs the "setup" for SPLASH_SCREEN state.
    stateChanged = true;
}


void loop() {

    switch (currentState) {
        case SPLASH_SCREEN:
            SplashScreen();
            break;
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

void SplashScreen() {
    if (stateChanged) {
        ImageReturnCode stat;
        stat = reader.drawBMP("/keen.bmp", tft, 0, 0);
        reader.printStatus(stat);  // Print status to Serial.
        stateChanged = false;
    }
    /** If not playing song begin(init) the song for Idle*/
    if (!rtttl::isPlaying()) {
        rtttl::begin(BUZZER_PIN, tetris2);
    }
    /** Keeps playing song.*/
    rtttl::play();


    byte okBtnPressed = CheckButton(BTN_OK_PIN);
    if (okBtnPressed) {
        currentState = IDLE;
        stateChanged = true;
    }

}

/****************************************************************************
 * Prints the main screen containing main menu and handles the users choice.
****************************************************************************/
void Idle() {
    if (stateChanged) {
        rtttl::stop();
        currentPos = 11;
        PrintStartMenu();
        stateChanged = false;

    }

    byte okBtnPressed = CheckButton(BTN_OK_PIN);
    byte upBtnPressed = CheckButton(BTN_UP_PIN);
    byte downBtnPressed = CheckButton(BTN_DOWN_PIN);

    /** Ok button pressed -> Change to the state the choice refers to.*/
    if (okBtnPressed) {

        /** Start game.*/
        if (currentPos == 11) {
            currentState = GAME;
            stateChanged = true;

            /** Change difficulty.*/
        } else if (currentPos == 41) {
            currentState = GAME_DIFFICULTY;
            stateChanged = true;

            /** Print high-score.*/
        } else if (currentPos == 71) {
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

/******************************************************************************
 * Prints the different difficulties and handles the users chosen difficulty.
******************************************************************************/
void GameDifficulty() {

    if (stateChanged) {
        currentPos = 11;
        PrintDifficultyMenu();
        stateChanged = false;
    }

    byte okBtnPressed = CheckButton(BTN_OK_PIN);
    byte upBtnPressed = CheckButton(BTN_UP_PIN);
    byte downBtnPressed = CheckButton(BTN_DOWN_PIN);
    /** Ok button pressed -> Do what user have chosen.*/
    if (okBtnPressed) {

        /** User chose Easy.*/
        if (currentPos == 11) {
            difficulty = EASY;
            currentState = IDLE;
            stateChanged = true;

            /** User chose Medium.*/
        } else if (currentPos == 41) {
            difficulty = MEDIUM;
            currentState = IDLE;
            stateChanged = true;

            /** User chose Hard.*/
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

/********************************************************************************
  Game is running.
  * Plays song.
  * Print out timer.
  * If wire is touched buzz sound will come and player loose a life.
  * If game is lost state is changed to game over.
*********************************************************************************/
void Game() {

    /**
     * Setup for the game.
     * Stop the music.
     * Print text to screen.
     * Set difficulty and initiate time variables.
     * Begin playing the song for Game.
   */
    if (stateChanged) {
        rtttl::stop();
        DrawText("GO GO GO!!", ST77XX_GREEN, DEFAULT_TEXT_SIZE, 35, 75, true);
        DrawText("No time to loose!", ST77XX_BLUE, 2, 25, 105, false);
        playerLives = difficulty;
        textColor = ST77XX_BLUE;
        previousTime = GAME_DURATION; // Initial set it to duration of game.
        startTime = Rtc.GetDateTime();
        stateChanged = false;
    }
    if (!rtttl::isPlaying()) {
        rtttl::begin(BUZZER_PIN, mario);
    }
    /** Keeps playing song.*/
    rtttl::play();

    /** Calculate how much time has gone by and how much time is left and store it in buffer.*/
    currentTime = Rtc.GetDateTime();
    timeGoneBy = currentTime.TotalSeconds() - startTime.TotalSeconds();
    timeLeft = GAME_DURATION - timeGoneBy;
    snprintf(gameBuffer, ArraySize(gameBuffer), "%02lu", previousTime);

    /** No lives left.*/
    if (timeLeft == 0) {
        currentState = GAME_OVER;
        stateChanged = true;

        /** Time is not out.*/
    } else if (previousTime > timeLeft) {

        /** Change color on timer when the time left decreases.*/
        if (timeLeft <= (GAME_DURATION / 3)) {
            textColor = ST77XX_RED;
        } else if (timeLeft <= (GAME_DURATION / 3) * 2) {
            textColor = ST77XX_YELLOW;
        }

        /** Print the Timer that tells how much time is left.*/
        DrawText(gameBuffer, ST77XX_BLACK, 6, 80, 15, false);
        snprintf(gameBuffer, ArraySize(gameBuffer), "%02d", timeLeft);
        DrawText(gameBuffer, textColor, 6, 80, 15, false);
        previousTime = timeLeft;
    }

    byte wireState = digitalRead(WIRE_PIN);
    byte goalPinState = digitalRead(GOAL_PIN);

    /** Player Touches Wire, Loose one life.*/
    if (wireState == LOW) {
        playerLives--;

        /** All lives are lost.*/
        if (playerLives == 0) {
            currentState = GAME_OVER;
            stateChanged = true;

            /** Sound when touching wire. (loosing a life)*/
        } else {
            ledcWriteTone(0, 255);
            delay(500);
        }

    }

    /**Reached the goal*/
    if (goalPinState == LOW) {
        rtttl::stop();
        currentState = GAME_COMPLETE;
        stateChanged = true;
    }
}


/*************************************************************************
 * Prints game over screen and goes back to IDLE if ok button is pressed.
**************************************************************************/
void GameOver() {
    if (stateChanged) {
        rtttl::stop();
        DrawText("GAME OVER!", ST77XX_RED, DEFAULT_TEXT_SIZE, 25, 55, true);
        stateChanged = false;
    }

    if (!rtttl::isPlaying()) {
        rtttl::begin(BUZZER_PIN, death);
    }

    /** Keeps playing song.*/
    rtttl::play();

    byte buttonPressed = CheckButton(BTN_OK_PIN);
    if (buttonPressed) {
        currentState = IDLE;
        stateChanged = true;
    }
}

/**************************************************************************
 * Prints the setup screen and ask user for trying agan or save highscore.
***************************************************************************/
void GameComplete() {
    if (stateChanged) {
        rtttl::stop();
        PrintGameCompleteSetup();
        stateChanged = false;
    }

    if (!rtttl::isPlaying()) {
        rtttl::begin(BUZZER_PIN, indy);
    }

    /** Keeps playing song.*/
    rtttl::play();

    byte buttonPressed = CheckButton(BTN_OK_PIN);
    byte buttonDownPressed = CheckButton(BTN_DOWN_PIN);

    /** Player chose "try again".*/
    if (buttonPressed) {
        currentState = IDLE;
        stateChanged = true;

        /** Player chose "Enter highscore".*/
    } else if (buttonDownPressed) {
        currentState = ENTER_HIGHSCORE;
        stateChanged = true;
    }

}

/*********************************************************
 * Print text and players time to screen.
 * Print text to ask for storing highscore or try again.
**********************************************************/
void PrintGameCompleteSetup() {
    DrawText("SUCCESS!!", ST77XX_GREEN, DEFAULT_TEXT_SIZE, 35, 35, true);
    snprintf(gameBuffer, ArraySize(gameBuffer), "Your time:%02d.sec", GAME_DURATION - timeLeft);
    DrawText(gameBuffer, ST77XX_BLUE, 2, 20, 70, false);
    DrawText("Press OK to try again.", ST77XX_BLUE, 1, 45, 100, false);
    DrawText("Press DOWN to enter high-score.", ST77XX_BLUE, 1, 45, 110, false);
}

/*******************************************************
 * Takes in a pin number and checks if it was pressed.
 * Handles debounce problem with millis.
 *******************************************************/
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
    /** Reset the TFT to get a "clean" start.*/
    digitalWrite(TFT_RST, HIGH);
    digitalWrite(TFT_RST, LOW);

    /** Initiate ST7789 1.14" 240x135.*/
    tft.init(135, 240);
    /** Set start point to black screen.*/
    tft.fillScreen(ST77XX_BLACK);
    /** Rotate screen to show widesreen where 0,0 is top 135 line and left 240 line.*/
    tft.setRotation(1);
}

/*******************************************************************************************************
 * Print text to the screen in a certain color('RGB565 16bit' "ST77XX_<color>" pre-defined from lib),
   size, position(x=COLUMN, y=ROW) and clearing screen.
 * 240x135 x and y goes up in a unsigned 8 bit int (byte macro)
********************************************************************************************************/
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

/*************************************************************************
 * Runs the function to let user enter initials and when it returns DONE
   it updates the table with the user input.
**************************************************************************/
void EnterHighscore() {
    if (stateChanged) {
        rtttl::stop();
    }
    /** Check if player is done enter initials.*/
    int iReturnCode = EnterInitials();
    if (iReturnCode == DONE) {
        UpdateHighScore(gameBuffer, timeGoneBy);
        currentState = IDLE;
        stateChanged = true;
    }
}

/***************************************************************************
 * Print and handles the "scrolling" effect when user is choosing initials.
 * When user is not done choosing all 3 initials
   -> Return NOT_DONE
 * When all 3 initials is chosen and user press OK button
   -> return DONE
****************************************************************************/
int EnterInitials() {
    static char letterBuffer[2];
    static char firstLetter;
    static char secondLetter;
    static char thirdLetter;

    /**
     * Set the position for "x" on the first letter as base.
     * Sets the initial values for letterbuffer as 'A' and zeroterminator.
     * Prints the AAA as a base.
     * */
    if (stateChanged) {
        ResetGameBuffer(); // reset buffer in case of old values.
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

/*********************************************************************
 * Enter the highscore in the right order and write it to rtc memory.
**********************************************************************/
void UpdateHighScore(char *initials, byte time) {

    /** Add First Entry. */
    if (highScoreEntriesCount == 0) {
        AddHighScoreEntry(0, initials, time);

        /** Entry has worse time than current last place, but table is not full. add to last place.*/
    } else if (time > highScoreEntries[highScoreEntriesCount - 1].time) {
        if (highScoreEntriesCount != HIGH_SCORE_TABLE_SIZE) {
            AddHighScoreEntry(highScoreEntriesCount, initials, time);
        }

        /** Add entries sorted by time.*/
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

    /** Update the highscore list in rtc memory.*/
    WriteEntriesToRtcMemory();
}

/*****************************************************************************************
 * Adds entry with values (initials and time) on the given index of the high-score table.
******************************************************************************************/
void AddHighScoreEntry(int index, char *initials, byte time) {
    strncpy(highScoreEntries[index].initials, initials, 4);
    highScoreEntries[index].time = time;
    if (highScoreEntriesCount < HIGH_SCORE_TABLE_SIZE) {
        highScoreEntriesCount++;
    }
}

/*******************************************************************************************************
    * Push all entries after the index you want to replace one index up.(down on the high-score table).
    * Start from last entry to the entry you want to replace.
********************************************************************************************************/
void MakeSpaceForHighScoreEntry(int indexToReplace) {
    for (int j = highScoreEntriesCount - 1; j >= indexToReplace; j--) {

        /** list is full, jump one index up to avoid adding entry out of bounds.
          * Last entry will be replaced by the one over it( "Pushed" out of highscore list).*/
        if (j == HIGH_SCORE_TABLE_SIZE - 1) {
            continue;

            /** Push entry one index up (down on the high-score list).**/
        } else {
            strncpy(highScoreEntries[j + 1].initials, highScoreEntries[j].initials, 4);
            highScoreEntries[j + 1].time = highScoreEntries[j].time;
        }
    }
}

/*********************************************************
 * Prints high score table in right format and play song.
**********************************************************/
void PrintHighScoreTable() {

    if (stateChanged) {
        uint16_t color;
        byte xCorr;
        byte yCorr;
        stateChanged = false;
        rtttl::stop();
        DrawText("HIGH SCORES", ST77XX_GREEN, DEFAULT_TEXT_SIZE, 20, 5, true);

        /** Prints the layout for table and every highscore entry in the right format. */
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

    /** Play giana while showing highscore.*/
    if (!rtttl::isPlaying()) {
        rtttl::begin(BUZZER_PIN, giana);
    }

    rtttl::play();

    byte okBtnPressed = CheckButton(BTN_OK_PIN);
    byte upBtnPressed = CheckButton(BTN_UP_PIN);
    byte downBtnPressed = CheckButton(BTN_DOWN_PIN);

    /** Any button pressed then go back to IDLE.*/
    if (okBtnPressed || upBtnPressed || downBtnPressed) {
        currentState = IDLE;
        stateChanged = true;
    }
}

/************************************
 * Print start menu in right format.
*************************************/
void PrintStartMenu() {
    DrawText("Start game.", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 25, 10, true);
    DrawText(pzAsciArrow, ST77XX_BLUE, DEFAULT_TEXT_SIZE, 0, 11, false);
    DrawText("Difficulty.", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 25, 40, false);
    DrawText("High-score.", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 25, 70, false);
}

/******************************************
 * Print Difficulty menu in right format.
******************************************/
void PrintDifficultyMenu() {
    DrawText("Easy.", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 40, 10, true);
    DrawText(pzAsciArrow, ST77XX_BLUE, DEFAULT_TEXT_SIZE, 0, 11, false);
    DrawText("Medium.", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 40, 40, false);
    DrawText("Hard.", ST77XX_BLUE, DEFAULT_TEXT_SIZE, 40, 70, false);
}

/*******************************************************************************
// Writes the high-score entries to the RTC memory in the format listed below.
// [x] = 1 byte.
// [entryCount][inital][inital][inital][ZeroTerminator][time]
 *******************************************************************************/
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

    /** Print Return code.*/
    byte error = Wire.endTransmission();
    Serial.print("Return Code RtcMemory Write: ");
    Serial.println(error);
}

void ResetHighScoreInRtc() {
    Wire.beginTransmission(0x57);
    Wire.write(0x00);
    Wire.write(0x00);
    Wire.write(HIGH_SCORE_TABLE_SIZE);
    for (int i = 0; i < HIGH_SCORE_TABLE_SIZE; i++) {
        for (int j = 0; j < 3; j++) {
            Wire.write('A');
        }
        Wire.write('\0');
        Wire.write(GAME_DURATION);
    }

    /** Print Return code.*/
    byte error = Wire.endTransmission();
    Serial.print("Return Code RtcMemory Write: ");
    Serial.println(error);
    delay(100);
}

/*********************************************************************************************
 * Get high-score entries from RTC memory and update the highScoreEntries array accordingly.
*********************************************************************************************/
void ReadEntriesFromRtcMemory() {
    byte entry = 0;
    byte initialsIndex = 0;

    /** Memory address to read from.*/
    Wire.beginTransmission(0x57);
    Wire.write(0x00);
    Wire.write(0x00);
    Wire.endTransmission();
    /** Request.*/
    Wire.requestFrom(0x57, 1);
    byte countTemp = Wire.read();

    /** Check for the first byte is not ovcer the maximum size of highScoreEntries array.(Should not happen)*/
    if (countTemp <= HIGH_SCORE_TABLE_SIZE) {
        highScoreEntriesCount = countTemp;
    } else {
        highScoreEntriesCount = 0;
    }

    /** Delay to before starting a new conversation.*/
    delay(100);

    /** Memory address to read from.*/
    Wire.beginTransmission(0x57);
    Wire.write(0x00);
    Wire.write(0x01);
    Wire.endTransmission();
    /** Request.*/
    Wire.requestFrom(0x57, highScoreEntriesCount * 5); // amount of entries * size of each entry.

    while (Wire.available()) {
        /**
         * Check if memory has more entries than expressed by count (first byte in rtc memory).
         * It will result in problems in the rest of the code
         * and if highScoreEntries = HIGH_SCORE_TABLE_SIZE it will result in buffer overflow.
        */
        if (entry > highScoreEntriesCount - 1) {
            Serial.println(" Something went wrong trying to read Rtc memory");
            return;
        }

        /** Read the initials and the zero terminator.*/
        if (initialsIndex < 4) {
            highScoreEntries[entry].initials[initialsIndex] = (char) Wire.read();
            initialsIndex++;

            /** Read the time of the entry.*/
        } else {
            initialsIndex = 0;
            highScoreEntries[entry].time = Wire.read();
            entry++;
        }
    }
}

/***********************************************************************
 * Recalibrates the RTC if necessary REF: rtc by makuna simple example
************************************************************************/
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

/*****************************************
 * Set all indexes of gameBuffer to '\0'
******************************************/
void ResetGameBuffer() {
    memset(gameBuffer, 0, ArraySize(gameBuffer)); // "Reset" buffer.
}

/*****************************************************
 * Move the arrow up and "delete" the previous arrow.
******************************************************/
void MoveUpInMenu() {
    /** Highest position in menu screen is y = 11.*/
    if (currentPos > 11) {
        DrawText(pzAsciArrow, ST77XX_BLACK, DEFAULT_TEXT_SIZE, 0, currentPos, false);
        currentPos -= 30;
        DrawText(pzAsciArrow, ST77XX_BLUE, DEFAULT_TEXT_SIZE, 0, currentPos, false);
    }
}

/*******************************************************
 * Move the arrow down and "delete" the previous arrow.
********************************************************/
void MoveDownInMenu() {
    /** Lowest position in menu screen is y = 71.*/
    if (currentPos < 71) {
        DrawText(pzAsciArrow, ST77XX_BLACK, DEFAULT_TEXT_SIZE, 0, currentPos, false);
        currentPos += 30;
        DrawText(pzAsciArrow, ST77XX_BLUE, DEFAULT_TEXT_SIZE, 0, currentPos, false);
    }
}