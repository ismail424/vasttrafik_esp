#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>
#include <Adafruit_GFX.h>   
#include <Adafruit_ST7735.h> 
#include <NTPClient.h>         
#include <TimeLib.h>
#include <SPI.h>
#include <WiFiUdp.h>

/*         Change this values         */
// wifi creeds
const char* ssid = "wifi_name";
const char* password = "wifi_password";

// Västtrafik stop id (find it online)
String stop_id = "9021014003200000";

// path to python server
String serverPath = "python fastApi hostname";
// example http://192.168.0.157:8000/
/*------------------------------------*/

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "1.se.pool.ntp.org", 7200, 60000);

// Pins used
#define TFT_CS     D1
#define TFT_RST    D0  
#define TFT_DC     D2

#define COLOR_DARKMODE   0x31E9 // Darkmode color
#define SCREEN_WIDTH 128 // OLED display width,  in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// init adafruit library
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

void setup(void) {
  Serial.begin(115200);
  tft.initR(INITR_BLACKTAB);  
  tft.fillScreen(COLOR_DARKMODE);
  tft.setCursor(3,3);
  tft.println("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {delay(500);Serial.print(".");}
  Serial.println("Connected");
  timeClient.begin();
  
  tft.fillScreen(COLOR_DARKMODE);
  drawCenterText(drawStopName(), ST77XX_WHITE, 1, 35);
}

void loop() {
  JSONVar response_json = fetch_vasttrafik();
  int count = 12;
  int page = 0;
  int size_of = sizeof(response_json);

  while(true){
    
    drawClock();
    if (count >= 12){
      
      int first_index = page * 3;
      int last_index = ((page + 1) * 3) -1;
      int z = 0;
      page += 1;
      count = 0;
      clearVastTrafikTimes();
      for(int i = first_index; i <= last_index; i++){
         if (response_json[i] == null){
            page = 0;
            response_json = fetch_vasttrafik();
            break;
          }
        
         int bgColor = response_json[i]["bgColor"];
         int fgColor = response_json[i]["fgColor"];
         String line = (const char*)  response_json[i]["line"];
         int time_diffrence_min = response_json[i]["time_diffrence_min"];
         String tram_direction = (const char*)  response_json[i]["direction"];
         int row = 53 + (33 * z);         
         drawIconVastTrafk(bgColor,fgColor, line, time_diffrence_min, tram_direction, "20:00",row);
         z+=1;
      }
    }
     
    count += 1;
    delay(500);
  }
  
}

void drawIconVastTrafk(uint16_t bg_color, uint16_t text_color,String line, int time_diffrence_min, String line_direction, String arrival, int height){
  
    tft.fillRoundRect(8, height, 22, 26, 2, bg_color);  
    
    tft.setTextColor(text_color);
    tft.setTextSize(2);
    tft.setCursor(14, height + 6);
    tft.println(line);

    tft.setTextColor(0xffff);
    tft.setTextSize(1);
    tft.setCursor(35, height + 9);
    tft.println(line_direction);


    String time_diffrence = String(time_diffrence_min) + "min";

    if (time_diffrence_min <= 2){
        tft.setTextColor(0xF800);
    }
    else if ( 5 >= time_diffrence_min){
        tft.setTextColor(0xFFE0);
    }
    else{
        tft.setTextColor(0x57e7);
    }
    
    tft.setTextSize(1.5);
    tft.setCursor(SCREEN_WIDTH-34, height + 9);
    tft.println(time_diffrence);
    
}

JSONVar fetch_vasttrafik(){
  String full_path = serverPath + "/get_times/"  + stop_id;
  String response = httpGETRequest(full_path.c_str());
  return JSON.parse(response);
}

String drawStopName(){
  String full_path = serverPath + "/get_name/"  + stop_id;
  String response = httpGETRequest(full_path.c_str());
  JSONVar resp_json = JSON.parse(response);
  String stop_name =  (const char*) resp_json["stop_name"];
  return stop_name;
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

void drawClock(){
    timeClient.update();
    String formattedTime = timeClient.getFormattedTime();
    tft.fillRect(0, 0, SCREEN_WIDTH, 30, COLOR_DARKMODE);
    drawCenterText(formattedTime, ST77XX_WHITE, 2, 13);
}

void clearVastTrafikTimes(){
  tft.fillRect(0, 50, SCREEN_WIDTH, 126, COLOR_DARKMODE);
 }


void drawCenterText(String text, uint16_t text_color, int text_size, int margin_top) {
  int16_t x, y;
  uint16_t width, height;
       
  tft.setTextColor(text_color);
  tft.setTextSize(text_size);
  
  tft.getTextBounds(text, 0, 0, &x, &y, &width, &height);
  tft.setCursor((SCREEN_WIDTH - width) / 2, margin_top);
  tft.println(text);
}

