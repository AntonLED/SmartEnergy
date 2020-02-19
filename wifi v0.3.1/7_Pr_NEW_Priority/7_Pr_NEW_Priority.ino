
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
#include <EEPROM.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

#define PARSE_AMOUNT 4  // число значений в массиве, который хотим получить
#define PARSE_AMOUNT_GEN 7
#define RELAY_PIN 12
#define MAXSLOT_EEPROM 2048

WiFiServer server(80); 
WiFiClient client; 
WiFiClientSecure client_secure; 

IPAddress ip(192, 168, 4, 11); 
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

IPAddress ip_null(0, 0, 0, 0); 
IPAddress gateway_null(0, 0, 0, 0); 
IPAddress subnet_null(0, 0, 0, 0); 

String IP = "192.168.4.1";  // IP сервера 

String SSID_1Pr = "MG1p"; 
String PASSWORD_1Pr = "12345678901p"; 
String SSID_2Pr = "MG2p"; 
String PASSWORD_2Pr = "12345678902p"; 
String SSID_3Pr = "MG3p";
String PASSWORD_3Pr = "12345678903p";
String SSID_4Pr = "MG4p";
String PASSWORD_4Pr = "12345678904p";
String SSID_5Pr = "MG5p";
String PASSWORD_5Pr = "12345678905p";
String SSID_Bat = "MGBat"; 
String PASSWORD_Bat = "1234567890Bat";
String ssid = "WiFi-DOM.RU-7630";
String password = "JVLuSoUA";
//String ssid = "Redmi 777";
//String password = "77777777";  
//String ssid = "WiFi-DOM.ru-8773";
//String password = "29d7qmc7df";
//String ssid = "iPhone (Алина)";
//String password = "12345678";
//String ssid = "Masha Naumova";
//String password = "masha1500";

boolean startClient_1Pr;// флаг для единичного запуска setup клиента для подключения к серверу 1-ого приоритета 
boolean startClient_2Pr = true; // флаг для единичного запуска setup клиента для подключения к серверу 2-ого приоритета 
boolean startClient_3Pr = true; // флаг для единичного запуска setup клиента для подключения к серверу 3-его приоритета 
boolean startClient_4Pr = true;//  флаг для единичного запуска setup сервера 3-ого приоритета 
boolean startClient_5Pr = true; 
boolean startClient_Bat = true; 
boolean startServer_Gen = true; 
boolean Server_Gen_flag; // флаг на добро для понятия сервера 

boolean recievedFlag;
boolean getStarted;
byte ind;
String string_convert = "";

int data_1Pr[4]; int data_2Pr[4]; int data_3Pr[4]; int data_4Pr[4]; int data_5Pr[4];
int data_Bat[4]; int data_Gen_1[4]; int data_Gen_2[4]; int data_Gen[7];// data_GEn[7] для приоритетсности
Adafruit_INA219 inagen1; 
Adafruit_INA219 inagen2;
  int shuntvoltage1 = 0;
  int busvoltage1 = 0;
  int current_mA1 = 0;
  int loadvoltage1 = 0;
  int power_mW1 = 0;
  
  int shuntvoltage2 = 0;
  int busvoltage2 = 0;
  int current_mA2 = 0;
  int loadvoltage2 = 0;
  int power_mW2 = 0;

//EEPROM
int written_addr;
int outdata[4];
int id_7Pr = 7;
//get_serial.EEPROM
int show_m[4];
int serialRead_var = 0;

int id_Gen_1 = 7; int Current_Gen1; int Voltage_Gen1; int Power_Gen1; //нужно id-генераторра давать соответствующий глобальному идетнтификатору 
int id_Gen_2 = 8; int Current_Gen2; int Voltage_Gen2; int Power_Gen2;

String readString;
const char* host = "script.google.com";
const int httpsPort = 443;
const char* fingerprint = "5F F1 60 31 09 04 3E F2 90 D2 B0 8A 50 38 04 E8 37 9F BC 76";
String GAS_ID = "AKfycbzB256OcwgkvCPZ_KBmHuvp5WbUznJeBnUvDi_ZYdG-JawiA_Vv";
void Client_ToPr_Setup(String ssid, String password) {
  WiFi.mode(WIFI_STA); //режим клиента
  WiFi.disconnect();
  delay(100);
  WiFi.config(ip, gateway, subnet); 
  WiFi.begin(ssid, password); // подключаение к точке доступа сервера 
  while(WiFi.status() != WL_CONNECTED){ // пока не подключились, крутимся в цикле 
     delay(1); 
  }
  Serial.println();
  Serial.println("Client connected to WiFi"); 
  client.connect(IP, 80);
  Serial.println("Client connected to server");
  Serial.println("Client IP: "); 
  Serial.println(WiFi.localIP()); // IP клиента в подклбючении к серверу 
}
void Server_Gen_Setup(){
  WiFi.mode(WIFI_AP); 
  WiFi.softAP("MGGen", "1234567890Gen"); 
  server.begin(); 
  IPAddress IP = WiFi.softAPIP(); 
  Serial.println(); 
  Serial.print("Server IP is: "); 
  Serial.println(IP);
}
void parsing_1Pr() {
 while(client.available() > 0) {
 char incomingByte = client.read();   // обязательно ЧИТАЕМ входящий символ
 //Serial.println(incomingByte); 
 if (getStarted) {                         // если приняли начальный символ (парсинг разрешён)
  if (incomingByte != ' ' && incomingByte != 'B') {   // если это не пробел И не конец
  string_convert += incomingByte;       // складываем в строку
  } 
  else {                               
  data_1Pr[ind] = string_convert.toInt();  // преобразуем строку в int и кладём в массив
  string_convert = "";                  // очищаем строку
  ind++;                              // переходим к парсингу следующего элемента массива
  }
}
  if (incomingByte == 'A') {
  getStarted = true;                      // поднимаем флаг, что можно парсить
  ind = 0;                              // сбрасываем индекс
  string_convert = "";                    // очищаем строку
  }
  if(incomingByte == 'B'){
    getStarted = false; 
    recievedFlag = true; 
  }
 } 
}
void parsing_2Pr() {
 while(client.available() > 0) {
 char incomingByte = client.read();   // обязательно ЧИТАЕМ входящий символ
 //Serial.println(incomingByte); 
 if (getStarted) {                         // если приняли начальный символ (парсинг разрешён)
  if (incomingByte != ' ' && incomingByte != 'C') {   // если это не пробел И не конец
  string_convert += incomingByte;       // складываем в строку
  } 
  else {                                
  data_2Pr[ind] = string_convert.toInt();  // преобразуем строку в int и кладём в массив
  string_convert = "";                  // очищаем строку
  ind++;                              // переходим к парсингу следующего элемента массива
  }
}
  if (incomingByte == 'B') {     
  getStarted = true;                      // поднимаем флаг, что можно парсить
  ind = 0;                              // сбрасываем индекс
  string_convert = "";                    // очищаем строку
  }
  if(incomingByte == 'C'){
    getStarted = false; 
    recievedFlag = true;
  }
 } 
}
void parsing_3Pr() {
 while(client.available() > 0) {
 char incomingByte = client.read();   // обязательно ЧИТАЕМ входящий символ
 //Serial.println(incomingByte); 
 if (getStarted) {                         // если приняли начальный символ (парсинг разрешён)
  if (incomingByte != ' ' && incomingByte != 'D') {   // если это не пробел И не конец
  string_convert += incomingByte;       // складываем в строку
  } 
  else {                                
  data_3Pr[ind] = string_convert.toInt();  // преобразуем строку в int и кладём в массив
  string_convert = "";                  // очищаем строку
  ind++;                              // переходим к парсингу следующего элемента массива
  }
}
  if (incomingByte == 'C') {     
  getStarted = true;                      // поднимаем флаг, что можно парсить
  ind = 0;                              // сбрасываем индекс
  string_convert = "";                    // очищаем строку
  }
  if(incomingByte == 'D'){
    getStarted = false; 
    recievedFlag = true;
  }
 } 
}
void parsing_4Pr() {
 while(client.available() > 0) {
 char incomingByte = client.read();   // обязательно ЧИТАЕМ входящий символ
 //Serial.println(incomingByte); 
 if (getStarted) {                         // если приняли начальный символ (парсинг разрешён)
  if (incomingByte != ' ' && incomingByte != 'E') {   // если это не пробел И не конец
  string_convert += incomingByte;       // складываем в строку
  } 
  else {                                
  data_4Pr[ind] = string_convert.toInt();  // преобразуем строку в int и кладём в массив
  string_convert = "";                  // очищаем строку
  ind++;                              // переходим к парсингу следующего элемента массива
  }
}
  if (incomingByte == 'D') {     
  getStarted = true;                      // поднимаем флаг, что можно парсить
  ind = 0;                              // сбрасываем индекс
  string_convert = "";                    // очищаем строку
  }
  if(incomingByte == 'E'){
    getStarted = false; 
    recievedFlag = true;
  }
 } 
}
void parsing_5Pr() {
 while(client.available() > 0) {
 char incomingByte = client.read();   // обязательно ЧИТАЕМ входящий символ
 //Serial.println(incomingByte); 
 if (getStarted) {                         // если приняли начальный символ (парсинг разрешён)
  if (incomingByte != ' ' && incomingByte != 'F') {   // если это не пробел И не конец
  string_convert += incomingByte;       // складываем в строку
  } 
  else {                                
  data_5Pr[ind] = string_convert.toInt();  // преобразуем строку в int и кладём в массив
  string_convert = "";                  // очищаем строку
  ind++;                              // переходим к парсингу следующего элемента массива
  }
}
  if (incomingByte == 'E') {     
  getStarted = true;                      // поднимаем флаг, что можно парсить
  ind = 0;                              // сбрасываем индекс
  string_convert = "";                    // очищаем строку
  }
  if(incomingByte == 'F'){
    getStarted = false; 
    recievedFlag = true;
  }
 } 
}
void parsing_Bat() {
 while(client.available() > 0) {
 char incomingByte = client.read();   // обязательно ЧИТАЕМ входящий символ
 //Serial.println(incomingByte); 
 if (getStarted) {                         // если приняли начальный символ (парсинг разрешён)
  if (incomingByte != ' ' && incomingByte != 'G') {   // если это не пробел И не конец
  string_convert += incomingByte;       // складываем в строку
  } 
  else {                                
  data_Bat[ind] = string_convert.toInt();  // преобразуем строку в int и кладём в массив
  string_convert = "";                  // очищаем строку
  ind++;                              // переходим к парсингу следующего элемента массива
  }
}
  if (incomingByte == 'F') {     
  getStarted = true;                      // поднимаем флаг, что можно парсить
  ind = 0;                              // сбрасываем индекс
  string_convert = "";                    // очищаем строку
  }
  if(incomingByte == 'G'){
    getStarted = false; 
    recievedFlag = true;
    Server_Gen_flag = true;  
  }
 } 
}
void Cloud_Setup() {
  WiFi.config(ip_null, gateway_null, subnet_null); 
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

  client_secure.print(String("GET ") + "/macros/s/AKfycbzB256OcwgkvCPZ_KBmHuvp5WbUznJeBnUvDi_ZYdG-JawiA_Vv" + url + " HTTP/1.1\r\n" +
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
void Cloud_Send(String fro, int cap) {
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
  String string_capacity =  String(cap, DEC);

  String url = "/exec?from=" + fro + "&capacity=" + string_capacity;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client_secure.print(String("GET ") + "/macros/s/AKfycbzB256OcwgkvCPZ_KBmHuvp5WbUznJeBnUvDi_ZYdG-JawiA_Vv" + url + " HTTP/1.1\r\n" +
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
void sensor_update() {
///////////////Считывание с первого генератора//////////////// 
  shuntvoltage1 = inagen1.getShuntVoltage_mV();
  busvoltage1= inagen1.getBusVoltage_V();
  current_mA1 = inagen1.getCurrent_mA();
  power_mW1 = inagen1.getPower_mW();
  loadvoltage1 = busvoltage1 + (shuntvoltage1 / 1000);
////////////////Считывание со второго генератора////////////
  shuntvoltage2 = inagen2.getShuntVoltage_mV();
  busvoltage2 = inagen2.getBusVoltage_V();
  current_mA2 = inagen2.getCurrent_mA();
  power_mW2 = inagen2.getPower_mW();
  loadvoltage2 = busvoltage2 + (shuntvoltage2 / 1000);
  int power_gen = power_mW1; //Суммирование мощности с двух генераторов
  data_Gen[0] = current_mA1; //сила тока на 1-ом генераторе
  data_Gen[1] = loadvoltage1; //напряжение на 1-ом генераторе
  data_Gen[2] = power_mW1; // мощность на 1-ом генераторе
  data_Gen[3] = current_mA2; //сила тока на 2-ом генераторе
  data_Gen[4] = loadvoltage2; //напряжение на 2-ом генераторе
  data_Gen[5] = power_mW2; // мощность на 2-ом генераторе 
  data_Gen[6] = power_gen; //общая мощность генераторов
}
void get_serial_EEPROM() {
  int show_addr = 2;
  if (Serial.available() > 0) {
    serialRead_var = Serial.read();
    if (serialRead_var == 'R') {
      while (show_addr < 2048) {
        for (int r = 2; r < 2048; r = r + 8) {
          EEPROM.get(show_addr, show_m); EEPROM.commit ();
          show_addr = r + 8;
          Serial.print(r); Serial.print(" -- ");
          for (int i = 0; i < 4; i++) {
            Serial.print(show_m[i]); Serial.print(" ");
          }
          Serial.println(" ");
        }
      }
    }
  }
}
void restart_EEPROM () {
  if (written_addr > MAXSLOT_EEPROM) {
    int written_addr = 2;
    EEPROM.put(0, written_addr); EEPROM.commit ();
  }
}
void thewritten_addr () {
  EEPROM.get(0, written_addr); EEPROM.commit ();
     if (written_addr < 2 ) {
      written_addr = 2;
     }
}

void setup(){ 
   Serial.begin(115200); 
   inagen1.begin(); 
   inagen2.begin();
   pinMode(RELAY_PIN, OUTPUT);  
   EEPROM.begin(2048); 
}

void loop(){
// клиент для подключения к 1-ому приоритету 
    if(startClient_1Pr == false){
        Client_ToPr_Setup(SSID_1Pr, PASSWORD_1Pr);
        startClient_1Pr = true;
        Serial.println("C1S");  
      }
      while(client.available() == 0){
        delay(1); 
      }
        delay(200); 
          parsing_1Pr(); 
        if(recievedFlag){ 
            recievedFlag = false;
            for(byte i = 0; i < PARSE_AMOUNT; i++) { 
            Serial.print(data_1Pr[i]); Serial.print(" ");
            } 
            Serial.println();
            startClient_2Pr = false;
            client.stop();
      }
      delay(500);
      
  thewritten_addr();
  EEPROM.put(written_addr, data_1Pr); EEPROM.commit ();
  Serial.print("Written adress "); Serial.print(written_addr); Serial.println("\n");
  written_addr = written_addr + 8;
  EEPROM.put(0, written_addr); EEPROM.commit ();
  restart_EEPROM();
   
// клиент для подключения ко 2-ому приоритету       
    if(startClient_2Pr == false){
      Client_ToPr_Setup(SSID_2Pr, PASSWORD_2Pr); 
      startClient_2Pr = true; 
      Serial.println("C2S"); 
    }
    while(client.available() == 0){
      delay(1);
    }
      delay(200);
        parsing_2Pr(); 
      if(recievedFlag){
        recievedFlag = false; 
        for(byte i = 0; i < PARSE_AMOUNT; i++){
          Serial.print(data_2Pr[i]); Serial.print(" "); 
        }
        Serial.println(); 
        startClient_3Pr = false;
        client.stop();
      }
      delay(500); 
      
  thewritten_addr();
  EEPROM.put(written_addr, data_2Pr); EEPROM.commit ();
  Serial.print("Written adress "); Serial.print(written_addr); Serial.println("\n");
  written_addr = written_addr + 8;
  EEPROM.put(0, written_addr); EEPROM.commit ();
  restart_EEPROM();
  
//клиент для подключения к 3-ему приоритету 
    if(startClient_3Pr == false){
      Client_ToPr_Setup(SSID_3Pr, PASSWORD_3Pr); 
      startClient_3Pr = true; 
      Serial.println("C3S"); 
    }
    while(client.available() == 0){
      delay(1);
    }
      delay(200);
        parsing_3Pr(); 
      if(recievedFlag){
        recievedFlag = false; 
        for(byte i = 0; i < PARSE_AMOUNT; i++){
          Serial.print(data_3Pr[i]); Serial.print(" "); 
        }
        Serial.println(); 
        startClient_4Pr = false;
        client.stop();
      }
      delay(500); 
      
  thewritten_addr();
  EEPROM.put(written_addr, data_3Pr); EEPROM.commit ();
  Serial.print("Written adress "); Serial.print(written_addr); Serial.println("\n");
  written_addr = written_addr + 8;
  EEPROM.put(0, written_addr); EEPROM.commit ();
  restart_EEPROM();
  
//клиент для подключения к 4-ому приоритету 
    if(startClient_4Pr == false){
      Client_ToPr_Setup(SSID_4Pr, PASSWORD_4Pr); 
      startClient_4Pr = true; 
      Serial.println("C4S"); 
    }
    while(client.available() == 0){
      delay(1);
    }
      delay(200);
        parsing_4Pr(); 
      if(recievedFlag){
        recievedFlag = false; 
        for(byte i = 0; i < PARSE_AMOUNT; i++){
          Serial.print(data_4Pr[i]); Serial.print(" "); 
        }
        Serial.println(); 
        startClient_5Pr = false;
        client.stop();
      }     
      delay(500);  
      
  thewritten_addr();
  EEPROM.put(written_addr, data_4Pr); EEPROM.commit ();
  Serial.print("Written adress "); Serial.print(written_addr); Serial.println("\n");
  written_addr = written_addr + 8;
  EEPROM.put(0, written_addr); EEPROM.commit ();
  restart_EEPROM();
  
//клиент для подключения к 5-ому приоритету 
    if(startClient_5Pr == false){
      Client_ToPr_Setup(SSID_5Pr, PASSWORD_5Pr); 
      startClient_5Pr = true; 
      Serial.println("C5S"); 
    }
    while(client.available() == 0){
      delay(1);
    }
      delay(200);
        parsing_5Pr(); 
      if(recievedFlag){
        recievedFlag = false; 
        for(byte i = 0; i < PARSE_AMOUNT; i++){
          Serial.print(data_5Pr[i]); Serial.print(" "); 
        }
        Serial.println(); 
        startClient_Bat = false; 
        client.stop();
      }
    delay(500);
    
  thewritten_addr();
  EEPROM.put(written_addr, data_5Pr); EEPROM.commit ();
  Serial.print("Written adress "); Serial.print(written_addr); Serial.println("\n");
  written_addr = written_addr + 8;
  EEPROM.put(0, written_addr); EEPROM.commit ();
  restart_EEPROM();
  
//клиент для подключения к 6-ому приоритету 
    if(startClient_Bat == false){
      Client_ToPr_Setup(SSID_Bat, PASSWORD_Bat); 
      startClient_Bat = true; 
      Serial.println("C6S"); 
    }
    while(client.available() == 0){
      delay(1);
    }
      delay(200);
        parsing_Bat(); 
      if(recievedFlag){
        recievedFlag = false; 
        for(byte i = 0; i < PARSE_AMOUNT; i++){
          Serial.print(data_Bat[i]); Serial.print(" "); 
        }
        Serial.println(); 
        startServer_Gen = false; 
        client.stop();
      }
      delay(500); 
      
  thewritten_addr();
  EEPROM.put(written_addr, data_Bat); EEPROM.commit ();
  Serial.print("Written adress "); Serial.print(written_addr); Serial.println("\n");
  written_addr = written_addr + 8;
  EEPROM.put(0, written_addr); EEPROM.commit ();
  restart_EEPROM();
 
//режим сервера для транслирования информации
  shuntvoltage1 = inagen1.getShuntVoltage_mV();
  busvoltage1= inagen1.getBusVoltage_V();
  current_mA1 = inagen1.getCurrent_mA();
  power_mW1 = inagen1.getPower_mW();
  loadvoltage1 = busvoltage1 + (shuntvoltage1 / 1000);
  
  shuntvoltage2 = inagen2.getShuntVoltage_mV();
  busvoltage2 = inagen2.getBusVoltage_V();
  current_mA2 = inagen2.getCurrent_mA();
  power_mW2 = inagen2.getPower_mW();
  loadvoltage2 = busvoltage2 + (shuntvoltage2 / 1000);

  data_Gen_1[0] = id_Gen_1;
  data_Gen_1[1] = current_mA1; //сила тока на 1-ом генераторе
  data_Gen_1[2] = loadvoltage1; //напряжение на 1-ом генераторе
  data_Gen_1[3] = power_mW1; // мощность на 1-ом генераторе
  data_Gen_2[0] = id_Gen_2;
  data_Gen_2[1] = current_mA2; //сила тока на 2-ом генераторе
  data_Gen_2[2] = loadvoltage2; //напряжение на 2-ом генераторе
  data_Gen_2[3] = power_mW2; // мощность на 2-ом генераторе 
  Serial.print("записываемый объект: ");
      for (int i = 0; i < 4; i++) {
        Serial.print(data_Gen_1[i]); Serial.print(" ");
      }
  Serial.println(" ");
  Serial.print("записываемый объект: ");
      for (int i = 0; i < 4; i++) {
        Serial.print(data_Gen_2[i]); Serial.print(" ");
      }
  Serial.println(" ");
  // запись в еепром
  thewritten_addr();
  EEPROM.put(written_addr, data_Gen_1); EEPROM.commit ();
  Serial.print("Written adress "); Serial.print(written_addr); Serial.println("\n");
  written_addr = written_addr + 8;
  EEPROM.put(0, written_addr); EEPROM.commit ();
  restart_EEPROM();

  thewritten_addr();
  EEPROM.put(written_addr, data_Gen_2); EEPROM.commit ();
  Serial.print("Written adress "); Serial.print(written_addr); Serial.println("\n");
  written_addr = written_addr + 8;
  EEPROM.put(0, written_addr); EEPROM.commit ();
  restart_EEPROM();
  
  get_serial_EEPROM();// функция для вывода по запросу в сериал
  /*Если хотим просмотреть всю базу данных, то нужно здесь вставить функцию,
    которая отключает вывод в ком порт,
    затем влючает его по запросу вкючает*/
  Serial.println(" ");
     if(Server_Gen_flag == true){
       if(startServer_Gen == false){
        Server_Gen_Setup(); 
        startServer_Gen = true; 
        Serial.println("S7S"); 
      }
      for (int x=0; x<6; x++){
      sensor_update();
      delay(500); 
         while(true){
           WiFiClient client = server.available(); // объект клиент
            if(client && client.connected()){   // если есть клиент с непрочитанными данными 
                  client.write('G');    // контрольный символ отправки (кто отпраляет)  
                  for(int i = 0; i < PARSE_AMOUNT_GEN; i++){  // отправка массива данных 
                    client.print(data_Gen[i]);
                    client.print(" "); 
                    Serial.print(data_Gen[i]); 
                    Serial.print(" "); 
                  }
                  client.write('A'); // контрольный символ отправки (кому передать эстафету)
                  break;
            }  
            delay(1); 
         }
       delay(50);
      }
     delay(500);
     Server_Gen_flag = false; 
     startClient_1Pr = false; 
     delay(100);
  }
  
  delay(1000);
  Cloud_Setup();
  Cloud_Send("1_Priority", data_1Pr[0], data_1Pr[1], data_1Pr[2]); delay(1000);
  Cloud_Send("2_Priority", data_2Pr[0], data_2Pr[1], data_2Pr[2]); delay(1000);
  Cloud_Send("3_Priority", data_3Pr[0], data_3Pr[1], data_3Pr[2]); delay(1000);
  Cloud_Send("4_Priority", data_4Pr[0], data_4Pr[1], data_4Pr[2]); delay(1000);
  Cloud_Send("5_Priority", data_5Pr[0], data_5Pr[1], data_5Pr[2]); delay(1000);
  Cloud_Send("Battery", data_Bat[0], data_Bat[1], data_Bat[2]); delay(1000);
  Cloud_Send("Generator1", data_Gen[0], data_Gen[1], data_Gen[2]); delay(1000);
  Cloud_Send("Generator2", data_Gen[3], data_Gen[4], data_Gen[5]); delay(1000);
  Cloud_Send("All_Power", data_Gen[6]);
  WiFi.disconnect();
  delay(1000);
}
