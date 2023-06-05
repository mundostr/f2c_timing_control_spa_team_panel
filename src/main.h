#pragma once

#include "config.h"

byte getDigits(byte number) {
    int digits = 0;
    
    if (number == 0) return 1;
    
    while (number != 0) {
        number /= 10;
        digits++;
    }
    
    return digits;
}

void rtc_interrupt_check() {
    interruptCount++;

    if (interruptCount % 3276 == 0) {
        ts++;
        update_display = true;
    }
    
    if (interruptCount == 32768) {
        ss++;
        ts = -1;
        interruptCount = 0;

        if (ss > 59) { ss = 0; mm++; }
        if (mm > 9) { mm = 0; }
    }
}

void update_fault_lights() {
    static const int start_x_coord = 0;
    static const int end_x_coord = 127;
    int limit_x_coord;
    bool show_faults = true;

    switch (faults_counter) {
        case 0: {
            show_faults = false;
            break;
        }

        case 1: {
            limit_x_coord = 31;
            break;
        }

        case 2: {
            limit_x_coord = 63;
            break;
        }

        case 3: {
            limit_x_coord = 95;
            break;
        }

        case 4: {
            limit_x_coord = 127;
            break;
        }

        default: {}
    }

    // Limpieza fila inferior matriz (infracciones) / Cleanup of matrix bottom row (warnings)
    led_matrix.drawFilledBox(start_x_coord, 33, end_x_coord, 47, GRAPHICS_INVERSE);
    if (show_faults) { led_matrix.drawFilledBox(start_x_coord, 33, limit_x_coord, 47, GRAPHICS_NORMAL); }
}

void update_two_digits(byte counter, char *laps_str) {
    if (counter % 10 == 0) {
        led_matrix.drawChar(17, 5, laps_str[1], GRAPHICS_NORMAL);
        led_matrix.drawChar(29, 5, '0', GRAPHICS_NORMAL);
    } else {
        led_matrix.drawChar(29, 5, laps_str[2], GRAPHICS_NORMAL);
    }
}

void update_laps_in_display() {
    // Refresco alternativo caracter por caracter
    static char laps_string[4];
    snprintf(laps_string, sizeof(laps_string), "%03d", laps_counter);

    if (getDigits(laps_counter) == 1) {
        led_matrix.drawChar(5, 5, laps_string[2], GRAPHICS_NORMAL);
    } else if (getDigits(laps_counter) == 2) {
        update_two_digits(laps_counter, laps_string);
    } else {
        if (laps_counter % 100 == 0) {
            led_matrix.drawChar(5, 5, laps_string[0], GRAPHICS_NORMAL);
            led_matrix.drawChar(17, 5, '0', GRAPHICS_NORMAL);
            led_matrix.drawChar(29, 5, '0', GRAPHICS_NORMAL);
        } else {
            update_two_digits(laps_counter, laps_string);
        }
    }
}

void verify_payload_data(char *data) {
    unsigned int command = 99;

    for (unsigned int c = 0; c < sizeof(COMMANDS_ARRAY) / sizeof(COMMANDS_ARRAY[0]); c++) {
        if (strcmp(data, COMMANDS_ARRAY[c]) == 0) { command = c; }
    }

    switch (command) {
        // RFP
        case 0: {
            faults_counter < 4 ? faults_counter++ : faults_counter = 4;
            update_fault_lights();
            break;
        }
        
        // RFM
        case 1: {
            faults_counter > 0 ? faults_counter-- : faults_counter = 0;
            update_fault_lights();
            break;
        }

        // SRS
        case 2: {
            race_started = true;
            mm = 0, ss = 0, ts = 0, interruptCount = 0;
            attachInterrupt(digitalPinToInterrupt(RTC_EXT_INT_PIN), rtc_interrupt_check, FALLING);
            break;
        }

        // RRS
        case 3: {
            race_started = false;
            update_display = true;
            mm = 0; ss = 0; ts = 0;
            detachInterrupt(digitalPinToInterrupt(RTC_EXT_INT_PIN));
            break;
        }

        // SES (pendiente)
        case 4: {
            break;
        }

        // 100
        case 5: {
            laps_limit = 100;
            break;
        }

        // 200
        case 6: {
            laps_limit = 200;
            break;
        }
        
        default: {}
    }
    
    #ifdef DEBUG
    Serial.println(COMMANDS_ARRAY[command]);
    #endif
}


void init_pins() {
    laps_pulse.attach(LAP_COUNTER_INTERRUPT_PIN, INPUT);
}

void init_radio() {
    if (!radio.begin()) {
        #ifdef DEBUG
        Serial.println(F("NRF24: ERROR"));
        #endif
        for(;;);
    }

    radio.setPALevel(RF24_PA_MAX);   //(RF24_PA_MIN|RF24_PA_LOW|RF24_PA_HIGH|RF24_PA_MAX)
    radio.setDataRate(RF24_250KBPS); //(RF24_250KBPS|RF24_1MBPS|RF24_2MBPS)
    radio.setChannel(NRF_CHANNEL);
    radio.setPayloadSize(sizeof(payload));
    radio.openReadingPipe(0, RADIO_ADDRESS);
    radio.startListening(); // Modo RX / RX Mode
    
    #ifdef DEBUG
    Serial.println(F("NRF24: OK modo RX / RX mode OK"));
    #endif
}

void init_led_matrix() {
    led_matrix.clearScreen(true);
    led_matrix.selectFont(Droid_Sans_24);

    led_matrix.drawString(5, 5, "000", 3, GRAPHICS_NORMAL);
    led_matrix.drawString(60, 5, "0:00.0", 6, GRAPHICS_NORMAL);

    led_matrix.drawBox(0, 0, 127, 31, GRAPHICS_NORMAL);
    led_matrix.drawBox(1, 1, 126, 30, GRAPHICS_NORMAL);
    
    led_matrix.drawLine(44, 0, 44, 31, GRAPHICS_NORMAL);
    led_matrix.drawLine(45, 0, 45, 31, GRAPHICS_NORMAL);
}

void init_rtc() {
    Wire.begin();
    if (!rtc.begin()) {
        #ifdef DEBUG
        Serial.flush();
        Serial.println(F("RTC: ERROR"));
        #endif
        for(;;);
    }
    // rtc.writeSqwPinMode(DS3231_SquareWave8kHz);
    rtc.enable32K();
    
    pinMode(RTC_EXT_INT_PIN, INPUT_PULLUP);

    #ifdef DEBUG
    Serial.println(F("RTC: OK 32K output enabled"));
    #endif
}


void loop_radio() {
    static int receivedCount = 0;

    if (radio.available() > 0) {
        radio.read(&payload, sizeof(payload));
        receivedCount++;
        verify_payload_data(payload.data);

        #ifdef DEBUG
        char outputBuf[50];
        snprintf(outputBuf, sizeof(outputBuf), "Contador: %d, ID: %s, comando: %s", receivedCount, payload.id, payload.data);
        Serial.println(outputBuf);
        #endif
    }
}

void loop_matrix() {
    static char display_buffer[7];
    
    if (update_display) {
        snprintf(display_buffer, sizeof(display_buffer), "%d:%02d.%d", mm, ss, ts);
        for (int y = 2; y < 30; y++) { led_matrix.drawLine(46, y, 125, y, GRAPHICS_INVERSE); }
        led_matrix.drawString(60, 5, display_buffer, sizeof(display_buffer) - 1, GRAPHICS_NORMAL);

        update_display = false;
        
        #ifdef DEBUG
        Serial.println(display_buffer);
        #endif
    }

    led_matrix.scanDisplayBySPI();
}

void loop_laps_pulse() {
    laps_pulse.update();

    if (laps_pulse.rose()) {
        laps_counter++;
        update_laps_in_display();
        
        if (laps_counter == laps_limit) {
            race_started = false;
            detachInterrupt(digitalPinToInterrupt(RTC_EXT_INT_PIN));
        }
    }
}