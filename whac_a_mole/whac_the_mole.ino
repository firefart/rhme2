/*
WIRING: See whac_the_mole.fzz
*/

#include <SoftwareSerial.h>

#define NUMBER_OF_PINS 11

// pin for reading in the pulses
const int PIN_READ = 13;

// matches the nano digital pins, value the pin on the uno
const int PIN_2 = 2;
const int PIN_3 = 3;
const int PIN_4 = 4;
const int PIN_5 = 5;
const int PIN_6 = 6;
const int PIN_7 = 7;
const int PIN_8 = 14;
const int PIN_9 = 15;
const int PIN_10 = 16;
const int PIN_11 = 17;
const int PIN_12 = 18;

const int INVALID = -100;

// monitor pin for the oscilloscope
const int PIN_MONITOR = 12;

int state = 0;
int peak_count = 0;
unsigned long peak_start = 0;
char incomingChar;
String strBuffer;
String str;

SoftwareSerial ser(8, 9);

// bruteforce stuff
// index: number of peaks
int pinout[NUMBER_OF_PINS];
int last_tried_pin;
int last_tried_value;

// https://arduino.stackexchange.com/questions/14407/use-all-pins-as-digital-i-o
int candidates_peak_1[] = { PIN_2, PIN_3, PIN_4, PIN_5, PIN_6, PIN_7, PIN_8, PIN_9, PIN_10, PIN_11, PIN_12 };
int candidates_peak_2[] = { PIN_2, PIN_3, PIN_4, PIN_5, PIN_6, PIN_7, PIN_8, PIN_9, PIN_10, PIN_11, PIN_12 };
int candidates_peak_3[] = { PIN_2, PIN_3, PIN_4, PIN_5, PIN_6, PIN_7, PIN_8, PIN_9, PIN_10, PIN_11, PIN_12 };
int candidates_peak_4[] = { PIN_2, PIN_3, PIN_4, PIN_5, PIN_6, PIN_7, PIN_8, PIN_9, PIN_10, PIN_11, PIN_12 };
int candidates_peak_5[] = { PIN_2, PIN_3, PIN_4, PIN_5, PIN_6, PIN_7, PIN_8, PIN_9, PIN_10, PIN_11, PIN_12 };
int candidates_peak_6[] = { PIN_2, PIN_3, PIN_4, PIN_5, PIN_6, PIN_7, PIN_8, PIN_9, PIN_10, PIN_11, PIN_12 };

void setup() {
  // UNO <--> PC
  Serial.begin(57600);
  // Nano <--> UNO
  ser.begin(19200);
  pinMode(PIN_READ, INPUT);
  pinMode(PIN_2, OUTPUT);
  pinMode(PIN_3, OUTPUT);
  pinMode(PIN_4, OUTPUT);
  pinMode(PIN_5, OUTPUT);
  pinMode(PIN_6, OUTPUT);
  pinMode(PIN_7, OUTPUT);
  pinMode(PIN_8, OUTPUT);
  pinMode(PIN_9, OUTPUT);
  pinMode(PIN_10, OUTPUT);
  pinMode(PIN_11, OUTPUT);
  pinMode(PIN_12, OUTPUT);
  pinMode(PIN_MONITOR, OUTPUT);
  
  digitalWrite(PIN_2, LOW);
  digitalWrite(PIN_3, LOW);
  digitalWrite(PIN_4, LOW);
  digitalWrite(PIN_5, LOW);
  digitalWrite(PIN_6, LOW);
  digitalWrite(PIN_7, LOW);
  digitalWrite(PIN_8, LOW);
  digitalWrite(PIN_9, LOW);
  digitalWrite(PIN_10, LOW);
  digitalWrite(PIN_11, LOW);
  digitalWrite(PIN_12, LOW);
  digitalWrite(PIN_MONITOR, LOW);
}

void remove_found_pin(int pin) {
  int i;
  for (i=0; i < NUMBER_OF_PINS; i++) {
    if (candidates_peak_1[i] == pin) {
      candidates_peak_1[i] = INVALID;
    }
    if (candidates_peak_2[i] == pin) {
      candidates_peak_2[i] = INVALID;
    }
    if (candidates_peak_3[i] == pin) {
      candidates_peak_3[i] = INVALID;
    }
    if (candidates_peak_4[i] == pin) {
      candidates_peak_4[i] = INVALID;
    }
    if (candidates_peak_5[i] == pin) {
      candidates_peak_5[i] = INVALID;
    }
    if (candidates_peak_6[i] == pin) {
      candidates_peak_6[i] = INVALID;
    }
  }
}

int get_next_pin(int array[]) {
  int i;
  for (i=0; i < NUMBER_OF_PINS; i++) {
    if (array[i] == INVALID) {
      continue;
    } else {
      int v = array[i];
      array[i] = INVALID;
      return v;
    }
  } 
}

void print_array(int array[]) {
  for (int i=0;i < NUMBER_OF_PINS; i++) {
    Serial.println(array[i]);
  }
}

void try_pins(int peak) {
  int possible_pin = pinout[peak - 1];
  if (possible_pin == 0) {
    // try another pin
    // matches the pin out
     switch(peak) {
       case 1:
         possible_pin = get_next_pin(candidates_peak_1);
         break;
       case 2:
         possible_pin = get_next_pin(candidates_peak_2);
         break;
       case 3:
         possible_pin = get_next_pin(candidates_peak_3);
         break;
       case 4:
         possible_pin = get_next_pin(candidates_peak_4);
         break;
       case 5:
         possible_pin = get_next_pin(candidates_peak_5);
         break;
       case 6:
         possible_pin = get_next_pin(candidates_peak_6);
         break;
       default:
         Serial.println("Unknown peak count");
         break;
     }
  } else {
    // Serial.print("Using cached pin ");
    // Serial.println(possible_pin);
  }
  if (possible_pin == INVALID) {
    Serial.print("Failed port with ");
    Serial.print(peak);
    Serial.println(" peaks");
    Serial.println("1:");
    print_array(candidates_peak_1);
    Serial.println("2:");
    print_array(candidates_peak_2);
    Serial.println("3:");
    print_array(candidates_peak_3);
    Serial.println("4:");
    print_array(candidates_peak_4);
    Serial.println("5:");
    print_array(candidates_peak_5);
    Serial.println("6:");
    print_array(candidates_peak_6);
  } else {
    last_tried_value = peak;
    last_tried_pin = possible_pin;
    send_peak(possible_pin);  
  }
}

void send_peak(int pin) {  
    // Serial.print(F("Sending peak to pin "));
    // Serial.println(pin);
    digitalWrite(PIN_MONITOR, HIGH);
    digitalWrite(pin, HIGH);
    delay(50);
    digitalWrite(pin, LOW);
    digitalWrite(PIN_MONITOR, LOW);
}

void loop() {
  // Get text from other arduino's serial line
  while (ser.available()) {
    incomingChar = ser.read();
    strBuffer += incomingChar;
  }
  if (strBuffer.endsWith(F("\n"))) {
    strBuffer.trim();
    if (strBuffer.length() > 0) {
      Serial.println(strBuffer);
      str = strBuffer;
    }
    strBuffer = "";
  }

  // we failed, try again
  if (str.indexOf(F("You missed it")) >= 0) {
    str = "";
    // Press enter
    ser.write("\r");
  // only run if we haven't found the pin yet
  } else if (str.indexOf(F("You whacked it")) >= 0) {
    if (pinout[last_tried_value - 1] == 0) {
      Serial.print(F("Found!!! Peaks: "));
      Serial.print(last_tried_value);
      Serial.print(F(" Pin: "));
      Serial.println(last_tried_pin);
      pinout[last_tried_value - 1] = last_tried_pin;
      // remove the found pin from all arrays to speed up bruteforcing
      remove_found_pin(last_tried_pin);
    }
    str = "";
  } else if (str.indexOf(F(" please step into the yard by pressing")) >= 0) {
    str = "";
    ser.write("\r");
  }
  
  // get mole hole
  int value = digitalRead(PIN_READ);
  if (value != state) {
    state = value;
    // only count highs
    if (state == 1) {
      peak_start = millis();
      peak_count ++;
    }
  }

  // one peak ~50ms. So if there is no change after 2 peaks
  // we are finished
  // round finished, send peak
  if (peak_start && peak_start + 103 < millis() && state == 0) {
    try_pins(peak_count);
    peak_start = 0;
    peak_count = 0;
  }
}


