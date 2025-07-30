#include "exponential_delay.h"

#include "Arduino.h"
#include "HardwareSerial.h"

void ExponentialDelay_init(ExponentialDelay* exp_delay, uint32_t start_delay,
                           uint32_t max_delay, uint32_t exponent) {
    exp_delay->start_delay = start_delay;
    exp_delay->current_delay = start_delay;
    exp_delay->max_delay = max_delay;
    exp_delay->exponent = exponent;
}

void ExponentialDelay_reset(ExponentialDelay* exp_delay) {
    exp_delay->current_delay = exp_delay->start_delay;
}

void ExponentialDelay_wait(ExponentialDelay* exp_delay) {
    Serial.printf("Delay: Waiting for %d seconds\r\n",
                  exp_delay->current_delay / 1000);
    delay(exp_delay->current_delay);
    exp_delay->current_delay *= exp_delay->exponent;
    if (exp_delay->current_delay > exp_delay->max_delay) {
        exp_delay->current_delay = exp_delay->max_delay;
    }
}