/* TEST PLAN
 *
 * 1) PC can count from 0 to 15
 * 2) PC counts with count enable and holds its value otherwise
 * 3) JMP loads a new value into the PC
 * 4) RST sets value back to 0
 * 5) Count is output when count out is active and not otherwise (bus pins pull high)
 * 6) Count overflows from 15 back to 0
 *
 */

#define CLK_HALF_PERIOD_MS 1

#define BUS_SIZE 8
#define COUNTER_SIZE 4
#define BUS_FLOATING 0xFF

#define CO_N 10
#define RST_N 11
#define CLK 12
#define CE 13
#define JMP_N 18 /* SDA */

int BUS[BUS_SIZE] = {2, 3, 4, 5, 6, 7, 8, 9};

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
  digitalWrite(RST_N, LOW);
  clock_pulse();
  digitalWrite(RST_N, HIGH);
}

void jump(uint8_t address) {
  for (int i=0; i<8; i++) {
    pinMode(BUS[i], OUTPUT);
    digitalWrite(BUS[i], (address>>i)&0x1);
  }
  digitalWrite(JMP_N, LOW);
  digitalWrite(CO_N, HIGH);
  
  clock_pulse();
  
  digitalWrite(JMP_N, HIGH);
  for (int i=0; i<8; i++) {
    pinMode(BUS[i], INPUT_PULLUP);
  }
  digitalWrite(CO_N, LOW);
}

uint8_t read_bus() {
  uint8_t bus=0;

  for (int i=BUS_SIZE-1; i>=0; i--) {
    bus += digitalRead(BUS[i]) << i;
  }

  return bus;
}

bool test_count_0_to_max() {
  reset();
  bool pass = true;
  digitalWrite(CO_N, LOW);
  digitalWrite(CE, HIGH);

  uint8_t expected_value = 0;
  uint8_t current_value;

  for (int i=0; i<(1<<COUNTER_SIZE); i++) {
    current_value = read_bus();
    pass &= test_equal(current_value, expected_value++, "Count 0 to max test");
    clock_pulse();
  }

  digitalWrite(CE, LOW);
  digitalWrite(CO_N, HIGH);
  return pass;
}

bool test_count_disable() {
  reset();
  bool pass = true;
  digitalWrite(CO_N, LOW);
  digitalWrite(CE, HIGH);

  /* Count to 3 */
  for (int i=0; i<3; i++) {
    clock_pulse();
  }

  /* Test output doesn't change when count disabled */
  digitalWrite(CE, LOW);
  uint8_t expected_value = 3;
  uint8_t current_value;

  for (int i=0; i<(1<<COUNTER_SIZE); i++) {
    current_value = read_bus();
    pass &= test_equal(current_value, expected_value, "Count disable test");
    clock_pulse();
  }

  digitalWrite(CO_N, HIGH);
  return pass;
}

bool test_jump() {
  jump(12);
  return test_equal(read_bus(), 12, "Jump test");
}

bool test_reset() {
  reset();
  bool pass = true;
  digitalWrite(CO_N, LOW);
  digitalWrite(CE, HIGH);

  /* Count to 3 */
  for (int i=0; i<3; i++) {
    clock_pulse();
  }
  
  pass &= test_equal(read_bus(), 3, "Reset test bus setup");
  clock_pulse();
  reset();
  pass &= test_equal(read_bus(), 0, "Reset test");
  return pass;
}

bool test_count_output() {
  reset();
  bool pass = true;
  digitalWrite(CO_N, LOW);
  digitalWrite(CE, HIGH);

  /* Count to 4 */
  for (int i=0; i<5; i++) {
    pass &= test_equal(read_bus(), i, "Output test");
    clock_pulse();
  }

  /* Continue counting to 9, but without output */
  digitalWrite(CO_N, HIGH);
  for (int i=5; i<10; i++) {
    pass &= test_equal(read_bus(), BUS_FLOATING, "Output test");
    clock_pulse();
  }

  /* Continue counting to 15, with output */
  digitalWrite(CO_N, LOW);
  for (int i=10; i<=15; i++) {
    pass &= test_equal(read_bus(), i, "Output test");
    clock_pulse();
  }

  digitalWrite(CO_N, HIGH);
  digitalWrite(CE, LOW);
  return pass;
}

bool test_overflow() {
  jump(15);
  bool pass = true;

  pass &= test_equal(read_bus(), 15, "Overflow test jump to end");

  digitalWrite(CE, HIGH);
  clock_pulse();
  pass &= test_equal(read_bus(), 0, "Overflow test");

  digitalWrite(CE, LOW);
  return pass;
}

void setup() {
  Serial.begin(57600);

  /* Pin setup */
  pinMode(RST_N, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(JMP_N, OUTPUT);
  pinMode(CO_N, OUTPUT);
  pinMode(CE, OUTPUT);

  digitalWrite(RST_N, HIGH);
  digitalWrite(CLK, LOW);
  digitalWrite(JMP_N, HIGH);
  digitalWrite(CO_N, HIGH);
  digitalWrite(CE, LOW);

  for (int i=0; i<8; i++) {
    pinMode(BUS[i], INPUT_PULLUP);
  }
  
  /* Tests */
  bool count_0_to_15 = test_count_0_to_max();
  bool count_disable = test_count_disable();
  bool jmp = test_jump();
  bool reset = test_reset();
  bool output = test_count_output();
  bool overflow = test_overflow();

  Serial.print("Count 0 to 15:\t");
  Serial.println(count_0_to_15 ? "PASS" : "FAIL");

  Serial.print("Count disable:\t");
  Serial.println(count_disable ? "PASS" : "FAIL");

  Serial.print("Jump:\t\t");
  Serial.println(jmp ? "PASS" : "FAIL");

  Serial.print("Reset:\t\t");
  Serial.println(reset ? "PASS" : "FAIL");

  Serial.print("Output:\t\t");
  Serial.println(output ? "PASS" : "FAIL");

  Serial.print("Overflow:\t");
  Serial.println(overflow ? "PASS" : "FAIL");
}

void loop() {}
