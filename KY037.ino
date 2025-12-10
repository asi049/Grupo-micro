#define PIN_MIC 34

void setup() {
  Serial.begin(115200);
}

void loop() {
  int sonido = analogRead(PIN_MIC);
  Serial.println(sonido);
  delay(300);
}
