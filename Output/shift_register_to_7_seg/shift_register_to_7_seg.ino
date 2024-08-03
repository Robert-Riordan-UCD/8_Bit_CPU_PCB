#define CLOCK_HALF_PERIOD_uS 1000

#define HOLD_TIME_MS 1000

uint8_t seven_segment_digits[16] = {
  0b11111100, // 0
  0b01100000, // 1
  0b11011010, // 2
  0b11110010, // 3
  0b01100110, // 4
  0b10110110, // 5
  0b10111110, // 6
  0b11100000, // 7
  0b11111110, // 8
  0b11110110, // 9
  0b11101110, // A
  0b00111110, // b
  0b10011100, // C
  0b01111010, // d
  0b10011110, // E
  0b10001110  // F
};

int CLK = 2;
int SER = 3;
int SR_CLEAR = 4;
int RCLK = 5;

void clk_pulse() {
  delayMicroseconds(CLOCK_HALF_PERIOD_uS);
  digitalWrite(CLK, HIGH);
  delayMicroseconds(CLOCK_HALF_PERIOD_uS);
  digitalWrite(CLK, LOW);
}

void clear() {
  digitalWrite(SR_CLEAR, LOW);
  clk_pulse();
  digitalWrite(SR_CLEAR, HIGH);
}

void shift_in(uint8_t n) {
  for (int i = 0; i < 8; i++) {
    digitalWrite(SER, (n>>i)&(0x01));
    clk_pulse();
  }

  digitalWrite(RCLK, HIGH);
  clk_pulse();
  digitalWrite(RCLK, LOW);
}

void setup() {
  pinMode(CLK, OUTPUT);
  pinMode(SER, OUTPUT);
  pinMode(SR_CLEAR, OUTPUT);
  pinMode(RCLK, OUTPUT);
  
  digitalWrite(CLK, LOW);
  digitalWrite(SER, LOW);
  digitalWrite(SR_CLEAR, HIGH);
  digitalWrite(RCLK, LOW);
}

void loop() {
  for (int i = 0; i < 16; i++){
    shift_in(seven_segment_digits[i]);
    delay(HOLD_TIME_MS);
  }
}
