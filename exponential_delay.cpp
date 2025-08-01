#include "exponential_delay.h"

#include "Arduino.h"
#include "HardwareSerial.h"

void ExponentialDelay_init(ExponentialDelay* exp_delay, uint32_t start_delay,
                           uint32_t max_delay, uint32_t exponent) {
    exp_delay->start_delay = start_delay;
    exp_delay->max_delay = max_delay;
    exp_delay->exponent = exponent;
    exp_delay->current_delay = start_delay;
    exp_delay->delay_started_millis = 0;
}

void ExponentialDelay_reset(ExponentialDelay* exp_delay) {
    exp_delay->current_delay = exp_delay->start_delay;
    exp_delay->delay_started_millis = 0;
}

bool ExponentialDelay_is_active(const ExponentialDelay* exp_delay) {
    return exp_delay->delay_started_millis > 0;
}

void ExponentialDelay_delay(ExponentialDelay* exp_delay) {
    exp_delay->current_delay *= exp_delay->exponent;
    if (exp_delay->current_delay > exp_delay->max_delay) {
        exp_delay->current_delay = exp_delay->max_delay;
    }
    Serial.printf("Starting delay for %d seconds\r\n",
                  exp_delay->current_delay / 1000);
    exp_delay->delay_started_millis = millis();
}

bool ExponentialDelay_try_enter(const ExponentialDelay* exp_delay) {
    const uint32_t time_ellapsed = millis() - exp_delay->delay_started_millis;
    return !ExponentialDelay_is_active(exp_delay) ||
           time_ellapsed >= exp_delay->current_delay;
}