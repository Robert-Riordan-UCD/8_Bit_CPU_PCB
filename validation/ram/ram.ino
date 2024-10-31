/* TEST PLAN
 *
 * 1) Write to all addresses
 * 2) Read from all addresses
 * 3) No-op
 *  a) Intenal state should remain as long as read in is not asserted
 * 4) Test write then read for every value 0x00 to 0xFF
 * 5) Unable to write while PROG is asserted
 */

#define CLK_HALF_PERIOD_MS 10

#define BUS_SIZE 8
#define ADDR_SIZE 4

#define PROG 22
#define RO_N 10
#define CLK 11
#define RI 12

int BUS[BUS_SIZE] = {2, 3, 4, 5, 6, 7, 8, 9};
int ADDRESS[ADDR_SIZE] = {24, 26, 28, 30};

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

// Load a value from the bus
void write_to_ram(uint8_t value, uint8_t addr) {
  for (int i=0; i<ADDR_SIZE; i++) {
    digitalWrite(ADDRESS[i], (addr>>i)&1);
  }
  
  for (int i=0; i<BUS_SIZE; i++) {
    pinMode(BUS[i], OUTPUT);
    digitalWrite(BUS[i], (value>>i)&1);
  }
  digitalWrite(RI, HIGH);
  
  clock_pulse();

  digitalWrite(RI, LOW);
  for (int i=0; i<8; i++) {
    pinMode(BUS[i], INPUT_PULLUP);
  }
}

// Get the value from the register on to the bus
uint8_t read_from_ram(uint8_t addr) {
  uint8_t bus = 0;
  
  for (int i=0; i<ADDR_SIZE; i++) {
    digitalWrite(ADDRESS[i], (addr>>i)&1);
  }
  
  digitalWrite(RO_N, LOW);
  for (int i=0; i<BUS_SIZE; i++) {
    bus += digitalRead(BUS[i])<<i;
  }
  
  digitalWrite(RO_N, HIGH);
  return bus;
}

/* Test functions */

void setup() {
  Serial.begin(9600);

  /* Pin setup */
  pinMode(PROG, OUTPUT);
  pinMode(RO_N, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(RI, OUTPUT);

  digitalWrite(PROG, HIGH);
  digitalWrite(RO_N, HIGH);
  digitalWrite(CLK, LOW);
  digitalWrite(RI, LOW);
  
  for (int i=0; i<8; i++) {
    pinMode(BUS[i], INPUT_PULLUP);
    pinMode(ADDRESS[i], OUTPUT);
  }

  /* Tests */
  write_to_ram(0b11110000, 0);
  Serial.println(read_from_ram(0));
}

void loop() {}
