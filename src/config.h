/*
https://lastminuteengineers.com/nrf24l01-arduino-wireless-communication/
https://circuitdigest.com/microcontroller-projects/bluetooth-controlled-arduino-scoreboard-using-p10-led-matrix-display
*/

#pragma once

#include <Arduino.h>
#include <Bounce2.h>
#include <SPI.h>
#include <RF24.h>
#include <DMD2.h>
#include "arial_regular_24.h"

// Constantes pines / pins definition
#define NRF_CE_PIN 15
#define NRF_CSN_PIN 10
#define LAP_COUNTER_INTERRUPT_PIN 2
// #define INFO_LED_PIN 18

// Constantes generales / general constants
#define DEBUG // Si se define se habilita la consola serial / If defined, serial console is enabled
#define SERIAL_MONITOR_BAUDS 115200
#define RADIO_COMMAND_LENGTH 3
#define NRF_CHANNEL 108
#define LED_MATRIX_ROWS 4
#define LED_MATRIX_COLS 2

// Objecto para control de radio y pulso vueltas / Object for radio control and laps pulse
// Objeto para manejo de matriz mosaicos led P10 / Object for handling P10 modules matrix
RF24 radio(NRF_CE_PIN, NRF_CSN_PIN);
Bounce laps_pulse = Bounce();
SoftDMD led_matrix(LED_MATRIX_ROWS, LED_MATRIX_COLS);
DMD_TextBox laps_box(led_matrix, 5, 5, 50, 24);

// Dirección de comunicación
const byte RADIO_ADDRESS[6] = "00001";

// Payload para manejo de datos / Payload for data handling
struct Payload {
    char id[6];
    char data[RADIO_COMMAND_LENGTH + 1] = "000";
};

Payload payload;

byte laps_counter = 0;