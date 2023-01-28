#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_DotStar.h>
#include <radio.h>
#include <SI4703.h>

#define FIX_BAND RADIO_BAND_FM // FM band 87.5 - 108 MHz (USA, Europe) selected.
#define FIX_STATION 8910       // aka 89.30 MHz.
#define FIX_VOLUME 15
#define MIN_VARIABLE_CAPACITOR 70
#define MAX_VARIABLE_CAPACITOR 570

#define ADC_IN A0
#define ADC_VCC A4
#define ADC_RESOLUTION 12

const float R_PULLUP = 34.8;
const int MAX_ADC_VALUE = (0x01 << ADC_RESOLUTION) - 1;
const float STRAY_CAP_TO_GND = 49.63;

typedef struct{
  float val;
  uint32_t raw;
}cap_measurement_t;

SI4703 radio; // Create an instance of Class for Si4703 Chip
Adafruit_DotStar strip = Adafruit_DotStar(DOTSTAR_NUM, PIN_DOTSTAR_DATA, PIN_DOTSTAR_CLK, DOTSTAR_BGR);

void measureCap(cap_measurement_t* cap)
{
  pinMode(ADC_IN, INPUT);
  pinMode(ADC_VCC, OUTPUT);
  digitalWrite(ADC_VCC, HIGH);

  cap->raw = analogRead(ADC_IN);

  digitalWrite(ADC_VCC, LOW); // discharge
  pinMode(ADC_IN, OUTPUT);

  // The voltage on ADC will settle quickly, it is a simple voltage divider with stray capacitance
  cap->val = (float)cap->raw * STRAY_CAP_TO_GND / (float)(MAX_ADC_VALUE - cap->raw);
}

static float measureCapLong()
{
  pinMode(ADC_IN, OUTPUT);
  delay(1);
  pinMode(ADC_VCC, INPUT_PULLUP);
  uint32_t u1 = micros();
  uint32_t t;
  uint8_t digVal;

  do
  {
    digVal = digitalRead(ADC_VCC);
    uint32_t u2 = micros();
    t = u2 > u1 ? u2 - u1 : u1 - u2; // rollover
  } while ((digVal < 1) && (t < 400000L));

  uint32_t val = analogRead(ADC_VCC);
  delay((int)(t / 1000L) * 5); // Discharge 5 * RC
  pinMode(ADC_VCC, OUTPUT);
  digitalWrite(ADC_VCC, LOW);
  digitalWrite(ADC_IN, LOW);

  float capacitance = -(float)t / R_PULLUP / log(1.0 - (float)val / (float)MAX_ADC_VALUE);

  return capacitance * 1000;
}

void setup()
{
  Serial.begin(115200);

  while (!Serial)
  {
  }

  Serial.printf("Version: %s\n", VERSION);

  // Enable information to the Serial port
  radio.debugEnable(true);
  radio._wireDebug(true);

  Wire.begin(); // a common pins for I2C = SDA:A4, SCL:A5
  radio.setup(RADIO_RESETPIN, A3);
  radio.setup(RADIO_SDAPIN, SDA);

  // Initialize the Radio
  radio.initWire(Wire);

  // Set all radio setting to the fixed values.
  radio.setBand(FIX_BAND);
  radio.setVolume(FIX_VOLUME);
  radio.setMono(false);
  radio.setMute(false);

  pinMode(ADC_VCC, OUTPUT);
  pinMode(ADC_IN, INPUT);
  analogReadResolution(ADC_RESOLUTION);
  // analogReference(AR_INTERNAL2V23); // Max 2,23V

  strip.begin();
}

void loop()
{
  float avgCap = 0;
  uint32_t avgRaw = 0;
  const uint8_t avg = 100;
  cap_measurement_t cap;

  for (size_t i = 0; i < avg; i++)
  {
    measureCap(&cap);
    avgCap += cap.val;
    avgRaw += cap.raw;
  }

  avgCap /= (float)avg;
  avgRaw /= avg;

  Serial.print(F("Capacitance ValueAVG = \t\t"));
  Serial.println(avgCap, 1);
  Serial.print(F("RAW ValueAVG = \t\t\t"));
  Serial.println(avgRaw);

  Serial.print(F("Capacitance Short Single = \t"));
  measureCap(&cap);
  Serial.print(cap.val, 1);
  Serial.print("pF - ");
  Serial.println(cap.raw);

  Serial.print(F("Capacitance ValueLong = \t"));
  Serial.println(measureCapLong(), 1);

  delay(1000);
  return;

  if (fabs(cap.val - avgCap) > 1) // TODO
  {
    // FM band 87.5 - 108
    // My variable capacitor ranges from 70 to 570pF
    uint16_t newFreq = map(cap.val, MIN_VARIABLE_CAPACITOR, MAX_VARIABLE_CAPACITOR, 8750, 10800);
    radio.setFrequency(newFreq);
    cap.val = avgCap;
  }

  // 9710
  // 9830
  // 9650
  // 9570
  // 9450
  // 9390
  // 9370
  // 9280

  strip.setPixelColor(0, 0x00f3c98e);
  strip.show();

  char s[12];
  radio.formatFrequency(s, sizeof(s));
  Serial.print("Station:");
  Serial.println(s);

  Serial.print("Radio:");
  radio.debugRadioInfo();

  Serial.print("Audio:");
  radio.debugAudioInfo();

  delay(1000);
}
