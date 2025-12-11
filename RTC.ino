#include <Wire.h>
#include <RTClib.h>

// Pines 
#define RTC_SDA 27
#define RTC_SCL 14

RTC_DS1307 rtc;

void setup() {
  Serial.begin(115200);
  Serial.println("🕐 Prueba RTC DS1307");
  
  Wire.begin(RTC_SDA, RTC_SCL);
  
  if (!rtc.begin()) {
    Serial.println("❌ No se encontró el RTC");
    while (1);
  }
  
  if (!rtc.isrunning()) {
    Serial.println("⚠️ RTC NO está funcionando!");
    // Ajustar a fecha/hora de compilación
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.println("✅ RTC ajustado a fecha/hora de compilación");
  } else {
    Serial.println("✅ RTC funcionando correctamente");
  }
}

void loop() {
  DateTime now = rtc.now();
  
  Serial.println("========================");
  Serial.printf("📅 Fecha: %02d/%02d/%04d\n", 
                now.day(), now.month(), now.year());
  Serial.printf("⏰ Hora: %02d:%02d:%02d\n", 
                now.hour(), now.minute(), now.second());
  
  // Día de la semana
  const char* dias[] = {"Domingo", "Lunes", "Martes", "Miércoles", 
                       "Jueves", "Viernes", "Sábado"};
  Serial.printf("📅 Día de semana: %s\n", dias[now.dayOfTheWeek()]);
  
  Serial.printf("⏱️  Unix: %lu\n", now.unixtime());
  
  delay(5000); // Actualizar cada 5 segundos
}