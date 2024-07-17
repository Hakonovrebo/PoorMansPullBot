#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "logo.h" //tmbn logen

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin
#define SCREEN_ADDRESS 0x3C // I2C address for OLED
#include <SoftwareSerial.h>
#define setPin 19 //kontroler at denne er til set pin på HC12
#define NUM_LINES 3

//globale values
int multibuton = 15;
int targets = 2;
String* menyArray = new String[targets +3];
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SoftwareSerial HC12(18, 17);  //HC-12 TX pin, RX pin 
byte incomingByte;
String readBuffer = "";
int lineHeight;
int previusMenuPos = 0;
int screnPos = 0;
int pressed;


void setup() {
  Serial.begin(9600);
  HC12.begin(9600);
  lineHeight = SCREEN_HEIGHT / NUM_LINES; //sets the line higth for the screen. 
  pinMode (setPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);   // initialize digital pin LED_BUILTIN as an output.
  pinMode(multibuton, INPUT_PULLDOWN);
  digitalWrite(setPin, LOW);  //setpin to high for noraml mode tx/rx LOW to prog
  buildMenu();
  delay(1000); //delay to alow serial to begin. and HC12 to enter progaming mode 

  for (int i = 0; i < targets + 3; i++){
    Serial.println(menyArray[i]);
  } 

  Serial.println("Starting setup...");
  /*
  HC12.write("AT+DEFAULT"); //sets HC12 to defalt. 
  delay(150);  // Read response from HC-12
  while (HC12.available()) {
    incomingByte = HC12.read();
    readBuffer += char(incomingByte);
  }
  // Print the response to Serial Monitor
  Serial.print("HC-12 Defalt Response: ");
  Serial.println(readBuffer);
  
  // Clear readBuffer for next use
  readBuffer = "";
  
  delay(250);
 
  HC12.write("AT+P1");  //justere tx power til -1 for testing. 
  delay(150);  // Read response from HC-12
  while (HC12.available()) {
    incomingByte = HC12.read();
    readBuffer += char(incomingByte);
  }
  // Print the response to Serial Monitor
  Serial.print("HC-12 power Response: ");
  Serial.println(readBuffer);
  // Clear readBuffer for next use
  readBuffer = "";
  digitalWrite(setPin, HIGH); //HIGH sets HC12 back to rx/TX mode
  delay(250);

    // Set SET-pin to HIGH for normal TX/RX mode
  digitalWrite(setPin, HIGH);
  Serial.println("SET pin is HIGH (TX/RX mode)");
  */
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
}
void loop() {
  // 1 = up, 2 = test/back, 3 = down, 4 = fire/ok
  //oppdatere skjermen
  pressed = analogRead(multibuton);
  if (pressed > 100){
    Serial.print("pressed reading = ");
    Serial.println(pressed);
    int thebutton = butonPressed(pressed);
    Serial.print("buton presed = ");
    Serial.println(thebutton);
    if (thebutton == 1) {
      if (screnPos == 0){
        screnPos = targets + 2;
      }
      else{
        screnPos = (screnPos - 1);
      }
      Serial.println(screnPos);
      drawScreen(screnPos);
    }
    else if (thebutton == 3) {
      screnPos = ((screnPos + 1) % (targets + 3));
      Serial.print("linje 126 = ");
      Serial.println(screnPos);
      drawScreen(screnPos);
      }
    else if(thebutton == 4){
      /*
      if (screnPos == 0){
        testAll();
        }
      else if (screnPos == 1){
        raisAll();
      }
      else if(screnPos == targets +2){
        setings();
      }
      else if(screnPos > 1 || screnPos < targets + 2){
        reisOne(screnPos-1); //minus 1, sins first target is array pos 2. 
      }
      */
      Serial.print("screnpos = ");
      Serial.println(screnPos);
    }
  delay(200); // Debounce delay
  }
  
  while(Serial.available()){ //If HC-12 has data 
    incomingByte = HC12.read(); //Store each incoming byte from HC-12
    readBuffer += char(incomingByte);  // Add each byte to ReadBuffer string variable
  }
  delay(150);
  pressed = analogRead(multibuton);

  if(readBuffer != ""){
    Serial.println(readBuffer);
  }

  readBuffer = "";
  digitalWrite(LED_BUILTIN, LOW); 
  delay(200);
}
// globale functions
int butonPressed(int butonValue){
  //verdiene til butonValue vil være 40950, 1926, 1276 og 946
  // dette er ved usb strøm så på batterier vil det nok vere noe lavaere
  if(butonValue > 2000){ return 1;}
  else if (butonValue > 1500) {return 2;}
  else if (butonValue > 1150) {return 3;}
  else if (butonValue < 1150) {return 4;} // satt litt over 0 her tilfelle pull down skulle feile og sende svake signaler. 
  else {return 0;}
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

void setings(){
  screnPos = 0;
  display.clearDisplay();
  display.println("Setings:");
  display.print("tagets: ");
  display.print(targets);
  display.display();
  pressed = butonPressed(analogRead(multibuton));
  // 1 = up, 2 = test/back, 3 = down, 4 = fire/ok
  unsigned long looptimerstart = millis();
  pressed = 5; //to start the loop
  while ((pressed % 2) == 1){
    if (pressed = 1){
      targets ++;
      looptimerstart = millis(); //restart timeout timeer
      display.clearDisplay();
      display.println("Setings:");
      display.print("tagets: ");
      display.print(targets);
      display.display();
      pressed = 6; // so that its not an infedent loop
    }
      else if (pressed = 3){
      targets --;
      looptimerstart = millis();
      display.clearDisplay();
      display.println("Setings:");
      display.print("tagets: ");
      display.print(targets);
      display.display();
      pressed = 5; // so that its not an infedent loop
    }
    delay(200);
    unsigned long currentmils = millis();
    if (currentmils - looptimerstart > 10000){
      pressed = 2; //usikker her på om en kunne satt return. 
    }

    drawScreen(0); //må kalles her da den ikke kalles i loopen. 
  }
  
  //to do
  //tenken her er at en skal kunne endre antal mål og kalle på buildMeny på nytt. 
}
void drawScreen(int startPos) {
  display.clearDisplay();
/*
  for (int i = 0; i < 3; i++) {
    display.setCursor(0, i * lineHeight);
    int get = (startPos + i) % (targets + 3);
    display.println(menyArray[get]);
    Serial.print("display.print = ");
    Serial.println(menyArray[get]);
  }
*/
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(menyArray[startPos]);
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
  display.clearDisplay();
  display.println("Rais target:");
  display.print(targetNum);
  display.print(" ?");
  display.display();

  pressed = butonPressed(analogRead(multibuton));
  int loopteller = 0;
  while (pressed != 2){
    if (pressed = 4 or loopteller > 25){ // 4 = back/cansel user has 5seconds to ack
      drawScreen(targetNum);
      return false;
      }
    delay(200);
    pressed = butonPressed(analogRead(multibuton));
    loopteller ++;
  }

  String send = String(targetNum) + 'R';
  HC12.write(send.c_str()); // Send byte to HC-12
  readBuffer = "";
  delay(200);
  int teller = 0;
  while (teller < 20){ //the loop waits 5 sek for a reply. 
    while(Serial.available()){ //If HC-12 has data 
      incomingByte = HC12.read(); //Store each incoming byte from HC-12
      readBuffer += char(incomingByte);  // Add each byte to ReadBuffer string variable
    }
 
    if (readBuffer != ""){
      break;
    }
    delay(250);
    teller++;
  }
  if (readBuffer == ""){
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("waiting for");
    display.print("respons");
    display.display();
    delay(2500);
  }
  else if (readBuffer == ""){
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("no respons");
    display.print("from target");
    display.display();
    delay(1000);
  }
  else if (readBuffer == "OK"){
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("target released");
    display.display();
    delay(1000);
    readBuffer = "";
  Serial.println(readBuffer);
  readBuffer = "";
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
