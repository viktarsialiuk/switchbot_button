#include <Arduino.h>
#include <stdint.h>

#include "exponential_delay.h"
#include "moving_average.h"
#include "switchbot_client.h"

static ExponentialDelay exp_delay;
static SwitchBotClient switchbot_client;
static MovingAverage brightness;

void led_blink(uint8_t pin) {
    for (int i = 0; i < 3; ++i) {
        digitalWrite(pin, HIGH);
        delay(200);
        digitalWrite(pin, LOW);
        delay(200);
    }
}

void setup() {
    Serial.begin(9600);
    Serial.println("Starting Arduino BLE Client application...");

    // Delay in the setup to avoid crashes that prevent uploading new sketches.
    delay(10000);

    // Initi leds.
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    led_blink(LED_BUILTIN);

    // The remote service we wish to connect to.
    static BLEUUID serviceUUID("cba20d00-224d-11e6-9fb8-0002a5d5c51b");
    // The characteristic of the remote service we are interested in.
    static BLEUUID charUUID("cba20002-224d-11e6-9fb8-0002a5d5c51b");
    // static BLEUUID notifyUUID( "cba20003-224d-11e6-9fb8-0002a5d5c51b" );
    SwitchBotClient_init(&switchbot_client, serviceUUID, charUUID,
                         /*max_clients=*/5, /*max_keep_alive=*/50000);
    ExponentialDelay_init(&exp_delay);
    MovingAverage_init(&brightness);
}

// This is the Arduino main loop function.
void loop() {
    MovingAverage_add_value(&brightness, analogRead(A1));
    const uint16_t avg_birghtness = MovingAverage_get_average(&brightness);
    Serial.printf("Brightness sensor %d\r\n", avg_birghtness);
    if (avg_birghtness > 1000) {
        if (ExponentialDelay_try_enter(&exp_delay)) {
            led_blink(LED_BUILTIN);

            uint8_t pressCommand[] = {0x57, 0x01, 0x00};
            SwitchBotClient_smart_send(&switchbot_client, &pressCommand[0],
                                       sizeof(pressCommand));
            ExponentialDelay_delay(&exp_delay);
        }
    } else {
        SwitchBotClient_diconnect(&switchbot_client);
        ExponentialDelay_reset(&exp_delay);
    }
    delay(1000);
}
