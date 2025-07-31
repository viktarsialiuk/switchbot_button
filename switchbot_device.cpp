#include "switchbot_device.h"

#include <cstdlib>

#include "BLEClient.h"
#include "BLEDevice.h"
#include "HardwareSerial.h"
#include "esp_bt_defs.h"

void SwitchBotDevice_init(SwitchBotDevice *sb) {
    sb->client = BLEDevice::createClient();
    sb->service = nullptr;
    sb->characteristic = nullptr;
}

bool SwitchBotDevice_connect(SwitchBotDevice *sb, const BLEAddress address,
                             BLEUUID service_uuid, BLEUUID char_uuid) {
    if (sb->client->isConnected()) {
        Serial.println("SwitchBotDevice is already connected.");
    }
    if (!sb->client->connect(address, BLE_ADDR_TYPE_RANDOM)) {
        Serial.println("SwitchBotDevice failed to connect.");
        return false;
    }
    // Largest transmission unit.
    // sb->client->setMTU(517);
    sb->service = sb->client->getService(service_uuid);
    if (sb->service == nullptr) {
        Serial.println("SwitchBotDevice failed to get service.");
        SwitchBotDevice_disconnect(sb);
        return false;
    }
    sb->characteristic = sb->service->getCharacteristic(char_uuid);
    if (sb->characteristic == nullptr) {
        Serial.println("SwitchBotDevice failed to get characteristic.");
        SwitchBotDevice_disconnect(sb);
        return false;
    }
    return true;
}

void SwitchBotDevice_disconnect(SwitchBotDevice *sb) {
    // BLECharacteristic is owned by BLERemotService which are in turn owned by
    // BLEClient, all we need is to clean up the client.
    sb->characteristic = nullptr;
    sb->service = nullptr;
    // Disconnect client.
    if (sb->client->isConnected()) {
        // Somehow this makes arduino to fail.
        // Maybe we should delete and recreate the client.
        sb->client->disconnect();
        Serial.println("SwitchBotDevice disconnected");
    }
}

bool SwitchBotDevice_connected(SwitchBotDevice *sb) {
    return sb->client->isConnected();
}

bool SwitchBotDevice_send(SwitchBotDevice *sb, uint8_t *command,
                          size_t bytes_size) {
    if (!SwitchBotDevice_connected(sb)) {
        return false;
    }
    // uint8_t pressCommand[] = {0x57, 0x01, 0x00};
    sb->characteristic->writeValue(command, bytes_size, false);
    return true;
}

void SwitchBotDevice_free(SwitchBotDevice *sb) {
    // BLECharacteristic is owned by BLERemotService which are in turn owned by
    // BLEClient, all we need is to delete client.
    if (sb->client) {
        if (sb->client->isConnected()) {
            sb->client->disconnect();
        }
        delete sb->client;
        sb->client = nullptr;
    }
}