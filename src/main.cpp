/*
  CRONOMETRAJE F2C TEAM RACING / F2C TEAM RACING TIMING
  CAJA PANEL EQUIPO módulo 2.4 gHz / TEAM PANEL BOX 2.4 gHz module
  v2.1 actualizado 25may2023 / updated may252023
*/

#include "config.h"
#include "main.h"

void setup() {
    Serial.begin(SERIAL_MONITOR_BAUDS);
    init_pins();
    init_radio();
    init_led_matrix();

    #ifdef DEBUG
    Serial.println("CAJA PANEL EQUIPO INICIALIZADA / TEAM PANEL BOX STARTED");
    #endif
}

void loop() {
    loop_radio();
    loop_laps_pulse();
}