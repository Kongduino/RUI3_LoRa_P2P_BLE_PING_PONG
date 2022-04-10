long startTime;
// LoRa SETUP
// The LoRa chip come pre-wired: all you need to do is define the parameters:
// frequency, SF, BW, CR, Preamble Length and TX power
double myFreq = 868000000;
uint16_t counter = 0, sf = 12, bw = 125, cr = 0, preamble = 8, txPower = 22;

void hexDump(uint8_t* buf, uint16_t len) {
  // Something similar to the Unix/Linux hexdump -C command
  // Pretty-prints the contents of a buffer, 16 bytes a row
  char alphabet[17] = "0123456789abcdef";
  uint16_t i, index;
  Serial.print(F("   +------------------------------------------------+ +----------------+\n"));
  Serial.print(F("   |.0 .1 .2 .3 .4 .5 .6 .7 .8 .9 .a .b .c .d .e .f | |      ASCII     |\n"));
  for (i = 0; i < len; i += 16) {
    if (i % 128 == 0) Serial.print(F("   +------------------------------------------------+ +----------------+\n"));
    char s[] = "|                                                | |                |\n";
    uint8_t ix = 1, iy = 52, j;
    for (j = 0; j < 16; j++) {
      if (i + j < len) {
        uint8_t c = buf[i + j];
        s[ix++] = alphabet[(c >> 4) & 0x0F];
        s[ix++] = alphabet[c & 0x0F];
        ix++;
        if (c > 31 && c < 128) s[iy++] = c;
        else s[iy++] = '.';
      }
    }
    index = i / 16;
    if (i < 256) Serial.write(' ');
    Serial.print(index, HEX); Serial.write('.');
    Serial.print(s);
  }
  Serial.print(F("   +------------------------------------------------+ +----------------+\n"));
}

/*
  typedef struct rui_lora_p2p_revc {
    // Pointer to the received data stream
    uint8_t *Buffer;
    // Size of the received data stream
    uint8_t BufferSize;
    // Rssi of the received packet
    int16_t Rssi;
    // Snr of the received packet
    int8_t Snr;
  } rui_lora_p2p_recv_t;
*/
void recv_cb(rui_lora_p2p_recv_t data) {
  // RX callback
  if (data.BufferSize == 0) {
    // This should not happen. But, you know...
    // will not != should not
    // Serial.println("Empty buffer.");
    return;
  }
  char msg[92];
  sprintf(msg, "Incoming message, length: %d, RSSI: %d, SNR: %d\n", data.BufferSize, data.Rssi, data.Snr);
  Serial.print(msg);
  // Adafruit's Bluefruit connect is being difficult, and seems to require \r\n to fully receive a message..
  sprintf(msg, "[%d] RSSI %d SNR %d\r\n", data.BufferSize, data.Rssi, data.Snr);
#ifdef __RAKBLE_H__
  api.ble.uart.write((uint8_t*)msg, strlen(msg));
#endif
  hexDump(data.Buffer, data.BufferSize);
#ifdef __RAKBLE_H__
  Serial.println("Sending to BLE");
  sprintf(msg, "%s\r\n", (char*)data.Buffer);
  api.ble.uart.write((uint8_t*)msg, strlen(msg));
#endif
}

void send_cb(void) {
  // TX callback
  Serial.printf("Set infinite Rx mode %s\r\n", api.lorawan.precv(65534) ? "Success" : "Fail");
  // set the LoRa module to indefinite listening mode:
  // no timeout + no limit to the number of packets
  // NB: 65535 = wait for ONE packet, no timeout
}

void sendPing() {
  char payload[48];
  sprintf(payload, "PING #0x%04x", counter++);
  sendMsg(payload);
}


void sendMsg(char* msg) {
  uint8_t ln = strlen(msg);
  api.lorawan.precv(0);
  // turn off reception – a little hackish, but without that send might fail.
  char buff[ln + 20];
  memset(buff, 0, ln + 20);
  sprintf(buff, "Sending `%s`: %s\n\r", msg, api.lorawan.psend(ln, (uint8_t*)msg) ? "Success" : "Fail");
  Serial.print(buff);
  Serial.println("Sending to BLE...");
#ifdef __RAKBLE_H__
  ln = strlen(msg);
  api.ble.uart.write((uint8_t*)msg, ln);
#endif
}

void handleCommands(char *cmd) {
  if (cmd[0] != '/') return;
  // If the string doesn't start with / – it's not a command
  if (strcmp(cmd, "/ping") == 0) {
    sendPing();
    return;
  }

#ifdef __RAKBLE_H__
  if (strcmp(cmd, "/whoami") == 0) {
    char msg[64];
    sprintf(msg, "Broadcast name: %s\n\r", api.ble.settings.broadcastName.get());
    Serial.println(msg);
    Serial.println("Sending to BLE");
    uint16_t ln = strlen(msg);
    api.ble.uart.write((uint8_t*)msg, ln);
    return;
  }
#endif
}


void setup() {
  Serial.begin(115200, RAK_CUSTOM_MODE);
  // RAK_CUSTOM_MODE disables AT firmware parsing
  time_t timeout = millis();
  while (!Serial) {
    // on nRF52840, Serial is not available right away.
    // make the MCU wait a little
    if ((millis() - timeout) < 5000) {
      delay(100);
    } else {
      break;
    }
  }
  uint8_t x = 5;
  while (x > 0) {
    Serial.printf("%d, ", x--);
    delay(500);
  } // Just for show
  Serial.println("0!");
  Serial.println("RAKwireless LoRa P2P BLE Example");
  Serial.println("------------------------------------------------------");
  Wire.begin();
  //Wire.setClock(400000);

  Serial.println("P2P Start");
  char HardwareID[16]; // nrf52840
  strcpy(HardwareID, api.system.chipId.get().c_str());
  Serial.printf("Hardware ID: %s\r\n", HardwareID);
  if (strcmp(HardwareID, "nrf52840") == 0) {
    Serial.println("BLE compatible!");
  }
  Serial.printf("Model ID: %s\r\n", api.system.modelId.get().c_str());
  Serial.printf("RUI API Version: %s\r\n", api.system.apiVersion.get().c_str());
  Serial.printf("Firmware Version: %s\r\n", api.system.firmwareVersion.get().c_str());
  Serial.printf("AT Command Version: %s\r\n", api.system.cliVersion.get().c_str());

  // LoRa setup – everything else has been done for you. No need to fiddle with pins, etc
  Serial.printf("Set work mode to P2P: %s\r\n", api.lorawan.nwm.set(0) ? "Success" : "Fail");
  Serial.printf("Set P2P frequency to %3.3f: %s\r\n", (myFreq / 1e6), api.lorawan.pfreq.set(myFreq) ? "Success" : "Fail");
  Serial.printf("Set P2P spreading factor to %d: %s\r\n", sf, api.lorawan.psf.set(sf) ? "Success" : "Fail");
  Serial.printf("Set P2P bandwidth to %d: %s\r\n", bw, api.lorawan.pbw.set(bw) ? "Success" : "Fail");
  Serial.printf("Set P2P code rate to 4/%d: %s\r\n", (cr + 5), api.lorawan.pcr.set(0) ? "Success" : "Fail");
  Serial.printf("Set P2P preamble length to %d: %s\r\n", preamble, api.lorawan.ppl.set(8) ? "Success" : "Fail");
  Serial.printf("Set P2P TX power to %d: %s\r\n", txPower, api.lorawan.ptp.set(22) ? "Success" : "Fail");

  // LoRa callbacks
  api.lorawan.registerPRecvCallback(recv_cb);
  api.lorawan.registerPSendCallback(send_cb);

  // api.system.restoreDefault();
  // This causes various issues. Including a reboot. Let's stay away from that.

#ifdef __RAKBLE_H__
  Serial6.begin(115200, RAK_CUSTOM_MODE);
  // If you want to read and write data through BLE API operations,
  // you need to set BLE Serial (Serial6) to Custom Mode

  uint8_t pairing_pin[] = "004631";
  Serial.print("Setting pairing PIN to: ");
  Serial.println((char *)pairing_pin);
  api.ble.uart.setPIN(pairing_pin, 6); //pairing_pin = 6-digit (digit 0..9 only)
  // Set Permission to access BLE Uart is to require man-in-the-middle protection
  // This will cause apps to perform pairing with static PIN we set above
  // now support SET_ENC_WITH_MITM and SET_ENC_NO_MITM
  api.ble.uart.setPermission(RAK_SET_ENC_WITH_MITM);

  char ble_name[] = "3615_My_RAK"; // You have to be French to understand this joke
  Serial.print("Setting Broadcast Name to: ");
  Serial.println(ble_name);
  api.ble.settings.broadcastName.set(ble_name, strlen(ble_name));
  api.ble.uart.start();
  api.ble.advertise.start(0);
#endif
  // This version doesn't have an automatic Tx functionality:
  // YOU are in charge of sending, either via Serial or BLE.
  startTime = millis();
}

void loop() {
  if (millis() - startTime > 30000) {
    if (hasTH) {
      // Get sensor RAK1901 values
      th_sensor.update();
      temp = th_sensor.temperature();
      humid = th_sensor.humidity();
    }
    if (hasPA) {
      HPa = p_sensor.pressure(MILLIBAR);
    }
    startTime = millis();
  }
#ifdef __RAKBLE_H__
  if (api.ble.uart.available()) {
    // store the incoming string into a buffer
    Serial.println("\nBLE in:");
    char str1[256];
    uint8_t ix = 0;
    // with a 256-byte buffer and a uint8_t index
    // you won't get a buffer overrun :-)
    while (api.ble.uart.available()) {
      char c = api.ble.uart.read();
      if (c > 31) str1[ix++] = c;
      // strip \r\n and the like
      // this is ok because we expect text. You might want to adjust if you are sending binary data
    }
    str1[ix] = 0;
    Serial.println(str1);
    handleCommands(str1);
    // pass the string to the command-handling fn
  }
#endif
  if (Serial.available()) {
    Serial.println("\nIncoming:");
    char str1[256];
    uint8_t ix = 0;
    while (Serial.available()) {
      char c = Serial.read();
      if (c > 31) str1[ix++] = c;
    }
    str1[ix] = 0;
    Serial.println(str1);
    handleCommands(str1);
  }
}
