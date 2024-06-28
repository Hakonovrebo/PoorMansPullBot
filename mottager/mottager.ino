#include <SoftwareSerial.h>
#include <ESP32Servo.h>
#define setPin 27 //kontroller at denne er til set pin på HC12
#define buzzer 32
#define servo 13

Servo myServo;

SoftwareSerial HC12(25, 26); // TX, RX
int teller = 0;

byte incomingByte;
String readBuffer = "";

void setup() {
  Serial.begin(9600);
  HC12.begin(9600);
  pinMode(setPin, OUTPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(setPin, LOW);  // set pin to low for programming mode
  delay(1000); 

  // Print a startup message
  Serial.println("Starting setup...");

  // Send AT-kommando for å tilbakestille til fabrikkinnstillinger
  HC12.write("AT+DEFAULT");
  delay(100);

  // Read response from HC-12
  while (HC12.available()) {
    incomingByte = HC12.read();
    readBuffer += char(incomingByte);
  }

  // Print the response to Serial Monitor
  Serial.print("HC-12 Defalt Response: ");
  Serial.println(readBuffer);
  // Clear readBuffer for next use
  delay(250);
  //set TXpower tp -1
  readBuffer = "";  
  HC12.write("AT+P1");
  delay(100);

  // Read response from HC-12
  while (HC12.available()) {
    incomingByte = HC12.read();
    readBuffer += char(incomingByte);
  }

  // Print the response to Serial Monitor
  Serial.print("HC-12 Power Response: ");
  Serial.println(readBuffer); //shoud be OK+P1
  // Clear readBuffer for next use
  readBuffer = "";

  // Sett HC-12 i normal modus
  digitalWrite(setPin, HIGH);
  Serial.println("SET pin is, (TX/RX mode):");
  Serial.print(setPin);

  pinMode(LED_BUILTIN, OUTPUT);
  
  // Indikator LED og buzzer
  for(int i = 0; i < 3; i++) {
    digitalWrite(LED_BUILTIN, HIGH); 
    digitalWrite(buzzer, HIGH);
    delay(250);                      
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(buzzer, LOW);
    delay(250);
  }
  //Servo setup
  myServo.attach(servo);

  // Confirm setup completed
  Serial.println("Setup completed.");
  Serial.flush();
}

void loop() {
  Serial.print("teller = ");
  Serial.println(teller);
  teller++;
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(100);

  // Read data from HC-12
  while (HC12.available()) {
    incomingByte = HC12.read();
    readBuffer += char(incomingByte);
  }
  
  delay(100);

  if (readBuffer != "") {
    Serial.println("******************");
    Serial.print("Mottok = ");
    Serial.println(readBuffer);

    // Remove any unexpected characters (e.g., newlines, spaces)
    readBuffer.trim();
    Serial.print("readBuffer trimmet = ");
    Serial.println(readBuffer);
    // Convert the received string to an integer
    int receivedValue = readBuffer.toInt();
    Serial.print("receivedValue convertet to int: ");
    Serial.println(receivedValue);

    // Map the received value (0-17) to 0-170 degrees
    int angle = map(receivedValue, 0, 17, 1, 179);

    // Write the angle to the servo
    myServo.write(angle);

    // Print the angle for debugging
    Serial.print("Mapped angle: ");
    Serial.println(angle);

    // Activate buzzer
    for (int i = 0; i < 5; i++) {
      digitalWrite(buzzer, HIGH);
      delay(250);
      digitalWrite(buzzer, LOW);
      delay(250);
    }
  }

  delay(200);
  readBuffer = "";
  digitalWrite(LED_BUILTIN, LOW);
}