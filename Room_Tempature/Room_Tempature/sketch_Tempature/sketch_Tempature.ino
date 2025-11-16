#include <DHT.h>

#define DHTPIN 2        // DATA pin connected to D2
#define DHTTYPE DHT11   // Sensor type

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();
}

void loop() {
  float temp = dht.readTemperature();  // Celsius

  if (isnan(temp)) {
    Serial.println("Error reading from DHT11");
  } else {
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println(" °C");
  }

  delay(2000);  // DHT11 needs at least 2 sec delay
}
