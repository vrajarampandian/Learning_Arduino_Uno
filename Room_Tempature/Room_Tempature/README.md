**Room Temperature Monitor (Arduino + Qt C++)******

This project reads room temperature using a DHT11 sensor connected to an Arduino Uno, sends the data over Serial, and displays the live temperature inside a Qt C++ desktop application.

**Features**

Read temperature using DHT11 sensor

Sends data via Serial (USB) at 9600 baud

Qt app receives and displays temperature

Port selection, refresh, connect/disconnect

Automatically parses values like TEMP:25.3 or Temperature: 28.4 °C

Clean UI built using Qt Widgets

**Hardware Requirements**

Arduino Uno (3-pin or 4-pin DHT11 supported)

DHT11 temperature sensor

Breadboard + jumper wires

USB cable for Arduino

**Wiring Diagram (DHT11 → Arduino Uno)**
DHT11 (3-pin Module)
DHT11 Pin	Arduino Pin
S (Signal)	D2
+ (VCC)	5V
– (GND)	GND

Ensure that Arduino IDE Serial Monitor is closed, otherwise Qt cannot open the port.
