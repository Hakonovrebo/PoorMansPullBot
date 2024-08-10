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
  //WiFi.mode(WIFI_OFF);

  Serial.begin(9600);
  HC12.begin(2400);
  pinMode(setPin, OUTPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(setPin, LOW);  // set pin to low for programming mode
  delay(1000); 

  // Print a startup message
  Serial.println("Starting setup...");
  Serial.print("SET pin is, (TX/RX mode):");
  Serial.println(digitalRead(setPin));
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
  Serial.print("HC-12 boude Response: ");
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
  Serial.print("SET pin is, (TX/RX mode):");
  Serial.println(digitalRead(setPin));

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
  delay(200);

  if (readBuffer != "") {
    Serial.println("******************");
    Serial.print("Mottok = ");
    Serial.println(readBuffer);

    if (readBuffer[0] == targetnumber){
      if (readBuffer[1] == 'R'){
        //myServo.write(80);
        digitalWrite(buzzer, HIGH);
        sendReplay(1);
        delay(500);
        //myServo.write(0);
        digitalWrite(buzzer, LOW); 
      }
      else if (readBuffer[1] == 'P'){
        sendReplay(1);
      }
      else {
        sendReplay(0);
      }
    }
  }
  readBuffer = "";
  delay(200);
}
void sendReplay(bool x){
  if (x){
    String send = targetnumber +"OK";
    Serial.println(send);
    HC12.write(send.c_str());
    delay(200);
  }
  else {
    String send = targetnumber +"ERROR";
    HC12.write(send.c_str());
    delay(200);
  }
}
/*
0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: Starting setup...
SET pin is, (TX/RX mode):0
HC-12 Defalt Response: OK+DEFAULT
HC-12 Power Response: 
OK+P1
HC-12 boude Response: OK+B2400

HC-12 FU Response: OK+FU1

SET pin is, (TX/RX mode):1
Setup completed.
******************
Mottok = 1R
%s: Target frequency %dMHz higher than supported.

******************
Mottok = 1R
%s: Target frequency %dMHz higher than supported.

******************
Mottok = 1R
%s: Target frequency %dMHz higher than supported.
*/
