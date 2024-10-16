/* TEST PLAN
 *
 * 1) Read in
 *  a) Internal state should update
 *    i) Instruction register only has 4 bits of readable internal state
 *  b) Bus should be floating
 * 2) Write out
 *  a) Bus should match value writen earlier
 *  b) Internals state should match bus
 * 3) Reset
 *  a) Internal state to 0
 * 4) No-op
 *  a) Intenal state should remain as long as read in or reset are not asserted
 * 5) Test write then read for every value 0x00 to 0xFF
 */

#define CLK_HALF_PERIOD_MS 10

#define BUS_SIZE 8
#define INTERNAL_BUS_SIZE 4

#define WO_N 5
#define RST 4
#define RI_N 3
#define CLK 2

int BUS[BUS_SIZE] = {36, 34, 32, 30, 28, 26, 24, 22};
int INTERNAL_STATE[INTERNAL_BUS_SIZE] = {9, 8, 7, 6};

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
  digitalWrite(RST, HIGH);
  clock_pulse();
  digitalWrite(RST, LOW);
}

// Load a value from the bus to the register
void read_in(int value) {
  for (int i=0; i<8; i++) {
    pinMode(BUS[i], OUTPUT);
    digitalWrite(BUS[i], (value>>i)&0x1);
  }
  digitalWrite(RI_N, LOW);
  
  clock_pulse();

  digitalWrite(RI_N, HIGH);
  for (int i=0; i<8; i++) {
    pinMode(BUS[i], INPUT_PULLUP);
  }
}

uint8_t read_bus() {
  uint8_t bus=0;

  for (int i=BUS_SIZE-1; i>=0; i--) {
    bus += digitalRead(BUS[i]) << i;
  }

  return bus;
}

uint8_t read_internal_state() {
  uint8_t internal_state=0;

  for (int i=INTERNAL_BUS_SIZE-1; i>=0; i--) {
    internal_state += digitalRead(INTERNAL_STATE[i]) << i;
  }

  return internal_state<<(BUS_SIZE-INTERNAL_BUS_SIZE);
}

// Get the value from the register on to the bus
uint8_t write_out() {
  digitalWrite(WO_N, LOW);
  clock_pulse();

  uint8_t bus = read_bus();
  digitalWrite(WO_N, HIGH);
  clock_pulse();
  
  return bus;
}

/* Test functions */
bool test_read_in() {
  reset();
  bool pass = true;

  read_in(0b00000000);
  pass &= test_equal(read_internal_state(), 0b0000, "Read in");

  read_in(0b00010000);
  pass &= test_equal(read_internal_state(), 0b00010000, "Read in");

  read_in(0b00100000);
  pass &= test_equal(read_internal_state(), 0b00100000, "Read in");

  read_in(0b01000000);
  pass &= test_equal(read_internal_state(), 0b01000000, "Read in");

  read_in(0b10000000);
  pass &= test_equal(read_internal_state(), 0b10000000, "Read in");

  read_in(0b11110000);
  pass &= test_equal(read_internal_state(), 0b11110000, "Read in");

  return pass;
}

bool test_write_out() {
  reset();
  bool pass = true;

  read_in(0b10101010);
  uint8_t value = write_out();

  pass &= test_equal(value, 0b10101010, "Write out");
  return pass;
}

bool test_reset() {
  reset();
  bool pass = true;

  read_in(0b10101010);

  pass &= test_equal(read_internal_state(), 0b10100000, "Reset test setup");
  reset();
  pass &= test_equal(read_internal_state(), 0, "Reset test");
  return pass;
}

bool test_noop() {
  reset();
  bool pass = true;

  read_in(0b10101010);

  for (int i = 0; i < 5; i++) {
    pass &= test_equal(read_internal_state(), 0b10100000, "No OP");
    clock_pulse();
  }
  return pass;
}

int test_read_in_write_out_all() {
  reset();
  int pass_count = 0;

  for (unsigned int i = 0; i <= 0xFF; i++) {
    read_in(i & 0xFF);
    pass_count += test_equal(write_out(), i & 0xFF, "Read/Write");
  }
  return pass_count;
}

void setup() {
  Serial.begin(9600);

  /* Pin setup */
  pinMode(WO_N, OUTPUT);
  pinMode(RST, OUTPUT);
  pinMode(RI_N, OUTPUT);
  pinMode(CLK, OUTPUT);
  
  digitalWrite(WO_N, HIGH);
  digitalWrite(RST, LOW);
  digitalWrite(RI_N, HIGH);
  digitalWrite(CLK, LOW);
  
  for (int i=0; i<8; i++) {
    pinMode(BUS[i], INPUT_PULLUP);
    pinMode(INTERNAL_STATE[i], INPUT_PULLUP);
  }

  /* Tests */
  bool read_in = test_read_in();
  bool write_out = test_write_out();
  bool reset = test_reset();
  bool noop = test_noop();
  int read_in_write_out_all = test_read_in_write_out_all();

  Serial.println();

  Serial.print("Read in:\t");
  Serial.println(read_in ? "PASS" : "FAIL");
  
  Serial.print("Write out:\t");
  Serial.println(write_out ? "PASS" : "FAIL");
  
  Serial.print("Reset:\t\t");
  Serial.println(reset ? "PASS" : "FAIL");
  
  Serial.print("No OP:\t\t");
  Serial.println(noop ? "PASS" : "FAIL");
  
  Serial.print("Read/Write:\t");
  Serial.print(read_in_write_out_all >= 100 ? "PASS" : "FAIL");
  Serial.print("\t(Passed ");
  Serial.print(read_in_write_out_all);
  Serial.println("/256)");
}

void loop() {}
