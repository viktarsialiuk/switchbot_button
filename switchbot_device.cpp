#include "switchbot_device.h"

#include "esp_bt_defs.h"
#include "BLEClient.h"
#include "BLEDevice.h"
#include "HardwareSerial.h"

void SwitchBotDevice_init(SwitchBotDevice *sb) {
    sb->client = BLEDevice::createClient();
    sb->service = NULL;
    sb->characteristic = NULL;
}

bool SwitchBotDevice_disconnect(SwitchBotDevice *sb) {
    if (sb->characteristic) {
        delete sb->characteristic;
        sb->characteristic = nullptr;
    }
    if (sb->service) {
        delete sb->service;
        sb->service = nullptr;
    }
    if (sb->client->isConnected()) {
        sb->client->disconnect();
    }
}

bool SwitchBotDevice_connect(SwitchBotDevice *sb, const BLEAddress address,
                             BLEUUID service_uuid, BLEUUID char_uuid) {
    if (sb->client->isConnected()) {
        Serial.println("SwitchBotDevice is already connected.");
        return false;
    }
    if (!sb->client->connect(address, BLE_ADDR_TYPE_RANDOM)) {
        Serial.println("SwitchBotDevice failed to connect.");
        goto error;
    }
    // Largest transmission unit.
    // sb->client->setMTU(517);
    sb->service = sb->client->getService(service_uuid);
    if (sb->service == nullptr) {
        Serial.println("SwitchBotDevice failed to get service.");
        goto error;
    }
    sb->characteristic = sb->service->getCharacteristic(char_uuid);
    if (sb->characteristic == nullptr) {
        Serial.println("SwitchBotDevice failed to get characteristic.");
        goto error;
    }
    return true;
error:
    SwitchBotDevice_disconnect(sb);
    return false;
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
    if (sb->characteristic) {
        delete sb->characteristic;
    }
    if (sb->service) {
        delete sb->service;
    }
    if (sb->client) {
        if (sb->client->isConnected()) {
            sb->client->disconnect();
        }
        delete sb->client;
    }
}