*****************   WebLights v1.08 : 23.12.2017   ***********************

This project is a web-controlled text script interpreter for interacting with WS2811 chip in LED garlands. Script interpreter allows set an individual color for each led in garland. It supports up to 256 led with WS2811 \ WS2812 chip in strip. The interpreter can also play BMP files to produce interesting visual effects.

If you can't store files througth WEB interface, press and hold button at power-up. File system would be formatted.

Changes history:

v1.08 - 23.12.2017
Brightness control for BMP added. 


v1.07 - 21.12.2017
Brightness control added. Led RGB order fixed. Long press for reset fixed. Trace output added. IR command output added.


v1.06 - 15.12.2017

!!!Button moved to D2 PIN !!! Longpress at restart for format is working.
Bug in test scripts fixed. 
Cyclic restart when no script file found fixed.
LED RGB order fixed. 
IRemoteESP8266 2.3.0 added.


v1.05 - 13.12.2017

Some bug fixed. Script constructor call fixed.


v1.04 - 10.04.2017

LED RGB order mapping added. LED Order switch added to web page.


v1.03 - 14.03.2017

Built-in LED and IRDA were assigned to same pin. IRDA receiver moved to pin D5.
main.h and WebLights-wiring.png changed


v1.02 - 05.03.2017

Corrected bug in "BMP One" mode select file for play.
minor changes.
sample3.MP4 video added. It shows, how sample3.bmp file looks like when played by controller.

v1.01 - 03.03.2017

First public release


