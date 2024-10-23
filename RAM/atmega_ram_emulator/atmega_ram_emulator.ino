uint8_t memory[16]; // Using 16 8-bit values because there are no 4-bit data types. Bits 7 to 4 are unused on each byte.

int address_pins[4] = {A0, A1, A2, A3};
int data_output_pins[4] = {5, 4, 3, 2};
int data_input_pins[4] = {7, 6, 9, 10};

int n_write_enable_pin = 8; // Write when low. Read when high

void setup() {
  for (int i = 0; i < 4; i++) {
    pinMode(address_pins[i], INPUT);
    pinMode(data_output_pins[i], INPUT); // High impedence when not in use
    pinMode(data_input_pins[i], INPUT);
  }
  pinMode(n_write_enable_pin, INPUT);
}

void writeToRAM () {
  for (int i = 0; i < 4; i++) { // Set output to high impedence when not in use
    pinMode(data_output_pins[i], INPUT);
  }
  
  uint16_t address = digitalRead(address_pins[0]) + 2*digitalRead(address_pins[1]) + 4*digitalRead(address_pins[2])+8*digitalRead(address_pins[3]);
  uint8_t data_in = 
    ((uint8_t)digitalRead(data_input_pins[0])<<0) +
    ((uint8_t)digitalRead(data_input_pins[1])<<1) +
    ((uint8_t)digitalRead(data_input_pins[2])<<2) +
    ((uint8_t)digitalRead(data_input_pins[3])<<3);
  memory[address] = data_in ^ 0b1111;
}

void readFromRAM() {
  uint16_t address = digitalRead(address_pins[0]) + 2*digitalRead(address_pins[1]) + 4*digitalRead(address_pins[2])+8*digitalRead(address_pins[3]);
  for (int i = 0; i < 4; i++) {
    pinMode(data_output_pins[i], OUTPUT); // Change from high impedence to output
    digitalWrite(data_output_pins[i], (memory[address]>>i)&0x01);
  }
}

void loop() {
  if (digitalRead(n_write_enable_pin) == LOW) { // Write to RAM
    writeToRAM();
  } else { // Read from RAM
    readFromRAM();
  }
}
