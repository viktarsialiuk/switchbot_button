#ifndef SWITCHBOT_CLIENT_H_
#define SWITCHBOT_CLIENT_H_

#include "Arduino.h"
#include "BLEDevice.h"
#include "switchbot_device.h"

#define MAX_SWITCHBOT_CLIENTS 10
struct SwitchBotClient {
    SwitchBotDevice devices[MAX_SWITCHBOT_CLIENTS];
    uint32_t num_clients;
    BLEUUID service_uuid;
    BLEUUID char_uuid;
    uint32_t last_alive;
    uint32_t max_keep_alive;
};

void SwitchBotClient_init(SwitchBotClient* client, BLEUUID service_uuid,
                          BLEUUID char_uuid, uint32_t max_keep_alive = 10000);
void SwitchBotClient_free(SwitchBotClient* client);
bool SwitchBotClient_connected(SwitchBotClient* client);
bool SwitchBotClient_connect(SwitchBotClient* client, BLEAddress address);
void SwitchBotClient_disconnect(SwitchBotClient* client);
void SwitchBotClient_scan(SwitchBotClient* client);
bool SwitchBotClient_send(SwitchBotClient* client, uint8_t* command,
                          uint32_t byte_size);
void SwitchBotClient_smart_send(SwitchBotClient* client, uint8_t* command,
                                uint32_t byte_size);

#endif  // SWITCHBOT_CLIENT_H_