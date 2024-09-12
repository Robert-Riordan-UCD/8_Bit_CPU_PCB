/* TEST PLAN
 *
 * 1) Read in
 *  a) Internal state should update
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

#define CLK_HALF_PERIOD_MS 1

#define BUS_SIZE 8
#define BUS_FLOATING 0xFF

#define WO_N 13
#define RST 12
#define RI_N 11
#define CLK 10

int BUS[BUS_SIZE] = {2, 3, 4, 5, 6, 7, 8, 9};
int INTERNAL_STATE[BUS_SIZE] = {18 /* SDA */, 19 /* SCL */, A0, A1, A2, A3, A4, A5};

/* Basic utility functions */
bool test_equal(int value, int expected, char* test) {
  if (value == expected) {
    Serial.print("Pass  (");
    Serial.print(test);
    Serial.print("): Expected ");
    Serial.print(expected);
    Serial.print(" - Actual ");
    Serial.println(value);
    return true;
  } else {
    Serial.print("ERROR (");
    Serial.print(test);
    Serial.print("): Expected ");
    Serial.print(expected);
    Serial.print(" - Actual ");
    Serial.println(value);
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
  uint8_t interanl_state=0;

  for (int i=BUS_SIZE-1; i>=0; i--) {
    interanl_state += digitalRead(INTERNAL_STATE[i]) << i;
  }

  return interanl_state;
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

  read_in(0b10101010);

  pass &= test_equal(read_internal_state(), 0b10101010, "Read in");
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

  pass &= test_equal(read_internal_state(), 0b10101010, "Reset test setup");
  reset();
  pass &= test_equal(read_internal_state(), 0, "Reset test");
  return pass;
}

bool test_noop() {
  reset();
  bool pass = true;

  read_in(0b10101010);

  for (int i = 0; i < 5; i++) {
    pass &= test_equal(read_internal_state(), 0b10101010, "No OP");
    clock_pulse();
  }
  return pass;
}

bool test_read_in_write_out_all() {
  reset();
  bool pass = true;

  for (unsigned int i = 0; i <= 0xFF; i++) {
    read_in(i & 0xFF);
    pass &= test_equal(read_internal_state(), i & 0xFF, "Read/Write");
  }
  return pass;
}

void setup() {
  Serial.begin(57600);

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
    pinMode(INTERNAL_STATE[i], INPUT);
  }

  /* Tests */
  bool read_in = test_read_in();
  bool write_out = test_write_out();
  bool reset = test_reset();
  bool noop = test_noop();
  bool read_in_write_out_all = test_read_in_write_out_all();

  Serial.println();

  Serial.print("Read in:\t");
  Serial.println(read_in ? "PASS" : "FAIL");
  Serial.print("Write out:\t");
  Serial.println(write_out ? "PASS" : "FAIL");
  Serial.print("Reset:\t\t");
  Serial.println(reset ? "PASS" : "FAIL");
  Serial.print("No OP:\t\t");
  Serial.println(reset ? "PASS" : "FAIL");
  Serial.print("Read/Write:\t");
  Serial.println(read_in_write_out_all ? "PASS" : "FAIL");
}

void loop() {}
