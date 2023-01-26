#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_DotStar.h>
#include <radio.h>
#include <SI4703.h>

#define FIX_BAND RADIO_BAND_FM // FM band 87.5 - 108 MHz (USA, Europe) selected.
#define FIX_STATION 8910       // aka 89.30 MHz.
#define FIX_VOLUME 15

#define ADC_IN A0
#define ADC_VCC A4

const float IN_STRAY_CAP_TO_GND = 24.48;
const float IN_CAP_TO_GND = IN_STRAY_CAP_TO_GND;
const float R_PULLUP = 34.8;
const int MAX_ADC_VALUE = 4095;

SI4703 radio; // Create an instance of Class for Si4703 Chip
Adafruit_DotStar strip = Adafruit_DotStar(DOTSTAR_NUM, PIN_DOTSTAR_DATA, PIN_DOTSTAR_CLK, DOTSTAR_BGR);

static float measureCap()
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

  Serial.print(F("Capacitance Value = "));
  Serial.print(capacitance *= 1000, 2);
  Serial.print(F(" pF\n"));
  Serial.printf("t = %dus\n", t);

  return capacitance;
}

void setup()
{
  Serial.begin(115200);

  // while (!Serial) {}

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

  strip.begin();
}

void loop()
{
  static float cap = 0;
  float avgCap = 0;
  const uint8_t avg = 10;

  for (size_t i = 0; i < avg; i++)
  {
    avgCap += measureCap();
  }

  avgCap /= (float)avg;

  Serial.print(F("Capacitance Value = "));
  Serial.print(avgCap, 2);

  if (fabs(cap - avgCap) > 1) // TODO
  {
    // FM band 87.5 - 108
    // My variable capacitor ranges from 70 to 570pF
    uint16_t newFreq = map(cap, 70, 570, 8750, 10800);
    radio.setFrequency(newFreq);
    cap = avgCap;
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
