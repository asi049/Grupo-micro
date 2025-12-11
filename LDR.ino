#define LDR_PIN 34
#define UMBRAL_DIA_NOCHE 1000

void setup() {
  Serial.begin(115200);
  Serial.println("💡 Prueba Sensor LDR (Luz)");
  Serial.printf("📊 Umbral Día/Noche: %d\n", UMBRAL_DIA_NOCHE);
  Serial.println("💡 Menos luz = Noche, Más luz = Día");
}

String getModoDiaNoche(int valorLuz) {
  if (valorLuz < UMBRAL_DIA_NOCHE) {
    return "🌙 NOCHE";
  } else {
    return "☀️ DÍA";
  }
}

String getEstadoLuz(int valorLuz) {
  if (valorLuz < 100) return "🌑 OSCURIDAD TOTAL";
  if (valorLuz < 300) return "🌒 MUY OSCURO";
  if (valorLuz < UMBRAL_DIA_NOCHE) return "🌓 NOCHE";
  if (valorLuz < 2000) return "⛅ DÍA NUBLADO";
  if (valorLuz < 3000) return "☀️ DÍA SOLEADO";
  return "🔥 LUZ MUY INTENSA";
}

void loop() {
  int luz = analogRead(LDR_PIN);
  
  Serial.println("========================");
  Serial.printf("💡 Valor LDR: %d\n", luz);
  Serial.printf("🌗 Modo: %s\n", getModoDiaNoche(luz).c_str());
  Serial.printf("📊 Estado: %s\n", getEstadoLuz(luz).c_str());
  
  Serial.print("[");
  int barras = map(luz, 0, 4095, 0, 50);
  for(int i = 0; i < 50; i++) {
    if(i < barras) Serial.print("█");
    else Serial.print(" ");
  }
  Serial.println("]");
  
  delay(2000);
}
