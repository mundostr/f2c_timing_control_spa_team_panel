/*
https://lastminuteengineers.com/nrf24l01-arduino-wireless-communication/
https://circuitdigest.com/microcontroller-projects/bluetooth-controlled-arduino-scoreboard-using-p10-led-matrix-display
https://forum.arduino.cc/t/need-numeric-or-another-font-for-a-clock-in-dmd2-library/492186/4
https://lygte-info.dk/project/PrecisionTime%20UK.html
https://lastminuteengineers.com/ds3231-rtc-arduino-tutorial/#:~:text=The%20INT%2FSQW%20pin%20on,clock%20on%20the%2032K%20pin.
https://forum.arduino.cc/t/p10-with-nrf24l01/570541/10
*/

#pragma once

#include <Arduino.h>
#include <Bounce2.h>
#include <SPI.h>
#include <RF24.h>
#include <Wire.h>
#include <RTClib.h>
#include "DMD.h"
#include "droid_sans_24.h"
// #include "arial_regular_24.h"

// Constantes pines / pins definition
#define NRF_CE_PIN 15
#define NRF_CSN_PIN 10
#define LAP_COUNTER_INTERRUPT_PIN 2
#define RTC_EXT_INT_PIN 3

// Constantes generales / general constants
// #define DEBUG // Si se define se habilita la consola serial / If defined, serial console is enabled
#define SERIAL_MONITOR_BAUDS 115200
#define RADIO_COMMAND_LENGTH 3
#define NRF_CHANNEL 108
#define LED_MATRIX_ROWS 3
#define LED_MATRIX_COLS 4

// Objecto para control de radio y pulso vueltas / Object for radio control and laps pulse
// Objeto para manejo de matriz mosaicos led P10 / Object for handling P10 modules matrix
RF24 radio(NRF_CE_PIN, NRF_CSN_PIN);
Bounce laps_pulse = Bounce();
DMD led_matrix(LED_MATRIX_COLS, LED_MATRIX_ROWS);
RTC_DS3231 rtc;

// Dirección de comunicación / Comm address
const byte RADIO_ADDRESS[6] = "00001";

// Array de comandos / Commands array
const char COMMANDS_ARRAY[7][4] = {"RFP", "RFM", "SRS", "RRS", "SES", "100", "200"};

// Payload para manejo de datos / Payload for data handling
struct Payload {
    char id[6];
    char data[RADIO_COMMAND_LENGTH + 1] = "000";
};

Payload payload;

byte laps_limit = 100; // Límite para detención de crono / Limit to stop timing
byte laps_counter = 0; // Contador de vueltas de carrera / Race laps counter
byte faults_counter = 0; // Contador de infracciones / Warnings counter

// Flags para marcas de inicio warmup y carrera / Flags for warmup and race init marks
bool race_started = false;
bool warmup_started = false;

// Flags para interrupción control cronómetro / Flags for stopwatch control interrupt
volatile bool update_display = false;
volatile int mm = 0, ss = 0, ts = 0;
volatile unsigned int interruptCount = 0;