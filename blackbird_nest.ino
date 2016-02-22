/*-------------------------------------------------------------------------
 * Copyright 2016 Matthew Wood
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------
 */
// Designed to run on an Arduino 101, but should be Arduino Uno compatible

#include <Adafruit_Soundboard.h>

//#define DEBUG_MODE  1
//#define VERBOSE_DEBUG_MODE 1

// Connect to output signal of distance sensor
#define PROXIMITY_SENSOR_PIN A0
// Connect to the RST pin on the Sound Board
#define SFX_RST 4
// Connect to the ACT pin on the Sound Board
#define SFX_ACT 5
// Connect to LED positive
#define LED     7

typedef enum {
  NOTHING,
  SOMETHING
} DistanceRange;

// Sensor value indicating the something in range (~90-100cm)
#define IN_RANGE_START  150

// Sound pattern when sensor picks up something in-range
// chirp -> welcome -> chirp x CHIRPS_PER_WELCOME -> repeat

// Milliseconds between samples
#define SAMPLE_TIME 500
// Milliseconds to pause between two chirps
#define CHIRP_PAUSE (120 * 1000)
// How many time to chirp between welcome messages
#define CHIRPS_PER_WELCOME 5

// Pass the Serial1 interface (pins 1&2 on Arduino 101) to
// Adafruit_Soundboard, the second argument should be NULL, and the third
// arg is the reset pin
Adafruit_Soundboard sfx = Adafruit_Soundboard(&Serial1, NULL, SFX_RST);

// Support up to MAX_SOUNDS chirping sounds and MAX_SOUNDS welcome messages
// Files containing chirp sounds must start with CHIRP_SOUND_PREFIX (case
// insensitive). Files containing welcome sound must start with
// WELCOME_SOUND_PREFIX (case insensitive).
#define MAX_SOUNDS  16
#define CHIRP_SOUND_PREFIX "CHIRP"
#define WELCOME_SOUND_PREFIX "WELC"
int num_chirp_sounds = 0;
int chirp_sounds[MAX_SOUNDS];
int num_welcome_sounds = 0;
int welcome_sounds[MAX_SOUNDS];

void setup() {
  // Open serial communications and wait for port to open
  Serial.begin(115200);
#ifdef DEBUG_MODE
  while (!Serial) {
    ; // wait for serial port to connect.
  }
#endif

  pinMode(LED, OUTPUT);
  
  Serial1.begin(9600); // AudioFX boards use 9600 baud
  while (!Serial1) {
    ; // wait for serial port to connect.
  }
  Serial.println("Starting");

  // Initialize the comms with the audio board on serial
  pinMode(SFX_ACT, INPUT);
  if (!sfx.reset()) {
    Serial.println("Not found");
    while (1);
  }
  Serial.println("SFX board found");
  
  // Discover the set of audio files to use
  uint8_t files = sfx.listFiles();
  Serial.println("File Listing");
  Serial.println("========================");
  Serial.println();
  Serial.print("Found "); Serial.print(files); Serial.println(" Files");
  for (uint8_t f = 0; f < files; f++) {
    Serial.print(f);
    Serial.print("\tname: "); Serial.print(sfx.fileName(f));
    Serial.print("\tsize: "); Serial.println(sfx.fileSize(f));
    if (strstr(sfx.fileName(f), CHIRP_SOUND_PREFIX) == sfx.fileName(f)) {
      Serial.println("chirp sound found");
      chirp_sounds[num_chirp_sounds++] = f;
    }
    else if (strstr(sfx.fileName(f), WELCOME_SOUND_PREFIX) == sfx.fileName(f)) {
      Serial.println("welcome sound found");
      welcome_sounds[num_welcome_sounds++] = f;
    }
  }
  Serial.println("========================");

  // Seed the PRNG from noise in the sensor
  srand(rand() + analogRead(PROXIMITY_SENSOR_PIN));
}

int getFilteredSensorValue() {
  // The sensor is somewhat noisy. Keep a circular buffer and use the average.
#define NUM_SENSOR_VALUES 3
  static int index = 0;
  static int sensorValues[NUM_SENSOR_VALUES] = {0};
  
  // read the value from the sensor:
  sensorValues[index] = analogRead(PROXIMITY_SENSOR_PIN);

  // update the random seed
  srand(rand() + sensorValues[index]);
  
  // get simple mean
  int sum = 0;
  for (int i = 0; i < NUM_SENSOR_VALUES; i++) {
    sum += sensorValues[i];
  }

#ifdef VERBOSE_DEBUG_MODE
  for (int i = 0; i < NUM_SENSOR_VALUES; i++) {
    Serial.print(sensorValues[i]);
    Serial.print(" ");
  }
  Serial.println("");
#endif

  index = (index + 1) % NUM_SENSOR_VALUES;
  return sum / NUM_SENSOR_VALUES;
}

DistanceRange getDistanceRange(int sensor_value) {
  if (sensor_value >= IN_RANGE_START) {
    return SOMETHING;
  }
  return NOTHING;
}

DistanceRange last_distance = NOTHING;
int chirp_count = 0;
int chirps_remaining = 0;
boolean welcome_msg = false;
unsigned pause_time = 0;
unsigned pause_time_remaining = 0;

void setNothingState() {
  chirp_count = 0;
  chirps_remaining = 0;
  welcome_msg = false;
  pause_time = 0;
  pause_time_remaining = 0;  
}

void setSomethingState() {
  chirp_count = CHIRPS_PER_WELCOME;
  chirps_remaining = (last_distance == NOTHING) ? 1 : 0;
  welcome_msg = true;
  pause_time = CHIRP_PAUSE;
  pause_time_remaining = 0;
}

void playSound(int id) {
  digitalWrite(LED, HIGH);
  
  if (sfx.playTrack(id)) {
    // While track plays, keep the sensor sampling running
    while (digitalRead(SFX_ACT) == LOW) {
      Serial.print(".");
      delay(SAMPLE_TIME);
      getFilteredSensorValue();
    }
    Serial.println("+");
  }
  else {
    Serial.println("!");
  }
  
  digitalWrite(LED, LOW);
}

void loop() {
  pause_time_remaining -= SAMPLE_TIME;
  
  // Figure out if we should change state
  DistanceRange distance = getDistanceRange(getFilteredSensorValue());
  if (last_distance != distance) {
    switch (distance) {
    case NOTHING:
      setNothingState();
      break;
    case SOMETHING:
      setSomethingState();
      break;
    }
  }

  // Play a sound, if appropriate
  if (pause_time_remaining == 0 && pause_time > 0) {
    if (chirps_remaining-- >= 0) {
      Serial.println("chirp");
      playSound(chirp_sounds[rand() % num_chirp_sounds]);
    }
    if (chirps_remaining == 0 && welcome_msg) {
      // Make sure the person is still there after the first chirp
      if (getDistanceRange(getFilteredSensorValue()) != NOTHING) {
        Serial.println("welcome");
        playSound(welcome_sounds[rand() % num_welcome_sounds]);
      }
    }

    // reset the chirp count
    if (chirps_remaining < 0) {
      chirps_remaining = chirp_count;
    }

    pause_time_remaining = pause_time;
  }

  last_distance = distance;
#ifdef DEBUG_MODE
  Serial.print("ld: ");
  Serial.print(last_distance);
  Serial.print(" cc: ");
  Serial.print(chirp_count);
  Serial.print(" cr: ");
  Serial.print(chirps_remaining);
  Serial.print(" wm: ");
  Serial.print(welcome_msg);
  Serial.print(" pt: ");
  Serial.print(pause_time);
  Serial.print(" ptr: ");
  Serial.println(pause_time_remaining);
#endif
  delay(SAMPLE_TIME);
}
