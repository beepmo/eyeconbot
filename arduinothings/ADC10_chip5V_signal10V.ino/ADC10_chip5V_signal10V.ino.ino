// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {

  // read the input on analog pin 1:
  int R_ADC = analogRead(A1);

  Serial.println((float) R_ADC / 1023 * 10);

  delay(1000000);  // 1 read per second
}



