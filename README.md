# ESP8266 ArducamMega capture and stream
The example sketch by Arducam for the Arducam Mega for esp32 translated for esp8266.

With this example you should be able to capture and directly send to a webserved page an image or video stream (from the esp8266).

This is the translated example sketch, have fun trying to understand how to make it work with the esp8266.
If you find how to make it work seamlessly don't hesitate to update it or add branches.

Remember in fact that this is an esp8266 running a camera that wasn't rated for it.
Also remember to connect pins correctly (find your cs pin)

For the d1 mini pro board i have, i've used d0 as cs pin (16 in Arduino IDE), and d5 (14) for SCK, d6 (12) for MISO, d7 (13) for MOSI.
