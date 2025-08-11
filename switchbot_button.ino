#include <Arduino.h>
#include <stdint.h>

#include "exponential_delay.h"
#include "moving_average.h"
#include "switchbot_device.h"

// The remote service we wish to connect to.
static BLEUUID serviceUUID("cba20d00-224d-11e6-9fb8-0002a5d5c51b");
// The characteristic of the remote service we are interested in.
static BLEUUID charUUID("cba20002-224d-11e6-9fb8-0002a5d5c51b");
// static BLEUUID notifyUUID( "cba20003-224d-11e6-9fb8-0002a5d5c51b" );

static ExponentialDelay exp_delay;
static SwitchBotDevice device;
static MovingAverage brightness;
static const char* known_devices[] = {"c1:02:ba:5a:0f:d0", "d0:02:60:11:1f:c7",
                                      "c7:6a:02:06:1d:45"};

void led_blink(uint8_t pin) {
    for (int i = 0; i < 3; ++i) {
        digitalWrite(pin, HIGH);
        delay(200);
        digitalWrite(pin, LOW);
        delay(200);
    }
}

void send_press() {
    const int devices_count = sizeof(known_devices) / sizeof(known_devices[0]);
    for (int i = 0; i < devices_count; ++i) {
        if (!SwitchBotDevice_connect_str(&device, known_devices[i], serviceUUID,
                                         charUUID)) {
            Serial.printf("Failed to connect to device %s\r\n",
                          known_devices[i]);
            continue;
        }
        uint8_t pressCommand[] = {0x57, 0x01, 0x00};
        SwitchBotDevice_send(&device, &pressCommand[0], sizeof(pressCommand));
        SwitchBotDevice_disconnect(&device);
    }
}

void setup() {
    Serial.begin(9600);
    Serial.println("Starting Arduino BLE Client application...");

    // Delay in the setup to avoid crashes that prevent uploading new sketches.
    delay(1000);

    // Initi leds.
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    led_blink(LED_BUILTIN);

    SwitchBotDevice_init(&device);
    ExponentialDelay_init(&exp_delay);
    MovingAverage_init(&brightness);
}

// This is the Arduino main loop function.
void loop() {
    // SwitchBotDevice_scan(serviceUUID);
    // return;

    MovingAverage_add_value(&brightness, analogRead(A1));
    const uint16_t avg_birghtness = MovingAverage_get_average(&brightness);
    Serial.printf("Brightness sensor %d\r\n", avg_birghtness);

    const uint16_t button_signal = analogRead(A2);
    Serial.printf("Button sensor %d\r\n", button_signal);

    static const uint16_t brightness_threshold = 2000;
    static const uint16_t button_threshold = 2000;
    
    if (avg_birghtness > brightness_threshold ||
        button_signal > button_threshold) {
        if (ExponentialDelay_try_enter(&exp_delay)) {
            led_blink(LED_BUILTIN);
            send_press();
            ExponentialDelay_delay(&exp_delay);
        }
    } else {
        ExponentialDelay_reset(&exp_delay);
    }
    delay(1000);
}
