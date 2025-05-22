/***************************************************************
 * PirBox by PricelessToolkit - Modified for Interrupt-Driven Operation
 ***************************************************************/

#include "config.h"
#include <SPI.h>
#include "Ra01S.h"

// -------------------- PIN ASSIGNMENTS -------------------- //
#define PIR_PIN         PIN_PC0  // Motion sensor input
#define OFF_PIN         PIN_PB0  // Pull HIGH for Power OFF
#define BATT_PIN        PIN_PB4  // Battery pin


// RA-01SH Pins
#define RADIO_RESET_PIN  PIN_PA6
#define RADIO_NSS_PIN    PIN_PA4
#define RADIO_BUSY_PIN   PIN_PC2
#define RADIO_DIO1_PIN   PIN_PC3

SX126x lora(RADIO_NSS_PIN, RADIO_RESET_PIN, RADIO_BUSY_PIN);

// -------------------- GLOBAL VARIABLES -------------------- //
// Flag to indicate a sensor interrupt occurred (set in ISR)
volatile bool sensorTriggered = false;
// Timestamp of the last sensor event (updated in ISR)
volatile unsigned long lastInterruptTime = 0;


// Global flag to force sending on the very first run
bool firstRun = true;

// Global variables to track when to turn off each relay (non-blocking)
unsigned long relay1OffTime = 0;
unsigned long relay2OffTime = 0;

unsigned long powerTimerStart = 0;
bool timerStarted = false;


// -------------------- Xor Encryp/Decrypt -------------------- //

String xorCipher(String input) {
  const byte key[] = encryption_key;
  const int keyLength = encryption_key_length;

  String output = "";
  for (int i = 0; i < input.length(); i++) {
    byte keyByte = key[i % keyLength];
    output += char(input[i] ^ keyByte);
  }
  return output;
}

// -------------------- INTERRUPT SERVICE ROUTINE -------------------- //
void sensorISR() {
  sensorTriggered = true;
  lastInterruptTime = millis();
}

// -------------------- FUNCTIONS -------------------- //
void initLoRaRadio() {
  int16_t ret = lora.begin(BAND, TX_OUTPUT_POWER);
  if (ret != ERR_NONE) {
    while (1);  // Halt if init fails
  }

  //Set your custom sync word (example: 0x2D01)
//  lora.SetSyncWord(SYNC_WORD);

  // Now configure LoRa parameters
  lora.LoRaConfig(
    LORA_SPREADING_FACTOR,
    LORA_BANDWIDTH,
    LORA_CODINGRATE,
    LORA_PREAMBLE_LENGTH,
    LORA_PAYLOADLENGTH,
    LORA_CRC_ON,
    false,  // No IQ inversion
    LORA_SYNC_WORD
  );
}


void powerOFF() {
  delay(100);
  // Serial.println(F("Sleeping"));
  delay(10);
  digitalWrite(OFF_PIN, HIGH); // Powering off
}

// -------------------- BATTERY MEASUREMENT -------------------- //
int batt() {
  float totalVolts = 0;
  for (int i = 0; i < 5; i++) {
    float volts = analogReadEnh(PIN_PB4, 12) * (2.5 / 4096) * 2; // Voltage divider calculation
    totalVolts += volts;
    delay(2);
  }
  float averageVolts = totalVolts / 5;
  float percentage = ((averageVolts - 3.3) / (4.1 - 3.3)) * 100;
  percentage = constrain(percentage, 0, 100);
  return (int)percentage;
}


// -------------------- SETUP -------------------- //
void setup() {
  // Set up power off pin
  pinMode(OFF_PIN, OUTPUT);
  digitalWrite(OFF_PIN, LOW);
  
  pinMode(PIR_PIN, INPUT);
  
  pinMode(RADIO_DIO1_PIN, INPUT);
  delay(10);
  
  Serial.begin(9600);
  // Serial.println(F("Serial Debugging Enabled!"));

  // Initialize LoRa radio
  initLoRaRadio();

  // set reference to the desired voltage
  analogReference(INTERNAL2V5);
  //analogReference(VDD);

  // -------------------- ATTACH INTERRUPTS -------------------- //
  // Attach interrupts to sensor pins (trigger on any state change)

  attachInterrupt(digitalPinToInterrupt(PIR_PIN), sensorISR, CHANGE);

}

// -------------------- LOOP -------------------- //
void loop() {
  // Check if an interrupt (sensor event) has been triggered
  if (sensorTriggered || firstRun) {
    // Clear the flag so we don’t repeatedly process the same event
    sensorTriggered = false;

    // Read sensor inputs (HIGH means "on", LOW means "off")

    int currentPIR  = digitalRead(PIR_PIN);
 
    // Clear firstRun flag after sending the first payload
    if (firstRun) {
      firstRun = false;
    }

    // Build the base JSON payload with gateway key and node name
    String payload = "{\"k\":\"" + String(GATEWAY_KEY)
                     + "\",\"id\":\"" + String(NODE_NAME) + "\"";
                     
    // Append sensor states:
    // "m" for motion sensor
    payload += ",\"m\":\""  + String(currentPIR  == HIGH ? "on" : "off") + "\"";

    // Append battery measurement
    int intPercentage = batt();
    payload += ",\"b\":" + String(intPercentage) + "}";

    #if Encryption
    payload = xorCipher(payload);
    #endif

    // Convert payload to raw bytes for sending
    const char* cstrPayload = payload.c_str();
    uint8_t length = payload.length();

    // Send the payload using synchronous mode
    if (lora.Send((uint8_t*)cstrPayload, length, SX126x_TXMODE_SYNC)) {
      // Serial.println(payload);
      delay(20);
    } else {
      // Serial.println(F("Send fail"));
    }
    
    powerOFF();
    
  }

  // Other non-blocking tasks could go here...
}
