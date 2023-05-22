/*
  CRONOMETRAJE F2C TEAM RACING, MASTER PANELES
  F2C TEAM RACING TIMING, PANELS MASTER
  Código v2.0 actualizado 22may2023
  Code v2.0 updated may222023
*/

#include "config.h"
#include "main.h"

void setup() {
    Serial.begin(SERIAL_MONITOR_BAUDS);
    init_pins();
    init_radio();
    Serial.println("SLAVE PANEL EQUIPO INICIALIZADO");
}

void loop()
{
    loop_radio();
}