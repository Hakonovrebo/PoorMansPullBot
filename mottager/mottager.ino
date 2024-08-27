#include <SoftwareSerial.h>
#include <ESP32Servo.h>
#define setPin 19 //kontroller at denne er til set pin på HC12
#define buzzer 15
#define servo 16

Servo myServo;

SoftwareSerial HC12(18, 17); // TX, RX
const char targetnumber = '1'; //targets unik number. 
const int HC12_Boude = 2400;
const int Serial_Boude = 9600;
byte incomingByte;
String readBuffer = "";

void setup() {
  //deactivating bt and wifi
  btStop();
  //WiFi.mode(WIFI_OFF);

  Serial.begin(Serial_Boude);

  pinMode(setPin, OUTPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, HIGH);
  delay(1000); 

  // Print a startup message
  Serial.println("Starting setup...");

  HC12Setup();
  //Servo setup
  myServo.attach(servo);
  digitalWrite(buzzer, LOW);
  // Confirm setup completed
  myServo.write(0);

  Serial.println("Setup completed.");
  Serial.flush();
}

void loop() {
  // Read data from HC-12
  while (HC12.available()) {
    incomingByte = HC12.read();
    readBuffer += char(incomingByte);
    delay(5);
  }

  if (readBuffer != "") {
    digitalWrite(buzzer, HIGH);
    Serial.println("******************");
    Serial.print("Mottok = ");
    Serial.println(readBuffer);

    if (readBuffer[0] == targetnumber){
      if (readBuffer[1] == 'R'){
        myServo.write(100);
        digitalWrite(buzzer, HIGH);
        sendReplay(1);
        delay(500);
        myServo.write(0);
        digitalWrite(buzzer, LOW); 
      }
      else if (readBuffer[1] == 'P'){
        sendReplay(1);
        Serial.println("mottokk P");
      }
      else {
        Serial.print("mottokk ukjent: ");
        Serial.println(readBuffer);
        sendReplay(0);
      }
    }
    digitalWrite(buzzer, LOW);
  }
  readBuffer = "";
  delay(200);
}
void sendReplay(bool x){
  if (x){
    String send = String(targetnumber) +"OK";
    Serial.print("sending respons: ");
    Serial.println(send);
    HC12.write(send.c_str());
    delay(200);
    HC12.write(send.c_str());
    delay(200);
  }
  else {
    String send = String(targetnumber) +"ERROR";
    HC12.write(send.c_str());
    delay(200);
  }
}

bool HC12Setup(){
  HC12.begin(HC12_Boude);
  digitalWrite(setPin, LOW);  // set pin to low for programming mode
  delay(1000); // to alow hc12 to switch to programing mode. 
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

/*
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
*/
  // Sett HC-12 i normal modus
  digitalWrite(setPin, HIGH);
  Serial.print("SET pin is, (TX/RX mode):");
  Serial.println(digitalRead(setPin));
  return digitalRead(setPin); // soud bo 1 or 0. 
}
