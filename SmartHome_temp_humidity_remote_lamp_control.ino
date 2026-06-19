#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

// ── Wi-Fi
const char* ssid     = "Victor";
const char* password = "12345678";

// ── IP Statique
IPAddress local_IP(192, 168, 66, 100);
IPAddress gateway(192, 168, 66, 1);
IPAddress subnet(255, 255, 255, 0);

// ── Pins
#define DHT_PIN   4
#define RELAY_PIN 5
#define DHT_TYPE  DHT11

DHT dht(DHT_PIN, DHT_TYPE);
WebServer server(80);

bool lampState = false;

// ── PAGE WEB
String getPage() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  String state = lampState ? "ALLUMÉE 💡" : "ÉTEINTE";
  String btnOn  = lampState ? "opacity:0.4" : "opacity:1";
  String btnOff = lampState ? "opacity:1"   : "opacity:0.4";
  String temp   = isnan(t) ? "--" : String(t, 1);
  String hum    = isnan(h) ? "--" : String(h, 1);

  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<meta http-equiv='refresh' content='3'>";
  html += "<title>Smart Home</title>";
  html += "<style>";
  html += "body{font-family:Arial,sans-serif;background:#0D1B4B;color:white;text-align:center;margin:0;padding:20px;}";
  html += "h1{color:#C8A951;font-size:28px;margin-bottom:5px;}";
  html += "p.sub{color:#aaa;font-size:14px;margin-bottom:30px;}";
  html += ".card{background:#1A2A6C;border-radius:16px;padding:20px;margin:15px auto;max-width:340px;}";
  html += ".card h2{color:#C8A951;font-size:16px;margin-bottom:12px;}";
  html += ".state{font-size:22px;font-weight:bold;margin:10px 0;}";
  html += ".btn{display:inline-block;padding:12px 30px;border-radius:10px;font-size:16px;font-weight:bold;text-decoration:none;margin:6px;color:white;}";
  html += ".on{background:#27AE60;}";
  html += ".off{background:#E74C3C;}";
  html += ".sensor{font-size:36px;font-weight:bold;color:white;}";
  html += ".unit{font-size:16px;color:#aaa;}";
  html += ".row{display:flex;justify-content:space-around;margin-top:10px;}";
  html += ".col{flex:1;}";
  html += "</style></head><body>";
  
  html += "<h1>🏠 Smart Home</h1>";
  html += "<p class='sub'>Réseau : Victor &nbsp;|&nbsp; IP : 192.168.66.100</p>";
  
  // Carte lampe
  html += "<div class='card'>";
  html += "<h2>💡 Contrôle Lampe</h2>";
  html += "<div class='state'>Lampe : " + state + "</div>";
  html += "<a href='/ledon'  class='btn on'  style='" + btnOn  + "'>Allumer</a>";
  html += "<a href='/ledoff' class='btn off' style='" + btnOff + "'>Éteindre</a>";
  html += "</div>";
  
  // Carte capteurs
  html += "<div class='card'>";
  html += "<h2>🌡️ Capteurs — Mise à jour auto (3s)</h2>";
  html += "<div class='row'>";
  html += "<div class='col'><div class='unit'>Température</div><div class='sensor'>" + temp + "°</div><div class='unit'>Celsius</div></div>";
  html += "<div class='col'><div class='unit'>Humidité</div><div class='sensor'>" + hum + "%</div><div class='unit'>Relative</div></div>";
  html += "</div></div>";
  
  html += "</body></html>";
  return html;
}

// ── HANDLERS
void handleRoot() {
  server.send(200, "text/html", getPage());
}

void handleLedOn() {
  lampState = true;
  digitalWrite(RELAY_PIN, LOW);  // actif BAS
  server.send(200, "text/html", getPage());
}

void handleLedOff() {
  lampState = false;
  digitalWrite(RELAY_PIN, HIGH);
  server.send(200, "text/html", getPage());
}

void handleRead() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  String data = "T:" + String(t, 1) + " H:" + String(h, 1);
  server.send(200, "text/plain", data);
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // relais OFF au démarrage
  dht.begin();

  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  Serial.print("Connexion Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnecté ! IP : " + WiFi.localIP().toString());

  server.on("/",       handleRoot);
  server.on("/ledon",  handleLedOn);
  server.on("/ledoff", handleLedOff);
  server.on("/read",   handleRead);
  server.begin();
  Serial.println("Serveur démarré !");
}

void loop() {
  server.handleClient();
}