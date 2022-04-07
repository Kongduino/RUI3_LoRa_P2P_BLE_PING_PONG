# RUI3_LoRa_P2P_BLE_PING_PONG

A BLE-enabled PING-PONG LoRa P2P sketch for RUI3 / RAK4631 and RAK3172. But but but, you say, the RAK3172 doesn't have BLE?!? Well, then, disable it, mah dude!

![BLE](BLE.png)


It accepts four commands, either via Serial or BLE:

* `/whomai`: get the BLE broadcast name. Useful when you have a few devices. You entre this command on Serial, and get the right name.
* `/ping`: self-explanatory I believe...
* `/> xxxxx`: send a custom message. Notice the space between `/>` and `xxxxxx`.
* `/th`: send the temperature and humidity if you have a [RAK1901](https://store.rakwireless.com/products/rak1901-shtc3-temperature-humidity-sensor) connected. Yes, the sketch does it on its own.

![Serial_Screenshot](Serial_Screenshot.png)
![BLE_Screenshot](BLE_Screenshot.jpg)
