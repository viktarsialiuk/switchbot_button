#include "switchbot_device.h"

#include <BLEAddress.h>
#include <BLEClient.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <HardwareSerial.h>
#include <esp_bt_defs.h>
#include <stdlib.h>
#include <string.h>

void SwitchBotDevice_init(SwitchBotDevice *sb) {
    BLEDevice::init("");
    sb->client = BLEDevice::createClient();
    sb->service = nullptr;
    sb->characteristic = nullptr;
}

bool SwitchBotDevice_connect(SwitchBotDevice *sb, const BLEAddress address,
                             BLEUUID service_uuid, BLEUUID char_uuid) {
    if (SwitchBotDevice_is_connected(sb)) {
        Serial.println("Warning: SwitchBotDevice is already connected.");
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

bool SwitchBotDevice_connect_str(SwitchBotDevice *sb, const char *address,
                                 BLEUUID service_uuid, BLEUUID char_uuid) {
    if (strlen(address) != 17) {
        Serial.printf("Bad BLE address string '%s'\r\n", address);
        return false;
    }

    // ESP_BD_ADDR_LEN == 6
    int data[6];
    sscanf(address, "%x:%x:%x:%x:%x:%x", &data[0], &data[1], &data[2], &data[3],
           &data[4], &data[5]);

    // typedef uint8_t esp_bd_addr_t[ESP_BD_ADDR_LEN];
    esp_bd_addr_t native;
    for (size_t index = 0; index < sizeof(native); index++) {
        native[index] = (uint8_t)data[index];
    }

    return SwitchBotDevice_connect(sb, BLEAddress(native), service_uuid,
                                   char_uuid);
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

bool SwitchBotDevice_is_connected(SwitchBotDevice *sb) {
    return sb->characteristic != nullptr;
}

bool SwitchBotDevice_send(SwitchBotDevice *sb, uint8_t *command,
                          size_t bytes_size) {
    if (sb->characteristic == nullptr) {
        // Not connected;
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

void SwitchBotDevice_scan(BLEUUID service_uuid) {
    // Init global BLEScan.
    BLEScan *ble_scan = BLEDevice::getScan();
    ble_scan->setInterval(1349);
    ble_scan->setWindow(449);
    ble_scan->setActiveScan(true);

    BLEScanResults results = ble_scan->start(5, false);
    for (uint32_t i = 0; i < results.getCount(); ++i) {
        BLEAdvertisedDevice device = results.getDevice(i);
        if (device.haveServiceUUID() &&
            device.isAdvertisingService(service_uuid)) {
            Serial.printf("Found Switchbot! %s\r\n", device.toString().c_str());
        }
    }
}