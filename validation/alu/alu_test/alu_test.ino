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
 * 6) Subtract
 *  a) Check every value of A and B can be subtracted correctly
 *  b) Check flags are set correctly
 * 7) Flags
 *  a) Check flags toggle only on clock/flags in (I'm using the ALU without the FI signal so just checking on CLK)
 * 8) Flags reset
 *  a) Check flags are cleared on reset
 */
#define DEBUG false

#define CLK_HALF_PERIOD_MS 100

#define BUS_SIZE 8
#define BUS_FLOATING 0xFF

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

uint8_t twos_comp(uint8_t value) {
  return (value^0xFF)+1;
}

bool test_equal(int value, int expected, char* test) {
  char print_buffer[200];
  if (value == expected) {
    if (DEBUG) {
//      print_bin8(expected);
//      print_bin8(value);
      sprintf(print_buffer, "PASS (%s): Expected %03d - Actual %03d", test, expected, value);
      Serial.println(print_buffer);
    }
    return true;
  } else {
    sprintf(print_buffer, "ERROR (%s): Expected %03d - Actual %03d", test, expected, value);
    Serial.println(print_buffer);
//    print_bin8(expected);
//    print_bin8(value);
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

/* Test functions */
bool test_a_pass_through() {
  bool pass = true;

  load_A(0);
  load_B(0);
  digitalWrite(WRITE_N, LOW);
  
  pass &= test_equal(read_bus(), 0, "A pass through");

  for (int i=0; i<BUS_SIZE; i++) {
    load_A(1<<i);
    pass &= test_equal(read_bus(), 1<<i, "A pass through");
  }

  digitalWrite(WRITE_N, HIGH);

  return pass;
}

bool test_b_pass_through() {
  bool pass = true;

  load_A(0);
  load_B(0);
  digitalWrite(WRITE_N, LOW);
  
  pass &= test_equal(read_bus(), 0, "B pass through");

  for (int i=0; i<BUS_SIZE; i++) {
    load_B(1<<i);
    pass &= test_equal(read_bus(), 1<<i, "B pass through");
  }

  digitalWrite(WRITE_N, HIGH);

  return pass;
}

bool test_b_comp_pass_through() {
  bool pass = true;

  load_A(0);
  load_B(0);
  digitalWrite(WRITE_N, LOW);
  digitalWrite(SUB, HIGH);
  
  pass &= test_equal(read_bus(), twos_comp(0), "2's comp pass through");

  for (int i=0; i<BUS_SIZE; i++) {
    load_B(1<<i);
    pass &= test_equal(read_bus(), twos_comp(1<<i), "2's comp pass through");
  }

  digitalWrite(WRITE_N, HIGH);
  digitalWrite(SUB, LOW);

  return pass;
}

bool test_bus_float() {
  bool pass = true;
  
  load_A(0);
  load_B(0);
  digitalWrite(WRITE_N, HIGH);
  
  pass &= test_equal(read_bus(), BUS_FLOATING, "Bus floating");
  digitalWrite(WRITE_N, LOW);
  pass &= test_equal(read_bus(), 0, "Bus not floating");
  digitalWrite(WRITE_N, HIGH);

  for (int i=0; i<BUS_SIZE; i++) {
    load_A(1<<i);
    pass &= test_equal(read_bus(), BUS_FLOATING, "Bus floating");
    digitalWrite(WRITE_N, LOW);
    pass &= test_equal(read_bus(), 1<<i, "Bus not floating");
    digitalWrite(WRITE_N, HIGH);
  }
  
  return pass;
}

float test_add() {
  reset();
  digitalWrite(WRITE_N, LOW);
  float pass_count = 0;

  float test = 0;
  uint16_t result = ((uint16_t)test&0xFF) + ((uint16_t)test>>8);
  char print_buffer[16];

  do {
    load_A((uint16_t)test&0xFF);
    load_B((uint16_t)test>>8);
    result = ((uint16_t)(test)&0xFF) + ((uint16_t)test>>8);
    sprintf(print_buffer, "Add %03d + %03d", (uint16_t)test&0xFF, (uint16_t)test>>8);
    pass_count += test_equal(read_bus(), result%256, print_buffer);
    test++;
  } while (test < 0x10000);
  
  digitalWrite(WRITE_N, HIGH);
  
  return pass_count;
}

float test_sub() {
  reset();
  digitalWrite(WRITE_N, LOW);
  digitalWrite(SUB, HIGH);
  float pass_count = 0;

  float test = 0;
  uint16_t result = ((uint16_t)test&0xFF) + ((uint16_t)test>>8);
  char print_buffer[16];

  do {
    load_A((uint16_t)test&0xFF);
    load_B((uint16_t)test>>8);
    result = ((uint16_t)(test)&0xFF) + twos_comp(((uint16_t)test>>8));
    sprintf(print_buffer, "Subtract %03d - %03d", (uint16_t)test&0xFF, (uint16_t)test>>8);
    pass_count += test_equal(read_bus(), result%256, print_buffer);
    test++;
  } while (test < 0x10000);
  
  digitalWrite(WRITE_N, HIGH);
  digitalWrite(SUB, LOW);
  
  return pass_count;
}

/* Main program */
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
  Serial.println();

  Serial.print("A pass through:\t\t\t");
  bool a_pass_through = test_a_pass_through();
  Serial.println(a_pass_through ? "PASS" : "FAIL");

  Serial.print("B pass through:\t\t\t");
  bool b_pass_through = test_b_pass_through();
  Serial.println(b_pass_through ? "PASS" : "FAIL");
  
  Serial.print("B 2's comp pass through:\t");
  bool b_comp_pass_through = test_b_comp_pass_through();
  Serial.println(b_comp_pass_through ? "PASS" : "FAIL");
  
  Serial.print("Bus floating:\t\t\t");
  bool bus_float = test_bus_float();
  Serial.println(bus_float ? "PASS" : "FAIL");
  
  Serial.print("Add:\t\t\t\t");
  float add = test_add();
  Serial.print(add >= pow(2, 2*BUS_SIZE) ? "PASS" : "FAIL");
  Serial.print("\t(Passed ");
  Serial.print(add);
  Serial.print("/");
  Serial.print(pow(2, 2*BUS_SIZE));
  Serial.println(")");

  Serial.print("Subtract:\t\t\t");
  float sub = test_sub();
  Serial.print(sub >= pow(2, 2*BUS_SIZE) ? "PASS" : "FAIL");
  Serial.print("\t(Passed ");
  Serial.print(sub);
  Serial.print("/");
  Serial.print(pow(2, 2*BUS_SIZE));
  Serial.println(")");
}

void loop() {}
