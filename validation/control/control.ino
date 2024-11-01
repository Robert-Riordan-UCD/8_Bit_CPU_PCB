/* TEST PLAN
 *
 * 1) Test each instruction
 *  a) Check each step though the microcode cycle
 *  b) Check with all flag states
 */
#define DEBUG false

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
    if (DEBUG == false) {return true;}
    Serial.print("Pass  (");
    Serial.print(test);
    Serial.print("): Expected ");
    Serial.print(expected);
    Serial.print(" (");
    print_signals(expected);
    Serial.print(")");
    Serial.print(" - Actual ");
    Serial.print(value);
    Serial.print(" (");
    print_signals(value);
    Serial.print(")");
    Serial.println();
    return true;
  } else {
    Serial.print("ERROR (");
    Serial.print(test);
    Serial.print("): Expected ");
    Serial.print(expected);
    Serial.print(" (");
    print_signals(expected);
    Serial.print(")");
    Serial.print(" - Actual ");
    Serial.print(value);
    Serial.print(" (");
    print_signals(value);
    Serial.print(")");
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

// Get the user to press the reset button to reset the microcode timer
void reset_timer() {
  Serial.println("Please press reset");
  while(digitalRead(RST) == LOW) {}
  clock_pulse();
  while(digitalRead(RST) == HIGH) {}
}

/* Test functions */
int test_ins(uint8_t ins, uint16_t code[6], char *test_name) {
  int pass = 0;
  set_instruction(ins);
  for (uint8_t flags=0; flags<=3; flags++) {
    digitalWrite(ALU_CARRY, flags>>1);
    digitalWrite(ALU_ZERO, flags&1);
    for (int i=0; i<6; i++) {
      char buf[100];
      sprintf(buf, "%s (0x%x) T=%d (C=%d,Z=%d)", test_name, ins, i, (flags>>1)&1, flags&1);
      pass += test_equal(read_control_signals(), code[i], buf);
      clock_pulse();
    }
  }
  return pass;
}

int test_jmpc() {
  int pass = 0;
  uint16_t code_carry_set[6] = {MI|PCO, RO|II|PCE, IO|JMP, 0, 0, 0};
  uint16_t code_carry_not_set[6] = {MI|PCO, RO|II|PCE, 0, 0, 0, 0};
  set_instruction(0b0111);
  for (uint8_t flags=0; flags<=3; flags++) {
    digitalWrite(ALU_CARRY, flags>>1);
    digitalWrite(ALU_ZERO, flags&1);
    for (int i=0; i<6; i++) {
      char buf[100];
      sprintf(buf, "Jump Carry (0x0111) T=%d (C=%d,Z=%d)", i, (flags>>1)&1, flags&1);
      if (flags>>1) {
        pass += test_equal(read_control_signals(), code_carry_set[i], buf);
      } else {
        pass += test_equal(read_control_signals(), code_carry_not_set[i], buf);
      }
      clock_pulse();
    }
  }
  return pass;
}

int test_jmpz() {
  int pass = 0;
  uint16_t code_carry_set[6] = {MI|PCO, RO|II|PCE, IO|JMP, 0, 0, 0};
  uint16_t code_carry_not_set[6] = {MI|PCO, RO|II|PCE, 0, 0, 0, 0};
  set_instruction(0b1000);
  for (uint8_t flags=0; flags<=3; flags++) {
    digitalWrite(ALU_CARRY, flags>>1);
    digitalWrite(ALU_ZERO, flags&1);
    for (int i=0; i<6; i++) {
      char buf[100];
      sprintf(buf, "Jump Zero (0x1000) T=%d (C=%d,Z=%d)", i, (flags>>1)&1, flags&1);
      if (flags&1) {
        pass += test_equal(read_control_signals(), code_carry_set[i], buf);
      } else {
        pass += test_equal(read_control_signals(), code_carry_not_set[i], buf);
      }
      clock_pulse();
    }
  }
  return pass;
}

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

  pinMode(I_WO_N, INPUT);
  pinMode(I_RI_N, INPUT);
  pinMode(RAM_RI, INPUT);
  pinMode(RAM_WO_N, INPUT);
  pinMode(MAR_EN_N, INPUT);
  pinMode(CLK_HALT, INPUT);
  pinMode(PC_JMP_N, INPUT);
  pinMode(PC_EN, INPUT);
  pinMode(PC_CO_N, INPUT);
  pinMode(A_WO_N, INPUT);
  pinMode(A_RI_N, INPUT);
  pinMode(ALU_WO_N, INPUT);
  pinMode(ALU_SUB, INPUT);
  pinMode(ALU_FI, INPUT);
  pinMode(B_RI_N, INPUT);
  pinMode(OUT_EN_N, INPUT);

  reset_timer();
  
  /* Tests */
  uint16_t code[][6] = {
    {MI|PCO, RO|II|PCE, 0, 0, 0, 0}, // No OP
    {MI|PCO, RO|II|PCE, IO|MI, RO|AI, 0, 0}, // LDA
    {MI|PCO, RO|II|PCE, IO|MI, RO|BI, ALUO|FI|AI, 0}, // ADD
    {MI|PCO, RO|II|PCE, IO|MI, RO|BI, ALUO|SUB|FI|AI, 0}, // SUB
    {MI|PCO, RO|II|PCE, IO|MI, AO|RI, 0, 0}, // STA
    {MI|PCO, RO|II|PCE, IO|AI, 0, 0, 0}, // LDI
    {MI|PCO, RO|II|PCE, IO|JMP, 0, 0, 0}, // JMP
    {MI|PCO, RO|II|PCE, AO|OUT, 0, 0, 0}, // OUT
    {MI|PCO, RO|II|PCE, HLT, 0, 0, 0} // HALT
  };
  int noop = test_ins(0, code[0], "No OP");
  int lda = test_ins(1, code[1], "Load A");
  int add = test_ins(2, code[2], "Add");
  int sub = test_ins(3, code[3], "Subtract");
  int sta = test_ins(4, code[4], "Store A");
  int ldi = test_ins(5, code[5], "Load imediate");
  int jmp = test_ins(6, code[6], "Jump");
  int out = test_ins(14, code[7], "Output");
  int hlt = test_ins(15, code[8], "Halt");
  int jmpc = test_jmpc();
  int jmpz = test_jmpz();

  Serial.print("No OP:\t\t");
  Serial.print(noop);
  Serial.println("/24");

  Serial.print("Load A:\t\t");
  Serial.print(lda);
  Serial.println("/24");

  Serial.print("Add:\t\t");
  Serial.print(add);
  Serial.println("/24");

  Serial.print("Subtract:\t");
  Serial.print(sub);
  Serial.println("/24");

  Serial.print("Store A:\t");
  Serial.print(sta);
  Serial.println("/24");

  Serial.print("Load Imediate:\t");
  Serial.print(ldi);
  Serial.println("/24");

  Serial.print("Jump:\t\t");
  Serial.print(jmp);
  Serial.println("/24");

  Serial.print("Output:\t\t");
  Serial.print(out);
  Serial.println("/24");

  Serial.print("Halt:\t\t");
  Serial.print(hlt);
  Serial.println("/24");

  Serial.print("Jump carry:\t");
  Serial.print(jmpc);
  Serial.println("/24");

  Serial.print("Jump zero:\t");
  Serial.print(jmpz);
  Serial.println("/24");
  
  Serial.println();
}

void loop() {}
