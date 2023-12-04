// Versao 1.8 para aula de 27/11/2023
// PUC Campinas
#include "OV2640.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>

#include <ESP32Servo.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define ENABLE_WEBSERVER

#define I2C_SDA 14
#define I2C_SCL 15
TwoWire I2Cbus = TwoWire(0);

// Display defines
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1
#define SCREEN_ADDRESS  0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Cbus, OLED_RESET);

OV2640 cam;

#ifdef ENABLE_WEBSERVER
WebServer server(80);
WebServer serverCam(81);
#endif

#define botao 0
bool stable;    // Guarda o último estado estável do botão;
bool unstable;  // Guarda o último estado instável do botão;
uint32_t bounce_timer;
uint8_t counter = 0;
uint32_t loop_timer;

#define SERVO_1      12 //2. pino 12 com problema no boot...
#define SERVO_2      13 //13
Servo servoL;
Servo servoR;
int servoLPos = 95;
int servoRPos = 95;

#ifdef ENABLE_WEBSERVER
void handle_jpg_stream(void)
{
    WiFiClient client = serverCam.client();
    String response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
    serverCam.sendContent(response);
    while (1)
    {
        cam.run();
        if (!client.connected())
            break;
        response = "--frame\r\n";
        response += "Content-Type: image/jpeg\r\n\r\n";
        serverCam.sendContent(response);

        client.write((char *)cam.getfb(), cam.getSize());
        serverCam.sendContent("\r\n");
        if (!client.connected())
            break;
        #ifdef ENABLE_WEBSERVER
          server.handleClient();
        #endif
 
    }
}

void handle_jpg(void)
{
    WiFiClient client = server.client();

    cam.run();
    if (!client.connected())
    {
        return;
    }
    String response = "HTTP/1.1 200 OK\r\n";
    response += "Content-disposition: inline; filename=capture.jpg\r\n";
    response += "Content-type: image/jpeg\r\n\r\n";
    server.sendContent(response);
    client.write((char *)cam.getfb(), cam.getSize());
}

void handleNotFound()
{
    String message = "Server is running!\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    server.send(200, "text/plain", message);
}
void handle_carro0() {
  handle_carro(&server, "dd");
}
void handle_carro(WebServer *server, const char *content) {
  
  String message = "Server is running!\n\n";
  message += "URI: ";
  message += server->uri();
  message += "\nMethod: ";
  message += (server->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server->args();
  message += "\n";
  for (uint8_t i = 0; i < server->args(); i++) {
    message += " " + server->argName(i) + ": " + server->arg(i) + "\n";
    if ((server->argName(i) == "R") || (server->argName(i) == "r")){
        int vr = server->arg(i).toInt();//atoi(server->arg(i));
        servoR.write(vr);
        message += "Acionou R \r\n";
    }
    if ((server->argName(i) == "L") || (server->argName(i) == "l")){
        int v = server->arg(i).toInt();//atoi(server->arg(i));
        servoL.write(v);
        message += "Acionou L \r\n";
    }
  }    
  
  server->send(200, "text/plain", message);
}


void handleNotFound_1(WebServer *server) {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server->uri();
  message += "\nMethod: ";
  message += (server->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server->args();
  message += "\n";
  for (uint8_t i = 0; i < server->args(); i++) {
    message += " " + server->argName(i) + ": " + server->arg(i) + "\n";
  }
  server->send(404, "text/plain", message);
}
void handleNotFound0() {
  handleNotFound_1(&serverCam);
}

#endif

void menu(int counter, const char * ssid1, const char * ssid2,const char * ssid3, const char * ssid4){
  display.clearDisplay();
  display.display();      
  delay(100);
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.print("MENU : ");
  display.setCursor(0, 17);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  if (counter == 0){
    display.print("*");
    Serial.println("Casa Selecionado");
  }
  else
    display.print("_");
  // em pixel 
  display.setCursor(30, 17);  
  display.print(ssid1);

  display.setCursor(0, 30);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  if (counter == 1){
    display.print("*");
    Serial.println("SSID 1");
  }
  else
    display.print("_");

  // em pixel 
  display.setCursor(30, 30);
  display.print(ssid2);
  
  display.setCursor(0, 43);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
    if (counter == 2){
    display.print("*");
    Serial.println("SSID 2");
  }
  else
    display.print("_");

  // em pixel 
  display.setCursor(30, 43);
  display.print(ssid3);

  display.setCursor(0, 56);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  if (counter == 3){
    display.print("*");
    Serial.println("Carro PUC selecionaod");
  }
  else
    display.print("_");

  // em pixel 
  display.setCursor(30, 56);
  display.print(ssid4);


  display.display(); 
  
}

void startServo(){
  servoR.setPeriodHertz(50);    // standard 50 hz servo
  servoL.setPeriodHertz(50);    // standard 50 hz servo
 
  //servoN1.attach(SERVO_N1, 1000, 2000);
  //servoN2.attach(SERVO_N2, 1000, 2000);
 // servoN2.attach(SERVO_N3, 1000, 2000);

  servoR.attach(SERVO_1, 1000, 2000);
  servoL.attach(SERVO_2, 1000, 2000);
  
  servoR.write(servoRPos);
  servoL.write(servoLPos);
}

bool changed() {
  bool now = digitalRead(botao);   // Lê o estado atual do botão;
  if (unstable != now) {       // Checa se houve mudança;
    bounce_timer = millis();   // Atualiza timer;
    unstable = now;            // Atualiza estado instável;
  }
  else if (millis() - bounce_timer > 20) {  // Checa o tempo de trepidação acabou;
    if (stable != now) {           // Checa se a mudança ainda persiste;
      stable = now;                  // Atualiza estado estável;
      if (now == false){
        Serial.print(now);
        ++counter;
        if (counter == 4)
          counter = 0;
      }
      return 1;
    }
  }
  return 0;
}



void setup()
{
  
  loop_timer = millis();   

  

  Serial.begin(115200);
  while (!Serial)
    {
        ;
    }
  Serial.println("Iniciando configuração (apertar io0 se quiser mudar");
  pinMode(botao, INPUT_PULLUP);  // Configura pino 8 como entrada e habilita pull up interno;
  stable = digitalRead(botao);

 // Initialize I2C with our defined pins
  I2Cbus.begin(I2C_SDA, I2C_SCL, 100000);
  Serial.println("Initialize display");
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.printf("SSD1306 OLED display failed to initalize.\nCheck that display SDA is connected to pin %d and SCL connected to pin %d\n", I2C_SDA, I2C_SCL);
    while (true);
  }
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.print("Menu");
  display.display();
  Serial.println(loop_timer);
  Serial.println(millis());
  const char* ssid     = "casa";
  const char* ssid2     = "pucc";
  const char* ssid3     = "PUC-ACD"; 
  const char* ssid4a     = "CARRO_PUCC AP"; 
  const char* ssid4     = "CARRO_PUCC";
  const char* password2 = "123456789";
  menu(counter, ssid,ssid2,ssid3,ssid4a);

  while (millis() - loop_timer < 20000){
    if (changed()) {
       //++counter;
      Serial.println(counter);
      menu(counter, ssid,ssid2,ssid3,ssid4a);
    }

  } 
  
  IPAddress ip;
  if (counter == 0){    // LOW apertado High padrao          
    const char* password = "00000000";
    WiFi.begin(ssid, password);
  } else {
    if (counter == 1){    // LOW apertado High padrao          
      const char* password = "12345678";
      WiFi.begin(ssid2, password);
    } else {
      if (counter == 2){    // LOW apertado High padrao          
        const char* password = "";
        WiFi.begin(ssid3, password);
      } else {
        Serial.println("Ponto de acesso");
        WiFi.softAP(ssid4, password2);
        ip = WiFi.softAPIP();
     }
    }  
  }
  int i = 0;
  while ((WiFi.status() != WL_CONNECTED) && (counter != 3) ) {
      delay(300);
      Serial.print(".");
      display.clearDisplay();
      display.display();   
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);  
      display.setCursor(40, 0);    
      switch(i){
        case 0:
          display.print("|");
          break;
        case 1:
          display.print("/");
          break;
        case 2:
          display.print("-");
          break;
        case 3:
          display.print("\\");
          break;
      }
      display.display();   
      i++;
      if (i == 4) i = 0;
  }

  Serial.println("");
  Serial.println("WiFi connected");
  if (counter != 3)
    ip = WiFi.localIP();
  else
    ip = WiFi.softAPIP();
  Serial.println(ip.toString());  
 
  Serial.println(loop_timer);
  Serial.println(millis());
  
  display.clearDisplay();
  display.display();   
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.print("Conectado:");
  display.setCursor(0, 18);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.print(ip.toString());
  display.setCursor(0, 32);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.print("SSID -> ");
  display.setCursor(50, 32);
  switch( counter){
    case 0:
      display.print(ssid);
      break;
    case 1:
      display.print(ssid2);
      break;
    case 2:
      display.print(ssid3);
      break;
    case 3:
      display.print(ssid4);
      display.setCursor(0, 45);
      display.print("pwd : ");
      display.setCursor(40, 45);
      display.print(password2);
      break;      
  }

  display.display();


  cam.init(esp32cam_aithinker_config);//esp32cam_config);

 
#ifdef ENABLE_WEBSERVER
    
    server.on("/jpg", HTTP_GET, handle_jpg);
    server.on("/on", HTTP_GET, handle_carro0);
    server.onNotFound(handleNotFound);
    server.begin();

    serverCam.on("/", HTTP_GET, handle_jpg_stream);
    serverCam.onNotFound(handleNotFound0);
    serverCam.begin();
    
#endif

 
  startServo();

}

void loop()
{
#ifdef ENABLE_WEBSERVER
   server.handleClient();
   serverCam.handleClient();
#endif

}
