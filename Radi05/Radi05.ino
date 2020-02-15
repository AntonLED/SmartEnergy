#include <BearSSLHelpers.h>
#include <CertStoreBearSSL.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureAxTLS.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiServerSecureAxTLS.h>
#include <WiFiServerSecureBearSSL.h>
#include <WiFiUdp.h>

#include <Adafruit_INA219.h>
#include <Wire.h>

#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include <SPI.h>

#define RELAY_PIN 0 //GPIO 0 - нужно проверить , работает ли. Если не работает, то очень плохо! -- D3

WiFiClientSecure client_secure; 

//String ssid = "WiFi-DOM.ru-8773";
//String password = "29d7qmc7df";
String ssid = "iPhone (Алина)"; 
String password = "12345678";

String readString;
const char* host = "script.google.com";
const int httpsPort = 443; 
const char* fingerprint = "5F F1 60 31 09 04 3E F2 90 D2 B0 8A 50 38 04 E8 37 6F BC 76";

RF24 radio(2, 15); // "создать" модуль на пинах 9 и 10 Для Уно

byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; //возможные номера труб

boolean TransSetup_Flag = false; 
boolean TransLoop_Flag = false; 
boolean RecL_1Pr_Flag = false; 
boolean RecL_3Pr_Flag = false;
boolean RecL_2Pr_Flag = false; 
boolean RecL_4Pr_Flag = false; 
boolean RecL_6Pr_Flag = false; 
boolean RecL_7Pr_Flag = false;
boolean RecS_1Pr_Flag = true; 
boolean RecS_3Pr_Flag = false; 
boolean RecS_2Pr_Flag = false; 
boolean RecS_4Pr_Flag = false; 
boolean RecS_6Pr_Flag = false; 
boolean RecS_7Pr_Flag = false; 

int data_1Pr[3]; // массив, хранящий передаваемые данные
int data_2Pr[3]; 
int data_3Pr[3]; 
int data_4Pr[3]; 
int data_6Pr[6]; 
int data_7Pr[6]; 
int data_5Pr_1Pr[5] = {5, 55, 555, 5555, 1};
int data_5Pr_2Pr[5] = {5, 55, 555, 5555, 2};
int data_5Pr_3Pr[5] = {5, 55, 555, 5555, 3};
int data_5Pr_4Pr[5] = {5, 55, 555, 5555, 4};
int data_5Pr_6Pr[5] = {5, 55, 555, 5555, 6}; 
int data_5Pr_7Pr[5] = {5, 55, 555, 5555, 7}; 
int data[8]; 

int i = 0; 

char teleTo = 'E'; 
boolean Flag_1Pr = false; 
boolean Flag_2Pr = false; 
boolean Flag_3Pr = false; 
boolean Flag_4Pr = false; 
boolean Flag_6Pr = false;
boolean Flag_7Pr = false; 
char teleFrom;

byte alarm_1Pr; 
byte alarm_2Pr;
byte alarm_3Pr;
byte alarm_4Pr; 
byte alarm_5Pr; 
byte alarm_6Pr; 
byte alarm_7Pr; 

byte alarm[7]; 
byte Rec_Alarm[7] = {0, 0, 0, 0, 0, 0, 0}; 

Adafruit_INA219 sensor; 

  int shuntvoltage = 0;
  int busvoltage = 0;
  int current_mA = 0;
  int loadvoltage = 0;
  int power_mW = 0;

  int cur_1Gen = 0; 
  int vol_1Gen = 0;
  int pow_1Gen = 0;
  
  int cur_2Gen = 0;
  int vol_2Gen = 0;
  int pow_2Gen = 0; 

  int cur_3Gen = 0;
  int vol_3Gen = 0;
  int pow_3Gen = 0; 

  int allPower = 0; 

void Sensor(){
  shuntvoltage = sensor.getShuntVoltage_mV();
  busvoltage = sensor.getBusVoltage_V()*1000;
  current_mA = sensor.getCurrent_mA();
  power_mW = sensor.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  
    data_5Pr_1Pr[1] = current_mA; //сила тока на 1-ом генераторе
  data_5Pr_1Pr[2] = loadvoltage; //напряжение на 1-ом генераторе
  data_5Pr_1Pr[3] = power_mW; // мощность на 1-ом генераторе

    data_5Pr_2Pr[1] = current_mA; //сила тока на 1-ом генераторе
  data_5Pr_2Pr[2] = loadvoltage; //напряжение на 1-ом генераторе
  data_5Pr_2Pr[3] = power_mW; // мощность на 1-ом генераторе

    data_5Pr_3Pr[1] = current_mA; //сила тока на 1-ом генераторе
  data_5Pr_3Pr[2] = loadvoltage; //напряжение на 1-ом генераторе
  data_5Pr_3Pr[3] = power_mW; // мощность на 1-ом генераторе

    data_5Pr_4Pr[1] = current_mA; //сила тока на 1-ом генераторе
  data_5Pr_4Pr[2] = loadvoltage; //напряжение на 1-ом генераторе
  data_5Pr_4Pr[3] = power_mW; // мощность на 1-ом генераторе

    data_5Pr_6Pr[1] = current_mA; //сила тока на 1-ом генераторе
  data_5Pr_6Pr[2] = loadvoltage; //напряжение на 1-ом генераторе
  data_5Pr_6Pr[3] = power_mW; // мощность на 1-ом генераторе

    data_5Pr_7Pr[1] = current_mA; //сила тока на 1-ом генераторе
  data_5Pr_7Pr[2] = loadvoltage; //напряжение на 1-ом генераторе
  data_5Pr_7Pr[3] = power_mW; // мощность на 1-ом генераторе
}

void Cloud_Setup() {
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(","); 
    delay(500); 
  }
  Serial.println(WiFi.localIP()); 
  Serial.println(WiFi.gatewayIP()); 
  Serial.println(WiFi.subnetMask());
  Serial.println("");
  Serial.println("WiFi connected"); 
}
void Cloud_Send(String fro, int cur, int vol, int cap) {
  Serial.print("connecting to ");
  Serial.println(host);
  client_secure.setInsecure();
  while (!client_secure.connect(host, httpsPort)) {
    delay(1); 
  }
  if (client_secure.verify(fingerprint, host)) {
  Serial.println("certificate matches");
  } else {
  Serial.println("certificate doesn't match");
  } 
  String string_current =  String(cur, DEC); 
  String string_voltage =  String(vol, DEC); 
  String string_capacity = String(cap, DEC);
  String url = "/exec?from=" + fro + "&current=" + string_current + "&voltage=" + string_voltage + "&capacity=" + string_capacity;
  Serial.print("requesting URL: ");
  Serial.println(url);
  client_secure.print(String("GET ") + "/macros/s/AKfycbyGjkrZJdYt8bv4R4SZC2nPtGqI8kXOWeCCOgxp_aLdHFG7a5ac" + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");
  Serial.println("request sent");
  while (client_secure.connected()) {
  String line = client_secure.readStringUntil('\n');
  if (line == "\r") {
    Serial.println("headers received");
    break;
  }
  }
  String line = client_secure.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
  Serial.println("esp8266/Arduino CI successfull!");
  } else {
  Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");
}

void Rec_1Pr_Setup(){
  for(int i = 0; i < 7; i++){
    Rec_Alarm[i] = 0;
  }
  
  radio.openReadingPipe(1, address[1]);     //хотим слушать трубу 0
  radio.startListening();  //начинаем слушать эфир, мы приёмный модуль
  
  RecS_1Pr_Flag = false; 
  RecL_1Pr_Flag = true; 
}
void Rec_2Pr_Setup(){
  for(int i = 0; i < 7; i++){
    Rec_Alarm[i] = 0;
  }
  
  radio.openReadingPipe(1, address[1]);
  radio.startListening(); 

  RecS_2Pr_Flag = false; 
  RecL_2Pr_Flag = true; 
}
void Rec_3Pr_Setup(){
  for(int i = 0; i < 7; i++){
    Rec_Alarm[i] = 0;
  }
  
  radio.openReadingPipe(1, address[1]);     //хотим слушать трубу 0
  radio.startListening();  //начинаем слушать эфир, мы приёмный модуль
  
  RecS_3Pr_Flag = false;
  RecL_3Pr_Flag = true; 
}
void Rec_4Pr_Setup(){
  for(int i = 0; i < 7; i++){
    Rec_Alarm[i] = 0;
  }
  
  radio.openReadingPipe(1, address[1]); 
  radio.startListening(); 

  RecS_4Pr_Flag = false; 
  RecL_4Pr_Flag = true; 
}
void Rec_6Pr_Setup(){
  for(int i = 0; i < 7; i++){
    Rec_Alarm[i] = 0;
  }
  
  radio.openReadingPipe(1, address[1]); 
  radio.startListening(); 

  RecS_6Pr_Flag = false; 
  RecL_6Pr_Flag = true; 
}
void Rec_7Pr_Setup(){
  for(int i = 0; i < 7; i++){
    Rec_Alarm[i] = 0;
  }
  
  radio.openReadingPipe(1, address[1]); 
  radio.startListening(); 

  RecS_7Pr_Flag = false; 
  RecL_7Pr_Flag = true; 
}
void Rec_1Pr_Loop(){
  if (radio.available()) {  // слушаем эфир со всех труб
  radio.read(&data, sizeof(data));         // чиатем входящий сигнал
  if(data[0] == 1 && data[4] == 5){
    for(int i = 1; i < 4; i++){
      data_1Pr[i-1] = data[i]; 
      Serial.print(data_1Pr[i-1]); 
      Serial.print(" "); 
    }    
            delay(10);
    
        Serial.println(); 
       delay(15); 
     while(alarm_1Pr != 1){
        radio.read(&alarm_1Pr, sizeof(alarm_1Pr)); 
                delay(1); 

     }
     while(alarm_2Pr != 2){
        radio.read(&alarm_2Pr, sizeof(alarm_2Pr)); 
        delay(1); 
       }
      while(alarm_3Pr != 3){
        radio.read(&alarm_3Pr, sizeof(alarm_3Pr)); 
        delay(1); 
       }
      while(alarm_4Pr != 4){
        radio.read(&alarm_4Pr, sizeof(alarm_4Pr)); 
        delay(1); 
       }
      while(alarm_5Pr != 5){
        radio.read(&alarm_5Pr, sizeof(alarm_5Pr)); 
        delay(1); 
       }
      while(alarm_6Pr != 6){
        radio.read(&alarm_6Pr, sizeof(alarm_6Pr)); 
        delay(1); 
       }
      while(alarm_7Pr != 7){
        radio.read(&alarm_7Pr, sizeof(alarm_7Pr)); 
        delay(1); 
       }
     
    for(int i = 0; i < 7; i++){
      Serial.print(alarm[i]); 
      Serial.print(" "); 
    }
        Serial.println(); 
        
    radio.openWritingPipe(address[2]);   //мы - труба 0, открываем канал для передачи данных
    radio.stopListening();  //не слушаем радиоэфир, мы передатчик
    while(true){
       if(radio.write(&teleTo, sizeof(teleTo))){
        RecL_1Pr_Flag = false; 
        RecS_2Pr_Flag = true;
        radio.closeReadingPipe(1); 
        Serial.println("Отправил ответный 1-ому!"); 
        break;  
       }
       delay(25); 
    }
  }
 }
}
void Rec_2Pr_Loop(){
  if (radio.available()) {  // слушаем эфир со всех труб
  radio.read(&data, sizeof(data));         // чиатем входящий сигнал
  if(data[0] == 2 && data[4] == 5){
    for(int i = 1; i < 4; i++){
      data_2Pr[i-1] = data[i]; 
      Serial.print(data_2Pr[i-1]); 
      Serial.print(" "); 
    }    
            delay(10);
    
        Serial.println(); 
       delay(15); 
     while(alarm_1Pr != 1){
        radio.read(&alarm_1Pr, sizeof(alarm_1Pr)); 
                delay(1); 

     }
     while(alarm_2Pr != 2){
        radio.read(&alarm_2Pr, sizeof(alarm_2Pr)); 
        delay(1); 
       }
      while(alarm_3Pr != 3){
        radio.read(&alarm_3Pr, sizeof(alarm_3Pr)); 
        delay(1); 
       }
      while(alarm_4Pr != 4){
        radio.read(&alarm_4Pr, sizeof(alarm_4Pr)); 
        delay(1); 
       }
      while(alarm_5Pr != 5){
        radio.read(&alarm_5Pr, sizeof(alarm_5Pr)); 
        delay(1); 
       }
      while(alarm_6Pr != 6){
        radio.read(&alarm_6Pr, sizeof(alarm_6Pr)); 
        delay(1); 
       }
      while(alarm_7Pr != 7){
        radio.read(&alarm_7Pr, sizeof(alarm_7Pr)); 
        delay(1); 
       }
     
    for(int i = 0; i < 7; i++){
      Serial.print(alarm[i]); 
      Serial.print(" "); 
    }
        Serial.println(); 
        
    radio.openWritingPipe(address[2]);   //мы - труба 0, открываем канал для передачи данных
    radio.stopListening();  //не слушаем радиоэфир, мы передатчик
    while(true){
       if(radio.write(&teleTo, sizeof(teleTo))){
        RecL_2Pr_Flag = false; 
        RecS_3Pr_Flag = true;
        radio.closeReadingPipe(1); 
        Serial.println("Отправил ответный 2-ому!"); 
        break;  
       }
       delay(25); 
    }
  }
 }
}
void Rec_3Pr_Loop(){
  if(radio.available()){
    radio.read(&data, sizeof(data)); 
    if(data[0] == 3 && data[4] == 5){
        for(int i = 1; i < 4; i++){
          data_3Pr[i-1] = data[i]; 
          Serial.print(data_3Pr[i-1]);
          Serial.print(" "); 
        }
                delay(10);
    
        Serial.println(); 
       delay(15); 
     while(alarm_1Pr != 1){
        radio.read(&alarm_1Pr, sizeof(alarm_1Pr)); 
                delay(1); 

     }
     while(alarm_2Pr != 2){
        radio.read(&alarm_2Pr, sizeof(alarm_2Pr)); 
        delay(1); 
       }
      while(alarm_3Pr != 3){
        radio.read(&alarm_3Pr, sizeof(alarm_3Pr)); 
        delay(1); 
       }
      while(alarm_4Pr != 4){
        radio.read(&alarm_4Pr, sizeof(alarm_4Pr)); 
        delay(1); 
       }
      while(alarm_5Pr != 5){
        radio.read(&alarm_5Pr, sizeof(alarm_5Pr)); 
        delay(1); 
       }
      while(alarm_6Pr != 6){
        radio.read(&alarm_6Pr, sizeof(alarm_6Pr)); 
        delay(1); 
       }
      while(alarm_7Pr != 7){
        radio.read(&alarm_7Pr, sizeof(alarm_7Pr)); 
        delay(1); 
       }
     
    for(int i = 0; i < 7; i++){
      Serial.print(alarm[i]); 
      Serial.print(" "); 
    }
        Serial.println(); 
        
    radio.openWritingPipe(address[2]);   //мы - труба 0, открываем канал для передачи данных
    radio.stopListening();  //не слушаем радиоэфир, мы 
    while(true){
       if(radio.write(&teleTo, sizeof(teleTo))){
        RecS_4Pr_Flag = true;
        RecL_3Pr_Flag = false; 
        radio.closeReadingPipe(1); 
        Serial.println("Отправил ответный 3-ему!");
        break;  
       }
       delay(25); 
      }        
    }
  }
}
void Rec_4Pr_Loop(){
  if(radio.available()){
    radio.read(&data, sizeof(data)); 
    if(data[0] == 4 && data[4] == 5){
        for(int i = 1; i < 4; i++){
          data_4Pr[i-1] = data[i]; 
          Serial.print(data_4Pr[i-1]);
          Serial.print(" "); 
        }
                delay(10);
    
        Serial.println(); 
       delay(15); 
     while(alarm_1Pr != 1){
        radio.read(&alarm_1Pr, sizeof(alarm_1Pr)); 
                delay(1); 

     }
     while(alarm_2Pr != 2){
        radio.read(&alarm_2Pr, sizeof(alarm_2Pr)); 
        delay(1); 
       }
      while(alarm_3Pr != 3){
        radio.read(&alarm_3Pr, sizeof(alarm_3Pr)); 
        delay(1); 
       }
      while(alarm_4Pr != 4){
        radio.read(&alarm_4Pr, sizeof(alarm_4Pr)); 
        delay(1); 
       }
      while(alarm_5Pr != 5){
        radio.read(&alarm_5Pr, sizeof(alarm_5Pr)); 
        delay(1); 
       }
      while(alarm_6Pr != 6){
        radio.read(&alarm_6Pr, sizeof(alarm_6Pr)); 
        delay(1); 
       }
      while(alarm_7Pr != 7){
        radio.read(&alarm_7Pr, sizeof(alarm_7Pr)); 
        delay(1); 
       }
     
    for(int i = 0; i < 7; i++){
      Serial.print(alarm[i]); 
      Serial.print(" "); 
    }
        Serial.println(); 
        
    radio.openWritingPipe(address[2]);   //мы - труба 0, открываем канал для передачи данных
    radio.stopListening();  //не слушаем радиоэфир, мы 
    while(true){
       if(radio.write(&teleTo, sizeof(teleTo))){
        TransSetup_Flag = true; 
        RecL_4Pr_Flag = false; 
        radio.closeReadingPipe(1); 
        Serial.println("Отправил ответный 4-ому!");
        break;  
       }
       delay(25); 
      }        
    }
  }
}
void Rec_6Pr_Loop(){
  if(radio.available()){
    radio.read(&data, sizeof(data)); 
    if(data[0] == 6 && data[7] == 5){
        for(int i = 1; i < 7; i++){
          data_6Pr[i-1] = data[i]; 
          Serial.print(data_6Pr[i-1]);
          Serial.print(" "); 
        }
                delay(10);
    
        Serial.println(); 
       delay(15); 
     while(alarm_1Pr != 1){
        radio.read(&alarm_1Pr, sizeof(alarm_1Pr)); 
                delay(1); 

     }
     while(alarm_2Pr != 2){
        radio.read(&alarm_2Pr, sizeof(alarm_2Pr)); 
        delay(1); 
       }
      while(alarm_3Pr != 3){
        radio.read(&alarm_3Pr, sizeof(alarm_3Pr)); 
        delay(1); 
       }
      while(alarm_4Pr != 4){
        radio.read(&alarm_4Pr, sizeof(alarm_4Pr)); 
        delay(1); 
       }
      while(alarm_5Pr != 5){
        radio.read(&alarm_5Pr, sizeof(alarm_5Pr)); 
        delay(1); 
       }
      while(alarm_6Pr != 6){
        radio.read(&alarm_6Pr, sizeof(alarm_6Pr)); 
        delay(1); 
       }
      while(alarm_7Pr != 7){
        radio.read(&alarm_7Pr, sizeof(alarm_7Pr)); 
        delay(1); 
       }
     
    for(int i = 0; i < 7; i++){
      Serial.print(alarm[i]); 
      Serial.print(" "); 
    }
        Serial.println(); 
        
    radio.openWritingPipe(address[2]);   //мы - труба 0, открываем канал для передачи данных
    radio.stopListening();  //не слушаем радиоэфир, мы 
    while(true){
       if(radio.write(&teleTo, sizeof(teleTo))){
        RecS_7Pr_Flag = true;
        RecL_6Pr_Flag = false; 
        radio.closeReadingPipe(1); 
        Serial.println("Отправил ответный 6-ему!");
        break;  
       }
       delay(25); 
      }        
    }
  }
}
void Rec_7Pr_Loop(){
  if(radio.available()){
    radio.read(&data, sizeof(data)); 
    if(data[0] == 7 && data[7] == 5){
        for(int i = 1; i < 7; i++){
          data_7Pr[i-1] = data[i]; 
          Serial.print(data_7Pr[i-1]);
          Serial.print(" "); 
        }
                delay(10);
    
        Serial.println(); 
       delay(15); 
     while(alarm_1Pr != 1){
        radio.read(&alarm_1Pr, sizeof(alarm_1Pr)); 
                delay(1); 

     }
     while(alarm_2Pr != 2){
        radio.read(&alarm_2Pr, sizeof(alarm_2Pr)); 
        delay(1); 
       }
      while(alarm_3Pr != 3){
        radio.read(&alarm_3Pr, sizeof(alarm_3Pr)); 
        delay(1); 
       }
      while(alarm_4Pr != 4){
        radio.read(&alarm_4Pr, sizeof(alarm_4Pr)); 
        delay(1); 
       }
      while(alarm_5Pr != 5){
        radio.read(&alarm_5Pr, sizeof(alarm_5Pr)); 
        delay(1); 
       }
      while(alarm_6Pr != 6){
        radio.read(&alarm_6Pr, sizeof(alarm_6Pr)); 
        delay(1); 
       }
      while(alarm_7Pr != 7){
        radio.read(&alarm_7Pr, sizeof(alarm_7Pr)); 
        delay(1); 
       }
     
    for(int i = 0; i < 7; i++){
      Serial.print(alarm[i]); 
      Serial.print(" "); 
    }
        Serial.println(); 
        
    radio.openWritingPipe(address[2]);   //мы - труба 0, открываем канал для передачи данных
    radio.stopListening();  //не слушаем радиоэфир, мы 
    while(true){
       if(radio.write(&teleTo, sizeof(teleTo))){
        RecS_1Pr_Flag = true;
        RecL_7Pr_Flag = false; 
        radio.closeReadingPipe(1); 
        Serial.println("Отправил ответный 3-ему!");
        break;  
       }
       delay(25); 
      }        
    }
  }
}
void TransSetup(){
  for(int i = 0; i < 7; i++){
    Rec_Alarm[i] = 0;
  }
  
  radio.openWritingPipe(address[1]);   //мы - труба 0, открываем канал для передачи данных
  radio.stopListening();  //не слушаем радиоэфир, мы передатчик
  
  TransSetup_Flag = false;
  TransLoop_Flag = true;
  Flag_1Pr = true; 
}
void TransLoop(){
   while(Flag_4Pr){
      radio.openWritingPipe(address[1]); 
      radio.stopListening();  //не слушаем радиоэфир, мы передатчик
      radio.write(&data_5Pr_4Pr, sizeof(data_5Pr_4Pr), true);
      
      delay(10); 
      radio.write(&alarm_1Pr, sizeof(alarm_1Pr), true); 
      delay(10); 
      radio.write(&alarm_2Pr, sizeof(alarm_2Pr), true); 
      delay(10); 
      radio.write(&alarm_3Pr, sizeof(alarm_3Pr), true); 
      delay(10); 
      radio.write(&alarm_4Pr, sizeof(alarm_4Pr), true); 
      delay(10); 
      radio.write(&alarm_5Pr, sizeof(alarm_5Pr), true); 
      delay(10); 
      radio.write(&alarm_6Pr, sizeof(alarm_6Pr), true); 
      delay(10); 
      radio.write(&alarm_7Pr, sizeof(alarm_7Pr), true); 
      
      radio.openReadingPipe(1, address[2]);    
      radio.startListening();  
        if(radio.available()){
          radio.read(&teleFrom, sizeof(teleFrom)); 
          if(teleFrom == 'D'){
            Flag_4Pr = false; 
            Flag_7Pr = true;  
            radio.closeReadingPipe(1); 
            Serial.print("4-ый получил: "); 
            Serial.println(teleFrom); 
          }
        }
        delay(25);   
  }
   while(Flag_2Pr){
      radio.openWritingPipe(address[1]); 
      radio.stopListening();  //не слушаем радиоэфир, мы передатчик
      radio.write(&data_5Pr_2Pr, sizeof(data_5Pr_2Pr), true);
      
      delay(10); 
      radio.write(&alarm_1Pr, sizeof(alarm_1Pr), true); 
      delay(10); 
      radio.write(&alarm_2Pr, sizeof(alarm_2Pr), true); 
      delay(10); 
      radio.write(&alarm_3Pr, sizeof(alarm_3Pr), true); 
      delay(10); 
      radio.write(&alarm_4Pr, sizeof(alarm_4Pr), true); 
      delay(10); 
      radio.write(&alarm_5Pr, sizeof(alarm_5Pr), true); 
      delay(10); 
      radio.write(&alarm_6Pr, sizeof(alarm_6Pr), true); 
      delay(10); 
      radio.write(&alarm_7Pr, sizeof(alarm_7Pr), true); 
      
      radio.openReadingPipe(1, address[2]);    
      radio.startListening();  
        if(radio.available()){
          radio.read(&teleFrom, sizeof(teleFrom)); 
          if(teleFrom == 'B'){
            Flag_2Pr = false; 
            Flag_3Pr = true; 
            radio.closeReadingPipe(1); 
            Serial.print("2-ой получил: "); 
            Serial.println(teleFrom); 
          }
        } 
        delay(25);    
  }
  while(Flag_3Pr){
      radio.openWritingPipe(address[1]); 
      radio.stopListening();  //не слушаем радиоэфир, мы передатчик
      radio.write(&data_5Pr_3Pr, sizeof(data_5Pr_3Pr), true);
      
      delay(10); 
      radio.write(&alarm_1Pr, sizeof(alarm_1Pr), true); 
      delay(10); 
      radio.write(&alarm_2Pr, sizeof(alarm_2Pr), true); 
      delay(10); 
      radio.write(&alarm_3Pr, sizeof(alarm_3Pr), true); 
      delay(10); 
      radio.write(&alarm_4Pr, sizeof(alarm_4Pr), true); 
      delay(10); 
      radio.write(&alarm_5Pr, sizeof(alarm_5Pr), true); 
      delay(10); 
      radio.write(&alarm_6Pr, sizeof(alarm_6Pr), true); 
      delay(10); 
      radio.write(&alarm_7Pr, sizeof(alarm_7Pr), true); 
       
      radio.openReadingPipe(1, address[2]);    
      radio.startListening();  
        if(radio.available()){
          radio.read(&teleFrom, sizeof(teleFrom)); 
          if(teleFrom == 'C'){
            Flag_3Pr = false; 
            Flag_4Pr = true;
            radio.closeReadingPipe(1); 
            Serial.print("3-ий получил: "); 
            Serial.println(teleFrom); 
          }
        }     
        delay(25);  
   }
  while(Flag_1Pr){
      radio.openWritingPipe(address[1]); 
      radio.stopListening();  //не слушаем радиоэфир, мы передатчик
      radio.write(&data_5Pr_1Pr, sizeof(data_5Pr_1Pr), true);
      
      delay(10); 
      radio.write(&alarm_1Pr, sizeof(alarm_1Pr), true); 
      delay(10); 
      radio.write(&alarm_2Pr, sizeof(alarm_2Pr), true); 
      delay(10); 
      radio.write(&alarm_3Pr, sizeof(alarm_3Pr), true); 
      delay(10); 
      radio.write(&alarm_4Pr, sizeof(alarm_4Pr), true); 
      delay(10); 
      radio.write(&alarm_5Pr, sizeof(alarm_5Pr), true); 
      delay(10); 
      radio.write(&alarm_6Pr, sizeof(alarm_6Pr), true); 
      delay(10); 
      radio.write(&alarm_7Pr, sizeof(alarm_7Pr), true); 
      
      radio.openReadingPipe(1, address[2]);    
      radio.startListening();  
        if(radio.available()){
          radio.read(&teleFrom, sizeof(teleFrom)); 
          if(teleFrom == 'A'){
            Flag_1Pr = false; 
            Flag_2Pr = true;           
            radio.closeReadingPipe(1); 
            Serial.print("1-ый получил: "); 
            Serial.println(teleFrom); 
          }
        }     
        delay(25);  
   }  
   while(Flag_7Pr){
      radio.openWritingPipe(address[1]); 
      radio.stopListening();  //не слушаем радиоэфир, мы передатчик
      radio.write(&data_5Pr_7Pr, sizeof(data_5Pr_7Pr), true);
      
      delay(10); 
      radio.write(&alarm_1Pr, sizeof(alarm_1Pr), true); 
      delay(10); 
      radio.write(&alarm_2Pr, sizeof(alarm_2Pr), true); 
      delay(10); 
      radio.write(&alarm_3Pr, sizeof(alarm_3Pr), true); 
      delay(10); 
      radio.write(&alarm_4Pr, sizeof(alarm_4Pr), true); 
      delay(10); 
      radio.write(&alarm_5Pr, sizeof(alarm_5Pr), true); 
      delay(10); 
      radio.write(&alarm_6Pr, sizeof(alarm_6Pr), true); 
      delay(10); 
      radio.write(&alarm_7Pr, sizeof(alarm_7Pr), true); 
      
      radio.openReadingPipe(1, address[2]);    
      radio.startListening();  
        if(radio.available()){
          radio.read(&teleFrom, sizeof(teleFrom)); 
          if(teleFrom == 'G'){
            Flag_7Pr = false; 
            Flag_6Pr = true;  
            radio.closeReadingPipe(1); 
            Serial.print("7-ый получил: "); 
            Serial.println(teleFrom); 
          }
        }
        delay(25);   
  }
   while(Flag_6Pr){
      radio.openWritingPipe(address[1]); 
      radio.stopListening();  //не слушаем радиоэфир, мы передатчик
      radio.write(&data_5Pr_6Pr, sizeof(data_5Pr_6Pr), true);
      
      delay(10); 
      radio.write(&alarm_1Pr, sizeof(alarm_1Pr), true); 
      delay(10); 
      radio.write(&alarm_2Pr, sizeof(alarm_2Pr), true); 
      delay(10); 
      radio.write(&alarm_3Pr, sizeof(alarm_3Pr), true); 
      delay(10); 
      radio.write(&alarm_4Pr, sizeof(alarm_4Pr), true); 
      delay(10); 
      radio.write(&alarm_5Pr, sizeof(alarm_5Pr), true); 
      delay(10); 
      radio.write(&alarm_6Pr, sizeof(alarm_6Pr), true); 
      delay(10); 
      radio.write(&alarm_7Pr, sizeof(alarm_7Pr), true); 
      
      radio.openReadingPipe(1, address[2]);    
      radio.startListening();  
        if(radio.available()){
          radio.read(&teleFrom, sizeof(teleFrom)); 
          if(teleFrom == 'F'){
            Flag_6Pr = false; 
            TransLoop_Flag = false; 
            RecS_6Pr_Flag = true;
            radio.closeReadingPipe(1); 
            Serial.print("6-ый получил: "); 
            Serial.println(teleFrom); 
          }
        }
        delay(25);   
  } 
}

void setup() {
  Serial.begin(115200); //открываем порт для связи с ПК
  sensor.begin();
  radio.begin(); //активировать модуль
  radio.setAutoAck(1);         //режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);    //(время между попыткой достучаться, число попыток)
  radio.enableAckPayload();    //разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);     //размер пакета, в байтах

  pinMode(RELAY_PIN, OUTPUT); 

  digitalWrite(RELAY_PIN, HIGH);
  
  WiFi.mode(WIFI_OFF);
  
  radio.setChannel(0x60);  //выбираем канал (в котором нет шумов!)

  radio.setPALevel (RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate (RF24_1MBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  //должна быть одинакова на приёмнике и передатчике!
  //при самой низкой скорости имеем самую высокую чувствительность и дальность!!
  radio.enableDynamicAck();                                  // Разрешаем выборочно отключать запросы подтверждения приема данных.

  radio.powerUp(); //начать работу
  
 // Cloud_Setup();
}

void loop() {
  if(RecS_1Pr_Flag){
    Rec_1Pr_Setup();
  }
  while(RecL_1Pr_Flag){
    Rec_1Pr_Loop();
    delay(1);
  }
delay(300); 
  if(RecS_2Pr_Flag){
    Rec_2Pr_Setup(); 
  }
  while(RecL_2Pr_Flag){
    Rec_2Pr_Loop(); 
    delay(1); 
  }
delay(300); 
  if(RecS_3Pr_Flag){
    Rec_3Pr_Setup(); 
  }
  while(RecL_3Pr_Flag){
    Rec_3Pr_Loop(); 
    delay(1); 
  }
delay(300); 
  if(RecS_4Pr_Flag){
    Rec_4Pr_Setup(); 
  }
  while(RecL_4Pr_Flag){
    Rec_4Pr_Loop(); 
    delay(1); 
  }  
  delay(300); 
  Sensor(); 
  if(TransSetup_Flag){
    TransSetup(); 
  }
  while(TransLoop_Flag){
    TransLoop(); 
    delay(1); 
  }
  delay(300); 
  if(RecS_6Pr_Flag){
    Rec_6Pr_Setup();
  }
  while(RecL_6Pr_Flag){
    Rec_6Pr_Loop(); 
    delay(1); 
  }
  delay(300); 
  if(RecS_7Pr_Flag){
    Rec_7Pr_Setup(); 
  }
  while(RecL_7Pr_Flag){
    Rec_7Pr_Loop(); 
    delay(1); 
  }
  cur_1Gen = data_7Pr[0]; 
  vol_1Gen = data_7Pr[1]; 
  pow_1Gen = data_7Pr[2]; 
  cur_2Gen = data_7Pr[3]; 
  vol_2Gen = data_7Pr[4]; 
  pow_2Gen = data_7Pr[5];
  cur_3Gen = data_6Pr[3]; 
  vol_3Gen = data_6Pr[4]; 
  pow_3Gen = data_6Pr[5];

  allPower = pow_1Gen + pow_2Gen + pow_3Gen; 
  
  //  radio.closeReadingPipe(2); 

if(data_6Pr[2] <= 4){
  if(data_4Pr[0]>0){
  if (allPower>=20600){
    digitalWrite(RELAY_PIN,HIGH);  
  }
  else{
    digitalWrite(RELAY_PIN,LOW);
  }
}
else{
  if (allPower>=3620){
    digitalWrite(RELAY_PIN,HIGH);
  }
  else{
    digitalWrite(RELAY_PIN,LOW);
  }
}
}
else{
  digitalWrite(RELAY_PIN, HIGH); 
}


Serial.println(); 
  delay(2000);
}
