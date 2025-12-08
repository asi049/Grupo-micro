#include <DHT.h>

#define DHTPIN 33
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  Serial.println("📊 Prueba Sensor DHT22");
  dht.begin();
  delay(2000); 
}

void loop() {
  delay(2000); // Espera 2 segundos entre lecturas
  
  float temperatura = dht.readTemperature();
  float humedad = dht.readHumidity();
  
  if (isnan(temperatura) || isnan(humedad)) {
    Serial.println("❌ Error al leer DHT22!");
    return;
  }
  
  Serial.println("========================");
  Serial.printf("🌡️ Temperatura: %.1f °C\n", temperatura);
  Serial.printf("💧 Humedad: %.1f %%\n", humedad);
  
  // Interpretación de humedad
  if (humedad < 30) Serial.println("⚠️  Humedad: Muy baja");
  else if (humedad < 60) Serial.println("✅ Humedad: Normal");
  else Serial.println("⚠️  Humedad: Alta");
}
