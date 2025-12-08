#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_SDA 25
#define OLED_SCL 26

Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
  Serial.begin(115200);
  Serial.println("🖥️ Prueba Pantalla OLED");
  
  Wire.begin(OLED_SDA, OLED_SCL);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("❌ Error al inicializar OLED");
    while(1);
  }
  
  Serial.println("✅ OLED inicializada correctamente");
  
  // Limpiar pantalla
  display.clearDisplay();
  
  // Pantalla de inicio
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  display.println("OLED OK!");
  
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println("Resolucion: 128x64");
  display.println("Direccion: 0x3C");
  display.println("SDA: GPIO25");
  display.println("SCL: GPIO26");
  
  display.display();
  delay(3000);
}

void loop() {

  display.clearDisplay();
  
  // Título
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("Demo OLED");
  
  // Texto normal
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println("Texto tamano 1");
  
  display.setTextSize(2);
  display.setCursor(0, 35);
  display.println("Tamano 2");
  
  display.setTextSize(3);
  display.setCursor(0, 55);
  display.println("123");
  
  display.display();
  delay(3000);
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Numeros:");
  
  display.setTextSize(2);
  display.setCursor(0, 15);
  display.printf("Temp: 23.5C\n");
  display.setCursor(0, 35);
  display.printf("Hum: 65%%\n");
  display.setCursor(0, 55);
  display.printf("Luz: 2456");
  
  display.display();
  delay(3000);
  
  display.clearDisplay();
  
  // Líneas
  display.drawLine(0, 0, 127, 63, SSD1306_WHITE);
  display.drawLine(127, 0, 0, 63, SSD1306_WHITE);
  
  // Rectángulo
  display.drawRect(10, 10, 108, 44, SSD1306_WHITE);
  display.fillRect(15, 15, 98, 34, SSD1306_WHITE);
  
  // Texto invertido
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  display.setCursor(20, 22);
  display.setTextSize(1);
  display.println("TEXTO INVERTIDO");
  
  display.display();
  delay(3000);
}