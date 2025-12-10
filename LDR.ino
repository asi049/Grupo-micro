#define PIN_LDR 35

void setup() {
  Serial.begin(115200);
}

void loop() {
  int luz = analogRead(PIN_LDR);
  Serial.println(luz);
  delay(300);
}
