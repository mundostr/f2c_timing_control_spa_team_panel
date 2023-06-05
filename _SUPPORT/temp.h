#include <Wire.h>
#include <RTClib.h>

#define RTC_EXT_INT_PIN 3

RTC_DS3231 rtc;

volatile bool update_display = false;
volatile int mm = 0, ss = 0, ts = 0, intCount = 0, tenths_interval = 3276;

void rtc_interrupt_check() {
  static int cycles_count = 0;

  intCount++;

  if (intCount == tenths_interval) {
    intCount = 0;
    ts++;

    if (ts > 9) {
      ts = 0;
      ss++;
      cycles_count++;

      if (cycles_count == 4) {
        cycles_count = 0;
        tenths_interval == 3276 ? tenths_interval = 3279 : tenths_interval = 3276;
      }
    }
    if (ss > 59) {
      ss = 0;
      mm++;
    }
    if (mm > 9) {
      mm = 0;
    }

    update_display = true;
  }
}

void setup() {
    Serial.begin(115200);
    
    Wire.begin();
    rtc.begin();
    // rtc.writeSqwPinMode(DS3231_SquareWave8kHz);
    rtc.enable32K();
    
    pinMode(RTC_EXT_INT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(RTC_EXT_INT_PIN), rtc_interrupt_check, FALLING);
}

void loop() {
    if (update_display) {
        char display_buffer[7];
        snprintf(display_buffer, sizeof(display_buffer), "%d:%02d.%d", mm, ss, ts);
        update_display = false;
        Serial.println(display_buffer);
    }
}