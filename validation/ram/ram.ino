/* TEST PLAN
 *
 * 1) Write to all addresses
 * 2) Read from all addresses
 * 3) No-op
 *  a) Intenal state should remain as long as read in is not asserted
 * 4) Test write then read for every value 0x00 to 0xFF
 * 5) Unable to write while PROG is asserted
 */
#define DEBUG false

#define CLK_HALF_PERIOD_MS 20

#define BUS_SIZE 8
#define BUS_FLOATING 0xFF
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
    if (DEBUG == false) {return true;}
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
    Serial.print("): \n\tExpected \t");
    Serial.print(expected);
    Serial.print(" 0b");
    print_bin8(expected);
    Serial.print(" - \n\tActual \t\t");
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

// Get the value from RAM on to the bus
uint8_t read_from_ram(uint8_t addr) {
  uint8_t bus = 0;
  
  for (int i=0; i<ADDR_SIZE; i++) {
    digitalWrite(ADDRESS[i], (addr>>i)&1);
  }
  
  digitalWrite(RO_N, LOW);
  delay(1);
  for (int i=0; i<BUS_SIZE; i++) {
    bus += digitalRead(BUS[i])<<i;
  }  
  delay(1);
  digitalWrite(RO_N, HIGH);
  return bus;
}

void clear_ram() {
  for (int i=0; i<pow(2, ADDR_SIZE); i++) {
    write_to_ram(0, i);
  }
}

/* Test functions */
int test_write_read_all_addr() {
  clear_ram();
  int pass_count = 0;

  /* Write address value to each address */
  for (int i=0; i<pow(2, ADDR_SIZE); i++) {
    write_to_ram(i, i);
  }

  for (int i=0; i<pow(2, ADDR_SIZE); i++) {
    pass_count += test_equal(read_from_ram(i), i, "Write/Read all addr");
  }

  /* Write address value to each address */
  for (int i=0; i<pow(2, ADDR_SIZE); i++) {
    write_to_ram(i<<ADDR_SIZE, i);
  }

  for (int i=0; i<pow(2, ADDR_SIZE); i++) {
    pass_count += test_equal(read_from_ram(i), i<<ADDR_SIZE, "Write/Read all addr");
  }

  return pass_count;
}

bool test_noop() {
  bool pass = true;

  write_to_ram(0b10101010, 0);
  pass &= test_equal(read_from_ram(0), 0b10101010, "No OP setup");

  for (int i = 0; i < 5; i++) {
    pass &= test_equal(read_from_ram(0), 0b10101010, "No OP");
    clock_pulse();
  }

  return pass;
}

int test_read_in_write_out_all() {
  int pass_count = 0;

  for (unsigned int i = 0; i < pow(2, BUS_SIZE); i++) {
    write_to_ram(i, i&0xF);
    pass_count += test_equal(read_from_ram(i&0xF), i, "Read/Write");
  }
  return pass_count;
}

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
  int write_read_all_addr = test_write_read_all_addr();
  bool noop = test_noop();
  int read_in_write_out_all = test_read_in_write_out_all();

  Serial.println();
  Serial.print("Write/Read all addr:\t");
  Serial.print(write_read_all_addr >= 2*pow(2, ADDR_SIZE) ? "PASS" : "FAIL");
  Serial.print("\t(Passed ");
  Serial.print(write_read_all_addr);
  Serial.println("/32)");
  
  Serial.print("No OP:\t\t");
  Serial.println(noop ? "PASS" : "FAIL");

  Serial.print("Read/Write:\t");
  Serial.print(read_in_write_out_all >= pow(2, BUS_SIZE) ? "PASS" : "FAIL");
  Serial.print("\t(Passed ");
  Serial.print(read_in_write_out_all);
  Serial.println("/256)");
}

void loop() {}
