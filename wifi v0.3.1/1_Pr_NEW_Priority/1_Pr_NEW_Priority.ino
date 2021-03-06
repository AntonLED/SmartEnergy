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
#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

#define PARSE_AMOUNT 4         // число значений в массиве, который хотим получить
#define PARSE_AMOUNT_GEN 9
#define RELAY_PIN 12
#define MAXSLOT_EEPROM 2048

WiFiServer server(80);
WiFiClient client;
WiFiClientSecure client_secure;

IPAddress ip(192, 168, 4, 5);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

IPAddress ip_null(0, 0, 0, 0);
IPAddress gateway_null(0, 0, 0, 0);
IPAddress subnet_null(0, 0, 0, 0);

String IP = "192.168.4.1";  // IP сервера

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
String SSID_Gen = "MGGen";
String PASSWORD_Gen = "1234567890Gen";
String ssid = "WiFi-DOM.RU-7630";
String password = "JVLuSoUA";
//String ssid = "Masha Naumova";
//String password = "masha1500";
//String ssid = "WiFi-DOM.ru-8773";
//String password = "29d7qmc7df";

boolean startServer_1Pr; // флаг для единичного выполнения setup сервера
boolean startClient_2Pr = true; // флаг для единичного выполнения setup клинта для подключения к серверу 2-ого приоритета
boolean startClient_3Pr = true; // флаг для единичного выполнения setup клинта для подключения к серверу 3-его приоритета
boolean startClient_4Pr = true; // флаг для флаг для единичного выполнения setup клинта для подключения к серверу 4-ого приоритета
boolean startClient_5Pr = true;
boolean startClient_Bat = true;
boolean startClient_Gen = true;
boolean Server_1Pr_flag;  // флаг на добро становления сервером

boolean startFlag;

boolean recievedFlag;
boolean getStarted;
byte ind;
String string_convert = "";

String readString;
const char* host = "script.google.com";
const int httpsPort = 443;
const char* fingerprint = "5F F1 60 31 09 04 3E F2 90 D2 B0 8A 50 38 04 E8 37 9F BC 76";
String GAS_ID = "AKfycbxVsMvLKf0GXOhhUgag-wcHKVFy8m1jU1inqRzwVMfMh4Uu5nUl";

int data_1Pr[4]; int data_2Pr[4]; int data_3Pr[4]; int data_4Pr[4]; int data_5Pr[4];
int data_Bat[4]; int data_Gen_1[4]; int data_Gen_2[4]; int data_Gen[7];
Adafruit_INA219 ina1;
int shuntvoltage1 = 0;
int busvoltage1 = 0;
int current_mA1 = 0;
int power_mW1 = 0;
int loadvoltage1 = 0;
//EEPROM
int written_addr;
int outdata[4];
int id_1Pr = 1;
//get_serial.EEPROM
int show_m[4];
int serialRead_var = 0;

int id_Gen_1 = 7; int Current_Gen1; int Voltage_Gen1; int Power_Gen1; //нужно id-генераторра давать соответствующий глобальному идетнтификатору 
int id_Gen_2 = 8; int Current_Gen2; int Voltage_Gen2; int Power_Gen2;

void Server_1Pr_Setup() { // функция setup сервера для 1-ого приоритета
  WiFi.mode(WIFI_AP);
  WiFi.softAP("MG1p", "12345678901p");
  server.begin();
  IPAddress IP = WiFi.softAPIP();
  Serial.println();
  Serial.print("Server IP is: ");
  Serial.println(IP);
}
void Client_ToPr_Setup(String ssid, String password) {
  WiFi.mode(WIFI_STA); //режим клиента
  WiFi.disconnect();
  delay(100);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password); // подключаение к точке доступа сервера
  while (WiFi.status() != WL_CONNECTED) { // пока не подключились, крутимся в цикле
    delay(1);
  }
  Serial.println();
  Serial.println("Client connected to WiFi");
  client.connect(IP, 80);
  Serial.println("Client connected to server");
  Serial.println("Client IP: ");
  Serial.println(WiFi.localIP()); // IP клиента в подклбючении к серверу
}
void parsing_2Pr() {
  while (client.available() > 0) {
    char incomingByte = client.read();
    //Serial.println(incomingByte);
    if (getStarted) {                         // если приняли начальный символ (парсинг разрешён)
      if (incomingByte != ' ' && incomingByte != 'C') {
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
    if (incomingByte == 'C') {
      getStarted = false;
      recievedFlag = true;
    }
  }
}
void parsing_3Pr() {
  while (client.available() > 0) {
    char incomingByte = client.read();   // обязательно ЧИТАЕМ входящий символ
    //Serial.println(incomingByte);
    if (getStarted) {
      if (incomingByte != ' ' && incomingByte != 'D') {
        string_convert += incomingByte;
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
    if (incomingByte == 'D') {
      getStarted = false;
      recievedFlag = true;
    }
  }
}
void parsing_4Pr() {
  while (client.available() > 0) {
    char incomingByte = client.read();   // обязательно ЧИТАЕМ входящий символ
    //Serial.println(incomingByte);
    if (getStarted) {                         // если приняли начальный символ (парсинг разрешён)
      if (incomingByte != ' ' && incomingByte != 'E') {
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
    if (incomingByte == 'E') {
      getStarted = false;
      recievedFlag = true;
    }
  }
}
void parsing_5Pr() {
  while (client.available() > 0) {
    char incomingByte = client.read();   // обязательно ЧИТАЕМ входящий символ
    //Serial.println(incomingByte);
    if (getStarted) {                         // если приняли начальный символ (парсинг разрешён)
      if (incomingByte != ' ' && incomingByte != 'F') {
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
    if (incomingByte == 'F') {
      getStarted = false;
      recievedFlag = true;
    }
  }
}
void parsing_Bat() {
  while (client.available() > 0) {
    char incomingByte = client.read();   // обязательно ЧИТАЕМ входящий символ
    //Serial.println(incomingByte);
    if (getStarted) {                         // если приняли начальный символ (парсинг разрешён)
      if (incomingByte != ' ' && incomingByte != 'G') {
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
    if (incomingByte == 'G') {
      getStarted = false;
      recievedFlag = true;
    }
  }
}
void parsing_Gen() {
  while (client.available() > 0) {
    char incomingByte = client.read();   // обязательно ЧИТАЕМ входящий символ
    //Serial.println(incomingByte);
    if (getStarted) {                         // если приняли начальный символ (парсинг разрешён)
      if (incomingByte != ' ' && incomingByte != 'A') {
        string_convert += incomingByte;       // складываем в строку
      }
      else {
        data_Gen[ind] = string_convert.toInt();  // преобразуем строку в int и кладём в массив
        string_convert = "";                  // очищаем строку
        ind++;                              // переходим к парсингу следующего элемента массива
      }
    }
    if (incomingByte == 'G') {
      getStarted = true;                      // поднимаем флаг, что можно парсить
      ind = 0;                              // сбрасываем индекс
      string_convert = "";                    // очищаем строку
    }
    if (incomingByte == 'A') {
      getStarted = false;
      recievedFlag = true;
      Server_1Pr_flag = true;
    }
  }
}
void Cloud_Setup() {
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.config(ip_null, gateway_null, subnet_null);
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

  client_secure.print(String("GET ") + "/macros/s/AKfycbxVsMvLKf0GXOhhUgag-wcHKVFy8m1jU1inqRzwVMfMh4Uu5nUl" + url + " HTTP/1.1\r\n" +
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

  client_secure.print(String("GET ") + "/macros/s/AKfycbxVsMvLKf0GXOhhUgag-wcHKVFy8m1jU1inqRzwVMfMh4Uu5nUl" + url + " HTTP/1.1\r\n" +
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
//eeprom
void sensor_update() {
  shuntvoltage1 = ina1.getShuntVoltage_mV();
  busvoltage1 = ina1.getBusVoltage_V();
  current_mA1 = ina1.getCurrent_mA();
  power_mW1 = ina1.getPower_mW();
  loadvoltage1 = busvoltage1 + (shuntvoltage1 / 1000);
  
  data_1Pr[0] = id_1Pr;
  data_1Pr[1] = current_mA1; 
  data_1Pr[2] = loadvoltage1; 
  data_1Pr[3] = power_mW1;
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

void setup() {
  Serial.begin(115200);
  ina1.begin();
  pinMode(RELAY_PIN, OUTPUT);
  EEPROM.begin(2048);
}

void loop() {
//режим сервера для транслирования данных
  sensor_update();// функция снятия характреистик тока
  Serial.print("записываемый объект: ");
      for (int i = 0; i < 4; i++) {
        Serial.print(data_1Pr[i]); Serial.print(" ");
      }
  Serial.println(" ");
  // запись в еепром
  thewritten_addr();
  EEPROM.put(written_addr, data_1Pr); EEPROM.commit ();
  Serial.print("Written adress "); Serial.print(written_addr); Serial.println("\n");
  written_addr = written_addr + 8;
  EEPROM.put(0, written_addr); EEPROM.commit ();
  restart_EEPROM();
  
  get_serial_EEPROM();// функция для вывода по запросу в сериал
  /*Если хотим просмотреть всю базу данных, то нужно здесь вставить функцию,
    которая отключает вывод в ком порт,
    затем влючает его по запросу вкючает*/
  Serial.println(" ");

  if (Server_1Pr_flag == true || startFlag == false) {
    if (startServer_1Pr == false) {
      Server_1Pr_Setup();
      startServer_1Pr = true;
      Serial.println("S1S");
    }
    for (int x=0; x<6; x++){
    sensor_update();
    delay(500); //ПОДОБРАТЬ. БЫЛО 500 В КОНЦЕ ЦИКЛА ТОЖЕ
        while (true) {
          WiFiClient client = server.available(); // объект клиент
          if (client && client.connected()) { // если есть клиент с непрочитанными данными
            client.write('A');    // контрольный символ отправки (кто отпраляет)
            for (int i = 0; i < PARSE_AMOUNT; i++) { // отправка массива данных
              client.print(data_1Pr[i]);
              client.print(" ");
              Serial.print(data_1Pr[i]);
              Serial.print(" ");
            }
            client.write('B'); // контрольный символ отправки (кому передать эстафету)
            break;
          }
          delay(1);
          delay(50);
        }
    }
    delay(500);
    Serial.println();
    Server_1Pr_flag = false;
    startFlag = true;
    startClient_2Pr = false;
    delay(100);
  }
 
//режим клиента для подключения к серверу 2-ого приоритета
  if (startClient_2Pr == false) {
    Client_ToPr_Setup(SSID_2Pr, PASSWORD_2Pr);
    startClient_2Pr = true;
    Serial.println("C2S ");
  }
  while (client.available() == 0) {
    delay(1);
  }
  delay(200);
  parsing_2Pr(); // функция парсинга
  if (recievedFlag) {   // если получены данные
    recievedFlag = false;
    for (byte i = 0; i < PARSE_AMOUNT; i++) { // выводим элементы массива
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
  
//режим клиенат для поключению к серверу 3-его приоритета
  if (startClient_3Pr == false) {
    Client_ToPr_Setup(SSID_3Pr, PASSWORD_3Pr);
    startClient_3Pr = true;
    Serial.println("C3S");
  }
  while (client.available() == 0) {
    delay(1);
  }
  delay(200);
  parsing_3Pr(); // функция парсинга
  if (recievedFlag) {   // если получены данные
    recievedFlag = false;
    for (byte i = 0; i < PARSE_AMOUNT; i++) { // выводим элементы массива
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
   
  // режим клиента для подключения к серверу 4-ого приоритета
  if (startClient_4Pr == false) {
    Client_ToPr_Setup(SSID_4Pr, PASSWORD_4Pr);
    startClient_4Pr = true;
    Serial.println("C4S");
  }
  while (client.available() == 0) {
    delay(1);
  }
  delay(200);
  parsing_4Pr(); // функция парсинга
  if (recievedFlag) {   // если получены данные
    recievedFlag = false;
    for (byte i = 0; i < PARSE_AMOUNT; i++) { // выводим элементы массива
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
   
  // режим клиента для подключения к серверу 5-ого приоритета
  if (startClient_5Pr == false) {
    Client_ToPr_Setup(SSID_5Pr, PASSWORD_5Pr);
    startClient_5Pr = true;
    Serial.println("C5S");
  }
  while (client.available() == 0) {
    delay(1);
  }
  delay(200);
  parsing_5Pr(); // функция парсинга
  if (recievedFlag) {   // если получены данные
    recievedFlag = false;
    for (byte i = 0; i < PARSE_AMOUNT; i++) { // выводим элементы массива
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
   
  // режим клиента для подключения к серверу 6-ого приоритета
  if (startClient_Bat == false) {
    Client_ToPr_Setup(SSID_Bat, PASSWORD_Bat);
    startClient_Bat = true;
    Serial.println("C6S");
  }
  while (client.available() == 0) {
    delay(1);
  }
  delay(200);
  parsing_Bat(); // функция парсинга
  if (recievedFlag) {   // если получены данные
    recievedFlag = false;
    for (byte i = 0; i < PARSE_AMOUNT; i++) { // выводим элементы массива
      Serial.print(data_Bat[i]); Serial.print(" ");
    }
    Serial.println();
    startClient_Gen = false;
    client.stop();
  }
  delay(500);

   thewritten_addr();
  EEPROM.put(written_addr, data_Bat); EEPROM.commit ();
  Serial.print("Written adress "); Serial.print(written_addr); Serial.println("\n");
  written_addr = written_addr + 8;
  EEPROM.put(0, written_addr); EEPROM.commit ();
  restart_EEPROM();
   
  // режим клиента для подключения к серверу 7-ого приоритета
  if (startClient_Gen == false) {
    Client_ToPr_Setup(SSID_Gen, PASSWORD_Gen);
    startClient_Gen = true;
    Serial.println("C7S");
  }
  while (client.available() == 0) {
    delay(1);
  }
  delay(200);
  parsing_Gen(); // функция парсинга
  if (recievedFlag) {   // если получены данные
    recievedFlag = false;
    for (byte i = 0; i < PARSE_AMOUNT_GEN; i++) { // выводим элементы массива
      Serial.print(data_Gen[i]); Serial.print(" ");
    }
    //распаковка массива
    int Current_Gen1 = data_Gen[0];  //миллиаперы
    int Voltage_Gen1 = data_Gen[1];  // вольты
    int Power_Gen1 = data_Gen[2];
    int Current_Gen2 = data_Gen[3];  // миллиасперы
    int Voltage_Gen2 = data_Gen[4];  // вольты
    int Power_Gen2 = data_Gen[5];
    int allPower = data_Gen[6];      // милливатты


    if (allPower >= 2180) {
      digitalWrite(RELAY_PIN, LOW);
    }
    else {
      digitalWrite(RELAY_PIN, HIGH);
    }
    Serial.println();
    startServer_1Pr = false;
    client.stop();
  }

data_Gen_1[0] = id_Gen_1;
data_Gen_1[1] = Current_Gen1;
data_Gen_1[2] = Voltage_Gen1;
data_Gen_1[3] = Power_Gen1;

data_Gen_2[0] = id_Gen_2;
data_Gen_2[1] = Current_Gen2;
data_Gen_2[2] = Voltage_Gen2;
data_Gen_2[3] = Power_Gen2;

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
