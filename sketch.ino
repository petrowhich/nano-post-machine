#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define BTN_LEFT 12
#define BTN_CHANGE 11
#define BTN_RIGHT 10
#define BTN_RUN 9
#define BTN_PAUSE 8
#define BTN_STOP 7
#define BTN_CLEAR 6
#define SPEAKER 13

#define I2C_ADDRESS 39
#define COLUMNS 20
#define LINES 4

LiquidCrystal_I2C lcd(I2C_ADDRESS, COLUMNS, LINES);

#define arrayLength(x) (sizeof(x) / sizeof(x[0]))
#define anyButtonDown() (isButtonDown(BTN_LEFT) || isButtonDown(BTN_CHANGE) \
                         || isButtonDown(BTN_RIGHT) || isButtonDown(BTN_RUN) \
                         || isButtonDown(BTN_PAUSE) || isButtonDown(BTN_STOP) \
                         || isButtonDown(BTN_CLEAR))

struct Instruction {
  char operation;
  byte jump[2];
};

bool paused = false, firstTimePaused = true, executing = false, showCycles = false;
byte tape[COLUMNS] = {
  0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
const Instruction program[] = {
  {'?', 1, 3},
  {'1', 2, 0},
  {'>', 0, 0},
  {'.', 0, 0}
};
const size_t tapeSize = arrayLength(tape), initialHeadPosition = 2;
size_t head = initialHeadPosition, programPtr = 0, cyclesUsed = 0;
const size_t programSize = arrayLength(program);

void showError(const char * s) {
  size_t sl = strlen(s);
  tone(SPEAKER, 277.18, 200);
  delay(200);
  tone(SPEAKER, 261.63, 200);
  delay(200);
  int i = 0;
  while (!isButtonDown(BTN_STOP)) {
    lcd.setCursor(6, 3);
    if ((i = (i + 1) & 1) & 1)
      lcd.print(s);
    else
      for (int j = 0; j < sl; j++)
        lcd.print(" ");
    delay(350);
  }
  head = initialHeadPosition;
  programPtr = 0;
  executing = false;
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
        showError("End of tape!");
      break;
    case '>':
      if (head < (tapeSize - 1))
        head++;
      else
        showError("End of tape!");
      break;
    case '.': executing = false; break;
    default: break;
  }
  if (op == '?') {
    programPtr = tape[head] == 0 ? current->jump[0] : current->jump[1];
  } else if (op != '.') {
    programPtr = current->jump[0];
  }
  cyclesUsed++;
}

bool isButtonDown(int PIN) {
  delay(5);
  return digitalRead(PIN) == LOW;
}

void showTape() {
  lcd.setCursor(0, 1);
  for (size_t i = 0; i < tapeSize; i++)
    lcd.print(tape[i] == 1 ? "1" : "0");
  lcd.setCursor(0, 2);
  for (int i = 0; i < COLUMNS; i++)
    lcd.print(" ");
  lcd.setCursor(0, 2);
  for (size_t i = 0; i < head; i++)
    lcd.print(" ");
  lcd.print("^");
}

void showState() {
  lcd.setCursor(0, 3);
  for (int i = 0; i < COLUMNS; i++)
    lcd.print(" ");
  lcd.setCursor(0, 3);
  if (paused) {
    lcd.print("PAUSE");
  } else if (executing) {
    lcd.print("EXEC ");
  } else {
    lcd.print("IDLE ");
    // return;
  }

  if (executing) {
    lcd.print(" | ");

    Instruction * current = &program[programPtr];
    lcd.print(current->operation);

    bool doPrint = current->operation == '?';
    doPrint = doPrint || current->jump[0] != programPtr + 1;
    doPrint = doPrint && current->operation != '.';
    if (doPrint)
      lcd.print(current->jump[0] + 1);
    if (current->operation == '?') {
      lcd.print(",");
      lcd.print(current->jump[1] + 1);
    }
  } else if (showCycles && !firstTimePaused) {
    lcd.print(" | N = ");
    lcd.print(cyclesUsed);
    showCycles = false;
  }
}

void setup() {
  lcd.init();
  lcd.backlight();

  pinMode(BTN_LEFT, INPUT);
  pinMode(BTN_RIGHT, INPUT);
  pinMode(BTN_CHANGE, INPUT);
  pinMode(BTN_RUN, INPUT);
  pinMode(BTN_PAUSE, INPUT);
  pinMode(BTN_STOP, INPUT);
  pinMode(BTN_CLEAR, INPUT);
  pinMode(SPEAKER, OUTPUT);
  digitalWrite(BTN_LEFT, HIGH);
  digitalWrite(BTN_RIGHT, HIGH);
  digitalWrite(BTN_CHANGE, HIGH);
  digitalWrite(BTN_RUN, HIGH);
  digitalWrite(BTN_PAUSE, HIGH);
  digitalWrite(BTN_STOP, HIGH);
  digitalWrite(BTN_CLEAR, HIGH);

  lcd.setCursor(6, 1);
  const char * hello = "Welcome!";
  const size_t helloLen = strlen(hello);
  for (size_t i = 0; i < helloLen; i++) {
    lcd.print(hello[i]);
    delay(65);
  }
  tone(SPEAKER, 392, 100);
  delay(205);
  lcd.setCursor(6, 1);
  for (size_t i = 0; i < helloLen; i++) {
    lcd.print(" ");
    delay(35);
  }
  tone(SPEAKER, 440, 100);
  delay(205);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Nano Post Machine");
  showTape();
}

void loop() {
  bool ready = false;
  #define READY do { (ready) = true; } while (0)

  if (!executing || paused) {
    if (isButtonDown(BTN_LEFT) && head != 0) {
      head -= 1;
      READY;
    } else if (isButtonDown(BTN_RIGHT) && head < (tapeSize - 1)) {
      head += 1;
      READY;
    } else if (isButtonDown(BTN_CHANGE)) {
      tape[head] = 1 - tape[head];
      READY;
    } else if (isButtonDown(BTN_CLEAR)) {
      tone(SPEAKER, 293.66, 50);
      delay(25);
      tone(SPEAKER, 349.23, 50);
      delay(25);

      for (size_t i = 0; i < tapeSize; i++)
        tape[i] = 0;

      lcd.setCursor(6, 3);
      lcd.print("Tape cleared");
      showTape();
      while (!anyButtonDown());
      lcd.setCursor(6, 3);
      for (int i = 0; i < 12; i++)
        lcd.print(" ");
    }
  }
  if (isButtonDown(BTN_RUN)) {
    executing = true;
    cyclesUsed = 0;
    READY;
  } else if (isButtonDown(BTN_PAUSE)) {
    paused = !paused;
    if (paused)
      firstTimePaused = true;
    READY;
  } else if (isButtonDown(BTN_STOP)) {
    executing = false;
    paused = false;
    head = initialHeadPosition;
    programPtr = 0;
    showCycles = true;
    READY;
  }

  if (executing) {
    if (!paused)
      machineCycle();
    if (!paused || paused && firstTimePaused) {
      showState();
      firstTimePaused = false;
    }
  }
  if (ready || executing && !paused) {
    tone(SPEAKER, 2000, 5);
    showTape();
  }
}
