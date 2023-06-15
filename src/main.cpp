/*
  CRONOMETRAJE F2C TEAM RACING / F2C TEAM RACING TIMING
  CAJA PANEL EQUIPO módulo 2.4 gHz / TEAM PANEL BOX 2.4 gHz module
  v2.7 actualizado 15jun2023 / updated jun152023
*/

#include "config.h"
#include "main.h"

void setup() {
    #ifdef DEBUG
    Serial.begin(SERIAL_MONITOR_BAUDS);
    #endif

    init_pins();
    init_radio();
    init_led_matrix();
    init_rtc();

    #ifdef DEBUG
    Serial.println(F("CAJA PANEL EQUIPO INICIALIZADA / TEAM PANEL BOX STARTED"));
    #endif
}

void loop() {
    loop_radio();
    loop_matrix();
    loop_laps_pulse();
}