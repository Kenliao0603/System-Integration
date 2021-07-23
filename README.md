This project includes an Arduino UNO board that controls temperature sensor, current sensor, and a 3.5" smart display from WINSTAR. The system is operated and centered around the Cayenne MQTT IoT software via ESP32 Wifi Module. ESP32 Wifi module communicates between Arduino board and Cayenne software by receiving data from the mother board and send it to the Cayenne MQTT server. Temperature and current data can be displayed on both the 3.5" TFT panel and the Cayenne web page. Furthermore, the button on the 3.5" panel can control the relay that controls an external fan through the MQTT server. When the fan is turned on, the current sensor will received values in ampere and send them back to the display and the Cayenne web page.


system diagram            |  system set up
:-------------------------:|:-------------------------:
<img src="https://user-images.githubusercontent.com/87794246/126756561-20a1764d-cacc-4e29-9ee8-199190ba9d8f.jpg" width="700" height="350"> | <img src="https://user-images.githubusercontent.com/87794246/126756206-604ea1bc-f74f-4793-85de-a8772a206db2.jpg" width="650" height="350">


Cayenne MQTT web page     |  WINSTAR 3.5" smart display
:-------------------------:|:-------------------------:
<img src="https://user-images.githubusercontent.com/87794246/126757446-d6d5293c-1129-48fa-bdb6-4bf999ecf36a.png" width="700" height="350"> | <img src="https://user-images.githubusercontent.com/87794246/126757467-498384a4-11e0-4a61-8131-2639d81869c1.jpg" width="650" height="350">


[Youtube video demonstration link here](https://youtu.be/Wl9w_AdznLo "video")

