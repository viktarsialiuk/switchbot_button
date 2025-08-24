#ifndef MOVING_AVERAGE_H_
#define MOVING_AVERAGE_H_

#include <stdint.h>

struct MovingAverage {
    uint16_t values[3];
    uint16_t index;
};

void moving_average_init(MovingAverage* ma);
void moving_average_add_value(MovingAverage* ma, uint16_t value);
uint16_t moving_average_get_average(const MovingAverage* ma);

#endif  // MOVING_AVERAGE_H_