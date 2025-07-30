#include "Arduino.h"

#include "exponential_delay.h"
#include "switchbot_client.h"

static ExponentialDelay exp_delay;
static SwitchBotClient switchbot_client;

void setup() {
  Serial.begin(9600);
  Serial.println("Starting Arduino BLE Client application...");

  // The remote service we wish to connect to.
  static BLEUUID serviceUUID("cba20d00-224d-11e6-9fb8-0002a5d5c51b");
  // The characteristic of the remote service we are interested in.
  static BLEUUID charUUID("cba20002-224d-11e6-9fb8-0002a5d5c51b");
  //static BLEUUID notifyUUID( "cba20003-224d-11e6-9fb8-0002a5d5c51b" );
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
    ExponentialDelay_reset(&exp_delay);
    delay(1000);
  }
}
 