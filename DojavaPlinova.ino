#include <IRremote.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <math.h>

int RECV_PIN = 11;

SoftwareSerial GSMserial(4,5); //RX, TX
SoftwareSerial WIFIserial(2,3); //RX, TX

IRrecv irrecv(RECV_PIN);

decode_results results;

int mode = 1,sensorValue = 0,gasType = 1;
float gasValue = 0, RO = 14823.283361;
String level;
String ssidName = "iPhoneDaniel";
String ssidPass = "1234567890";

Adafruit_SSD1306 display(4);

float gas_value(){
  float sensV=0;
  float RS=0;
  float RsRo=0;
  float ppm;
  int sensorValue = analogRead(A0);
  sensV=float(sensorValue)/1023*5.0;
  RS=5000*(5.0-sensV)/sensV;
  RsRo=RS/RO;
  switch (gasType){
    case 1:
      ppm=573.05*pow(RsRo,-2.13);
      break;
    case 2:
      ppm=954.94*pow(RsRo,-2.115);
      break;
    case 3:
      ppm=4237.8*pow(RsRo,-2.625);
      break;
    case 4:
      ppm=28306*pow(RsRo,-2.967);
      break;
    case 5:
      ppm=3905.7*pow(RsRo,-2.311);
      break;
    case 6:
      ppm=628.46*pow(RsRo,-2.17);
      break;
  }
  return ppm;
}

void connectWifi(){
  WIFIserial.begin(115200);
  String ssid="AT+CWJAP=\"";
  ssid+=ssidName;
  ssid+="\",\"";
  ssid+=ssidPass;
  ssid+="\"";
  WIFIserial.println("AT+RST");
  delay(5000);
  WIFIserial.println(ssid);
  delay(7000);
}

void sendEmail()
{
  Serial.println("Sending e-mail");
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(2);
  display.println("Sending e-mail");
  display.display();
  display.setTextSize(3);
  
  String message="Gas detected! Level:";
  message+=level;
  message+="/10";

  WIFIserial.begin(115200);
  delay(1000);  
  WIFIserial.println("AT+CIPMUX=1");
  delay(10);
  WIFIserial.println("AT+CIPSERVER=1,80");
  delay(10);
  WIFIserial.println("AT+CIPSTART=4,\"TCP\",\"43.228.184.6\",2525");
  delay(3000);
  WIFIserial.println("AT+CIPSEND=4,20");
  delay(10);
  WIFIserial.println("EHLO 192.168.1.123");
  delay(500);
  WIFIserial.println("AT+CIPSEND=4,12");
  delay(10);
  WIFIserial.println("AUTH LOGIN");
  delay(500);
  WIFIserial.println("AT+CIPSEND=4,38");
  delay(10);
  WIFIserial.println("ZGFuaWVsLnBhdmxla292aWNAZ21haWwuY29t");     //base64 encoded username
  delay(500);
  WIFIserial.println("AT+CIPSEND=4,18");
  delay(10);
  WIFIserial.println("SGNWcmh2aTd2dVNT");                    //base64 encoded password
  delay(500);
  WIFIserial.println("AT+CIPSEND=4,41");
  delay(10);
  WIFIserial.println("MAIL FROM:<daniel.pavlekovic@gmail.com>");  // use your email address
  delay(500);  
  WIFIserial.println("AT+CIPSEND=4,39");
  delay(10);
  WIFIserial.println("RCPT To:<daniel.pavlekovic@gmail.com>");
  delay(500);
  WIFIserial.println("AT+CIPSEND=4,6");
  delay(10);
  WIFIserial.println("DATA");
  delay(500);
  WIFIserial.println("AT+CIPSEND=4,10");
  delay(10);
  WIFIserial.println("To: User");
  delay(500);
  WIFIserial.println("AT+CIPSEND=4,15");
  delay(10);
  WIFIserial.println("From: Arduino");
  delay(500);
  WIFIserial.println("AT+CIPSEND=4,23");
  delay(10);
  WIFIserial.println("Subject: Gas Warning!");
  delay(500);
  WIFIserial.println("AT+CIPSEND=4,27");
  delay(10);
  WIFIserial.println(message);
  delay(500);
  WIFIserial.println("AT+CIPSEND=4,3");
  delay(10);
  WIFIserial.println(".");
  delay(500);
  WIFIserial.println("AT+CIPSEND=4,6");
  delay(10);
  WIFIserial.println("QUIT");
  delay(500);
  
  Serial.println("Email Sent!");
  display.clearDisplay();
  display.setCursor(0,0);
      display.setTextSize(2);
      display.println("E-Mail Sent!");
      display.display();
      display.clearDisplay();
      delay(5000);
      display.setTextSize(3);
}

void sendSMS(){
  GSMserial.begin(115200);
  Serial.println("Sending sms");
  display.clearDisplay();
  display.setCursor(0,0);
      display.setTextSize(2);
      display.println("Sending sms..");
      display.display();
      display.setTextSize(3);
      display.clearDisplay();
  String message="Gas detected! Level:";
  message+=level;
  message+="/10";
  
  //Set SMS format to ASCII
  GSMserial.write("AT+CMGF=1\r\n");
  delay(1000);
 
  //Send new SMS command and message number
  GSMserial.write("AT+CMGS=\"0919799311\"\r\n");
  delay(1000);
   
  //Send SMS content
  GSMserial.println(message);
  delay(1000);
   
  //Send Ctrl+Z / ESC to denote SMS message is complete
  GSMserial.write((char)26);
  delay(1000);
     
  Serial.println("SMS Sent!");
  display.setCursor(0,0);
      display.setTextSize(2);
      display.println("SMS Sent!");
      display.display();
      display.clearDisplay();
      delay(5000);
      display.display();
      display.setTextSize(3);
      display.clearDisplay();
}

void displayGas(){
  Serial.println(sensorValue);
  gasValue=gas_value();
  display.setCursor(0,0);
  display.println(gasValue);
  display.display();
  display.clearDisplay();
  delay(50);
}

void setup()
{
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setCursor(0,0);
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.display();
  irrecv.enableIRIn(); // Start the receiver
  display.setTextSize(2);
  display.println("Calibration..");
  display.display();
  connectWifi();
  sensorValue = analogRead(A0);
  while(sensorValue>90){
    sensorValue=analogRead(A0);
    Serial.println(sensorValue);
  }
  display.setTextSize(3);
  display.clearDisplay();
  display.display();
}

void loop() {
  start:
  if (irrecv.decode(&results)) {
    Serial.println(results.value,HEX);
    switch(results.value){
      case 0xfd00ff:
      mode=1;
      display.setCursor(0,0);
      display.setTextSize(2);
      display.println("WIFI+GSM");
      display.display();
      display.clearDisplay();
      delay(2000);
      display.display();
      display.setTextSize(3);
      break;
      case 0xfd807f:
      mode=2;
      display.setCursor(0,0);
      display.println("WIFI");
      display.display();
      display.clearDisplay();
      delay(2000);
      display.display();
      break;
      case 0xfd40bf:
      mode=3;
      display.setCursor(0,0);
      display.println("GSM");
      display.display();
      display.clearDisplay();
      delay(2000);
      display.display();
      break;
      case 0xfd20df:
      mode=4;
      display.setCursor(0,0);
      display.println("OFF");
      display.display();
      display.clearDisplay();
      delay(2000);
      display.display();
      break;
      case 0xfda05f:
      gasType=1;
      display.setCursor(0,0);
      display.println("LPG");
      display.display();
      display.clearDisplay();
      delay(2000);
      display.display();
      break;
      case 0xfd609f:
      gasType=2;
      display.setCursor(0,0);
      display.println("H2");
      display.display();
      display.clearDisplay();
      delay(2000);
      display.display();
      break;
      case 0xfd10ef:
      gasType=3;
      display.setCursor(0,0);
      display.println("CH4");
      display.display();
      display.clearDisplay();
      delay(2000);
      display.display();
      break;
      case 0xfd906f:
      gasType=4;
      display.setCursor(0,0);
      display.println("CO");
      display.display();
      display.clearDisplay();
      delay(2000);
      display.display();
      break;
      case 0xfd50af:
      gasType=5;
      display.setCursor(0,0);
      display.setTextSize(2);
      display.println("Smoke");
      display.display();
      display.clearDisplay();
      display.setTextSize(3);
      delay(2000);
      display.display();
      break;
      case 0xfdb04f:
      gasType=6;
      display.setCursor(0,0);
      display.setTextSize(2);
      display.println("Propane");
      display.display();
      display.clearDisplay();
      display.setTextSize(3);
      delay(2000);
      display.display();
      break;
    }
    irrecv.resume(); // Receive the next value
  }
  sensorValue=analogRead(A0);
  displayGas();
  if(mode==4)
  goto start;
  if(sensorValue>100 && sensorValue<200){
    level="01";
    if(mode==1 || mode==3)
    sendSMS();
    if(mode==1 || mode==2)
    sendEmail();
    while(sensorValue>90 && sensorValue<200 && mode!=4){
    sensorValue=analogRead(A0);
    displayGas();
    }
  }
  if(sensorValue>200 && sensorValue<300){
    level="02";
    if(mode==1 || mode==3)
    sendSMS();
    if(mode==1 || mode==2)
    sendEmail();
    while(sensorValue>190 && sensorValue<300 && mode!=4){
    sensorValue=analogRead(A0);
    displayGas();
    }
  }
  if(sensorValue>300 && sensorValue<400){
    level="03";
    if(mode==1 || mode==3)
    sendSMS();
    if(mode==1 || mode==2)
    sendEmail();
    while(sensorValue>290 && sensorValue<400 && mode!=4){
    sensorValue=analogRead(A0);
    displayGas();
    }
  }
  if(sensorValue>400 && sensorValue<500){
    level="04";
    if(mode==1 || mode==3)
    sendSMS();
    if(mode==1 || mode==2)
    sendEmail();
    while(sensorValue>390 && sensorValue<500 && mode!=4){
    sensorValue=analogRead(A0);
    displayGas();
    }
  }
  if(sensorValue>500 && sensorValue<600){
    level="05";
    if(mode==1 || mode==3)
    sendSMS();
    if(mode==1 || mode==2)
    sendEmail();
    while(sensorValue>490 && sensorValue<600 && mode!=4){
    sensorValue=analogRead(A0);
    displayGas();
    }
  }
  if(sensorValue>600 && sensorValue<700){
    level="06";
    if(mode==1 || mode==3)
    sendSMS();
    if(mode==1 || mode==2)
    sendEmail();
    while(sensorValue>590 && sensorValue<700 && mode!=4){
    sensorValue=analogRead(A0);
    displayGas();
    }
  }
  if(sensorValue>700 && sensorValue<800){
    level="07";
    if(mode==1 || mode==3)
    sendSMS();
    if(mode==1 || mode==2)
    sendEmail();
    while(sensorValue>690 && sensorValue<800 && mode!=4){
    sensorValue=analogRead(A0);
    displayGas();
    }
  }
  if(sensorValue>800 && sensorValue<900){
    level="08";
    if(mode==1 || mode==3)
    sendSMS();
    if(mode==1 || mode==2)
    sendEmail();
    while(sensorValue>790 && sensorValue<900 && mode!=4){
    sensorValue=analogRead(A0);
    displayGas();
    }
  }
  if(sensorValue>900 && sensorValue<1000){
    level="09";
    if(mode==1 || mode==3)
    sendSMS();
    if(mode==1 || mode==2)
    sendEmail();
    while(sensorValue>890 && sensorValue<1000 && mode!=4){
    sensorValue=analogRead(A0);
    displayGas();
    }
  }
  if(sensorValue>1000){
    level="10";
    if(mode==1 || mode==3)
    sendSMS();
    if(mode==1 || mode==2)
    sendEmail();
    while(sensorValue>990 && mode!=4){
    sensorValue=analogRead(A0);
    displayGas();
    }
  }
}
