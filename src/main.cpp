#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

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
  return 0;
}
int _off(int num) {
  digitalWrite(num,LOW);
  Serial.println(num);
  return 0;
}

bool lf = 1;
bool rf = 1;

void setup()
{
  Serial.begin(115200);

  pinMode(rd,OUTPUT);
  pinMode(rsyl,OUTPUT);
  pinMode(ld,OUTPUT);
  pinMode(lsyl,OUTPUT);

  // 初期化 ピストンは両方オフで前進
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
    rf=1;
    lf=1;
    _off(rsyl);
    _off(lsyl);
    delay(100);
    _on(rd);
    _on(ld);
  });

  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "stop");
    delay(100);
    _off(rd);
    _off(ld);
  });

  server.on("/right", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "right");
    rf=1;
    lf=1;
    _off(rsyl);
    _off(lsyl);
    delay(100);
    _off(rd);
    _on(ld);
  });

  server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "left");
    rf=1;
    lf=1;
    _off(rsyl);
    _off(lsyl);
    delay(100);
    _on(rd);
    _off(ld);
  });

  server.on("/back", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "back");
    rf=0;
    _on(rsyl);
    lf=0;
    _on(lsyl);
    delay(100);
    _on(rd);
    _on(ld);
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
    if(rf==0){
      _off(rsyl);
      rf=1;
    }
    else if (rf==1)
    {
      _on(rsyl);
      rf=0;
    }
  });

  server.on("/lsyl", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "lsyl");
    if(lf==0){
      _off(lsyl);
      lf=1;
    }
    else if (lf==1)
    {
      _on(lsyl);
      lf=0;
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
      rf=1;
      lf=1;
      _off(rsyl);
      _off(lsyl);
      delay(100);
      _on(rd);
      _on(ld);
    }

    //とまる
    if (data=="stp") {
      delay(100);
      _off(rd);
      _off(ld);
    }
    
    //右
    if (data=="right") {
      rf=1;
      lf=1;
      _off(rsyl);
      _off(lsyl);
      delay(100);
      _off(rd);
      _on(ld);
    }

    //左
    if (data=="left") {
      rf=1;
      lf=1;
      _off(rsyl);
      _off(lsyl);
      delay(100);
      _on(rd);
      _off(ld);
    }
    
    //後ろ
    if (data=="back") {
      rf=0;
      _on(rsyl);
      lf=0;
      _on(lsyl);
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
    if (data=="rs"&&rf==0)
    {
      _off(rsyl);
      rf=1;
    }
    else if (data=="rs"&&rf==1)
    {
      _on(rsyl);
      rf=0;
    }
    
    //左シリンダー
    if (data=="ls"&&lf==0)
    {
      _off(lsyl);
      lf=1;
    }
    else if (data=="ls"&&lf==1)
    {
      _on(lsyl);
      lf=0;
    }
  }
}