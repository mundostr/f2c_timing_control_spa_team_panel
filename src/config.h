/*
https://lastminuteengineers.com/nrf24l01-arduino-wireless-communication/
*/

#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>

// Constantes pines / pins definition
#define NRF_CE_PIN 15
#define NRF_CSN_PIN 10
// #define INFO_LED_PIN 18

// Constantes generales / general constants
#define DEBUG // Si se define se habilita la consola serial / If defined, serial console is enabled
#define SERIAL_MONITOR_BAUDS 115200
#define RADIO_COMMAND_LENGTH 3
#define NRF_CHANNEL 108

// Objecto para control de radio / Object for radio control
RF24 radio(NRF_CE_PIN, NRF_CSN_PIN);

// Dirección de comunicación
const byte RADIO_ADDRESS[6] = "00001";

// Payload para manejo de datos / Payload for data handling
struct Payload {
    char id[6];
    char data[RADIO_COMMAND_LENGTH + 1] = "000";
};

Payload payload;