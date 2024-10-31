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

// Load a value from the bus to the output register
void display(int value) {
  for (int i=0; i<BUS_SIZE; i++) {
    digitalWrite(BUS[i], (value>>i)&0x1);
  }
  digitalWrite(EN_N, LOW);
  
  clock_pulse();

  digitalWrite(EN_N, HIGH);
}

/* Test functions */

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
  display(123);
}

void loop() {}
