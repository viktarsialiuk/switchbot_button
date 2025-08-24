#ifndef SWITCHBOT_DEVICE_H_
#define SWITCHBOT_DEVICE_H_

#include <BLEClient.h>
#include <BLERemoteCharacteristic.h>
#include <BLERemoteService.h>
#include <esp_bt_defs.h>

struct SwitchBotDevice {
    BLEClient *client;
    BLERemoteService *service;
    BLERemoteCharacteristic *characteristic;
};

void switch_bot_init(SwitchBotDevice *sb);
bool switch_bot_connect(SwitchBotDevice *sb, const char *address,
                        BLEUUID service_uuid, BLEUUID char_uuid);
bool switch_bot_connect_native(SwitchBotDevice *sb, esp_bd_addr_t native,
                               BLEUUID service_uuid, BLEUUID char_uuid);
void switch_bot_disconnect(SwitchBotDevice *sb);
bool switch_bot_is_connected(SwitchBotDevice *sb);
bool switch_bot_send(SwitchBotDevice *sb, uint8_t *command, size_t bytes_size);
bool switch_bot_read(SwitchBotDevice *sb);
void switch_bot_free(SwitchBotDevice *sb);
void switch_bot_scan(BLEUUID service_uuid);

#endif  // SWITCHBOT_DEVICE_H_