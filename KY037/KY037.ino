#define SOUND_SENSOR_PIN 35
#define LED_PIN 2  // LED interno

int silencioBase = 500;
int maxSonido = 0;
int minSonido = 4095;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("🎤 Prueba Sensor KY-037 (Sonido)");
  
  // Calibrar silencio 
  Serial.println("🔊 Calibrando... (mantén silencio)");
  delay(2000);
  
  long suma = 0;
  for(int i = 0; i < 10; i++) {
    suma += analogRead(SOUND_SENSOR_PIN);
    delay(50);
  }
  silencioBase = suma / 10;
  
  Serial.printf("✅ Silencio base: %d\n", silencioBase);
  Serial.println("🎤 Comienza a hacer ruido...");
}

void loop() {
  int lecturaRaw = analogRead(SOUND_SENSOR_PIN);
  int diferencia = abs(lecturaRaw - silencioBase);
  
  // Actualizar máximos y mínimos
  if (diferencia > maxSonido) maxSonido = diferencia;
  if (diferencia < minSonido) minSonido = diferencia;
  
  // Calcular nivel de sonido (0-100)
  int nivelSonido = map(diferencia, 0, 800, 0, 100);
  nivelSonido = constrain(nivelSonido, 0, 100);
  
  // Detectar sonido 
  bool sonidoDetectado = diferencia > 100;
  
  // Encender LED si hay sonido
  digitalWrite(LED_PIN, sonidoDetectado ? HIGH : LOW);
  
  // Mostrar en Serial
  Serial.printf("🔊 Raw: %4d | Dif: %4d | Nivel: %3d%% | ", 
                lecturaRaw, diferencia, nivelSonido);
  
  if (sonidoDetectado) {
    Serial.println("SONIDO DETECTADO 🔊");
  } else {
    Serial.println("SILENCIO 🔇");
  }
  
  // Mostrar estadísticas cada 10 ciclos
  static int contador = 0;
  if (++contador >= 10) {
    contador = 0;
    Serial.printf("📊 Estadísticas: Max: %d | Min: %d\n", maxSonido, minSonido);
  }
  
  delay(100); 
}