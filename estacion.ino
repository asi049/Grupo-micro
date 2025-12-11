#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>

// ================= CONFIGURACIÓN WIFI =================
const char* ssid = "16FMAGT";
const char* password = "miga1206";

// ================= DEFINICIÓN DE PINES =================
#define DHTPIN 33
#define DHTTYPE DHT22
#define LDR_PIN 34
#define SOUND_SENSOR_PIN 35

// ================= CONFIGURACIÓN CORREGIDA =================
#define UMBRAL_DIA_NOCHE 1000  // Lógica Invertida: < 1000 = DÍA, > 1000 = NOCHE
#define UMBRAL_SONIDO_ALTO 600 
#define MAX_LECTURA_SONIDO 800 

// Pines I2C (OJO: Verifica que tus cables coincidan con estos)
#define OLED_SDA 25
#define OLED_SCL 26
#define RTC_SDA 27
#define RTC_SCL 14

// ================= INICIALIZACIÓN DE OBJETOS =================
DHT dht(DHTPIN, DHTTYPE);
RTC_DS1307 rtc;
Adafruit_SSD1306 display(128, 64, &Wire, -1);
WebServer server(80);

// ================= VARIABLES GLOBALES =================
float temperatura = 0;
float humedad = 0;
int nivelSonido = 0;
bool sonidoDetectado = false;
int luzAmbiente = 0;
bool sistemaActivo = true;
bool rtcFound = false; // Bandera para saber si el RTC funciona

// Variables para sonido
int lecturaSonidoRaw = 0;
int silencioBase = 500;
bool sensorSonidoConectado = true;

// Variables para timing
unsigned long previousMillisSensores = 0;
unsigned long previousMillisDisplay = 0;
const long intervaloSensores = 2000;
const long intervaloDisplay = 1000;

// Variables para medición de sonido (Max/Min)
int maxSonido = 0;
int minSonido = 4095;
unsigned long lastSoundReset = 0;
const long soundResetInterval = 2000;

// ================= FUNCIONES AUXILIARES (LÓGICA INVERTIDA) =================
String getModoDiaNoche(int valorLuz) {
  // CORRECCIÓN: Si el valor es BAJO (<1000) es DÍA. Si es ALTO es NOCHE.
  if (valorLuz < UMBRAL_DIA_NOCHE) {
    return "DÍA"; 
  } else {
    return "NOCHE";
  }
}

String getEstadoLuz(int valorLuz) {
  // CORRECCIÓN: Escala invertida para el estado
  if (valorLuz < 300) return "DÍA SOLEADO";    
  if (valorLuz < UMBRAL_DIA_NOCHE) return "DÍA NUBLADO"; 
  if (valorLuz < 2000) return "ATARDECER";    
  return "MUY OSCURO";                        
}

// ================= INTERFAZ WEB (TU CÓDIGO HTML COMPLETO) =================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Estación Meteorológica IoT</title>
  <style>
    :root {
      --primary-color: #2c3e50;
      --secondary-color: #3498db;
      --success-color: #27ae60;
      --warning-color: #f39c12;
      --danger-color: #e74c3c;
      --light-bg: #ecf0f1;
      --dark-bg: #34495e;
      --text-light: #ecf0f1;
      --text-dark: #2c3e50;
    }
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      min-height: 100vh;
      padding: 20px;
      color: var(--text-light);
      transition: background 0.5s ease;
    }
    .modo-dia-body { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); }
    .modo-noche-body { background: linear-gradient(135deg, #2c3e50 0%, #3498db 100%); }
    .container { max-width: 1200px; margin: 0 auto; }
    .header {
      text-align: center;
      margin-bottom: 30px;
      background: rgba(255, 255, 255, 0.1);
      padding: 20px;
      border-radius: 15px;
      backdrop-filter: blur(10px);
    }
    .header h1 {
      font-size: 2.5rem;
      margin-bottom: 10px;
      background: linear-gradient(45deg, #fff, #ddd);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
    }
    .sensor-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
      gap: 20px;
      margin-bottom: 30px;
    }
    .sensor-card {
      background: rgba(255, 255, 255, 0.1);
      border-radius: 15px;
      padding: 25px;
      text-align: center;
      backdrop-filter: blur(10px);
      border: 1px solid rgba(255, 255, 255, 0.2);
      transition: transform 0.3s ease, box-shadow 0.3s ease;
    }
    .sensor-card:hover {
      transform: translateY(-5px);
      box-shadow: 0 10px 25px rgba(0, 0, 0, 0.2);
    }
    .sensor-icon { font-size: 3rem; margin-bottom: 15px; }
    .sensor-title { font-size: 1.3rem; margin-bottom: 15px; font-weight: 600; }
    .sensor-value { font-size: 2.5rem; font-weight: bold; margin-bottom: 10px; }
    .sensor-unit { font-size: 1.1rem; opacity: 0.8; }
    .sensor-status {
      margin-top: 10px;
      padding: 5px 15px;
      border-radius: 20px;
      font-size: 0.9rem;
      display: inline-block;
    }
    .status-low { background: var(--success-color); }
    .status-medium { background: var(--warning-color); }
    .status-high { background: var(--danger-color); }
    .status-active { background: var(--success-color); }
    .status-inactive { background: var(--danger-color); }
    .control-panel {
      background: rgba(255, 255, 255, 0.1);
      border-radius: 15px;
      padding: 25px;
      text-align: center;
      backdrop-filter: blur(10px);
      border: 1px solid rgba(255, 255, 255, 0.2);
    }
    .btn {
      background: var(--secondary-color);
      color: white;
      border: none;
      padding: 12px 30px;
      border-radius: 25px;
      cursor: pointer;
      font-size: 1.1rem;
      transition: all 0.3s ease;
      margin: 0 10px;
    }
    .btn:hover { background: #2980b9; transform: scale(1.05); }
    .sound-visualizer {
      width: 100%;
      height: 60px;
      background: rgba(0, 0, 0, 0.3);
      border-radius: 10px;
      margin-top: 15px;
      overflow: hidden;
      position: relative;
    }
    .sound-bar {
      position: absolute;
      bottom: 0;
      left: 0;
      width: 100%;
      background: linear-gradient(to top, #27ae60, #f39c12, #e74c3c);
      transition: height 0.3s ease;
    }
    .modo-indicator {
      margin-top: 8px;
      font-size: 1.1rem;
      font-weight: bold;
      padding: 8px 15px;
      border-radius: 20px;
      display: inline-block;
    }
    .modo-dia { background: linear-gradient(45deg, #f39c12, #e67e22); color: white; }
    .modo-noche { background: linear-gradient(45deg, #3498db, #2980b9); color: white; }
    .diagnostico { font-size: 0.8rem; margin-top: 5px; opacity: 0.7; }
    .debug-info { font-size: 0.7rem; margin-top: 3px; opacity: 0.5; }
    @media (max-width: 768px) { .sensor-grid { grid-template-columns: 1fr; } }
  </style>
</head>
<body class="modo-dia-body">
  <div class="container">
    <div class="header">
      <h1>🌤️ ESTACIÓN METEOROLÓGICA 🌤️</h1>
      <p>BIENVENIDO PAVO</p>
    </div>
    
    <div class="sensor-grid">
      <div class="sensor-card">
        <div class="sensor-icon">🌡️</div>
        <div class="sensor-title">Temperatura</div>
        <div class="sensor-value" id="temperature">--</div>
        <div class="sensor-unit">°C</div>
        <div class="sensor-status" id="temp-status">--</div>
      </div>
      
      <div class="sensor-card">
        <div class="sensor-icon">💧</div>
        <div class="sensor-title">Humedad</div>
        <div class="sensor-value" id="humidity">--</div>
        <div class="sensor-unit">%</div>
        <div class="sensor-status" id="hum-status">--</div>
      </div>
      
      <div class="sensor-card">
        <div class="sensor-icon">📢</div>
        <div class="sensor-title">Nivel de Sonido</div>
        <div class="sensor-value" id="sound">--</div>
        <div class="sensor-unit">/100</div>
        <div class="sound-visualizer">
          <div class="sound-bar" id="sound-bar" style="height: 0%;"></div>
        </div>
        <div class="sensor-status" id="sound-status">--</div>
        <div id="sound-digital" style="margin-top: 10px; font-size: 0.9rem;"></div>
        <div id="sonido-raw" class="diagnostico"></div>
        <div id="sonido-diagnostico" class="diagnostico"></div>
        <div id="sonido-debug" class="debug-info"></div>
      </div>
      
      <div class="sensor-card">
        <div class="sensor-icon">💡</div>
        <div class="sensor-title">Luz Ambiente</div>
        <div class="sensor-value" id="light">--</div>
        <div class="sensor-unit" id="light-unit">nivel</div>
        <div class="sensor-status" id="light-status">--</div>
        <div id="modo-dia-noche" class="modo-indicator">--</div>
        <div id="luz-debug" class="debug-info"></div>
      </div>
      
      <div class="sensor-card">
        <div class="sensor-icon">🕐</div>
        <div class="sensor-title">Fecha y Hora</div>
        <div class="sensor-value" id="date" style="font-size: 1.5rem;">--</div>
        <div class="sensor-unit" id="time">--</div>
        <div class="sensor-status" id="rtc-status">RTC DS1307</div>
      </div>
      
      <div class="sensor-card">
        <div class="sensor-icon">⚙️</div>
        <div class="sensor-title">Estado del Sistema</div>
        <div class="sensor-value" id="system-status">ACTIVO</div>
        <div class="sensor-unit">Modo</div>
        <div class="sensor-status status-active" id="status-indicator">ONLINE</div>
      </div>
    </div>
    
    <div class="control-panel">
      <h3 style="margin-bottom: 20px;">Control del Sistema</h3>
      <button class="btn" id="toggleBtn" onclick="toggleSystem()">⏸️ PAUSAR SISTEMA</button>
      <button class="btn" onclick="resetSound()">🔄 RESET SONIDO</button>
      <button class="btn" onclick="testSensor()">🎤 PROBAR SONIDO</button>
      <button class="btn" onclick="refreshData()">🔄 ACTUALIZAR</button>
    </div>
    
    <div class="last-update">
      Última actualización: <span id="last-update">--</span>
    </div>
  </div>

  <script>
    function updateData() {
      fetch('/sensors')
        .then(response => response.json())
        .then(data => {
          // Actualizar temperatura
          document.getElementById('temperature').textContent = data.temperatura.toFixed(1);
          document.getElementById('humidity').textContent = data.humedad.toFixed(1);
          
          // Actualizar sonido
          document.getElementById('sound').textContent = data.nivelSonido;
          document.getElementById('sound-bar').style.height = data.nivelSonido + '%';
          
          // Estado del sonido
          if (data.nivelSonido > 70) {
            document.getElementById('sound-status').textContent = 'ALTO';
            document.getElementById('sound-status').className = 'sensor-status status-high';
          } else if (data.nivelSonido > 30) {
            document.getElementById('sound-status').textContent = 'MODERADO';
            document.getElementById('sound-status').className = 'sensor-status status-medium';
          } else {
            document.getElementById('sound-status').textContent = 'BAJO';
            document.getElementById('sound-status').className = 'sensor-status status-low';
          }
          
          document.getElementById('sound-digital').textContent = data.sonidoDetectado ? '🔊 SONIDO DETECTADO' : '🔇 Silencio';
          document.getElementById('sound-digital').style.color = data.sonidoDetectado ? '#e74c3c' : '#27ae60';
          
          // Mostrar diagnóstico
          document.getElementById('sonido-raw').textContent = `Raw A0: ${data.lecturaSonidoRaw}`;
          document.getElementById('sonido-diagnostico').textContent = `Umbral: ${data.umbralSonido} | Silencio: ${data.silencioBase}`;
          document.getElementById('sonido-debug').textContent = `Max: ${data.maxSonido} | Min: ${data.minSonido}`;
          
          // Actualizar luz
          document.getElementById('light').textContent = data.luzAmbiente;
          document.getElementById('light-unit').textContent = data.modoDiaNoche;
          document.getElementById('light-status').textContent = data.estadoLuz;
          document.getElementById('luz-debug').textContent = `Umbral: ${data.umbralLuz}`;
          
          // Modo día/noche
          const modoElement = document.getElementById('modo-dia-noche');
          if (data.modoDiaNoche === 'DÍA') {
            modoElement.textContent = '☀️ MODO DÍA';
            modoElement.className = 'modo-indicator modo-dia';
            document.body.className = 'modo-dia-body';
          } else {
            modoElement.textContent = '🌙 MODO NOCHE';
            modoElement.className = 'modo-indicator modo-noche';
            document.body.className = 'modo-noche-body';
          }
          
          // Actualizar fecha y hora
          document.getElementById('date').textContent = data.fecha;
          document.getElementById('time').textContent = data.hora;
          
          // Actualizar sistema
          const toggleBtn = document.getElementById('toggleBtn');
          if (data.sistemaActivo) {
            toggleBtn.textContent = '⏸️ PAUSAR SISTEMA';
          } else {
            toggleBtn.textContent = '▶️ ACTIVAR SISTEMA';
          }
          
          document.getElementById('last-update').textContent = new Date().toLocaleTimeString();
        })
        .catch(error => { console.error('Error:', error); });
    }
    
    function toggleSystem() { fetch('/control?action=toggle').then(updateData); }
    function resetSound() { fetch('/control?action=resetsound').then(updateData); }
    function testSensor() { alert('¡Aplaude fuerte cerca del sensor KY-037!'); }
    function refreshData() { updateData(); }
    
    setInterval(updateData, 2000);
    window.onload = updateData;
  </script>
</body>
</html>
)rawliteral";

// ================= SETUP (AQUÍ ESTÁ LA MAGIA) =================
void setup() {
  Serial.begin(115200);
  Serial.println("🚀 Iniciando Estación Meteorológica...");
  Serial.println("🔧 Modo Día/Noche CORREGIDO");
  
  // 1. Inicializar OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("❌ Error al inicializar OLED");
  } else {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println("INICIANDO...");
    display.display();
  }
  
  dht.begin();
  
  // 2. Inicializar RTC (Reloj)
  Wire1.begin(RTC_SDA, RTC_SCL);
  if (!rtc.begin(&Wire1)) {
    Serial.println("❌ No se encontró el RTC");
    rtcFound = false;
  } else {
    Serial.println("✅ RTC Encontrado");
    rtcFound = true;

    // =================================================================
    // ZONA DE AJUSTE DE HORA
    // DESCOMENTA esta línea para subir la hora de tu PC al reloj.
    // LUEGO, vuelve a ponerle "//" y sube el código de nuevo.
    // =================================================================
    
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); 
    
    // =================================================================
  }
  
  pinMode(SOUND_SENSOR_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  
  // Calibrar silencio
  silencioBase = analogRead(SOUND_SENSOR_PIN);
  
  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("📡 Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Conectado!");
  Serial.println(WiFi.localIP());
  
  server.on("/", HTTP_GET, []() { server.send_P(200, "text/html", index_html); });
  server.on("/sensors", HTTP_GET, handleSensorData);
  server.on("/control", HTTP_GET, handleControl);
  server.begin();
  
  // Mostrar IP en pantalla
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("SISTEMA ONLINE");
  display.println("----------------");
  display.print("IP: ");
  display.println(WiFi.localIP());
  display.display();
}

// ================= MANEJO DE DATOS WEB =================
void handleSensorData() {
  String json = "{";
  json += "\"temperatura\":" + String(temperatura, 1) + ",";
  json += "\"humedad\":" + String(humedad, 1) + ",";
  json += "\"nivelSonido\":" + String(nivelSonido) + ",";
  json += "\"sonidoDetectado\":" + String(sonidoDetectado ? "true" : "false") + ",";
  json += "\"lecturaSonidoRaw\":" + String(lecturaSonidoRaw) + ",";
  json += "\"umbralSonido\":" + String(UMBRAL_SONIDO_ALTO) + ",";
  json += "\"silencioBase\":" + String(silencioBase) + ",";
  json += "\"maxSonido\":" + String(maxSonido) + ",";
  json += "\"minSonido\":" + String(minSonido) + ",";
  json += "\"luzAmbiente\":" + String(luzAmbiente) + ",";
  json += "\"modoDiaNoche\":\"" + getModoDiaNoche(luzAmbiente) + "\",";
  json += "\"estadoLuz\":\"" + getEstadoLuz(luzAmbiente) + "\",";
  json += "\"umbralLuz\":" + String(UMBRAL_DIA_NOCHE) + ",";
  json += "\"sistemaActivo\":" + String(sistemaActivo ? "true" : "false") + ",";
  
  // Lectura del RTC optimizada (sin reinicializar)
  if (rtcFound) {
    DateTime now = rtc.now();
    json += "\"fecha\":\"" + String(now.day()) + "/" + String(now.month()) + "/" + String(now.year()) + "\",";
    json += "\"hora\":\"" + String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second()) + "\"";
  } else {
    json += "\"fecha\":\"ERROR\",\"hora\":\"ERROR\"";
  }
  
  json += "}";
  server.send(200, "application/json", json);
}

void handleControl() {
  String action = server.arg("action");
  if (action == "toggle") {
    sistemaActivo = !sistemaActivo;
    server.send(200, "text/plain", sistemaActivo ? "Activado" : "Pausado");
  } else if (action == "resetsound") {
    maxSonido = 0;
    minSonido = 4095;
    server.send(200, "text/plain", "Sonido resetado");
    Serial.println("🔄 Sonido resetado");
  }
}

// ================= BUCLE PRINCIPAL =================
void loop() {
  server.handleClient();
  unsigned long currentMillis = millis();
  
  // Tarea 1: Leer sensores cada X segundos
  if (currentMillis - previousMillisSensores >= intervaloSensores && sistemaActivo) {
    previousMillisSensores = currentMillis;
    leerSensores();
  }
  
  // Tarea 2: Actualizar OLED cada Y segundos
  if (currentMillis - previousMillisDisplay >= intervaloDisplay) {
    previousMillisDisplay = currentMillis;
    actualizarDisplay();
  }
  
  // Tarea 3: Resetear picos de sonido
  if (currentMillis - lastSoundReset >= soundResetInterval) {
    lastSoundReset = currentMillis;
    maxSonido = 0;
    minSonido = 4095;
  }
}

void leerSensores() {
  temperatura = dht.readTemperature();
  humedad = dht.readHumidity();
  
  lecturaSonidoRaw = analogRead(SOUND_SENSOR_PIN);
  int diferenciaSonido = lecturaSonidoRaw - silencioBase;
  sonidoDetectado = (diferenciaSonido > 100); 
  
  if (diferenciaSonido > maxSonido) maxSonido = diferenciaSonido;
  if (diferenciaSonido < minSonido) minSonido = diferenciaSonido;
  
  int rangoSonido = maxSonido - minSonido;
  nivelSonido = map(rangoSonido, 0, MAX_LECTURA_SONIDO, 0, 100);
  nivelSonido = constrain(nivelSonido, 0, 100);
  
  luzAmbiente = analogRead(LDR_PIN);
  
  if (isnan(temperatura) || isnan(humedad)) {
    temperatura = -99; humedad = -99;
  }
}

void actualizarDisplay() {
  display.clearDisplay();
  display.setCursor(0,0);
  
  if (sistemaActivo) {
    display.println("ESTACION METEREOLO");
    display.println("-------------------");
    
    if (temperatura != -99) {
      display.printf("Temp: %.1fC", temperatura);
      display.setCursor(70, 8);
      display.printf("Hum:%.1f%%", humedad);
    }
    
    display.printf("Sonido: %d/100", nivelSonido);
    display.setCursor(70, 16);
    if (sonidoDetectado) display.print("SONIDO!"); else display.print("Silencio");  
    
    display.setCursor(0, 24);
    display.printf("Luz: %d", luzAmbiente);
    display.setCursor(70, 24);
    display.print(getModoDiaNoche(luzAmbiente));
    
    // Mostrar hora del RTC
    if (rtcFound) {
      DateTime now = rtc.now();
      display.setCursor(0, 32);
      display.printf("%02d/%02d %02d:%02d", now.day(), now.month(), now.hour(), now.minute());
    }
  } else {
    display.println("SISTEMA PAUSADO");
    display.println("-------------------");
    display.println("Activar desde web");
  }
  
  display.display();
}