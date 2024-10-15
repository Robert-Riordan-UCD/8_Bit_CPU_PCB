#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4

#define WRITE_EN_N 13

#define NUM_IO 8
int IO_PINS[8] = {5, 6, 7, 8, 9, 10, 11, 12};
// Address pin uses
#define ZERO       0
#define CARRY      1
#define T2         2
#define T1         3
#define T0         4
#define EEPROM_SEL 5
#define SPARE0     6
#define SPARE1     7
#define IR4        8
#define IR5        9
#define IR7        10
#define IR6        11
#define SPARE2     12

#define ADDRESS_SIZE 13

// Control signals
#define PCO     0b0000000000000001
#define ALU_SUB 0b0000000000000010
#define ALU_FI  0b0000000000000100
#define PC_JMP  0b0000000000001000
#define CLK_HLT 0b0000000000010000
#define PCE     0b0000000000100000
#define AI      0b0000000001000000
#define AO      0b0000000010000000

#define OUT_EN  0b0000000100000000
#define BI      0b0000001000000000
#define ALU_OUT 0b0000010000000000
#define IO      0b0000100000000000
#define II      0b0001000000000000
#define RI      0b0010000000000000
#define RO      0b0100000000000000
#define MI      0b1000000000000000

// Fetch cycle
#define FETCH   MI|PCO, RO|II|PCE

// Instruction set
#define NOOP { FETCH,         0,                         0, 0, 0, 0, 0}
#define LDA  { FETCH,     IO|MI,                     RO|AI, 0, 0, 0, 0}
#define STA  { FETCH,     IO|MI,                     AO|RI, 0, 0, 0, 0}
#define LDB  { FETCH,     IO|MI,                     RO|BI, 0, 0, 0, 0}
#define ADD  { FETCH,     IO|MI,         ALU_OUT|AI|ALU_FI, 0, 0, 0, 0}
#define SUB  { FETCH,     IO|MI, ALU_OUT|ALU_SUB|AI|ALU_FI, 0, 0, 0, 0}
#define JMP  { FETCH, IO|PC_JMP,                         0, 0, 0, 0, 0}
#define OUT  { FETCH, AO|OUT_EN,                         0, 0, 0, 0, 0}
#define HALT { FETCH,   CLK_HLT,                         0, 0, 0, 0, 0}
#define JMP_CARRY_FALSE NOOP
#define JMP_CARRY_TRUE  JMP
#define JMP_ZERO_FALSE  NOOP
#define JMP_ZERO_TRUE   JMP

const uint16_t code[4][16][8] = {
  {NOOP, LDA, STA, LDB, ADD, SUB, JMP, JMP_CARRY_FALSE, JMP_ZERO_FALSE, NOOP, NOOP, NOOP, NOOP, NOOP, OUT, HALT}, // Zero = 0, Carry = 0
  {NOOP, LDA, STA, LDB, ADD, SUB, JMP, JMP_CARRY_FALSE,  JMP_ZERO_TRUE, NOOP, NOOP, NOOP, NOOP, NOOP, OUT, HALT}, // Zero = 0, Carry = 1
  {NOOP, LDA, STA, LDB, ADD, SUB, JMP,  JMP_CARRY_TRUE, JMP_ZERO_FALSE, NOOP, NOOP, NOOP, NOOP, NOOP, OUT, HALT}, // Zero = 1, Carry = 0
  {NOOP, LDA, STA, LDB, ADD, SUB, JMP,  JMP_CARRY_TRUE,  JMP_ZERO_TRUE, NOOP, NOOP, NOOP, NOOP, NOOP, OUT, HALT}  // Zero = 1, Carry = 1
};

const int rom_size = pow(2, ADDRESS_SIZE);

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
  
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);

  digitalWrite(WRITE_EN_N, HIGH);
  pinMode(WRITE_EN_N, OUTPUT);

  Serial.println("Addr: Flags, Inst, Step, Control, Boot");
  for (int address = 0; address < rom_size; address++) {
    uint8_t bootload = (
      (((address>>SPARE1)&1) << 1) |
      (((address>>SPARE0)&1) << 0)
    );
    
    uint8_t flags = (
      (((address>>CARRY)&1) << 0) |
      (((address>>ZERO )&1) << 1)
    );
    
    uint8_t instruction = (
      (((address>>IR4)&1) << 0) |
      (((address>>IR5)&1) << 1) |
      (((address>>IR6)&1) << 2) |
      (((address>>IR7)&1) << 3)
    );

    uint8_t step = (
      (((address>>T0)&1) << 0) |
      (((address>>T1)&1) << 1) |
      (((address>>T2)&1) << 2)
    );

    char buffer[100];
    sprintf(buffer, "0x%03x:    %d, 0x%01x,    %d,  0x%04x, 0x%01x",
                     address, flags, instruction, step, code[flags][instruction][step], bootload);
    Serial.println(buffer);

    switch (bootload) {
      case 0b00:
        if ((address>>EEPROM_SEL)&1) {
          writeEEPROM(address, code[flags][instruction][step]);
        } else {
          writeEEPROM(address, code[flags][instruction][step]>>8);
        }
        break;
      case 0b01: writeEEPROM(address, MI); break;
      case 0b10: writeEEPROM(address, RI); break;
      case 0b11: writeEEPROM(address, 0); break;
    }
  }

  // NOTE: First bit read after a write is usually wrong so reading now to clear the issue
  readEEPROM(0);

  for (int i = 0; i < rom_size; i++) {
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
