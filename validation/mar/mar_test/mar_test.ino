/* TEST PLAN
 *
 * 1) Read in
 *  a) Internal state should update
 *  b) Should only work in run mode
 * 2) Reset
 *  a) Internal state to 0
 *  b) Should only work in run mode
 * 3) Read in all value
 */

#define CLK_HALF_PERIOD_MS 100

#define BUS_SIZE 4

#define EN_N 11
#define RST 12
#define CLK 13
#define PROG 10
#define IN_RUN_MODE true

int BUS[BUS_SIZE] = {2, 3, 4, 5};
int INTERNAL_STATE[BUS_SIZE] = {9, 8, 7, 6};

/* Basic utility functions */
void print_bin4(int n) {
  for(int i = 0; i < 4; i++) {
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
    print_bin4(expected);
    Serial.print(" - Actual ");
    Serial.print(value);
    Serial.print(" 0b");
    print_bin4(value);
    Serial.println();
    return true;
  } else {
    Serial.print("ERROR (");
    Serial.print(test);
    Serial.print("): Expected ");
    Serial.print(expected);
    Serial.print(" 0b");
    print_bin4(expected);
    Serial.print(" - Actual ");
    Serial.print(value);
    Serial.print(" 0b");
    print_bin4(value);
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
  for (int i=0; i<BUS_SIZE; i++) {
    digitalWrite(BUS[i], (value>>i)&0x1);
  }
  digitalWrite(EN_N, LOW);
  
  clock_pulse();

  digitalWrite(EN_N, HIGH);
}

bool read_prog_mode() {
  return digitalRead(PROG);
}

uint8_t read_internal_state() {
  uint8_t internal_state=0;

  for (int i=BUS_SIZE-1; i>=0; i--) {
    internal_state += digitalRead(INTERNAL_STATE[i]) << i;
  }

  return internal_state;
}

/* Test functions */
bool test_read_in() {
  reset();
  bool pass = true;

  read_in(0b0000);
  pass &= test_equal(read_internal_state(), 0b0000, "Read in");

  read_in(0b0001);
  pass &= test_equal(read_internal_state(), 0b0001, "Read in");

  read_in(0b0010);
  pass &= test_equal(read_internal_state(), 0b0010, "Read in");

  read_in(0b0100);
  pass &= test_equal(read_internal_state(), 0b0100, "Read in");

  read_in(0b1000);
  pass &= test_equal(read_internal_state(), 0b1000, "Read in");

  read_in(0b1111);
  pass &= test_equal(read_internal_state(), 0b1111, "Read in");

  return pass;
}

bool test_reset() {
  reset();
  bool pass = true;

  read_in(0b1010);

  pass &= test_equal(read_internal_state(), 0b1010, "Reset test setup");
  reset();
  pass &= test_equal(read_internal_state(), 0, "Reset test");
  return pass;
}

bool test_noop() {
  reset();
  bool pass = true;

  read_in(0b1010);

  for (int i = 0; i < 5; i++) {
    pass &= test_equal(read_internal_state(), 0b1010, "No OP");
    clock_pulse();
  }
  return pass;
}

bool test_read_in_all() {
  reset();
  bool pass = true;

  for (int i = 0; i < pow(BUS_SIZE, 2); i++) {
    read_in(i);
    pass &= test_equal(read_internal_state(), i, "Read in all");
  }

  return pass;
}


void setup() {
  Serial.begin(9600);

  /* Pin setup */
  pinMode(EN_N, OUTPUT);
  pinMode(RST, OUTPUT);
  pinMode(CLK, OUTPUT);
  
  digitalWrite(EN_N, HIGH);
  digitalWrite(RST, LOW);
  digitalWrite(CLK, LOW);
  
  for (int i=0; i<BUS_SIZE; i++) {
    pinMode(BUS[i], OUTPUT);
    pinMode(INTERNAL_STATE[i], INPUT);
  }

  /* Check MAR is in run mode */
  bool mode = read_prog_mode();
  while (mode != IN_RUN_MODE) {
    Serial.println("MAR in progamming mode. Switch to run mode to start validation.");
    delay(1000);
    mode = read_prog_mode();
  }
  Serial.println("MAR in run mode. Starting validation.");

  /* Tests */
  bool read_in = test_read_in();
  bool reset = test_reset();
  bool noop = test_noop();
  bool read_all = test_read_in_all();

  Serial.println();

  Serial.print("Read in:\t");
  Serial.println(read_in ? "PASS" : "FAIL");
  
  Serial.print("Reset:\t\t");
  Serial.println(reset ? "PASS" : "FAIL");
  
  Serial.print("No OP:\t\t");
  Serial.println(noop ? "PASS" : "FAIL");

  Serial.print("Read in all:\t");
  Serial.println(read_all ? "PASS" : "FAIL");
}

void loop() {}
