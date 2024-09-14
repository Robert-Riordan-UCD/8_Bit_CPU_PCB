uint8_t memory[16]; // Using 16 8-bit values because there are no 4-bit data types. Bits 7 to 4 are unused on each byte.

int address_pins[4] = {A0, A1, A2, A3};
int data_output_pins[4] = {5, 4, 3, 2};
int data_input_pins[4] = {7, 6, 9, 10};

int n_write_enable_pin = 8; // Write when low. Read when high
int n_chip_select_pin = 19; // Read/write enabled when low. Output to high impedence when high

void setup() {
  for (int i = 0; i < 4; i++) {
    pinMode(address_pins[i], INPUT);
    pinMode(data_output_pins[i], INPUT); // High impedence when not in use
    pinMode(data_input_pins[i], INPUT);
  }
  pinMode(n_write_enable_pin, INPUT);
  pinMode(n_chip_select_pin, INPUT);

//  Serial.begin(9600);
}

void loop() {
  if (digitalRead(n_chip_select_pin) == LOW) {
    if (digitalRead(n_write_enable_pin) == LOW) { // Write out
      uint16_t address = digitalRead(address_pins[0]) + 2*digitalRead(address_pins[1]) + 4*digitalRead(address_pins[2])+8*digitalRead(address_pins[3]);
      for (int i = 0; i < 4; i++) {
        pinMode(data_output_pins[i], OUTPUT); // Change from high impedence to output
        digitalWrite(data_output_pins[i], (memory[address]>>i)&0x01);
      }
    } else { // Read in
      uint16_t address = digitalRead(address_pins[0]) + 2*digitalRead(address_pins[1]) + 4*digitalRead(address_pins[2])+8*digitalRead(address_pins[3]);
      uint8_t data_in = 
        ((uint8_t)digitalRead(data_input_pins[0])<<0) +
        ((uint8_t)digitalRead(data_input_pins[1])<<1) +
        ((uint8_t)digitalRead(data_input_pins[2])<<2) +
        ((uint8_t)digitalRead(data_input_pins[3])<<3);
      memory[address] = data_in;

      for (int i = 0; i < 4; i++) { // Set output to high impedence when not in use
        pinMode(data_output_pins[i], INPUT);
      }
    }
  } else { // Set output to high impedence when not in use
    for (int i = 0; i < 4; i++) {
      pinMode(data_output_pins[i], INPUT);
    }
  }

//  Serial.print("~CS: ");
//  Serial.print(digitalRead(n_chip_select_pin));
//  Serial.print(", ~WE: ");
//  Serial.print(digitalRead(n_write_enable_pin));\
//  Serial.print(", Address: ");
//  Serial.print(digitalRead(address_pins[3]));
//  Serial.print(digitalRead(address_pins[2]));
//  Serial.print(digitalRead(address_pins[1]));
//  Serial.print(digitalRead(address_pins[0]));
//  Serial.print(", Data in: ");
//  Serial.print(digitalRead(data_input_pins[3]));
//  Serial.print(digitalRead(data_input_pins[2]));
//  Serial.print(digitalRead(data_input_pins[1]));
//  Serial.print(digitalRead(data_input_pins[0]));
//  Serial.print(", Data out: ");
//  Serial.print(digitalRead(data_output_pins[3]));
//  Serial.print(digitalRead(data_output_pins[2]));
//  Serial.print(digitalRead(data_output_pins[1]));
//  Serial.print(digitalRead(data_output_pins[0]));
//  
//  Serial.print(", Memory: ");
//  char strBuf[16*5];
//
//  for (int i = 0; i < 16; i++) {
//    sprintf(strBuf[5*i + 0], "%d", (int)((memory[i]>>3)&0x01));
//    sprintf(strBuf[5*i + 1], "%d", (int)((memory[i]>>2)&0x01));
//    sprintf(strBuf[5*i + 2], "%d", (int)((memory[i]>>1)&0x01));
//    sprintf(strBuf[5*i + 3], "%d", (int)((memory[i]>>0)&0x01));
//    sprintf(strBuf[5*i + 4], " ");
//      Serial.print();
//      Serial.print((int)(memory[i]&0x04));
//      Serial.print((int)(memory[i]&0x02));
//      Serial.print((int)(memory[i]&0x01));
//      Serial.print(' ');
//  }
  
//  Serial.println(strBuf);
}
