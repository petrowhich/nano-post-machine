#include <avr/pgmspace.h>
#include "Button.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define BTN_LEFT_PIN 12
#define BTN_CHANGE_PIN 11
#define BTN_RIGHT_PIN 10
#define BTN_RUN_PIN 9
#define BTN_PAUSE_PIN 8
#define BTN_STOP_PIN 7
#define BTN_CLEAR_PIN 6
#define BTN_LOAD_PIN 5
#define BTN_SAVE_PIN 4
#define SPEAKER_PIN 13

#define I2C_ADDRESS 39
#define COLUMNS 20
#define LINES 4

LiquidCrystal_I2C lcd(I2C_ADDRESS, COLUMNS, LINES);
Button btn_left(BTN_LEFT_PIN);
Button btn_change(BTN_CHANGE_PIN);
Button btn_right(BTN_RIGHT_PIN);
Button btn_run(BTN_RUN_PIN);
Button btn_pause(BTN_PAUSE_PIN);
Button btn_stop(BTN_STOP_PIN);
Button btn_clear(BTN_CLEAR_PIN);
Button btn_load(BTN_LOAD_PIN);
Button btn_save(BTN_SAVE_PIN);

#define arrayLength(x) (sizeof(x) / sizeof(x[0]))
#define tapeSize COLUMNS
#define isButtonDown(object) (object).pressed()
#define anyButtonDown() (isButtonDown(btn_left) || isButtonDown(btn_change) \
                         || isButtonDown(btn_right) || isButtonDown(btn_run) \
                         || isButtonDown(btn_pause) || isButtonDown(btn_stop) \
                         || isButtonDown(btn_clear) || isButtonDown(btn_load) \
                         || isButtonDown(btn_save))
#define strlen_F(s) strlen_PF((PGM_P)s)

struct Instruction {
  char operation;
  byte jump[2];
};

//bool paused = false, firstTimePaused = true, executing = false, showCycles = true;

byte generalBool = 0;
#define getBool(bs) (generalBool & (1 << bs))
#define setBool(bs, value) ((value) ? (generalBool |= (1 << bs)) : (generalBool &= ~(1 << bs)))
#define paused 0
#define firstTimePaused 1
#define executing 2
#define showCycles 4

byte tape[tapeSize] = {
  0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
byte * tapeStorage;
const Instruction program[] = {
  {'?', 1, 3},
  {'1', 2, 0},
  {'>', 0, 0},
  {'.', 0, 0}
};
const byte programSize = arrayLength(program), initialHeadPosition = 2;
byte tapeStorageSize, head = initialHeadPosition, programPtr = 0, cyclesUsed = 0;

void showError(const __FlashStringHelper * s) {
  byte sl = strlen_F(s);
  tone(SPEAKER_PIN, 276, 100);
  delay(100);
  tone(SPEAKER_PIN, 262, 100);
  delay(100);
  byte i = 0;

  byte state;
  while ((state = digitalRead(BTN_STOP_PIN)) != LOW) {
    lcd.setCursor(6, 3);
    if ((i = (i + 1) & 1)) {
      lcd.print(s);
      state = digitalRead(BTN_STOP_PIN);
      delay(400);
    } else {
      for (byte j = 0; j < sl; j++)
        lcd.print(F(" "));
      state = digitalRead(BTN_STOP_PIN);
      delay(200);
    }
  }
  head = initialHeadPosition;
  programPtr = 0;
  setBool(executing, false);
}

void machineCycle() {
  Instruction * current = &program[programPtr];
  char op = current->operation;
  switch (op) {
    case '0': case '1': tape[head] = op - '0'; break;
    case '<':
      if (head > 0)
        head--;
      else
        showError(F("End of tape!"));
      break;
    case '>':
      if (head < (tapeSize - 1))
        head++;
      else
        showError(F("End of tape!"));
      break;
    case '.': setBool(executing, false); break;
    default: break;
  }
  if (op == '?') {
    programPtr = tape[head] == 0 ? current->jump[0] : current->jump[1];
  } else if (op != '.') {
    programPtr = current->jump[0];
  }
  cyclesUsed++;
}

#define byteIntoBits(x) ( \
  (x << 7) & 1 |          \
  (x << 6) & 1 |          \
  (x << 5) & 1 |          \
  (x << 4) & 1 |          \
  (x << 3) & 1 |          \
  (x << 2) & 1 |          \
  (x << 1) & 1 |          \
  x & 1                   \
)

void loadTape() {
  // W.I.P.
  for (byte i = tapeSize + 1; i > 0; i--)
    tape[i] = (tapeStorage[i / 8] >> (i & 7)) & 1;
}

void saveTape() {
  // W.I.P.
  for (byte i = tapeStorageSize + 1; i > 0; i--)
    tapeStorage[i] = byteIntoBits(tape[i / 8]);
}

void showTape() {
  lcd.setCursor(0, 1);
  for (size_t i = 0; i < tapeSize; i++)
    lcd.print(tape[i] == 1 ? "1" : "0");
  lcd.setCursor(0, 2);
  for (int i = 0; i < COLUMNS; i++)
    lcd.print(F(" "));
  lcd.setCursor(0, 2);
  for (size_t i = 0; i < head; i++)
    lcd.print(F(" "));
  lcd.print(F("^"));
}

void showState() {
  lcd.setCursor(0, 3);
  for (int i = 0; i < COLUMNS; i++)
    lcd.print(F(" "));
  lcd.setCursor(0, 3);
  if (getBool(paused)) {
    lcd.print(F("PAUSE"));
  } else if (getBool(executing)) {
    lcd.print(F("EXEC "));
  } else {
    lcd.print(F("IDLE "));
  }

  if (getBool(executing)) {
    lcd.print(F(" | "));

    Instruction * current = &program[programPtr];
    lcd.print(current->operation);

    bool doPrint = current->operation == '?';
    doPrint = doPrint || current->jump[0] != programPtr + 1;
    doPrint = doPrint && current->operation != '.';
    if (doPrint)
      lcd.print(current->jump[0] + 1);
    if (current->operation == '?') {
      lcd.print(F(","));
      lcd.print(current->jump[1] + 1);
    }
  } else if (getBool(showCycles) && !getBool(firstTimePaused)) {
    lcd.print(F(" | N = "));
    lcd.print(cyclesUsed);
    setBool(showCycles, false);
  }
}

void setup() {
  // Initializations
  lcd.init();
  lcd.backlight();

  btn_left.begin();
  btn_change.begin();
  btn_run.begin();
  btn_pause.begin();
  btn_stop.begin();
  btn_clear.begin();
  btn_load.begin();
  btn_save.begin();
  pinMode(SPEAKER_PIN, OUTPUT);

  tapeStorageSize = (tapeSize + 7) / 8;
  tapeStorage = malloc(tapeStorageSize);

  // Visuals
  lcd.setCursor(6, 1);
  const __FlashStringHelper * hello = F("Welcome!");
  byte helloLen;
  for (helloLen = 0; ; helloLen++) {
    char c = pgm_read_byte((PGM_P)hello + helloLen);
    if (c == 0) {
      break;
    }
    lcd.print(c);
    delay(65);
  }

  tone(SPEAKER_PIN, 392, 100);
  delay(205);
  lcd.setCursor(6, 1);
  for (byte i = 0; i < helloLen; i++) {
    lcd.print(F(" "));
    delay(35);
  }
  tone(SPEAKER_PIN, 440, 100);
  delay(205);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Nano Post Machine"));
  showTape();
}

void loop() {
  bool ready = false;
#define READY ((ready) = true)

  if (!getBool(executing) || getBool(paused)) {
    if (isButtonDown(btn_left) && head != 0) {
      head -= 1;
      READY;
    } else if (isButtonDown(btn_right) && head < (tapeSize - 1)) {
      head += 1;
      READY;
    } else if (isButtonDown(btn_change)) {
      tape[head] = 1 - tape[head];
      READY;
    } else if (isButtonDown(btn_clear)) {
      tone(SPEAKER_PIN, 294, 50);
      delay(25);
      tone(SPEAKER_PIN, 348, 50);
      delay(25);

      for (byte i = 0; i < tapeSize; i++)
        tape[i] = 0;

      lcd.setCursor(8, 3);
      lcd.print(F("Tape cleared"));
      showTape();
      while (!anyButtonDown());
      lcd.setCursor(8, 3);
      for (int i = 0; i < 12; i++)
        lcd.print(" ");
    } else if (isButtonDown(btn_load)) {
      lcd.setCursor(8, 3);
      lcd.print(F("Tape loaded "));
      loadTape();
      showTape();
      while (!anyButtonDown());
      lcd.setCursor(8, 3);
      for (int i = 0; i < 12; i++)
        lcd.print(F(" "));
      READY;
    } else if (isButtonDown(btn_save)) {
      saveTape();
      lcd.setCursor(8, 3);
      lcd.print(F("Tape saved  "));
      READY;
    }
  }

  if (isButtonDown(btn_run)) {
    setBool(executing, true);
    cyclesUsed = 0;
    READY;
  } else if (isButtonDown(btn_pause)) {
    setBool(paused, !getBool(paused));
    if (getBool(paused))
      setBool(firstTimePaused, true);
    else
      setBool(firstTimePaused, false);
    READY;
  } else if (isButtonDown(btn_stop)) {
    setBool(executing, false);
    setBool(paused, false);
    head = initialHeadPosition;
    programPtr = 0;
    setBool(showCycles, true);
    READY;
  }

  if (getBool(executing)) {
    if (!getBool(paused))
      machineCycle();
    if (!getBool(paused) || getBool(paused) && getBool(firstTimePaused)) {
      showState();
      setBool(firstTimePaused, false);
    }
  }
  if (ready || getBool(executing) && !getBool(paused)) {
    tone(SPEAKER_PIN, 2000, 5);
    showTape();
  }
}