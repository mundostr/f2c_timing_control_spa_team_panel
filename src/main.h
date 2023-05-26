#pragma once

#include "config.h"

void init_pins() {
    // pinMode(INFO_LED_PIN, OUTPUT);
    // digitalWrite(INFO_LED_PIN, LOW);

    laps_pulse.attach(LAP_COUNTER_INTERRUPT_PIN, INPUT);
}

void init_led_matrix() {
    led_matrix.setBrightness(128);
    led_matrix.selectFont(Arial_Regular_24);
    led_matrix.begin();
    laps_box.print(100); // Primer test de caracteres / First characters test
}

// void blink_info_led() {
//     for(;;) {
//         digitalWrite(INFO_LED_PIN, !digitalRead(INFO_LED_PIN));
//         delay(250);
//     }
// }

// void verify_payload_data(char *data) {
//    if (strcmp(data, "100") == 0) Serial.println("Configurado a 100 vueltas");
//    if (strcmp(data, "200") == 0) Serial.println("Configurado a 200 vueltas");
// }

void init_radio() {
    if (!radio.begin()) {
        #ifdef DEBUG
        Serial.println(F("NRF24: ERROR"));
        #endif
        // blink_info_led();
    }

    radio.setPALevel(RF24_PA_MAX); //(RF24_PA_MIN|RF24_PA_LOW|RF24_PA_HIGH|RF24_PA_MAX)
    radio.setDataRate(RF24_250KBPS); //(RF24_250KBPS|RF24_1MBPS|RF24_2MBPS)
    radio.setChannel(NRF_CHANNEL);
    radio.setPayloadSize(sizeof(payload));
    radio.openReadingPipe(0, RADIO_ADDRESS);
    radio.startListening(); // Modo RX / RX Mode
    
    #ifdef DEBUG
    Serial.println(F("NRF24: OK modo RX / RX mode OK"));
    #endif
}

void loop_radio() {
    static int receivedCount = 0;
    
    if (radio.available() > 0) {
        radio.read(&payload, sizeof(payload));
        receivedCount++;

        char outputBuf[50];
        sprintf(outputBuf, "Contador: %d, ID: %s, comando: %s", receivedCount, payload.id, payload.data);
        
        #ifdef DEBUG
        Serial.println(outputBuf);
        #endif

        // verify_payload_data(payload.data);
    }
}

void loop_laps_pulse() {
    laps_pulse.update();

    if (laps_pulse.rose()) {
        laps_counter++;
        
        #ifdef DEBUG
        Serial.println(laps_counter);
        #endif
    }
}