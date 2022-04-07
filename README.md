# RUI3_LoRa_P2P_BLE_PING_PONG

A BLE-enabled PING-PONG LoRa P2P sketch for RUI3 / RAK4631.
It accepts three commands, either via Serial or BLE:

* `/whomai`: get the BLE broadcast name. Useful when you have a few devices. You entre this command on Serial, and get the right name.
* `/ping`: self-explanatory I believe...
* `/> xxxxx`: send a custom message. Notice the space between `/>` and `xxxxxx`.

![Serial_Screenshot](Serial_Screenshot.png)
![BLE_Screenshot](BLE_Screenshot.jpg)
