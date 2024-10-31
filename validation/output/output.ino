/* TEST PLAN
 *
 * 1) Display all numbers
 * 2) Enable test
 *  a) Count in 1s
 *  b) Only enable on even numbers
 * 3) Reset
 * 4) Display all numbers with 2's complement
 */

#define CLK_HALF_PERIOD_MS 10
#define WAIT_TIME 250

#define BUS_SIZE 8

#define CLK 2
#define EN_N 3
#define RST 4
#define TWO_COMP 5

int BUS[BUS_SIZE] = {6, 7, 8, 9, 10, 11, 12, 13};

/* Basic utility functions */
void print_bin8(int n) {
  for(int i = 7; i >= 0; i--) {
    Serial.print((n>>i)&1);
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

void set_bus(uint8_t value) {
  for (int i=0; i<BUS_SIZE; i++) {
    digitalWrite(BUS[i], (value>>i)&0x1);
  }
}

// Load a value from the bus to the output register
void display(int value) {
  set_bus(value);
  digitalWrite(EN_N, LOW);
  clock_pulse();
  digitalWrite(EN_N, HIGH);
}

/* Test functions */
void test_count_all() {
  Serial.println("Count all test running");
  Serial.println("\tDisplay should begin at 0 and count to 255");
  for (int i=0; i<pow(2, BUS_SIZE); i++) {
    display(i);
    delay(WAIT_TIME);
  }
}

void test_enable() {
  Serial.println("Enable test running");
  Serial.println("\tDisplay should begin at 0 and count only even numbers up to 255");
  for (int i=0; i<pow(2, BUS_SIZE); i++) {
    if (i%2) {
      set_bus(i);
      clock_pulse();
    } else {
      display(i);
    }
    delay(WAIT_TIME);
  }
}

void test_reset() {
  Serial.println("Reset test running");
  Serial.println("\tDisplay should read 123");
  display(123);
  delay(WAIT_TIME);
  Serial.println("\tReseting");
  Serial.println("\tDisplay should read 0");
  reset();
  delay(WAIT_TIME);
}

void test_count_all_two_comp() {
  Serial.println("Twos complement test running");
  Serial.println("\tDisplay should begin at 0, count to 127, then count from -128 to -1");
  digitalWrite(TWO_COMP, HIGH);
  for (int i=0; i<pow(2, BUS_SIZE); i++) {
    display(i);
    delay(WAIT_TIME);
  }
}

void setup() {
  Serial.begin(9600);

  /* Pin setup */
  pinMode(CLK, OUTPUT);
  pinMode(EN_N, OUTPUT);
  pinMode(RST, OUTPUT);
  pinMode(TWO_COMP, OUTPUT);
  
  digitalWrite(CLK, LOW);
  digitalWrite(EN_N, HIGH);
  digitalWrite(RST, LOW);
  digitalWrite(TWO_COMP, LOW);

  for (int i=0; i<8; i++) {
    pinMode(BUS[i], OUTPUT);
  }

  reset();

  /* Tests */
  test_count_all();
  test_enable();
  test_reset();
  test_count_all_two_comp();
}

void loop() {}
