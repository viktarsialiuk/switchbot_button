/**
 * A BLE client example that is rich in capabilities.
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara
 */

#include <cstddef>

#include "esp_bt_defs.h"
#include "BLEDevice.h"
#include "BLEScan.h"

struct SwitchBotDevice {
  BLEClient* client;
  BLERemoteService* service;
  BLERemoteCharacteristic* characteristic;
};

#define MAX_SWITCHBOT_CLIENTS 10
struct SwitchBotClient {
   SwitchBotDevice devices[MAX_SWITCHBOT_CLIENTS];
   uint32_t num_clients;
   BLEUUID service_uuid;
   BLEUUID char_uuid;
   uint32_t last_alive;
   uint32_t max_keep_alive;
};

struct ExponentialDelay {
  uint32_t start_delay;
  uint32_t current_delay;
  uint32_t max_delay;
  uint32_t exponent;
};

void SwitchBotDevice_init(SwitchBotDevice* sb) {
  sb->client = BLEDevice::createClient();
  sb->service = NULL;
  sb->characteristic = NULL;
}

bool SwitchBotDevice_disconnect(SwitchBotDevice* sb) {
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

bool SwitchBotDevice_connect(SwitchBotDevice* sb, const BLEAddress address, BLEUUID service_uuid, BLEUUID char_uuid) {
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

bool SwitchBotDevice_connected(SwitchBotDevice* sb) {
  return sb->client->isConnected();
}

bool SwitchBotDevice_send(SwitchBotDevice* sb, uint8_t* command, size_t bytes_size) {
  if (!SwitchBotDevice_connected(sb)) {
    return false;
  }
  //uint8_t pressCommand[] = {0x57, 0x01, 0x00};
  sb->characteristic->writeValue(command, bytes_size, false);
  return true;
}

void SwitchBotDevice_free(SwitchBotDevice* sb) {
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

void SwitchBotClient_init(SwitchBotClient* client, BLEUUID service_uuid, BLEUUID char_uuid, uint32_t max_keep_alive = 10000) {
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
    if (device.haveServiceUUID() && device.isAdvertisingService(client->service_uuid)) {
      Serial.printf("Found Switchbot! %s\r\n", device.toString().c_str());
      SwitchBotClient_connect(client, device.getAddress());
    }
  }
  BLEDevice::getScan()->stop();  // Not sure if needed.
}

bool SwitchBotClient_send(SwitchBotClient* client, uint8_t* command, uint32_t byte_size) {
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

void SwitchBotClient_smart_send(SwitchBotClient* client, uint8_t* command, uint32_t byte_size) {
  uint32_t time_since_last_alive = millis() - client->last_alive;
  Serial.printf("SwitchBotClient smart send: last alive %u, time elapsed %u.\r\n",
                client->last_alive, time_since_last_alive);
  if (!SwitchBotClient_connected(client) ||
      time_since_last_alive >= client->max_keep_alive) {
    Serial.println("SwitchBotClient rescanning");
    SwitchBotClient_scan(client);
  }
  SwitchBotClient_send(client, command, byte_size);
}

void ExponentialDelay_init(ExponentialDelay* exp_delay, uint32_t start_delay = 10000, uint32_t max_delay = 18000000, uint32_t exponent = 2) {
  exp_delay->start_delay = start_delay;
  exp_delay->current_delay = start_delay;
  exp_delay->max_delay = max_delay;
  exp_delay->exponent = exponent;
}

void ExponentialDelay_reset(ExponentialDelay* exp_delay) {
  exp_delay->current_delay = exp_delay->start_delay;
}

void ExponentialDelay_wait(ExponentialDelay* exp_delay) {
  Serial.printf("Delay: Waiting for %d seconds\r\n", exp_delay->current_delay / 1000);
  delay(exp_delay->current_delay);
  exp_delay->current_delay *= exp_delay->exponent;
  if (exp_delay->current_delay > exp_delay->max_delay) {
    exp_delay->current_delay = exp_delay->max_delay;
  }
}

static ExponentialDelay exp_delay;
static SwitchBotClient switchbot_client;

void setup() {
  Serial.begin(9600);
  Serial.println("Starting Arduino BLE Client application...");

  // The remote service we wish to connect to.
  static BLEUUID serviceUUID("cba20d00-224d-11e6-9fb8-0002a5d5c51b");
  // The characteristic of the remote service we are interested in.
  static BLEUUID charUUID("cba20002-224d-11e6-9fb8-0002a5d5c51b");
  static BLEUUID notifyUUID( "cba20003-224d-11e6-9fb8-0002a5d5c51b" );
  SwitchBotClient_init(&switchbot_client, serviceUUID, charUUID, /*max_keep_alive=*/20000);

  ExponentialDelay_init(&exp_delay);
}

// This is the Arduino main loop function.
void loop() {
  int brightness = analogRead(A1);
  Serial.printf("Brightness sensor %d\r\n", brightness);
  if (brightness > 450) {
    uint8_t pressCommand[] = {0x57, 0x01, 0x00};
    SwitchBotClient_smart_send(&switchbot_client, &pressCommand[0], sizeof(pressCommand));
    ExponentialDelay_wait(&exp_delay);
  } else {
    delay(2000);
  }
}
 