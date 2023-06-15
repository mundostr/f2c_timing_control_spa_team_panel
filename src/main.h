#pragma once

#include "config.h"

void rtc_swatch_check() {
    interruptCount++;
    
    if (interruptCount % 102 == 0) {
    // if (interruptCount % 409 == 0) {
    // if (interruptCount % 819 == 0) {
    // if (interruptCount % 3276 == 0) {
        ts++;
        update_display = true;
    }
    
    if (interruptCount == 1024) {
    // if (interruptCount == 4096) {
    // if (interruptCount == 8192) {
    // if (interruptCount == 32768) {
        ss++;
        ts = -1;
        interruptCount = 0;

        if (ss > 59) { ss = 0; mm++; }
        if (mm > 9) { mm = 0; }
    }
}

void rtc_warmup_check() {
    interruptCount++;

    if (interruptCount == 1024) {
    // if (interruptCount == 4096) {
    // if (interruptCount == 8192) {
    // if (interruptCount == 32768) {
        ss--;
        interruptCount = 0;
        update_display = true;
    }
}

void update_fault_lights() {
    led_matrix.drawFilledBox(0, 33, 127, 47, GRAPHICS_INVERSE);

    if (faults_counter == 1) {
        led_matrix.drawFilledBox(1, 34, 30, 46, GRAPHICS_NORMAL);
    } else if (faults_counter == 2) {
        led_matrix.drawFilledBox(1, 34, 30, 46, GRAPHICS_NORMAL);
        led_matrix.drawFilledBox(33, 34, 62, 46, GRAPHICS_NORMAL);
    } else if (faults_counter == 3) {
        led_matrix.drawFilledBox(1, 34, 30, 46, GRAPHICS_NORMAL);
        led_matrix.drawFilledBox(33, 34, 62, 46, GRAPHICS_NORMAL);
        led_matrix.drawFilledBox(65, 34, 94, 46, GRAPHICS_NORMAL);
    } else {
        led_matrix.drawFilledBox(1, 34, 30, 46, GRAPHICS_NORMAL);
        led_matrix.drawFilledBox(33, 34, 62, 46, GRAPHICS_NORMAL);
        led_matrix.drawFilledBox(65, 34, 94, 46, GRAPHICS_NORMAL);
        led_matrix.drawFilledBox(97, 34, 126, 46, GRAPHICS_NORMAL);
    }

    /* switch (faults_counter) {
        case 0: {
            break;
        }

        case 1: {
            led_matrix.drawFilledBox(1, 34, 30, 46, GRAPHICS_NORMAL);
            break;
        }

        case 2: {
            led_matrix.drawFilledBox(1, 34, 30, 46, GRAPHICS_NORMAL);
            led_matrix.drawFilledBox(33, 34, 62, 46, GRAPHICS_NORMAL);
            break;
        }

        case 3: {
            led_matrix.drawFilledBox(1, 34, 30, 46, GRAPHICS_NORMAL);
            led_matrix.drawFilledBox(33, 34, 62, 46, GRAPHICS_NORMAL);
            led_matrix.drawFilledBox(65, 34, 94, 46, GRAPHICS_NORMAL);
            break;
        }

        case 4: {
            led_matrix.drawFilledBox(1, 34, 30, 46, GRAPHICS_NORMAL);
            led_matrix.drawFilledBox(33, 34, 62, 46, GRAPHICS_NORMAL);
            led_matrix.drawFilledBox(65, 34, 94, 46, GRAPHICS_NORMAL);
            led_matrix.drawFilledBox(97, 34, 126, 46, GRAPHICS_NORMAL);
            break;
        }

        default: {}
    } */

    #ifdef DEBUG
    Serial.print("Faltas: ");
    Serial.println(faults_counter);
    #endif
}

void update_laps_in_display() {
    char laps_string[4];
    snprintf(laps_string, 4, "%03d", laps_counter);
    
    led_matrix.drawFilledBox(POSX_LAPS, 5, 44, 29, GRAPHICS_INVERSE);
    led_matrix.drawChar(POSX_LAPS, 5, laps_string[0], GRAPHICS_NORMAL);
    led_matrix.drawChar(POSX_LAPS + CHAR_WIDTH, 5, laps_string[1], GRAPHICS_NORMAL);
    led_matrix.drawChar(POSX_LAPS + (CHAR_WIDTH * 2), 5, laps_string[2], GRAPHICS_NORMAL);
}

void init_led_matrix() {
    led_matrix.clearScreen(true);
    // led_matrix.selectFont(Droid_Sans_24);
    led_matrix.selectFont(BigNumber);

    led_matrix.drawString(POSX_LAPS, 5, "000", 3, GRAPHICS_NORMAL);
    led_matrix.drawString(POSX_SWATCH, 5, "0:00.0", 6, GRAPHICS_NORMAL);

    led_matrix.drawBox(0, 0, 127, 31, GRAPHICS_NORMAL);
    led_matrix.drawBox(1, 1, 126, 30, GRAPHICS_NORMAL);
    
    led_matrix.drawLine(POSX_VBAR, 0, POSX_VBAR, 31, GRAPHICS_NORMAL);
    led_matrix.drawLine(POSX_VBAR + 1, 0, POSX_VBAR + 1, 31, GRAPHICS_NORMAL);
}

void verify_payload_data(char *data) {
    // const char COMMANDS_ARRAY[7][4] = { "RFP", "RFM", "SRS", "RRS", "SES", "100", "200" };

    if (strcmp(data, "RFP") == 0) {
        faults_counter < 4 ? faults_counter++ : faults_counter = 4;
        update_fault_lights();
    
    } else if (strcmp(data, "RFM") == 0) {
        faults_counter > 0 ? faults_counter-- : faults_counter = 0;
        update_fault_lights();
    
    } else if (strcmp(data, "SRS") == 0) {
        race_started = true;
        update_display = true;
        laps_counter = 0, mm = 0, ss = 0, ts = 0, interruptCount = 0, display_timer = 0;
        attachInterrupt(digitalPinToInterrupt(RTC_EXT_INT_PIN), rtc_swatch_check, FALLING);
    
    } else if (strcmp(data, "RRS") == 0) {
        warmup_started = false;
        last30_started = false;
        race_started = false;
        update_display = true;
        laps_counter = 0, mm = 0, ss = 0, ts = 0, interruptCount = 0, display_timer = 0;
        detachInterrupt(digitalPinToInterrupt(RTC_EXT_INT_PIN));
        init_led_matrix();
    
    } else if (strcmp(data, "SES") == 0) {
        warmup_started = true;
        update_display = true;
        led_matrix.drawFilledBox(POSX_SWATCH, 5, 125, 29, GRAPHICS_INVERSE);
        led_matrix.drawString(POSX_SWATCH, 5, "1:30.0", 6, GRAPHICS_NORMAL);
        laps_counter = 0, mm = 0, ss = 90, ts = 0, interruptCount = 0, display_timer = 0;
        attachInterrupt(digitalPinToInterrupt(RTC_EXT_INT_PIN), rtc_warmup_check, FALLING);
    
    } else if (strcmp(data, "100") == 0) {
        if (!warmup_started && !last30_started && !race_started) {
            laps_limit = 100;
                
            #ifdef DEBUG
            Serial.println("Se ha cambiado a 100 vueltas");
            #endif
        }
    
    } else if (strcmp(data, "200") == 0) {
        if (!warmup_started && !last30_started && !race_started) {
            laps_limit = 200;

            #ifdef DEBUG
            Serial.println("Se ha cambiado a 200 vueltas");
            #endif
        }
    }

    // unsigned int command = 99;

    /* for (unsigned int c = 0; c < 7; c++) {
        if (strcmp(data, COMMANDS_ARRAY[c]) == 0) { command = c; }
    } */

    /* switch (command) {
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
            laps_counter = 0, mm = 0, ss = 0, ts = 0, interruptCount = 0, display_timer = 0;
            attachInterrupt(digitalPinToInterrupt(RTC_EXT_INT_PIN), rtc_swatch_check, FALLING);
            break;
        }

        // RRS (Reset race signal)
        case 3: {
            warmup_started = false;
            last30_started = false;
            race_started = false;
            update_display = true;
            laps_counter = 0, mm = 0, ss = 0, ts = 0, interruptCount = 0, display_timer = 0;
            detachInterrupt(digitalPinToInterrupt(RTC_EXT_INT_PIN));
            init_led_matrix();
            break;
        }

        // SES (Start engines signal)
        case 4: {
            warmup_started = true;
            update_display = true;
            led_matrix.drawFilledBox(POSX_SWATCH, 5, 125, 29, GRAPHICS_INVERSE);
            led_matrix.drawString(POSX_SWATCH, 5, "1:30.0", 6, GRAPHICS_NORMAL);
            laps_counter = 0, mm = 0, ss = 90, ts = 0, interruptCount = 0, display_timer = 0;
            attachInterrupt(digitalPinToInterrupt(RTC_EXT_INT_PIN), rtc_warmup_check, FALLING);

            #ifdef DEBUG
            Serial.println("Muestra 1:30");
            #endif

            break;
        }

        // 100
        case 5: {
            if (!warmup_started && !last30_started && !race_started) {
                laps_limit = 100;
                
                #ifdef DEBUG
                Serial.println("Se ha cambiado a 100 vueltas");
                #endif
            }
            break;
        }

        // 200
        case 6: {
            if (!warmup_started && !last30_started && !race_started) {
                laps_limit = 200;

                #ifdef DEBUG
                Serial.println("Se ha cambiado a 200 vueltas");
                #endif
            }
            break;
        }
        
        default: {}
    } */
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

void init_rtc() {
    Wire.begin();
    if (!rtc.begin()) {
        #ifdef DEBUG
        Serial.flush();
        Serial.println(F("RTC: ERROR"));
        #endif
        for(;;);
    }
    pinMode(RTC_EXT_INT_PIN, INPUT_PULLUP);
    rtc.writeSqwPinMode(DS3231_SquareWave1kHz);
    // rtc.writeSqwPinMode(DS3231_SquareWave4kHz);
    // rtc.writeSqwPinMode(DS3231_SquareWave8kHz);
    // rtc.enable32K();
    
    #ifdef DEBUG
    Serial.println(F("RTC: SWQ 1K output enabled"));
    #endif
}


void loop_radio() {
    // static int receivedCount = 0;

    if (radio.available() > 0) {
        radio.read(&payload, sizeof(payload));
        // receivedCount++;
        verify_payload_data(payload.data);

        #ifdef DEBUG
        char outputBuf[50];
        // snprintf(outputBuf, sizeof(outputBuf), "Contador: %d, ID: %s, comando: %s", receivedCount, payload.id, payload.data);
        snprintf(outputBuf, sizeof(outputBuf), "ID: %s, comando: %s", payload.id, payload.data);
        Serial.println(outputBuf);
        #endif
    }
}

void loop_matrix() {
    char display_buffer[7];

    if (warmup_started && update_display) {
        if (ss >= 60) {
            snprintf(display_buffer, 7, "1:%02d.0", ss - 60);
        } else {
            snprintf(display_buffer, 7, "0:%02d.0", ss);
        }
        led_matrix.drawFilledBox(POSX_SWATCH, 5, 125, 29, GRAPHICS_INVERSE);
        led_matrix.drawString(POSX_SWATCH, 5, display_buffer, 6, GRAPHICS_NORMAL);
        
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
        snprintf(display_buffer, 7, "0:%02d.0", ss);
        led_matrix.drawFilledBox(POSX_SWATCH, 5, 125, 29, GRAPHICS_INVERSE);
        led_matrix.drawString(POSX_SWATCH, 5, display_buffer, 6, GRAPHICS_NORMAL);

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
        snprintf(display_buffer, 7, "%d:%02d.%d", mm, ss, ts);
        led_matrix.drawFilledBox(POSX_SWATCH, 5, 126, 29, GRAPHICS_INVERSE);
        led_matrix.drawString(POSX_SWATCH, 5, display_buffer, 6, GRAPHICS_NORMAL);

        // Refresco alternativo
        /* if (ts % 10 == 0) { // ts está en cero, se actualiza ss y ts
            led_matrix.drawFilledBox(POSX_SWATCH + (CHAR_WIDTH * 2), 5, POSX_SWATCH + (CHAR_WIDTH * 4), 29, GRAPHICS_INVERSE);
            led_matrix.drawFilledBox(POSX_SWATCH + (CHAR_WIDTH * 5), 5, POSX_SWATCH + (CHAR_WIDTH * 6), 29, GRAPHICS_INVERSE);

            led_matrix.drawChar(POSX_SWATCH + (CHAR_WIDTH * 3), 5, display_buffer[2], GRAPHICS_NORMAL);
            led_matrix.drawChar(POSX_SWATCH + (CHAR_WIDTH * 4), 5, display_buffer[3], GRAPHICS_NORMAL);
            led_matrix.drawChar(POSX_SWATCH + (CHAR_WIDTH * 6), 5, display_buffer[5], GRAPHICS_NORMAL);
        } else { // solo se actualiza ts
            led_matrix.drawFilledBox(POSX_SWATCH + (CHAR_WIDTH * 2), 5, POSX_SWATCH + (CHAR_WIDTH * 4), 29, GRAPHICS_INVERSE);
            led_matrix.drawChar(POSX_SWATCH + (CHAR_WIDTH * 6), 5, display_buffer[5], GRAPHICS_NORMAL);
        } */

        update_display = false;
            
        #ifdef DEBUG
        Serial.println(display_buffer);
        #endif
    }

    if (millis() - display_timer >= 2) {
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

        #ifdef DEBUG
        Serial.println(laps_counter);
        #endif
    }
}