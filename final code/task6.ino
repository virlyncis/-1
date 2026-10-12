const byte BUTTON_MASK = 0b00011100; // PD2, PD3, PD4
const byte LED_MASK    = 0b00111000; // PB3, PB4, PB5

const unsigned long DEBOUNCE_MS = 50;
const unsigned long PERIOD_MS   = 100;

byte rawButtons = 0;
byte stableButtons = 0;
byte pressedButtons = 0;

unsigned long rawChangeTime = 0;
unsigned long lastPrintTime = 0;

byte lastOutput = 0;
bool firstOutputMethod = true;

void printBinary(byte value) {
  for (int i = 7; i >= 0; i--) {
    Serial.print((value >> i) & 1);
  }
}

int firstBitByLoop(byte value) {
  if (value == 0) return -1;
  for (int i = 0; i < 8; i++) {
    if (value & (1 << i)) return i;
  }
  return -1;
}

int firstBitByBuiltin(byte value) {
  if (value == 0) return -1;
  return __builtin_ctz((unsigned int)value);
}

void writeByMask(byte value) {
  PORTB = (PORTB & ~LED_MASK) | (value & LED_MASK);
}

void writeByBitSet(byte value) {
  for (byte bit = 3; bit <= 5; bit++) {
    bitClear(PORTB, bit);
  }
  for (byte bit = 3; bit <= 5; bit++) {
    if (value & (1 << bit)) {
      bitSet(PORTB, bit);
    }
  }
}

void setup() {
  Serial.begin(9600);

  DDRD &= ~BUTTON_MASK;
  PORTD |= BUTTON_MASK;

  DDRB |= LED_MASK;
  PORTB &= ~LED_MASK;

  rawButtons = PIND & BUTTON_MASK;
  stableButtons = rawButtons;
  rawChangeTime = millis();

  Serial.println("Laboratory work 1");
  Serial.println("Buttons: D2, D3, D4");
  Serial.println("LEDs: D11, D12, D13");
}

void loop() {
  unsigned long now = millis();

  byte portValue = PIND;
  byte currentRaw = portValue & BUTTON_MASK;

  if (currentRaw != rawButtons) {
    rawButtons = currentRaw;
    rawChangeTime = now;
  }

  if (now - rawChangeTime >= DEBOUNCE_MS) {
    stableButtons = rawButtons;
  }

  pressedButtons = (~stableButtons) & BUTTON_MASK;

  byte outputValue = 0;

  if (pressedButtons & (1 << 2)) outputValue |= (1 << 3);
  if (pressedButtons & (1 << 3)) outputValue |= (1 << 4);
  if (pressedButtons & (1 << 4)) outputValue |= (1 << 5);

  if (outputValue != lastOutput) {
    if (firstOutputMethod) {
      writeByMask(outputValue);
    } else {
      writeByBitSet(outputValue);
    }
    firstOutputMethod = !firstOutputMethod;
    lastOutput = outputValue;
  }

  if (now - lastPrintTime >= PERIOD_MS) {
    lastPrintTime = now;

    Serial.print("PIND = ");
    printBinary(portValue);

    Serial.print(" | Pressed positions: ");
    if (pressedButtons == 0) {
      Serial.print("none");
    } else {
      for (byte bit = 2; bit <= 4; bit++) {
        if (pressedButtons & (1 << bit)) {
          Serial.print(bit);
          Serial.print(" ");
        }
      }
    }

    Serial.print(" | Output = ");
    printBinary(outputValue);

    int loopResult = firstBitByLoop(pressedButtons);
    int builtinResult = firstBitByBuiltin(pressedButtons);

    Serial.print(" | First bit by loop = ");
    Serial.print(loopResult);
    Serial.print(" | First bit by builtin = ");
    Serial.println(builtinResult);
  }
}