#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "logo.h" //tmbn logen
#include <SoftwareSerial.h>


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin
#define SCREEN_ADDRESS 0x3C // I2C address for OLED
#define NUM_LINES 3

//globale values
const int setPin = 19; //kontroler at denne er til set pin på HC12
const int hardCodedMenuItems = 3;
const int espSerialBoud = 9600;
const int hc12Boud = 2400; 
const int multibuton = 15; //input pin for multibuton 
int targets = 2;
String* menyArray = new String[targets + hardCodedMenuItems];
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SoftwareSerial HC12(18, 17);  //HC-12 TX pin, RX pin 
byte incomingByte;
String readBuffer = "";
int lineHeight;
int previusMenuPos = 0;
int screnPos = 0;
int buttonInput;

enum button {
  UP,
  DOWN,
  RETURN_TEST,
  OK_RELISE,
  NONE //no buton are prest, null
};

void setup() {
  Serial.begin(espSerialBoud);
  HC12.begin(hc12Boud);
  lineHeight = SCREEN_HEIGHT / NUM_LINES; //sets the line higth for the screen. 
  pinMode (setPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);   // initialize digital pin LED_BUILTIN as an output.
  pinMode(multibuton, INPUT_PULLDOWN);
  buildMenu();
  delay(500); //delay to alow serial to begin.

  // printing the menu for debuging 
  for (int i = 0; i < targets + hardCodedMenuItems; i++){
    Serial.println(menyArray[i]);
  } 

  Serial.println("Starting setup...");
  hc12Setup();
  oledSetup();
}

void loop() {
  // 1 = up, 2 = test/back, 3 = down, 4 = fire/ok
  //oppdatere skjermen
  buttonInput = analogRead(multibuton);
  if (buttonInput > 100){
    Serial.print("buttonInput reading = ");
    Serial.println(buttonInput);
    button x = butonPressed(buttonInput);
    Serial.print("buton presed = ");
    Serial.println(x);
    switch (x)
    {
      case UP:  
        //move the scren pos -1 the modolo didnt work sis -1 % targest + 2 != targets + 2,  hens the it/else
        if (screnPos == 0){
          screnPos = targets + 2;
        }
        else{
          screnPos = (screnPos - 1);
        }
        Serial.println(screnPos);
        drawScreen(screnPos);
        break;
      case DOWN:
        screnPos = ((screnPos + 1) % (targets + 3));
        Serial.println(screnPos);
        drawScreen(screnPos);
        break;
      case OK_RELISE:
        if (screnPos == 0){
          testAll();
          }
        else if (screnPos == 1){
          raisAll();
        }
        else if(screnPos == targets +2){
          Serial.println("kaller på settings");
          setings();
        }
        else if(screnPos > 1 && screnPos < targets + 2){ 
          reisOne(screnPos-1); //minus 1, sins first target is array pos 2. 
        }
        Serial.print("screnpos = ");
        Serial.println(screnPos);
        break;
      case RETURN_TEST:
        if(screnPos > 1 && screnPos < targets + 2){ 
          testOne(screnPos-1); //minus 1, sins first target is array pos 2. 
        }
        break;
      case NONE:
        break;
    }
  delay(100); // Debounce delay
  }
  
  while(Serial.available()){ //If HC-12 has data 
    incomingByte = HC12.read(); //Store each incoming byte from HC-12
    readBuffer += char(incomingByte);  // Add each byte to ReadBuffer string variable
  }
  delay(150);

  if(readBuffer != ""){
    Serial.println(readBuffer);
  }
  readBuffer = "";
}

// globale functions
button butonPressed(int butonValue){
  //verdiene til butonValue vil være 40950, 1926, 1276 og 946
  // dette er ved usb strøm så på batterier vil det nok vere noe lavaere
  //til fysisk prototype vil nok ikke knappene dele en input, dette vil gjøre denne funksjoen unødvendig. 
  if(butonValue > 2000){ return button::UP;}
  else if (butonValue > 1500) {return button::RETURN_TEST;}
  else if (butonValue > 1150) {return button::DOWN;}
  else if (butonValue < 1150 && butonValue > 250) {return button::OK_RELISE;} // satt litt over 0 her tilfelle pull down skulle feile og sende svake signaler. 
  else {return button::NONE;}
}
void buildMenu(){
  if (menyArray != nullptr) {
    delete[] menyArray;
  }
  menyArray = new String[targets + 3];
  menyArray[0] = "Test all";
  menyArray[1] = "Relise all";
  for (int i = 2; i < targets + 2; i++){
    menyArray[i] = "target " + String(i-1);
  } 
  menyArray[targets + 2] = "Setings";
}
void drawSetings(){
  display.clearDisplay();
  delay(100); 
  display.setTextColor(SSD1306_WHITE); 
  display.setCursor(0, 0); 
  display.println("Setings:");
  display.print("tagets: ");
  display.print(targets);
  display.display();
}
void setings(){
  screnPos = 0;
  display.clearDisplay();
  delay(100); // Gi skjermen litt tid til å oppdatere
  drawSetings();
  unsigned long looptimerstart = millis();
  button pressed = button::NONE; //to start the loop
  while (pressed != button::RETURN_TEST){ 
    Serial.print("pressed = ");
    Serial.println(pressed);
    int x = analogRead(multibuton);
    if (x > 100){
      pressed = butonPressed(x);
    }
    if (pressed == button::UP){
      targets ++;
      looptimerstart = millis(); //restart timeout timeer
      drawSetings();
      pressed = button::NONE; 
    }
    else if (pressed == button::DOWN){
      targets --;
      looptimerstart = millis();
      drawSetings();
      pressed = button::NONE; 
    }
    delay(100);
    unsigned long currentmils = millis();

    if (currentmils - looptimerstart > 10000){
      break;
    }
  }
  buildMenu();
  delay(50);
  drawScreen(0); //må kalles her da den ikke kalles i loopen. 
}
void drawScreen(int startPos) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.fillRect(0, 0, 128, 16, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(0, 0);
  display.println(menyArray[startPos]);
  display.setTextColor(SSD1306_WHITE);
  display.println(menyArray[((startPos + 1) % (targets + 3))]);
  display.println(menyArray[((startPos + 2) % (targets + 3))]);
  display.println(menyArray[((startPos + 3) % (targets + 3))]);
  display.display();
}
void testAll(){
  //to do 
  //ping all targes and display respons
}
void raisAll(){
  //to do
  //rais all targes, display respons
}
bool reisOne(int targetNum){
  Serial.print("funk raisOne, targetNum = ");
  Serial.println(targetNum);
  display.clearDisplay();
  delay(50); 
  display.setTextColor(SSD1306_WHITE); 
  display.setCursor(0, 0); 
  display.println("Rais:");
  display.print(targetNum);
  display.println(" ?");
  display.println("press up");
  display.display();
  delay(200); //buton debounce

  button pressed = NONE;
  int loopteller = 0;
  while (pressed != button::UP){
    Serial.print("!= 2 loop, pressed = ");
    Serial.print(pressed);
    Serial.print(", loopteller =  ");
    Serial.println(loopteller);
    if (pressed == button::RETURN_TEST || loopteller > 25){ // 4 = back/cansel user has 5seconds to ack
      drawScreen(targetNum +1);
      Serial.print("avbryter loop, return");
      return false;
      }
    delay(200);
    pressed = butonPressed(analogRead(multibuton));
    loopteller ++;
  }

  String send = String(targetNum) + 'R';
  Serial.println("sender "+ send);
  HC12.write(send.c_str()); // Send byte to HC-12
  readBuffer = "";

  int teller = 0;
  Serial.println("sent data. ");
  display.clearDisplay();
  delay(50);
  display.setTextColor(SSD1306_WHITE); 
  display.setCursor(0, 0); 
  display.println("waiting");
  display.println("for");
  display.println("response");
  display.display();
  unsigned long timeout = millis() + 10000;
  while (unsigned long x = millis() < timeout){ //the loop waits 5 sek for a reply. 
    //Serial.print("waiting for response from target, teller = ");
    //Serial.println(teller);
    while(HC12.available()){ //If HC-12 has data 
      incomingByte = HC12.read(); //Store each incoming byte from HC-12
      readBuffer += char(incomingByte);  // Add each byte to ReadBuffer string variable
      delay(5); // smal delay to enchur that HC12 dosent have more data incoming. 
    }
 
    if (readBuffer != ""){
      Serial.println(readBuffer);
      break;
    }
  }
  Serial.print("readBuffer = ");
  Serial.println(readBuffer);
  if (readBuffer == ""){
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("no respons");
    display.println("from");
    display.println("target");
    display.display();
    delay(1000);
    drawScreen(targetNum +1);
    return false;
  }
  if (readBuffer.indexOf("1") != -1){
    if (readBuffer.indexOf("OK") != -1){
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("target");
      display.print("released");
      display.display();
      delay(3000);
      Serial.println("Target released, OK. readBuffer = ");
      Serial.println(readBuffer);
      readBuffer = "";
      drawScreen(targetNum +1);
      return true;
    }
    else{ 
      Serial.println("unknown message form target. readBuffer = ");
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("unknown");
      display.println("target");
      display.print("respons");
      display.display();
      Serial.println("unkown target respons, readBuffer = ");
      Serial.println(readBuffer);
      delay(3000);
      readBuffer = "";
      drawScreen(targetNum +1);
      return false;
    }
  }
}
int testOne(int targetNum){

  String send = String(targetNum) + 'P';
  HC12.write(send.c_str()); // Send byte to HC-12
  readBuffer = "";
  delay(200);

  while(Serial.available()){ 
    incomingByte = HC12.read(); 
    readBuffer += char(incomingByte);
  }
}
bool hc12Setup(){
  digitalWrite(setPin, LOW);  //setpin to high for noraml mode tx/rx LOW to prog
  delay(1000);
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
  return digitalRead(setPin); //her er tanken og få på tre tester som går gjennom at HC12 er satt opp korekt og ev restarte alt om ikke alt av setup går gjennom 
}
bool oledSetup(){
  // Initialize I2C with correct pins for ESP32
  Wire.begin(22, 23); // SDA, SCL

  // initialize the OLED object
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  Serial.println(F("SSD1306 initialized successfully"));

  // Clear the buffer
  display.clearDisplay();
  display.setRotation(2); // setter rotasjonen på displayet til 180grader
  // Display TMBN Logo!
  Serial.println("print Logo");
  display.clearDisplay();
  display.drawBitmap(0, 0, logo_bmp, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
  display.display();
  delay(3000); //to show off logo 
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  drawScreen(0);
  return true; //not shure how to confirm setup compleet
}
