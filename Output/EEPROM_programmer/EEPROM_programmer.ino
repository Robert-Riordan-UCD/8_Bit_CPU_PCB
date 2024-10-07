#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4

#define WRITE_EN_N 13

#define NUM_IO 8
const int IO_PINS[8] = {5, 6, 7, 8, 9, 10, 11, 12};

const uint8_t digits[10] = {
  0b11111100, /* 0 */
  0b01100000, /* 1 */
  0b11011010, /* 2 */
  0b11110010, /* 3 */
  0b01100110, /* 4 */
  0b10110110, /* 5 */
  0b10111110, /* 6 */
  0b11100000, /* 7 */
  0b11111110, /* 8 */
  0b11110110  /* 9 */  
};

void setAddress(uint16_t address, bool output_enable) {
  digitalWrite(SHIFT_LATCH, LOW);

  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address>>8 | (output_enable ? 0 : 0b10000000));
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);

  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
}

uint8_t readEEPROM(uint16_t address) {
  setAddress(address, true);
  
  uint8_t data = 0;
  for (int i = NUM_IO-1; i >= 0; i--) {
    pinMode(IO_PINS[i], INPUT);
    data = (data<<1) + digitalRead(IO_PINS[i]);
  }

  return data;
}

void writeEEPROM(uint16_t address, uint8_t data) {
  digitalWrite(WRITE_EN_N, HIGH);
  setAddress(address, false);

  for (int i = 0; i < NUM_IO; i++) {
    pinMode(IO_PINS[i], OUTPUT);
    digitalWrite(IO_PINS[i], (data>>i)&1);
  }

  digitalWrite(WRITE_EN_N, LOW);
  delayMicroseconds(1);
  digitalWrite(WRITE_EN_N, HIGH);
  delay(15);
}

void setup() {
  Serial.begin(9600);
  Serial.println();

  /* Pin setup */
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);

  digitalWrite(WRITE_EN_N, HIGH);
  pinMode(WRITE_EN_N, OUTPUT);
  
  for (int i = NUM_IO-1; i >= 0; i--) {
    pinMode(IO_PINS[i], INPUT);
  }

  /* Write digits to all values address values */
  for (int address = 0; address <= 255; address++) {
    writeEEPROM(address, digits[(address/100)%100]); // 100's
    writeEEPROM(address+256, digits[address%10]); // 1's
    writeEEPROM(address+256*2, digits[(address/10)%10]); // 10's
    writeEEPROM(address+256*3, 0); // 1000's. Always off because the max is 255 (8-bits)
    Serial.print("Writing address ");
    Serial.println(address);
  }

  /* Read data out */
  for (int i = 0; i < 256; i++) {
    uint8_t data = readEEPROM(i);
    if (data < 0x10) {
      Serial.print('0');
      Serial.print(data, HEX);      
    } else {
      Serial.print(data, HEX);
    }
    Serial.print(' ');
    if ((i+1)%16 == 0) {
      Serial.println();
    }
  }
}

void loop() {}
