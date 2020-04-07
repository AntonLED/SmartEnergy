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

String ssid = "WiFi-DOM.ru-8773";
String password = "29d7qmc7df";
//String ssid = "iPhone (Алина)"; 
//String password = "12345678"; 
//String ssid = "sombaree";
//String password = "87654321";

String readString;
const char* host = "script.google.com";
const int httpsPort = 443; 
const char* fingerprint = "5F F1 60 31 09 04 3E F2 90 D2 B0 8A 50 38 04 E8 37 6F BC 76";

RF24 radio(2,15);

byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; //возможные номера труб

byte Ind; // переменная индекса 

boolean IndFlag = false; 
boolean SortFlag = false; 
boolean TransmittFlag = false;
boolean FirstRecieveFlag = false;  
boolean RecieveFlag = false; 

int MembersNumber = 7; // колчичество модулей в системе

int MembersPr[7]; // массив с приоритетами модулей системы

int AlarmCount = 0; // аварийный счетчик 

int dataFrom[8]; // для хранения пришедших данных 
int dataTo[5] = {1, 22, 222, 2222, 2}; // для хранения отсылаемых данных 
int dataSystem[7][6]; // массив-база данных

byte myPriority = 6; // для хранения приоритета данного модуля

byte teleTo = myPriority; //  совпадает с номером приоритета 
byte teleFrom;  // совпадает с номером приоритета приемника 

int latestFrom; // для того, чтобы 2 раза от одного и того же не принять 
int From; // идентификация того, от кого присылается пакет 
int Me; // наше положение в массиве приоритетов

int noZero = 0; // счетчик тех элементов MembersPr, которые не ноль (для того, чтобы работало, когда всего двое в системе)

boolean includeFlag = true;  // флаг для включения в сеть 
char including ; // переменная для приема от передатчика при первом подкючении к сети и для последующего подключения других 
int includeCount = 0; // просто счетчик для INCLUDING 

Adafruit_INA219 sensor; // активация датчика

  int shuntvoltage = 0; // переменные для показаний с датчика 
  int busvoltage = 0;
  int current_mA = 0;
  int loadvoltage = 0;
  int power_mW = 0;

  int power_1Gen = 0;
  
  int power_2Gen = 0; 

  int batteryStat = 0; // статус не включена ли батарея?

  int allPower = 0; // общая генерируемая мощность 



void sensorUpdate(){
  shuntvoltage = sensor.getShuntVoltage_mV();
  busvoltage = sensor.getBusVoltage_V()*1000;
  current_mA = sensor.getCurrent_mA();
  power_mW = sensor.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
   
  dataTo[1] = current_mA; //параметры на 1-ом генераторе
  dataTo[2] = loadvoltage; 
  dataTo[3] = power_mW; 

  for(int i = 1; i <= 3; i++){
    dataSystem[myPriority - 1][i - 1] = dataTo[i]; 
  }
}

void Transmitt_Setup(byte addr){
  radio.openWritingPipe(address[addr]);  
  radio.stopListening(); 
}

void Transmitt_Loop(byte MyIndex, byte YouIndex){

      dataTo[0] = MyIndex;
      dataTo[4] = YouIndex; 

  AlarmCount = 0;     
  while(true){
      radio.openWritingPipe(address[1]); 
      radio.stopListening();  //не слушаем радиоэфир, мы передатчик
      
      radio.write(&dataTo, sizeof(dataTo), true);

      ToInclude(); // функция для кратковременного спама в 5 трубу с целью найти там кого-нибудь (НЕ ЗАБУТЬ CLOSE'ануть REadig Pipe!!!) 
            
      radio.openReadingPipe(1, address[2]);    
      radio.startListening();  
      
        if(radio.available()){
          radio.read(&teleFrom, sizeof(teleFrom)); 
          if(teleFrom == YouIndex){
            radio.closeReadingPipe(1); 
            Serial.println(); 
            Serial.println(teleFrom); 
            break; 
          }
        }

       Alarm(100); 
        
       delay(25);    
  }         
}

void Recieve_Setup(byte addr){
  radio.openReadingPipe(1, address[addr]);  
  radio.startListening(); 
}

int Recieve_Loop(){
  AlarmCount = 0; 
  while(true){
    if(radio.available()){
      radio.read(&dataFrom, sizeof(dataFrom)); 
      if(dataFrom[0] != 0 && (dataFrom[4] == myPriority || dataFrom[7] == myPriority)){

       From = dataFrom[0]; 
       
       for(int i = 1; i < 7; i++){  // формирование БД в From строчке соответвенно ток, напряжение и мощность модуля From
         dataSystem[From - 1][i - 1] = dataFrom[i];
       }
          
       AlarmCount = 0;         
       
       if(latestFrom != From){
         radio.openWritingPipe(address[2]); 
         radio.stopListening(); 
          while(true){
            if(radio.write(&teleTo, sizeof(teleTo))){
              radio.closeReadingPipe(1); 
              latestFrom = From; 
              return(From);  
            }

            Alarm(500); 
            
            delay(5); 
          }        
       }

        for(int i = 0; i < MembersNumber; i++){
          if(MembersPr[i] != 0){
            noZero++; 
          }
        }       

        AlarmCount = 0; 
        if(noZero == 2){
          noZero = 0; 
          radio.openWritingPipe(address[2]); 
          radio.stopListening(); 
           while(true){
             if(radio.write(&teleTo, sizeof(teleTo))){
               radio.closeReadingPipe(1); 
               return(From);  
             }

             Alarm(500); 
             
             delay(5); 
           }           
        }
        
      }
    } 

    Alarm(2500); 
    
    delay(1);   
  } 
}

void Indexing(){ // индексация - прием данных должен быть универсален для всех приориетов 

 AlarmCount = 0; 
    
 while(IndFlag){
  
  radio.openReadingPipe(1, address[3]);     //хотим слушать трубу 0
  radio.startListening();  //начинаем слушать эфир, мы приёмный модуль
  
   if (radio.available()) {
      
     radio.read(&Ind, sizeof(Ind));        
   
     if(Ind == myPriority){  
      Serial.print("Мой индекс - ");
      Serial.println(Ind); 
        
      radio.openWritingPipe(address[4]); 
      radio.stopListening();

     AlarmCount = 0;
      
      while(true){
        if(radio.write(&teleTo, sizeof(teleTo))){
          Serial.println("Получил!"); 
          radio.closeReadingPipe(1);  
          IndFlag = false; 
          break; 
        }        
        Alarm(500); 
        delay(5); 
      }
     } 
    }
   if(AlarmCount++ >= 5000/2){
     Including('I'); 
   }
   delay(1);   
 }
}

void Alarm(int limit){
  AlarmCount++; 
  
  if(AlarmCount >= limit){
    AlarmCount = 0; 
    ESP.reset(); 
  }
}

void ToInclude(){
  Transmitt_Setup(0); 

  including = 'A'; 

    if(radio.write(&including, sizeof(including))){ // если кого-то нашли, то ждем, пока все удйт в ресет (2.5 сек) и сами перезагружаемся 
      radio.startListening();  //начинаем слушать эфир, мы приёмный модуль
      //delay(1000); 
      ESP.reset(); 
    }    
    
  Transmitt_Setup(1); 
}

void Including(char option){

  including = 0;

if(option == 'F'){
 Recieve_Setup(0);    
  while(true){
    if(radio.available()){
      radio.read(&including, sizeof(including));
      if(including == 'A'){
        includeFlag = false; 
        IndFlag = true;
        radio.closeReadingPipe(1); 
        break; 
      }      
    }
    delay(5); 
  }  
}

else if(option == 'I'){
radio.closeReadingPipe(1); 
radio.openReadingPipe(1, address[0]);     //хотим слушать трубу 0
radio.startListening();  //начинаем слушать эфир, мы приёмный модуль
 if(radio.available()){
  radio.read(&including, sizeof(including));
  if(including == 'A'){
    //delay(5000);
    ESP.reset(); 
  }  
 }
 radio.closeReadingPipe(1); 
} 
}

void setup() {
  Serial.begin(115200); //открываем порт для связи с ПК
  radio.begin(); //активировать модуль
  sensor.begin(); // активация датчика 
  radio.setAutoAck(1);         //режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 0);    //(время между попыткой достучаться, число попыток)
  radio.enableAckPayload();    //разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);     //размер пакета, в байтах

  pinMode(RELAY_PIN, OUTPUT); 

  digitalWrite(RELAY_PIN, HIGH); 

  radio.setChannel(0x60);  //выбираем канал (в котором нет шумов!)

  radio.setPALevel (RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate (RF24_1MBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  //должна быть одинакова на приёмнике и передатчике!
  //при самой низкой скорости имеем самую высокую чувствительность и дальность!!
  radio.enableDynamicAck();                                  // Разрешаем выборочно отключать запросы подтверждения приема данных.
  radio.powerUp(); //начать работу
}

void loop() {
// INCLUDING 
if(includeFlag){  
  Including('F'); 
}
 
// INDEXING 
if(IndFlag){
  
  Indexing();
  
  FirstRecieveFlag = true; 
 }
 
// FIRST MAIN LOOP ITTERATION (ARRAY GETTING) 
  // FIRST RECIEVE 
if(FirstRecieveFlag){
  
  Recieve_Setup(1);

  AlarmCount = 0;

  while(radio.available() == false){
    delay(1); 
    //Alarm(5000); 
  }

  radio.read(&MembersPr, sizeof(MembersPr));        
    
  FirstRecieveFlag = false; 
  SortFlag = true; 
}
 
// ARRAY SORTING 
if(SortFlag){     
    for(int u = 0; u < MembersNumber; u++) { // сортировка пузырком (возрастание . нули в начале) 
        for(int j = 1; j < (MembersNumber - u); j++) {
            if((MembersPr[j] < MembersPr[j - 1])) {
                int prev;
                prev = MembersPr[j-1];
                MembersPr[j-1] = MembersPr[j];
                MembersPr[j] = prev;
            }
        }
    }

  SortFlag = false; 
  RecieveFlag = true; 

  for(int k = 0; k < MembersNumber; k++){ // отладка 
    Serial.print(MembersPr[k]); 
    Serial.print(" "); 
    if(MembersPr[k] == myPriority){
      Me = k; 
    }
  }
  Serial.println();  
}

// MAIN CODE 
  // TRANSMITT
if(TransmittFlag){
  Transmitt_Setup(1); 

  sensorUpdate(); 
  
  for(int i = 0; i < MembersNumber; i++){
    if(MembersPr[i] > 0){
     if(Me == MembersNumber - 1){
        if(MembersPr[i] != 1 && i != Me){
        Transmitt_Loop(myPriority, MembersPr[i]);         
      }
     }
     else {
        if(MembersPr[i] != MembersPr[Me + 1] && i != Me){
          Transmitt_Loop(myPriority, MembersPr[i]); 
        }
     }
    }
    delay(10); 
  }
 if(Me != MembersNumber - 1){
   Transmitt_Loop(myPriority, MembersPr[Me + 1]); // передача следающему, кто начнет вещать (для n-ого это n+1, для последнего элемента в MembersPr это 1-ый)
 }
 else {
  Transmitt_Loop(myPriority, 1);
 }

  TransmittFlag = false; 
  RecieveFlag = true; 
} 

  // RECIEVE
if(RecieveFlag){
  AlarmCount = 0;   
  while(true){
   Recieve_Setup(1); 
   
   dataFrom[4] = 0; 
    if(Recieve_Loop() != MembersPr[Me - 1]){
      Serial.println(); 
      Serial.print("Данные пришли от ");
      Serial.println(From);
      for(int j = 0; j < 8; j++){
        Serial.print(dataFrom[j]);
        Serial.print(" "); 
      }
    }
    else { // если приняли от того, после кого должны начать вещать (для n-ого это n-1, для 1-ого последний элемент массива MembersPr)
        Serial.println(); 
        Serial.print("Данные пришли от ");
        Serial.println(From);
        for(int j = 0; j < 8; j++){
          Serial.print(dataFrom[j]);
          Serial.print(" "); 
        }
        
      RecieveFlag = false; 
      TransmittFlag = true;
      break; 
    }

   Alarm(2500); 
 
   delay(1); 
  }
}
  delay(50/2);
}
