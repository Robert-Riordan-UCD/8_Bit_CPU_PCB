/* TEST PLAN
 *
 * 1) Test each instruction
 *  a) Check each step though the microcode cycle
 *  b) Check with all flag states
 * 2) Read from all addresses
 * 3) No-op
 *  a) Intenal state should remain as long as read in is not asserted
 * 4) Test write then read for every value 0x00 to 0xFF
 * 5) Unable to write while PROG is asserted
 */
#define CLK_HALF_PERIOD_MS 20

// Control output pins
#define I_WO_N   37
#define I_RI_N   35
#define RAM_RI   33
#define RAM_WO_N 31
#define MAR_EN_N 29
#define CLK_HALT 27
#define PC_JMP_N 25
#define PC_EN    23

#define PC_CO_N  36
#define A_WO_N   34
#define A_RI_N   32
#define ALU_WO_N 30
#define ALU_SUB  28
#define ALU_FI   26
#define B_RI_N   24
#define OUT_EN_N 22

#define RST 41

// Control input pins
#define ALU_CARRY 47
#define ALU_ZERO  45
#define CLK       43
#define INS_SIZE  4
const int instruction[INS_SIZE] = {44, 42, 40, 38};

// Signal bit mapping
#define IO   0x8000
#define II   0x4000
#define RI   0x2000
#define RO   0x1000
#define MI   0x0800
#define HLT  0x0400
#define JMP  0x0200
#define PCE  0x0100
#define PCO  0x0080
#define AO   0x0040
#define AI   0x0020
#define ALUO 0x0010
#define SUB  0x0008
#define FI   0x0004
#define BI   0x0002
#define OUT  0x0001

/* Basic utility functions */
void print_bin8(int n) {
  for(int i = 7; i >= 0; i--) {
    Serial.print((n>>i)&1);
  }
}

void print_signals(uint16_t signals) {
  if (signals & IO)   {Serial.print("IR OUT, ");}
  if (signals & II)   {Serial.print("IR IN, ");}
  if (signals & RI)   {Serial.print("RAM IN, ");}
  if (signals & RO)   {Serial.print("RAM OUT, ");}
  if (signals & MI)   {Serial.print("MAR EN, ");}
  if (signals & HLT)  {Serial.print("CLK HLT, ");}
  if (signals & JMP)  {Serial.print("JMP, ");}
  if (signals & PCE)  {Serial.print("PC EN, ");}
  
  if (signals & PCO)  {Serial.print("PC OUT, ");}
  if (signals & AO)   {Serial.print("AR OUT, ");}
  if (signals & AI)   {Serial.print("AR IN, ");}
  if (signals & ALUO) {Serial.print("ALU OUT, ");}
  if (signals & SUB)  {Serial.print("ALU SUB, ");}
  if (signals & FI)   {Serial.print("ALU FI, ");}
  if (signals & BI)   {Serial.print("BR IN, ");}
  if (signals & OUT)  {Serial.print("OUT EN, ");}
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

uint16_t read_control_signals() {
  uint16_t state = 0;
  state += !digitalRead(I_WO_N);
  state <<= 1;
  state += !digitalRead(I_RI_N);
  state <<= 1;
  state += digitalRead(RAM_RI);
  state <<= 1;
  state += !digitalRead(RAM_WO_N);
  state <<= 1;
  state += !digitalRead(MAR_EN_N);
  state <<= 1;
  state += digitalRead(CLK_HALT);
  state <<= 1;
  state += !digitalRead(PC_JMP_N);
  state <<= 1;
  state += digitalRead(PC_EN);
  state <<= 1;
  
  state += !digitalRead(PC_CO_N);
  state <<= 1;
  state += !digitalRead(A_WO_N);
  state <<= 1;
  state += !digitalRead(A_RI_N);
  state <<= 1;
  state += !digitalRead(ALU_WO_N);
  state <<= 1;
  state += digitalRead(ALU_SUB);
  state <<= 1;
  state += digitalRead(ALU_FI);
  state <<= 1;
  state += !digitalRead(B_RI_N);
  state <<= 1;
  state += !digitalRead(OUT_EN_N);

  return state;
}

void set_instruction(uint8_t ins) {
  for (int i=0; i<INS_SIZE; i++) {
    digitalWrite(instruction[i], (ins>>i)&1);
  }
}

void reset_timer() {
  Serial.println("Please press reset");
  while(digitalRead(RST) == LOW) {}
  clock_pulse();
  while(digitalRead(RST) == HIGH) {}
  Serial.println("Done waiting");
}
/* Test functions */

void setup() {
  Serial.begin(9600);

  /* Pin setup */
  pinMode(ALU_CARRY, OUTPUT);
  digitalWrite(ALU_CARRY, LOW);
  pinMode(ALU_ZERO, OUTPUT);
  digitalWrite(ALU_ZERO, LOW);
  pinMode(CLK, OUTPUT);
  digitalWrite(CLK, LOW);
  for (int ins=0; ins<pow(2, INS_SIZE); ins++) {
    pinMode(ins, OUTPUT);
    digitalWrite(ins, LOW);
  }

  pinMode(RST, INPUT);

  /* Tests */
  reset_timer();
  set_instruction(1);
  uint16_t signals = read_control_signals();
  print_signals(signals);
}

void loop() {}
