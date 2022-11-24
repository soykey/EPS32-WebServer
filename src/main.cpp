#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

const char ssid[] = "AirRobot";
const char pass[] = "nogami1103";       // パスワードは8文字以上
const IPAddress ip(192,168,4,1);
const IPAddress subnet(255,255,255,0);
AsyncWebServer server(80);            // ポート設定

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
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/favicon.ico", "text/css");
  });


  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200);
    delay(100);
    _off(rd);
    _off(rs);
  });

  //For Controller
  server.on("/rdon", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200);
    _on(rd);
  });

  server.on("/rdoff", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200);
    _off(rd);
  });

  server.on("/rs", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200);
    rssw();
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

    //とまる
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
    
    //後ろ
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