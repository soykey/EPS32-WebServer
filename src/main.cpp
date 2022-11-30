#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

const char ssid[] = "AirRobot";
const char pass[] = "nogami1103";
const IPAddress ip(192,168,4,1);
const IPAddress subnet(255,255,255,0);
AsyncWebServer server(80);

#define rd 26
#define rs 25
#define ld 33
#define ls 32

#define grs 1
#define brs 0

#define gls 1
#define bls 0

bool rf = grs;
bool lf = gls;

int _on(int num) {
  digitalWrite(num,HIGH);
  Serial.println(num);
  return 0;
}
int _off(int num) {
  digitalWrite(num,LOW);
  Serial.println(num);
  return 0;
}

int rssw(void) {
  if (rf==grs)
  {
    rf = brs;
    _on(rs);
  } else {
    rf = grs;
    _off(rs);
  }
  return 0;
}

int lssw(void) {
  if (lf==gls)
  {
    lf = bls;
    _off(ls);
  } else {
    lf = gls;
    _on(ls);
  }
  return 0;
}

void setup()
{
  Serial.begin(115200);

  pinMode(rd,OUTPUT);
  pinMode(rs,OUTPUT);
  pinMode(ld,OUTPUT);
  pinMode(ls,OUTPUT);

  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  
  WiFi.softAP(ssid, pass);
  delay(100);
  WiFi.softAPConfig(ip, ip, subnet);
  
  IPAddress myIP = WiFi.softAPIP();

  delay(1000);

  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/favicon.ico", "text/css");
  });

  server.on("/straight", HTTP_GET, [](AsyncWebServerRequest *request){
    _on(rd);
    _on(ld);
  });

  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    delay(100);
    _off(rd);
    _off(ld);
  });

  server.on("/right", HTTP_GET, [](AsyncWebServerRequest *request){
    rf=1;
    lf=1;
    _off(rs);
    delay(100);
    _off(ls);
    delay(100);
    _off(rd);
    _on(ld);
  });

  server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request){
    rf=1;
    lf=1;
    _off(rs);
    delay(100);
    _off(ls);
    delay(100);
    _on(rd);
    _off(ld);
  });

  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200);
    delay(100);
    _off(rd);
    _off(rs);
  });

  //コントローラー向け
  server.on("/rdon", HTTP_GET, [](AsyncWebServerRequest *request){
    _on(rd);
  });

  server.on("/rdoff", HTTP_GET, [](AsyncWebServerRequest *request){
    _off(rd);
  });

  server.on("/ldon", HTTP_GET, [](AsyncWebServerRequest *request){
    _on(ld);
  });

  server.on("/ldoff", HTTP_GET, [](AsyncWebServerRequest *request){
    _off(ld);
  });

  server.on("/rs", HTTP_GET, [](AsyncWebServerRequest *request){
    rssw();
  });

  server.on("/ls", HTTP_GET, [](AsyncWebServerRequest *request){
    lssw();
  });


  server.begin();
  delay(1000);
  Serial.println("Server start!");
}

void loop() {
  if(Serial.available()>0){
    String data = Serial.readStringUntil('\n');
    
    //直進
    if (data=="g") {
      if (rf!=grs)
      {
        rssw();
      }
      if (lf!=grs)
      {
        lssw();
      }
      delay(100);
      
      _on(rd);
      _on(ld);
    }

    //停止
    if (data=="s") {
      delay(100);
      _off(rd);
      _off(ld);
    }
    
    //右
    if (data=="r") {
      if (lf!=gls)
      {
        lssw();
      }
      delay(100);
      _off(rd);
      _on(ld);
    }

    //左
    if (data=="l") {
      if (rf!=grs)
      {
        rssw();
      }
      delay(100);
      _on(rd);
      _off(ld);
    }
    
    //後進
    if (data=="b") {
      if (rf!=brs)
      {
        rssw();
      }
      if (lf!=bls)
      {
        lssw();
      }
      delay(100);
      _on(rd);
      _on(ld);
    }

    //右ドリル
    if (data=="rdon") {
      _on(rd);     
    }
    if (data=="rdoff")
    {
      _off(rd);
    }
    
    //左ドリル
    if (data=="ldon")
    {
      _on(ld);
    }
    if (data=="ldoff")
    {
      _off(ld);
    }
    
    //右シリンダー
    if (data=="rs")
    {
      rssw();
    }
    
    
    //左シリンダー
    if(data=="ls"){
      lssw();
    }
  }
}