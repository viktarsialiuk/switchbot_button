#ifndef EXPONENTIAL_DELAY_H_
#define EXPONENTIAL_DELAY_H_

#include "Arduino.h"

struct ExponentialDelay {
    uint32_t start_delay;
    uint32_t max_delay;
    uint32_t exponent;
    uint32_t current_delay;
    uint32_t delay_started_millis;
};

void ExponentialDelay_init(ExponentialDelay* exp_delay,
                           uint32_t start_delay = 5000,
                           uint32_t max_delay = 18000000,
                           uint32_t exponent = 2);
void ExponentialDelay_reset(ExponentialDelay* exp_delay);
bool ExponentialDelay_is_active(const ExponentialDelay* exp_delay);
void ExponentialDelay_start(ExponentialDelay* exp_delay);
bool ExponentialDelay_try_enter(const ExponentialDelay* exp_delay);

#endif  // EXPONENTIAL_DELAY_H_