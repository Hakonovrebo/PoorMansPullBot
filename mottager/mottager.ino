#include <SoftwareSerial.h>
#include <ESP32Servo.h>
#define setPin 27 //kontroller at denne er til set pin på HC12
#define buzzer 32
#define servo 13

Servo myServo;

SoftwareSerial HC12(25, 26); // TX, RX
const char targetnumber = '1';

byte incomingByte;
String readBuffer = "";

void setup() {
  //deactivating bt and wifi
  btStop();
  WiFi.mode(WIFI_OFF);

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

//set boudrate to 2'400 for longer range
  readBuffer = "";  
  HC12.write("AT+B2400");
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

  //set FU1 
  readBuffer = "";  
  HC12.write("AT+FU1");
  delay(100);

  // Read response from HC-12
  while (HC12.available()) {
    incomingByte = HC12.read();
    readBuffer += char(incomingByte);
  }

  // Print the response to Serial Monitor
  Serial.print("HC-12 FU Response: ");
  Serial.println(readBuffer); //shoud be OK+P1
  // Clear readBuffer for next use
  readBuffer = "";

  // Sett HC-12 i normal modus
  digitalWrite(setPin, HIGH);
  Serial.println("SET pin is, (TX/RX mode):");
  Serial.print(setPin);

  pinMode(LED_BUILTIN, OUTPUT);
  

  //Servo setup
  myServo.attach(servo);

  // Confirm setup completed
  Serial.println("Setup completed.");
  Serial.flush();
}

void loop() {
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

    if (readBuffer[0] == targetnumber){
      if (readBuffer[1] == 'R'){
        myServo.write(90);
        digitalWrite(buzzer, HIGH);
        reply(true);
        delay(5000);
        myServo.write(0);
        digitalWrite(buzzer, LOW); 
      }
      else if (readBuffer[1] == 'P'){
        reply(true);
      }
      else {
        reply(false);
      }
    }
  }
  readBuffer = "";
}
void reply(bool x){
  if (x){

  }
  else {

  }
}
