#pragma once

#include "config.h"

void rtc_swatch_check() {
    interruptCount++;

    if (interruptCount % 819 == 0) {
        ts++;
        update_display = true;
    }
    
    if (interruptCount == 8192) {
        ss++;
        ts = -1;
        interruptCount = 0;

        if (ss > 59) { ss = 0; mm++; }
        if (mm > 9) { mm = 0; }
    }
}

void rtc_warmup_check() {
    interruptCount++;

    if (interruptCount == 8192) {
        ss--;
        interruptCount = 0;
        update_display = true;
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

    led_matrix.drawFilledBox(start_x_coord, 33, end_x_coord, 47, GRAPHICS_INVERSE);
    if (show_faults) { led_matrix.drawFilledBox(start_x_coord, 33, limit_x_coord, 47, GRAPHICS_NORMAL); }
}

void update_laps_in_display() {
    static char laps_string[4];
    snprintf(laps_string, sizeof(laps_string), "%03d", laps_counter);
    
    // for (int y = 5; y < 30; y++) { led_matrix.drawLine(5, y, 44, y, GRAPHICS_INVERSE); }
    led_matrix.drawFilledBox(5, 5, 44, 29, GRAPHICS_INVERSE);
    led_matrix.drawChar(5, 5, laps_string[0], GRAPHICS_NORMAL);
    led_matrix.drawChar(18, 5, laps_string[1], GRAPHICS_NORMAL);
    led_matrix.drawChar(31, 5, laps_string[2], GRAPHICS_NORMAL);
}

void verify_payload_data(char *data) {
    unsigned int command = 99;

    for (unsigned int c = 0; c < sizeof(COMMANDS_ARRAY) / sizeof(COMMANDS_ARRAY[0]); c++) {
        if (strcmp(data, COMMANDS_ARRAY[c]) == 0) { command = c; }
    }

    switch (command) {
        // RFP (Red fault plus)
        case 0: {
            faults_counter < 4 ? faults_counter++ : faults_counter = 4;
            update_fault_lights();
            break;
        }
        
        // RFM (Red fault minus)
        case 1: {
            faults_counter > 0 ? faults_counter-- : faults_counter = 0;
            update_fault_lights();
            break;
        }

        // SRS (Start race signal)
        case 2: {
            race_started = true;
            update_display = true;
            laps_counter = 0, mm = 0, ss = 0, ts = 0, interruptCount = 0;
            attachInterrupt(digitalPinToInterrupt(RTC_EXT_INT_PIN), rtc_swatch_check, FALLING);
            break;
        }

        // RRS (Reset race signal)
        case 3: {
            race_started = false;
            update_display = true;
            laps_counter = 0, mm = 0, ss = 0, ts = 0, interruptCount = 0;
            detachInterrupt(digitalPinToInterrupt(RTC_EXT_INT_PIN));
            update_laps_in_display();
            break;
        }

        // SES (Start engines signal)
        case 4: {
            Serial.println("Muestra 1:30");
            warmup_started = true;
            led_matrix.drawFilledBox(60, 5, 125, 29, GRAPHICS_INVERSE);
            led_matrix.drawString(60, 5, "1:30", 6, GRAPHICS_NORMAL);
            laps_counter = 0, mm = 0, ss = 90, ts = 0, interruptCount = 0;
            attachInterrupt(digitalPinToInterrupt(RTC_EXT_INT_PIN), rtc_warmup_check, FALLING);

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
    
    led_matrix.drawLine(45, 0, 45, 31, GRAPHICS_NORMAL);
    led_matrix.drawLine(46, 0, 46, 31, GRAPHICS_NORMAL);
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
    rtc.writeSqwPinMode(DS3231_SquareWave8kHz);
    // rtc.enable32K();
    
    pinMode(RTC_EXT_INT_PIN, INPUT_PULLUP);

    #ifdef DEBUG
    Serial.println(F("RTC: OK 8K output enabled"));
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
    static unsigned long display_timer = 0;

    if (warmup_started && update_display) {
        if (ss >= 60) {
            snprintf(display_buffer, sizeof(display_buffer), "1:%02d", ss - 60);
        } else {
            snprintf(display_buffer, sizeof(display_buffer), "0:%02d", ss);
        }
        led_matrix.drawFilledBox(60, 5, 125, 29, GRAPHICS_INVERSE);
        led_matrix.drawString(60, 5, display_buffer, sizeof(display_buffer) - 1, GRAPHICS_NORMAL);
        
        #ifdef DEBUG
        Serial.println(display_buffer);
        #endif

        if (ss == 0) {
            warmup_started = false;
            last30_started = true;
            ss = 30, interruptCount = 0;

            #ifdef DEBUG
            Serial.println("Limpia warmup");
            #endif
        }

        update_display = false;
    }

    if (last30_started && update_display) {
        snprintf(display_buffer, sizeof(display_buffer), "0:%02d", ss);
        led_matrix.drawFilledBox(60, 5, 125, 29, GRAPHICS_INVERSE);
        led_matrix.drawString(60, 5, display_buffer, sizeof(display_buffer) - 1, GRAPHICS_NORMAL);

        #ifdef DEBUG
        Serial.println(display_buffer);
        #endif

        if (ss == 5) {
            last30_started = false;
            detachInterrupt(digitalPinToInterrupt(RTC_EXT_INT_PIN));
            init_led_matrix();

            #ifdef DEBUG
            Serial.println("Limpia crono");
            #endif
        }

        update_display = false;
    }

    if (race_started && update_display) {
        snprintf(display_buffer, sizeof(display_buffer), "%d:%02d.%d", mm, ss, ts);
        // for (int y = 5; y < 30; y++) { led_matrix.drawLine(47, y, 125, y, GRAPHICS_INVERSE); }
        led_matrix.drawFilledBox(47, 5, 125, 29, GRAPHICS_INVERSE);
        led_matrix.drawString(60, 5, display_buffer, sizeof(display_buffer) - 1, GRAPHICS_NORMAL);

        update_display = false;
            
        #ifdef DEBUG
        Serial.println(display_buffer);
        #endif
    }

    if (millis() - display_timer >= 5) {
        led_matrix.scanDisplayBySPI();
        display_timer = millis();
    }
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