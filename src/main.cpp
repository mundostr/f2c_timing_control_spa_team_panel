/*
  CRONOMETRAJE F2C TEAM RACING / F2C TEAM RACING TIMING
  CAJA PANEL EQUIPO módulo 2.4 gHz / TEAM PANEL BOX 2.4 gHz module
  v2.4 actualizado 06jun2023 / updated jun062023
*/

#include "config.h"
#include "main.h"

void setup() {
    Serial.begin(SERIAL_MONITOR_BAUDS);
    #ifdef DEBUG
    #endif

    init_pins();
    // init_radio();
    init_led_matrix();
    init_rtc();

    #ifdef DEBUG
    Serial.println(F("CAJA PANEL EQUIPO INICIALIZADA / TEAM PANEL BOX STARTED"));
    #endif

    delay(3000);
    race_started = true;
    update_display = true;
    attachInterrupt(digitalPinToInterrupt(RTC_EXT_INT_PIN), rtc_interrupt_check, FALLING);
}

void loop() {
    // loop_radio();
    loop_matrix();
    loop_laps_pulse();
}