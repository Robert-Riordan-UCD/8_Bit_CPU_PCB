/* TEST PLAN
 *
 * 1) A reg pass through
 *  a) Set B to 0
 *  b) Check each of the bits on A can be added correctly to 0
 * 2) B reg pass through
 *  a) Set A to 0
 *  b) Check each of the bits on B can be added correctly to 0
 * 3) B reg 2's complement pass through
 *  a) Set A to 0
 *  b) Check each of the bits on B are inverted corretly when subtracted from 0
 * 4) Bus floating
 *  a) Check bus in only driven when write is set
 * 5) Add
 *  a) Check every value of A and B can be added correctly
 *  b) Check flags are set correctly
 * 6) Add
 *  a) Check every value of A and B can be subtracted correctly
 *  b) Check flags are set correctly
 * 7) Flags
 *  a) Check flags toggle only on clock/flags in (I'm using the ALU without the FI signal so just checking on CLK)
 * 8) Flags reset
 *  a) Check flags are cleared on reset
 */

#define CLK_HALF_PERIOD_MS 10

#define BUS_SIZE 8

#define WRITE_N 22
#define SUB 24
#define RST_N 26
#define CLK 28
#define CARRY 30
#define ZERO 32

const int BUS[BUS_SIZE] = {37, 35, 33, 31, 29, 27, 25, 23};
const int A_REG[BUS_SIZE] = {39, 41, 43, 45, 47, 49, 51, 53};
const int B_REG[BUS_SIZE] = {38, 40, 42, 44, 46, 48, 50, 52};

/* Basic utility functions */
void print_bin8(int n) {
  for(int i = 7; i >= 0; i--) {
    Serial.print((n>>i)&1);
  }
}

bool test_equal(int value, int expected, char* test) {
  if (value == expected) {
    Serial.print("Pass  (");
    Serial.print(test);
    Serial.print("): Expected ");
    Serial.print(expected);
    Serial.print(" 0b");
    print_bin8(expected);
    Serial.print(" - Actual ");
    Serial.print(value);
    Serial.print(" 0b");
    print_bin8(value);
    Serial.println();
    return true;
  } else {
    Serial.print("ERROR (");
    Serial.print(test);
    Serial.print("): Expected ");
    Serial.print(expected);
    Serial.print(" 0b");
    print_bin8(expected);
    Serial.print(" - Actual ");
    Serial.print(value);
    Serial.print(" 0b");
    print_bin8(value);
    Serial.println();
    return false;
  }
}

void clock_pulse() {
  digitalWrite(CLK, HIGH);
  delay(CLK_HALF_PERIOD_MS);
  digitalWrite(CLK, LOW);
  delay(CLK_HALF_PERIOD_MS);
}

void reset() {
  digitalWrite(RST_N, LOW);
  clock_pulse();
  digitalWrite(RST_N, HIGH);
}

uint8_t read_bus() {
  uint8_t bus=0;

  for (int i=BUS_SIZE-1; i>=0; i--) {
    bus += digitalRead(BUS[i]) << i;
  }

  return bus;
}

void load_A(uint8_t value) {
  for (int i=0; i<8; i++) {
    digitalWrite(A_REG[i], (value>>i)&0x1);
  }
}

void load_B(uint8_t value) {
  for (int i=0; i<8; i++) {
    digitalWrite(B_REG[i], (value>>i)&0x1);
  }
}

void setup() {
  Serial.begin(9600);

  /* Pin setup */
  pinMode(WRITE_N, OUTPUT); digitalWrite(WRITE_N, HIGH);
  pinMode(SUB, OUTPUT); digitalWrite(SUB, LOW);
  pinMode(RST_N, OUTPUT); digitalWrite(RST_N, HIGH);
  pinMode(CLK, OUTPUT); digitalWrite(CLK, LOW);
  pinMode(CARRY, INPUT);
  pinMode(ZERO, INPUT);

  for (int i=0; i<8; i++) {
    pinMode(BUS[i], INPUT_PULLUP);
    pinMode(A_REG[i], OUTPUT);
    pinMode(B_REG[i], OUTPUT);
  }

  /* Tests */
}

void loop() {}
