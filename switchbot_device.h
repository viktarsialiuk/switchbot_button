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
bool SwitchBotDevice_connect(SwitchBotDevice *sb, const BLEAddress address,
                             BLEUUID service_uuid, BLEUUID char_uuid);
bool SwitchBotDevice_connect_str(SwitchBotDevice *sb, const char* address,
                                 BLEUUID service_uuid, BLEUUID char_uuid);
void SwitchBotDevice_disconnect(SwitchBotDevice *sb);
bool SwitchBotDevice_is_connected(SwitchBotDevice *sb);
bool SwitchBotDevice_send(SwitchBotDevice *sb, uint8_t *command,
                          size_t bytes_size);
void SwitchBotDevice_free(SwitchBotDevice *sb);
void SwitchBotDevice_scan(BLEUUID service_uuid);

#endif  // SWITCHBOT_DEVICE_H_