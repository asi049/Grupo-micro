#include <DHT.h>

#define PIN_DHT 4
#define DHTTYPE DHT22

DHT dht(PIN_DHT, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
}

void loop() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  Serial.print("Temp: ");
  Serial.print(t);
  Serial.print("  Hum: ");
  Serial.println(h);

  delay(1000);
}
