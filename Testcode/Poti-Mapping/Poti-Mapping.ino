int mapping;


void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
pinMode(A1, INPUT_PULLUP);
}
void loop() {
  // put your main code here, to run repeatedly:
mapping = map(analogRead(A1), 0, 1023, 0, 4);
Serial.print(analogRead(A1));
Serial.print("\t");
Serial.println(mapping);
delay(250);
}
