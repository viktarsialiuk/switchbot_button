#ifndef SWITCHBOT_DEVICE_H_
#define SWITCHBOT_DEVICE_H_

#include "BLEClient.h"
#include "BLERemoteCharacteristic.h"
#include "BLERemoteService.h"

struct SwitchBotDevice {
    BLEClient *client;
    BLERemoteService *service;
    BLERemoteCharacteristic *characteristic;
};

void SwitchBotDevice_init(SwitchBotDevice *sb);
void SwitchBotDevice_disconnect(SwitchBotDevice *sb);
bool SwitchBotDevice_connect(SwitchBotDevice *sb, const BLEAddress address,
                             BLEUUID service_uuid, BLEUUID char_uuid);
bool SwitchBotDevice_send(SwitchBotDevice *sb, uint8_t *command,
                          size_t bytes_size);
void SwitchBotDevice_free(SwitchBotDevice *sb);

#endif  // SWITCHBOT_DEVICE_H_