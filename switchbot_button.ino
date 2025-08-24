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
// static MovingAverage brightness;
static ExponentialDelay exp_delay;
static SwitchBotDevice devices[3];
static const char* addresses[] = {
    "c7:6a:02:06:1d:45",
    "c1:02:ba:5a:0f:d0",
    "d0:02:60:11:1f:c7",
};

// Input pins.
static const uint8_t kDigitalVCCPin = D2;
static const uint8_t kSensorPin = A1;
static const uint8_t kButtonPin = A2;

static void led_blink(uint8_t pin) {
    for (int i = 0; i < 3; ++i) {
        digitalWrite(pin, HIGH);
        delay(200);
        digitalWrite(pin, LOW);
        delay(200);
    }
}

static void init_light_sensor() {
    pinMode(kDigitalVCCPin, OUTPUT);
    digitalWrite(kDigitalVCCPin, LOW);
}

static uint16_t read_light_sensor() {
    // pinMode(kDigitalVCCPin, OUTPUT);
    digitalWrite(kDigitalVCCPin, HIGH);
    delay(10);
    uint16_t value = analogRead(kSensorPin);
    digitalWrite(kDigitalVCCPin, LOW);
    // pinMode(kDigitalVCCPin, INPUT);
    return value;
}

static uint16_t read_button_sensor() { return analogRead(kButtonPin); }

static void send_press() {
    const int devices_count = sizeof(devices) / sizeof(devices[0]);
    const int addresses_count = sizeof(addresses) / sizeof(addresses[0]);
    if (devices_count != addresses_count) {
        Serial.printf("devices_count %d != addresses_count %d\r\n",
                      devices_count, addresses_count);
        return;
    }
    for (int i = 0; i < devices_count; ++i) {
        if (!switch_bot_connect(&devices[i], addresses[i], serviceUUID,
                                charUUID)) {
            Serial.printf("Failed to connect to device %s\r\n", addresses[i]);
            continue;
        }
        uint8_t pressCommand[] = {0x57, 0x01, 0x00};
        switch_bot_send(&devices[i], &pressCommand[0], sizeof(pressCommand));
        switch_bot_read(&devices[i]);
        switch_bot_disconnect(&devices[i]);
    }
}

void setup() {
    Serial.begin(9600);
    Serial.println("Starting Arduino BLE Client application...");

    // Delay in the setup to avoid crashes that prevent uploading new sketches.
    while (!Serial && millis() < 4000);

    // Initi leds.
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    led_blink(LED_BUILTIN);

    for (int i = 0; i < sizeof(devices) / sizeof(devices[0]); ++i) {
        switch_bot_init(&devices[i]);
    }
    exponential_delay_init(&exp_delay);
    // moving_average_init(&brightness);
    init_light_sensor();
}

// This is the Arduino main loop function.
void loop() {
    if (Serial.available() > 0) {
        uint8_t command = Serial.read();
        if (command == 's') {
            switch_bot_scan(serviceUUID);
        }
    }

    // moving_average_add_value(&brightness, analogRead(A1));
    // const uint16_t avg_birghtness = moving_average_get_average(&brightness);
    const uint16_t brightness = read_light_sensor();
    Serial.printf("Brightness sensor %d\r\n", brightness);

    const uint16_t button_signal = read_button_sensor();
    Serial.printf("Button sensor %d\r\n", button_signal);

    // 1495 works stable on sunny days, but slow afternoon.
    // This is old threshold for photo led schema.
    // static const uint16_t brightness_threshold = 1493;
    // This is threahold for photo resistor schema.
    static const uint16_t brightness_threshold = 2000;
    static const uint16_t button_threshold = 2000;

    if (brightness > brightness_threshold || button_signal > button_threshold) {
        if (exponential_delay_try_enter(&exp_delay)) {
            // led_blink(LED_BUILTIN);
            send_press();
            exponential_delay_delay(&exp_delay);
        }
    } else {
        exponential_delay_reset(&exp_delay);
    }
    delay(200);
}
