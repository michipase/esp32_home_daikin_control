#include "Arduino.h"
#include "ESPAsyncWebServer.h"
#include "IRremoteESP8266.h"
#include "IRsend.h"
#include "ir_Daikin.h"

const char* ssid = "Pasetto Wi-Fi";
const char* password = "AriaCsrlW1968";

const uint16_t kIrLed = 4;
IRDaikinESP ac(kIrLed);

int temp = 21;
int fan = 2;
int mode = 3;

// Create an instance of the web server
AsyncWebServer server(80);

void execCommand(int temp, int fan, int mode, String poweroff) {

  ac.setTemp(temp);
  ac.setFan(fan);
  ac.setMode(mode == 1 ? kDaikinCool : mode == 2 ? kDaikinHeat : kDaikinDry);
  if(poweroff == "SAVE") ac.on();
  else ac.off();

  Serial.println(ac.toString());

  #if SEND_DAIKIN
    ac.send();
  #endif
}

void connectWifi() {
// Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi: " +  WiFi.localIP().toString());
}

void setupRoute() {



  // Route for serving the HTML form
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String content = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"><title>AC Camere</title><script src=\"https://cdn.tailwindcss.com\"></script><style>input{background-color:rgba(0 0 0/ 0);border:solid 1px #333;width:5em}table{border-spacing:5px 5em}</style></head><body class=\"bg-[#1a1a1a] text-[#bbb] w-full h-full overflow-hidden flex justify-center items-center\"><form><div class=\"w-96 h-fit border border-2 border-[#333] rounded-xl p-4 flex items-center flex-col gap-4\"><h1 class=\"text-xl\">Ac camere</h1><table class=\"border-separate border border-spacing-4\"><tr><td><label name=\"temperature\">Temperature</label></td><td><input type=\"number\" min=\"18\" max=\"32\" name=\"temperature\" value=\"{{temp}}\"> °C</td></tr><tr><td><label name=\"fan\">Fan speed</label></td><td><input type=\"number\" min=\"1\" max=\"4\" name=\"fan\" value=\"{{fan}}\"></td></tr><tr><td><label name=\"mode\">Mode</label></td><td><select name=\"mode\" id=\"mode\" value=\"{{mode}}\"><option value=\"1\">cool</option><option value=\"2\">heat</option><option value=\"3\">dry</option></select></td></tr></table><div class=\"flex gap-4\"><input type=\"submit\" name=\"poweroff\" class=\"p-4 rounded-md bg-red-600 bg-opacity-40\" value=\"OFF\" formmethod=\"post\" formaction=\"/send\"> <input type=\"submit\" name=\"poweroff\" class=\"p-4 rounded-md bg-green-600 bg-opacity-40\" value=\"SAVE\" formmethod=\"post\" formaction=\"/send\"></div></div></form></body></html>";
    content.replace("{{temp}}", String(temp));
    content.replace("{{fan}}", String(fan));
    content.replace("{{mode}}", String(mode));
    request->send(200, "text/html", content);
  });

      // Route for handling form submission
  server.on("/send", HTTP_POST, [](AsyncWebServerRequest *request){
      if (request->hasParam("temperature", true) && request->hasParam("fan", true) && request->hasParam("mode", true) && request->hasParam("poweroff", true)) {
          String temp = request->getParam("temperature", true)->value();
          String fan = request->getParam("fan", true)->value();
          String mode = request->getParam("mode", true)->value();
          String poweroff = request->getParam("poweroff", true)->value();

          execCommand(temp.toInt(), fan.toInt(), mode.toInt(), poweroff);

          request->send(200, "text/plain", "OK");
      } else {
          request->send(400, "text/plain", "Bad Request");
      }
  });

}

void setup() {
  Serial.begin(115200);
  Serial.println("Hello!");
  ac.begin();


  connectWifi();
  setupRoute();


  // Start server
  server.begin();
}

void loop() {
    // Your other loop code here
}
