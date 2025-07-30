#include "switchbot_client.h"

#include "BLEScan.h"
#include "HardwareSerial.h"
#include "switchbot_device.h"

void SwitchBotClient_init(SwitchBotClient* client, BLEUUID service_uuid,
                          BLEUUID char_uuid, uint32_t max_keep_alive) {
    BLEDevice::init("");
    // Init global BLEScan.
    BLEScan* ble_scan = BLEDevice::getScan();
    ble_scan->setInterval(1349);
    ble_scan->setWindow(449);
    ble_scan->setActiveScan(true);

    // Init devices. This is only memory allocation.
    for (uint32_t i = 0; i < MAX_SWITCHBOT_CLIENTS; ++i) {
        SwitchBotDevice_init(&client->devices[i]);
    }
    client->num_clients = 0;
    client->service_uuid = service_uuid;
    client->char_uuid = char_uuid;
    client->last_alive = 0;
    client->max_keep_alive = max_keep_alive;
}

void SwitchBotClient_free(SwitchBotClient* client) {
    for (uint32_t i = 0; i < MAX_SWITCHBOT_CLIENTS; ++i) {
        SwitchBotDevice_free(&client->devices[i]);
    }
}

bool SwitchBotClient_connected(SwitchBotClient* client) {
    return client->num_clients > 0;
}

bool SwitchBotClient_connect(SwitchBotClient* client, BLEAddress address) {
    if (client->num_clients >= MAX_SWITCHBOT_CLIENTS) {
        Serial.println("Max amount of clients reached.");
        return false;
    }
    if (!SwitchBotDevice_connect(&client->devices[client->num_clients], address,
                                 client->service_uuid, client->char_uuid)) {
        Serial.println("Failed to connect to Switchbot!");
        return false;
    }
    Serial.println("Connected to Switchbot!");
    ++(client->num_clients);
    client->last_alive = millis();
    return true;
}

void SwitchBotClient_disconnect(SwitchBotClient* client) {
    for (uint32_t i = 0; i < client->num_clients; ++i) {
        SwitchBotDevice_disconnect(&client->devices[i]);
    }

    client->num_clients = 0;
    client->last_alive = 0;
}

void SwitchBotClient_scan(SwitchBotClient* client) {
    // Disconnect if connected.
    SwitchBotClient_disconnect(client);

    // Scan BLE devices.
    BLEScanResults results = BLEDevice::getScan()->start(5, false);
    for (uint32_t i = 0; i < results.getCount(); ++i) {
        BLEAdvertisedDevice device = results.getDevice(i);
        if (device.haveServiceUUID() &&
            device.isAdvertisingService(client->service_uuid)) {
            Serial.printf("Found Switchbot! %s\r\n", device.toString().c_str());
            SwitchBotClient_connect(client, device.getAddress());
        }
    }
    BLEDevice::getScan()->stop();  // Not sure if needed.
}

bool SwitchBotClient_send(SwitchBotClient* client, uint8_t* command,
                          uint32_t byte_size) {
    bool all_succeeded = client->num_clients > 0;
    for (uint32_t i = 0; i < client->num_clients; ++i) {
        if (!SwitchBotDevice_send(&client->devices[i], command, byte_size)) {
            all_succeeded = false;
        }
    }
    if (all_succeeded) {
        client->last_alive = millis();
    } else {
        client->last_alive = 0;
    }
    return all_succeeded;
}

void SwitchBotClient_smart_send(SwitchBotClient* client, uint8_t* command,
                                uint32_t byte_size) {
    uint32_t time_since_last_alive = millis() - client->last_alive;
    Serial.printf(
        "SwitchBotClient smart send: last alive %u, time elapsed %u.\r\n",
        client->last_alive, time_since_last_alive);
    if (!SwitchBotClient_connected(client) ||
        time_since_last_alive >= client->max_keep_alive) {
        Serial.println("SwitchBotClient rescanning");
        SwitchBotClient_scan(client);
    }
    SwitchBotClient_send(client, command, byte_size);
}