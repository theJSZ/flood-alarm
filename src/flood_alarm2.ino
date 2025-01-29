#include <SoftwareSerial.h>
SoftwareSerial sim(9, 10);

const int SENSOR_PIN1 = A7;
const int SENSOR_PIN2 = 0;
const int BUTTON_PIN = 2;
const int FLOOD_THRESHOLD = 100;
const int SIM_DELAY = 500;

bool flood_detected = false;
bool button_pressed = false;

unsigned long int flood_timer;
unsigned long int button_timer;

// Time intervals in ms
// duration of constant water detection that will trigger alarm
const unsigned long int TIME_TO_TRIGGER_ALARM = 60000;
// duration of constant button press that will trigger test sms
const unsigned long int TIME_TO_TRIGGER_TEST = 2000;

// add emergency contact phone numbers to this array
// as many as necessary
char phone_numbers[][14] = {
  "+358xxxxxxxxx",
  "+358xxxxxxxxx"
};
int PHONE_NUMBER_COUNT = sizeof(phone_numbers) / sizeof(phone_numbers[0]);


void button_interrupt() {
  flood_detected = false;
}


bool registered_to_gsm_network() {
  return true;
}


bool verify_response() {
  return true;
}


bool send_sms(String message) {
  for (int i = 0; i < PHONE_NUMBER_COUNT; ++i) {
    bool message_sent = false;

    while (!message_sent) {
      if (registered_to_gsm_network()) {
        // set SMS mode to "text mode"
        sim.println("AT+CMGF=1");
        delay(SIM_DELAY);
        // set the number to send to
        sim.print("AT+CMGS=\"");
        sim.print(phone_numbers[i]);
        sim.println("\"");
        delay(SIM_DELAY);
        // set the actual message
        sim.println(message);
        delay(SIM_DELAY);
        // send message
        sim.println(char(26));
        delay(SIM_DELAY);
        message_sent = verify_response();
      }
    }
  }
  return true;
}


void setup() {
  pinMode(BUTTON_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), button_pressed, RISING);
}


void loop() {
  unsigned long int current_time = millis();
  // read the sensors, reading is the higher one
  int reading = max(analogRead(SENSOR_PIN1), analogRead(SENSOR_PIN2));
  // water detected
  if (reading > FLOOD_THRESHOLD) {
    // water was not detected previously
    if (!flood_detected) {
      flood_detected = true;
      // start the timer
      flood_timer = current_time;
    } else {  // water was already detected previously
      // time to trigger alarm has passed
      if (current_time - flood_timer > TIME_TO_TRIGGER_ALARM) {
        send_sms("tulva");
        // enter locked state
        while (flood_detected) {
          ;
          // button press will break this loop
        }
      }
    }
  } // end "water detected"

  if (digitalRead(BUTTON_PIN) == HIGH) {
    if (!button_pressed) {
      button_pressed = true;
      button_timer = current_time;
    }
    if (current_time - button_timer > TIME_TO_TRIGGER_TEST) {
      send_sms("testi");
    }
  }
}