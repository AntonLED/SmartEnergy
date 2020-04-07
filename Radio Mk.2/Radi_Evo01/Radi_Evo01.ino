#include <BearSSLHelpers.h>
#include <CertStoreBearSSL.h>д
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

WiFiClientSecure client_s; 

//String ssid = "WiFi-DOM.ru-8773";
//String password = "29d7qmc7df";
//String ssid = "iPhone (Алина)"; 
//String password = "12345678"; 
String ssid = "HVRedmi Note 4";
String password = "3141521848";

String readString;
const char* host = "script.google.com";
const int httpsPort = 443; 
const char* fingerprint = "5F F1 60 31 09 04 3E F2 90 D2 B0 8A 50 38 04 E8 37 6F BC 76";
String GAS_ID = "AKfycbxFDhS34rIIdCrAIbKVHabXCMXV-odPqQnVzBsstaHRIHR6dPE";   // Replace by your GAS service id           !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

RF24 radio(2,15);

byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; //возможные номера труб

/*int data_1Pr_2Pr[5] = {1, 11, 111, 1111, 2}; // массив, хранящий передаваемые данные
int data_2Pr[3];  
int data_7Pr[6];*/ 

byte Ind; // переменная индекса 

boolean IndFlag = false; 
boolean AlarmFlag = true;  
boolean SortFlag = false; 
boolean FirstTransmittFlag = false; 
boolean TransmittFlag = false; 
boolean RecieveFlag = false; 

int MembersNumber = 7; // колчичество модулей в системе

int MembersPr[7]; // массив с приоритетами модулей системы

int AlarmCount = 0; // аварийный счетчик 

int myPriority = 1; 

int dataFrom[8]; // для хранения пришедших данных 
int dataTo[5] = {1, 00, 000, 0000, 2}; // для хранения отсылаемых данных 
int dataSystem[7][6]; // массив-база данных о показаниях участников системы (MembersNumber строк, 6 столбца) ток, напряжение, мощность каждого + еще 3 на второй генератор 

byte E = 0; // счетчик для Export
byte freq = 15; // частота экспорта (каждые freq итераций) 
String whoIs; // для инициализации того, от кого данные в ГТ 

byte teleTo = myPriority; //  совпадает с номером приоритета 
byte teleFrom;  // совпадает с номером приоритета приемника 

int latestFrom; 
int From; // идентификация того, от кого присылается пакет 
int Me; // наше положение в массиве приоритетов 

int noZero = 0; // счетчик тех элементов MembersPr, которые не ноль (для того, чтобы работало, когда всего двое в системе)

boolean includeFlag = true;  // флаг для включения в сеть 
char including = 'A'; // переменная для приема от передатчика при первом подкючении к сети и для последующего подключения других 
int includeCount = 0; // просто счетчик для INCLUDING 

Adafruit_INA219 sensor; // активация датчика

  int shuntvoltage = 0; // переменные для показаний с датчика 
  int busvoltage = 0;
  int current_mA = 0;
  int loadvoltage = 0;
  int power_mW = 0;



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

void Cloud_Send(String from, int current, int voltage, int power)
{
  Serial.print("connecting to ");
  Serial.println(host);
  client_s.setInsecure();
  if (!client_s.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  if (client_s.verify(fingerprint, host)) {
  Serial.println("certificate matches");
  } else {
  Serial.println("certificate doesn't match");
  }
  
  String string_current =  String(current, DEC);
  String string_voltage =  String(voltage, DEC);
  String string_power = String(power, DEC); 
  
  String url = "/macros/s/" + GAS_ID + "/exec?from=" + from + "&current=" + string_current + "&voltage=" + string_voltage + "&power=" + string_power;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client_s.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client_s.connected()) {
  String line = client_s.readStringUntil('\n');
  if (line == "\r") {
    Serial.println("headers received");
    break;
  }
  }
  String line = client_s.readStringUntil('\n');
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

void Export(){
  for(int i = 0; i < MembersNumber; i++){
    whoIs = (String)(i + 1) + "_Priority";
    Cloud_Send(whoIs, dataSystem[i][0], dataSystem[i][1], dataSystem[i][2]); 
    delay(1); 
  }
}

void Transmitt_Setup(byte addr){
  radio.openWritingPipe(address[addr]);   //мы - труба 0, открываем канал для передачи данных
  radio.stopListening();  //не слушаем радиоэфир, мы передатчик
}

void Transmitt_Loop(byte MyIndex, byte YouIndex){

      dataTo[0] = MyIndex;
      dataTo[4] = YouIndex; 

  AlarmCount = 0; 
  while(true){    
      radio.openWritingPipe(address[1]); 
      radio.stopListening();  //не слушаем радиоэфир, мы передатчик
      
      radio.write(&dataTo, sizeof(dataTo), true);
            
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
        
       Alarm(100); // считаем попытки достукивания 
       
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

        AlarmCount = 0 ;

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

void Indexing(){ 
   Ind = 2; 
   while(Ind <= MembersNumber){
    // классическая отправка данных
      radio.openWritingPipe(address[3]); 
      radio.stopListening();  
      
      radio.write(&Ind, sizeof(Ind), true);
            
      radio.openReadingPipe(1, address[4]);    
      radio.startListening();  
      
        if(radio.available()){
          radio.read(&teleFrom, sizeof(teleFrom)); 
          if(teleFrom == Ind){
            MembersPr[Ind - 1] = teleFrom; 
            Ind++; 
            radio.closeReadingPipe(1); 
          }
        }
        delay(15);
      // если кто-то не отвечает, то идем дальше, т.е. на другой приоритет (следующий)
        AlarmCount ++; 
        if(AlarmCount >= 33){
          Ind++; 
          AlarmCount = 0;
        }
      } 
   delay(1); 
}

void Alarm(int limit){  // фнкция достукивания (limit - предельное число попыток) 
  AlarmCount++; 
  
  if(AlarmCount >= limit){
    ESP.reset(); 
  }
}

void ToInclude(){
  Transmitt_Setup(0); 

    if(radio.write(&including, sizeof(including))){ // если кого-то нашли, то ждем, пока все удйт в ресет (2.5 сек) и сами перезагружаемся 
      radio.startListening();  //начинаем слушать эфир, мы приёмный модуль
     // delay(1000); 
      ESP.reset(); 
    }   
    
  Transmitt_Setup(1); 
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
// TO INCUDING 
if(includeFlag){
  Transmitt_Setup(0); 

  while(true){
    if(radio.write(&including, sizeof(including))){
      includeFlag = false; 
      IndFlag = true;
      break; 
    }
    delay(1); 
  }
}

// INDEXATION
if(IndFlag){
  Transmitt_Setup(3); 
  
  Indexing(); 
                
  IndFlag = false;  
  SortFlag = true; 
 }
// ARRAY SORTING 
if(SortFlag){  
  MembersPr[0] = 1; // самообъявление первого (себя) 
   
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
  FirstTransmittFlag = true; 

  for(int k = 0; k < MembersNumber; k++){ // собственное положение в массиве приоритетов
    Serial.print(MembersPr[k]); 
    Serial.print(" "); 
    if(MembersPr[k] == 1){ 
      Me = k; 
    }
  }
  Serial.println();  
  Serial.println(Me);  
}

// FIRST TRANSMITT 
if(FirstTransmittFlag){
  Transmitt_Setup(1); 

  delay(100);

  radio.write(&MembersPr, sizeof(MembersPr), true);

  FirstTransmittFlag = false;
  TransmittFlag = true; 
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
        Transmitt_Loop(teleTo, MembersPr[i]); 
      }
     }
     else {
        if(MembersPr[i] != MembersPr[Me + 1] && i != Me){
          Transmitt_Loop(teleTo, MembersPr[i]); 
        }
     }
    }
    delay(10); 
  }
  
 if(Me != MembersNumber - 1){
   Transmitt_Loop(teleTo, MembersPr[Me + 1]); // передача следающему, кто начнет вещать (для n-ого это n+1, для последнего элемента в MembersPr это 1-ый)
 }
 else {
  Transmitt_Loop(teleTo, 1);
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
    if(Recieve_Loop() != MembersPr[MembersNumber - 1]){
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
  if(E++ >= freq){
    E = 0; 
    Cloud_Setup(); 
    Export(); 
  }
  delay(50/2); 
}
