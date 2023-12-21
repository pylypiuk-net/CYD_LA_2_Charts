#include <TFT_eSPI.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <stdlib.h>
#include <RingBuf.h>


///////////////////////////////////////////
// This is where goes your private info  //
// This file is not included in the repo //
// contains:                             //
// const char* ssid = "xxx";             //
// const char* password = "xxx";         //
// const char* mqtt_server = "xxx";      //
#include "private.h"                     //
///////////////////////////////////////////


WiFiClient espClient;
PubSubClient client(espClient);
TFT_eSPI tft = TFT_eSPI();

String hosts_array [] = {
    "k8s-1",
    "k8s-2",
    "k8s-3",
    "k8s-4",
    "mfs-1",
    "mfs-2",
    "mfs-3",
    "mfs-4"
};

long lastMsg = 0;
char msg[50];
int value = 0;
int offset = 20;
int x_line = 19;
int y_line_lenght = 101;
int y1_line = 101;
int y2_line = 120;
int screen_height = 240;
int screen_width = 320;
int x_step = 5;
int delta1 = 2;
int delta2 = 4;
int delta3 = 10;
int delta4 = 50;
int graph_height = 100;
int graph_width = 300;

int k8s_cpu_factor = 20;
int mfs_cpu_factor = 10;

RingBuf<float, 60> k1;
RingBuf<float, 60> k2;
RingBuf<float, 60> k3;
RingBuf<float, 60> k4;
RingBuf<float, 60> m1;
RingBuf<float, 60> m2;
RingBuf<float, 60> m3;
RingBuf<float, 60> m4;

float k1_last;
float k2_last;
float k3_last;
float k4_last;
float m1_last;
float m2_last;
float m3_last;
float m4_last;

bool k1_update = false;
bool k2_update = false;
bool k3_update = false;
bool k4_update = false;
bool m1_update = false;
bool m2_update = false;
bool m3_update = false;
bool m4_update = false;

#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include <AsyncTCP.h>

AsyncWebServer server(80);
unsigned long ota_progress_millis = 0;

int fontSize = 2;

void onOTAStart() {
  Serial.println("OTA update started!");
}

void onOTAProgress(size_t current, size_t final) {
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  if (success) {
    Serial.println("OTA update finished successfully!");
  } else {
    Serial.println("There was an error during OTA update!");
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");

  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  String K8S1_LA_Topic = "homeassistant/sensor/k8s-1/la1";
  String K8S2_LA_Topic = "homeassistant/sensor/k8s-2/la1";
  String K8S3_LA_Topic = "homeassistant/sensor/k8s-3/la1";
  String K8S4_LA_Topic = "homeassistant/sensor/k8s-4/la1";
  String MFS1_LA_Topic = "homeassistant/sensor/mfs-1/la1";  
  String MFS2_LA_Topic = "homeassistant/sensor/mfs-2/la1";
  String MFS3_LA_Topic = "homeassistant/sensor/mfs-3/la1";  
  String MFS4_LA_Topic = "homeassistant/sensor/mfs-4/la1";  

   if (String(topic) == K8S1_LA_Topic) {
    if (messageTemp.toFloat() > 5.0)
    {
        messageTemp = 5.0;
    }
    k1.pushOverwrite(messageTemp.toFloat());
    k1_last = messageTemp.toFloat();
    k1_update = true;
  }
   if (String(topic) == K8S2_LA_Topic) {
    if (messageTemp.toFloat() > 5)
    {
        messageTemp = 5.0;
    }
    k2.pushOverwrite(messageTemp.toFloat());
    k2_last = messageTemp.toFloat();
    k2_update = true;
    }
   if (String(topic) == K8S3_LA_Topic) {
 
    if (messageTemp.toFloat() > 5.0)
    {
        messageTemp = 5;
    }  
    k3.pushOverwrite(messageTemp.toFloat());
    k3_last = messageTemp.toFloat();
    k3_update = true;
    }

   if (String(topic) == K8S4_LA_Topic) {
    if (messageTemp.toFloat() > 5.0)
    {
        messageTemp = 5.0;
    }
    k4.pushOverwrite(messageTemp.toFloat());
    k4_last = messageTemp.toFloat();
    k4_update = true;
    }

   if (String(topic) == MFS1_LA_Topic) {
    if (messageTemp.toInt() > 10)
    {
        messageTemp = 10.0;
    }
    m1.pushOverwrite(messageTemp.toFloat());
    m1_last = messageTemp.toFloat();
    m1_update = true;
    }
   
   if (String(topic) == MFS2_LA_Topic) {
    if (messageTemp.toFloat() > 10.0)
    {
        messageTemp = 10;
    }
    m2.pushOverwrite(messageTemp.toFloat());
    m2_last = messageTemp.toFloat();
    m2_update = true;
    }

   if (String(topic) == MFS3_LA_Topic) {
    if (messageTemp.toFloat() > 10.0)
    {
        messageTemp = 10.0;
    }
    m3.pushOverwrite(messageTemp.toFloat());
    m3_last = messageTemp.toFloat();
    m3_update = true;
    }
   if (String(topic) == MFS4_LA_Topic) {
    if (messageTemp.toFloat() > 10.0)
    {
        messageTemp = 10.0;
    }
    m4.pushOverwrite(messageTemp.toFloat());
    m4_last = messageTemp.toFloat();
    m4_update = true;
    }
}
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("Cluster-Screen-2")) {
      Serial.println("connected");
        for (byte idx = 0; idx < sizeof(hosts_array) / sizeof(hosts_array[0]); idx++) {

        String la = "homeassistant/sensor/" + hosts_array[idx] + "/la1";
        client.subscribe(la.c_str());  
        }
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
        }
    } 
}

void drawCharts() {
  tft.fillRect(22, 0, 320, 98, TFT_BLACK);

  tft.setTextDatum(BC_DATUM);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("K8S:", 20, 235);
  tft.drawString("MFS:", 180, 235);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  String kla1 = String(k1_last, 1);
  String mla1 = String(m1_last, 1);
  tft.drawString(kla1, 50, 235);
  tft.drawString(mla1, 210, 235);
  tft.setTextColor(TFT_BLUE,TFT_BLACK);
  String kla2 = String(k2_last, 1);
  String mla2 = String(m2_last, 1);
  tft.drawString(kla2, 80, 235);
  tft.drawString(mla2, 240, 235);  
  tft.setTextColor(TFT_YELLOW,TFT_BLACK);
  String kla3 = String(k3_last, 1);
  String mla3 = String(m3_last, 1);
  tft.drawString(kla3, 110, 235);
  tft.drawString(mla3, 270, 235);
  tft.setTextColor(TFT_WHITE,TFT_BLACK);
  String kla4 = String(k4_last, 1);
  String mla4 = String(m4_last, 1); 
  tft.drawString(kla4, 140, 235);
  tft.drawString(mla4, 300, 235);

  int x_dot = 25;
  int y_dot = 5;
   for (int x = 25; x < 320; x += 5) {
    for (int y = 5; y < 100; y += 5) {
      tft.drawPixel(x, y, TFT_NAVY);
      
    }
  }
   for (int x = 25; x < 320; x += 5) {
    for (int y = 125; y < 220; y += 5) {
      tft.drawPixel(x, y, TFT_NAVY); 
    }
  }
    
  int dataSize = k1.size();
  int segmentWidth = graph_width / (dataSize - 1);
  Serial.println("");
  Serial.print("Data Size: ");
  Serial.print(dataSize);
  Serial.println("");
  for (int i = 1; i < dataSize; i++) {
    int k1y1 = String(k1[i - 1] * k8s_cpu_factor, 0).toInt();
    int k1y2 = String(k1[i] * k8s_cpu_factor, 0).toInt();
    int k2y1 = String(k2[i - 1] * k8s_cpu_factor, 0).toInt();
    int k2y2 = String(k2[i] * k8s_cpu_factor, 0).toInt();
    int k3y1 = String(k3[i - 1] * k8s_cpu_factor, 0).toInt();
    int k3y2 = String(k3[i] * k8s_cpu_factor, 0).toInt();
    int k4y1 = String(k4[i - 1] * k8s_cpu_factor, 0).toInt();
    int k4y2 = String(k4[i] * k8s_cpu_factor, 0).toInt();

    int k1x1 = (i - 1) * segmentWidth;
    int k1x2 = i * segmentWidth;
    int k2x1 = (i - 1) * segmentWidth;
    int k2x2 = i * segmentWidth;
    int k3x1 = (i - 1) * segmentWidth;
    int k3x2 = i * segmentWidth;
    int k4x1 = (i - 1) * segmentWidth;
    int k4x2 = i * segmentWidth;
    
    tft.drawLine(k1x1 + offset, graph_height - k1y1, k1x2 + offset, graph_height - k1y2, TFT_GREEN);
    tft.drawLine(k2x1 + offset, graph_height - k2y1, k2x2 + offset, graph_height - k2y2, TFT_BLUE);
    tft.drawLine(k3x1 + offset, graph_height - k3y1, k3x2 + offset, graph_height - k3y2, TFT_YELLOW);
    tft.drawLine(k4x1 + offset, graph_height - k4y1, k4x2 + offset, graph_height - k4y2, TFT_WHITE);
  
  for (int i = 1; i < dataSize; i++) {
    int m1y1 = String(m1[i - 1] * mfs_cpu_factor, 0).toInt();
    int m1y2 = String(m1[i] * mfs_cpu_factor, 0).toInt();
    int m2y1 = String(m2[i - 1] * mfs_cpu_factor, 0).toInt();
    int m2y2 = String(m2[i] * mfs_cpu_factor, 0).toInt();
    int m3y1 = String(m3[i - 1] * mfs_cpu_factor, 0).toInt();
    int m3y2 = String(m3[i] * mfs_cpu_factor, 0).toInt();
    int m4y1 = String(m4[i - 1] * mfs_cpu_factor, 0).toInt();
    int m4y2 = String(m4[i] * mfs_cpu_factor, 0).toInt();

    int m1x1 = (i - 1) * segmentWidth;
    int m1x2 = i * segmentWidth;
    int m2x1 = (i - 1) * segmentWidth;
    int m2x2 = i * segmentWidth;
    int m3x1 = (i - 1) * segmentWidth;
    int m3x2 = i * segmentWidth;
    int m4x1 = (i - 1) * segmentWidth;
    int m4x2 = i * segmentWidth;

    tft.drawLine(m1x1 + offset, 122 + m1y1, m1x2 + offset, 122 + m1y2, TFT_GREEN);
    tft.drawLine(m2x1 + offset, 122 + m2y1, m2x2 + offset, 122 + m2y2, TFT_BLUE);
    tft.drawLine(m3x1 + offset, 122 + m3y1, m3x2 + offset, 122 + m3y2, TFT_YELLOW);
    tft.drawLine(m4x1 + offset, 122 + m4y1, m4x2 + offset, 122 + m4y2, TFT_WHITE);
    }
  }
}
void plot_bg () {
  tft.drawFastVLine(x_line, 0, y1_line, TFT_WHITE);
  tft.drawFastHLine(x_line, y1_line, screen_width - x_line, TFT_WHITE);
  tft.drawTriangle(x_line, 0, x_line - delta1, delta3, x_line + delta1, delta3, TFT_WHITE);
  tft.drawTriangle(screen_width, y1_line, screen_width - delta3, y1_line + delta1 , screen_width - delta3, y1_line - delta1, TFT_WHITE);

  int horiz_x1 = 310;
  int horiz_x1_numbers = 320;
  int horiz_x1_counter = 0;
  int horiz_y1 = 101;
  int vert_y1 = 100;
  int vert_y1_counter = 0;

  while (horiz_x1 > 19) {
    tft.drawLine(horiz_x1, horiz_y1 + delta1, horiz_x1, horiz_y1, TFT_WHITE);
    horiz_x1 = horiz_x1 - x_step;
  }

  while (horiz_x1_numbers > 11) {
    if (horiz_x1_counter == 0) {
      tft.setTextDatum(ML_DATUM);
      tft.setTextColor(TFT_WHITE, TFT_TRANSPARENT);
      tft.drawString(String(horiz_x1_counter), horiz_x1_numbers - x_step, horiz_y1 + delta3, 1);
      horiz_x1_numbers = horiz_x1_numbers - delta4;
      horiz_x1_counter = horiz_x1_counter + delta3;
    } else {
      tft.setTextDatum(MC_DATUM);
      tft.setTextColor(TFT_WHITE, TFT_TRANSPARENT);
      tft.drawString(String(horiz_x1_counter), horiz_x1_numbers, horiz_y1 + delta3, 1);
      tft.drawLine(horiz_x1_numbers, horiz_y1 + 4, horiz_x1_numbers, horiz_y1, TFT_WHITE);
      horiz_x1_numbers = horiz_x1_numbers - delta4;
      horiz_x1_counter = horiz_x1_counter + delta3;
    }
  }

  while (vert_y1 > 19) {
    tft.drawLine(x_line -4, vert_y1, x_line, vert_y1, TFT_WHITE);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(String(vert_y1_counter), 10, vert_y1, 1);
    vert_y1 = vert_y1 - 20;
    vert_y1_counter++;
  }

  tft.drawFastVLine(x_line, y2_line, y_line_lenght, TFT_WHITE);
  tft.drawFastHLine(x_line, y2_line, screen_width - x_line, TFT_WHITE);
  tft.drawTriangle(x_line, y1_line + y2_line, x_line - delta1, y1_line + y2_line - delta3, x_line + delta1, y1_line + y2_line - delta3, TFT_WHITE);
  tft.drawTriangle(screen_width, y2_line, screen_width - delta3 , y2_line + delta1, screen_width - delta3, y2_line - delta1, TFT_WHITE);
  
  int horiz_x2 = 310;
  int horiz_x2_numbers = 320;
  int horiz_x2_counter = 0;
  int horiz_y2 = 120;

  int vert_y2 = 121;
  int vert_y2_counter = 0;

  while (horiz_x2 > x_line) {
    tft.drawLine(horiz_x2, horiz_y2 - delta1, horiz_x2, horiz_y2, TFT_WHITE);
    horiz_x2 = horiz_x2 - x_step;
  }

  while (horiz_x2_numbers > 11) {
    tft.drawLine(horiz_x2_numbers, horiz_y2 - delta2, horiz_x2_numbers, horiz_y2, TFT_WHITE);
    horiz_x2_numbers = horiz_x2_numbers - delta4;
  }

  while (vert_y2 < 221) {
    tft.drawLine(x_line - delta2, vert_y2, x_line, vert_y2, TFT_WHITE);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(String(vert_y2_counter), delta3, vert_y2, 1);
    vert_y2 = vert_y2 + 20;
    vert_y2_counter = vert_y2_counter + delta1;
  }
}

void clear_screen() {
    tft.fillRect(0, 0, screen_width, screen_width, TFT_BLACK);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Cluster's Screen 2 OTA Update Server.");
  });

  ElegantOTA.begin(&server);
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);

  server.begin();
  Serial.println("HTTP server started");

  Serial.println("Seting up lcd");
  tft.init();
  tft.setRotation(1);
  tft.fillRect(0, 0, screen_width, screen_width, TFT_BLACK);

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  while (k1.push(1.0))
  while (k2.push(1.0))
  while (k3.push(1.0))
  while (k4.push(1.0))

  while (m1.push(2.0))
  while (m2.push(2.0))
  while (m3.push(2.0))
  while (m4.push(2.0))

  plot_bg();
}

void loop() {
    ElegantOTA.loop();

    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    long now = millis();
    if (now - lastMsg > 5000) {
        lastMsg = now;
    }

    if (k1_update == true && k2_update == true && k3_update == true && k4_update == true && m1_update == true && m2_update == true && m3_update == true && m4_update == true) {
            clear_screen();
            plot_bg();
            drawCharts();
            k1_update = false;
            k2_update = false;
            k3_update = false;
            k4_update = false;
            m1_update = false;
            m2_update = false;
            m3_update = false;
            m4_update = false;
        }
}
