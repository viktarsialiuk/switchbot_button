#include "moving_average.h"

#include <string.h>

void moving_average_init(MovingAverage* ma) {
    ma->index = 0;
    memset(&ma->values[0], 0, sizeof(ma->values));
}

void moving_average_add_value(MovingAverage* ma, uint16_t value) {
    const uint16_t array_size = sizeof(ma->values) / sizeof(ma->values[0]);
    ma->values[ma->index] = value;
    ma->index = (ma->index + 1) % array_size;
}

uint16_t moving_average_get_average(const MovingAverage* ma) {
    const uint16_t array_size = sizeof(ma->values) / sizeof(ma->values[0]);
    uint16_t average = 0;
    for (uint16_t i = 0; i < array_size; ++i) {
        average += ma->values[i];
    }
    average /= array_size;
    return average;
}