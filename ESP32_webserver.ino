#include<Arduino.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

const char ssid[] = "AirRobot";
const char pass[] = "iRobotkara";       // パスワードは8文字以上
const IPAddress ip(192,168,4,1);
const IPAddress subnet(255,255,255,0);
AsyncWebServer server(80);            // ポート設定

#define rd 32
#define rsyl 33
#define ld 25
#define lsyl 26

int _on(int num) {
  digitalWrite(num,HIGH);
  Serial.println(num);
}
int _off(int num) {
  digitalWrite(num,LOW);
  Serial.println(num);
}

bool lf = false;
bool rf = false;

void setup()
{
  Serial.begin(115200);

  pinMode(rd,OUTPUT);
  pinMode(rsyl,OUTPUT);
  pinMode(ld,OUTPUT);
  pinMode(lsyl,OUTPUT);

  // 初期化
  _off(rd);
  _off(rsyl);
  _off(ld);
  _off(lsyl);

  // SPIFFSのセットアップ
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  
  WiFi.softAP(ssid, pass);           // SSIDとパスの設定
  delay(100);                        // このdelayを入れないと失敗する場合がある
  WiFi.softAPConfig(ip, ip, subnet); // IPアドレス、ゲートウェイ、サブネットマスクの設定
  
  IPAddress myIP = WiFi.softAPIP();  // WiFi.softAPIP()でWiFi起動

  delay(1000);
  // 各種情報を表示
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // GETリクエストに対するハンドラーを登録
  // rootにアクセスされた時のレスポンス
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });
  // style.cssにアクセスされた時のレスポンス
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.on("/straight", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "straight");
    _on(rd);
    _on(rsyl);
    rf=true;
    _on(ld);
    _off(lsyl);
    lf=false;
  });

  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "stop");
    _off(rd);
    _off(ld);
  });

  server.on("/right", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "right");
    _off(rd);
    _on(ld);
  });

  server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "left");
    _on(rd);
    _off(ld);
  });

  server.on("/back", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "back");
    _on(rd);
    rf=true;
    _off(rsyl);
    _on(ld);
    lf=true;
    _on(lsyl);
  });

  //For Controller
  server.on("/rdon", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "rdon");
    _on(rd);
  });

  server.on("/rdoff", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "rdoff");
    _off(rd);
  });

  server.on("/ldon", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "ldon");
    _on(ld);
  });

  server.on("/ldoff", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "ldoff");
    _off(ld);
  });

  server.on("/rsyl", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "rsyl");
    if(rf==true){
      _off(rsyl);
      rf=false;
    } else {
      _on(rsyl);
      rf=true;
    }
  });

  server.on("/lsyl", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "lsyl");
    if(lf==true){
      _off(lsyl);
      lf=false;
    } else {
      _on(lsyl);
      lf=true;
    }
  });
  
  // サーバースタート
  server.begin();
  delay(1000);
  Serial.println("Server start!");
}

void loop() {
  if(Serial.available()>0){
    String data = Serial.readStringUntil('\n');
    
    //まっすぐ
    if (data=="straight") {
      _on(rd);
      _on(ld);
    }

    //とまる
    if (data=="stop") {
      _off(rd);
      _off(ld);
    }
    
    //右
    if (data=="right") {
      _off(rd);
      _on(ld);
    }

    //左
    if (data=="left") {
      _on(rd);
      rf=true;
      _on(rsyl);
      _off(ld);
      lf=false;
    }
    
    //後ろ
    if (data=="back") {
      _on(rd);
      rf=true;
      _off(rsyl);
      _on(ld);
      lf=true;
      _on(lsyl);
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
    if (data=="rsyl"&&rf==true)
    {
      _off(rsyl);
      rf=true;
    }
    else if (data=="rsyl"&&rf==false)
    {
      _on(rsyl);
      rf=false;
    }
    
    //左シリンダー
    if (data=="lsyl"&&lf==true)
    {
      _off(lsyl);
      lf==false;
    }
    else if (data=="lsyl"&&lf==false)
    {
      _on(lsyl);
      lf==true;
    }
  }
}
