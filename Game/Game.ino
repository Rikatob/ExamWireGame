// EKSAMEN EMBEDDED SYSTEMS

#define WIRE_PIN 8


void setup() {
  Serial.begin(9600);
  // Use pullup to get less components on the board,
  // estimated 20 - 50 K resistor in arduino, 
  // if i want to have more control later on i can use pulldown and and add the chosen resistor. 
  pinMode(WIRE_PIN, INPUT_PULLUP);
}

void loop() {
  int wireState = digitalRead(WIRE_PIN);

  Serial.println(wireState);

}
