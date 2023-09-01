#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "Arducam_Mega.h"

const char* ssid = "ArducamWifi";
const char* password = NULL;

CAM_IMAGE_MODE imageMode = CAM_IMAGE_MODE_128X128; //CAM_IMAGE_MODE_QQVGA DOESN'T WORK FOR SOME REASON (128X128 is the best achievable in terms of fps, QVGA in term of balance fps/quality)
ESP8266WebServer server(80);
const int CS = 16; // Change the CS pin to match your board
Arducam_Mega myCAM( CS );
uint8_t imageData = 0;
uint8_t imageDataNext = 0;
uint8_t headFlag = 0;
const size_t bufferSize = 2048;
uint8_t buffer[bufferSize] = {0};

void sendImageData(void)
{
    int i = 0;
    WiFiClient client = server.client();
    if (!client.connected()) return;
    while (myCAM.getReceivedLength())
    {
        imageData = imageDataNext;
        imageDataNext = myCAM.readByte();
        if (headFlag == 1)
        {
            buffer[i++] = imageDataNext;
            if (i >= bufferSize)
            {
                if (!client.connected()) break;
                client.write(buffer, i);
                i = 0;
            }
        }
        if (imageData == 0xff && imageDataNext == 0xd8)
        {
            headFlag = 1;
            buffer[i++] = imageData;
            buffer[i++] = imageDataNext;
        }
        if (imageData == 0xff && imageDataNext == 0xd9)
        {
            headFlag = 0;
            if (!client.connected()) break;
            client.write(buffer, i);
            i = 0;
            break;
        }
    }
}

void captureCallbackFunction(void)
{
    myCAM.takePicture(imageMode, CAM_IMAGE_PIX_FMT_JPG);
    WiFiClient client = server.client();
    if (!client.connected()) return;
    String response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: image/jpeg\r\n";
    response += "Content-len: " + String(7000) + "\r\n\r\n";
    sendImageData();
}

void streamCallbackFunction(void)
{
    WiFiClient client = server.client();

    String response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
    server.sendContent(response);
    while (1)
    {
        myCAM.takePicture(imageMode, CAM_IMAGE_PIX_FMT_JPG);
        if (!client.connected()) break;
        response = "--frame\r\n";
        response += "Content-Type: image/jpeg\r\n\r\n";
        server.sendContent(response);
        sendImageData();
    }
}

void handleNotFound(void)
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
    Serial.println(message);
    if (server.hasArg("ql"))
    {
        imageMode = (CAM_IMAGE_MODE)server.arg("ql").toInt();
    }
}

void setup(){
    Serial.begin(115200);
    myCAM.begin();
    myCAM.takePicture(CAM_IMAGE_MODE_WQXGA2, CAM_IMAGE_PIX_FMT_JPG);
    myCAM.setImageQuality(LOW_QUALITY);
    myCAM.setAutoExposure(1);
    myCAM.setAutoISOSensitive(1);
    myCAM.setAutoWhiteBalance(1);
    myCAM.setAutoFocus(1); //try to reduce lag by setting off (1) all the auto functions
    WiFi.mode(WIFI_AP);
    while(!WiFi.softAP(ssid,password));
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
    server.on("/capture", HTTP_GET, captureCallbackFunction); //on http://<ip>/capture you'll get a jpg
    server.on("/stream", HTTP_GET, streamCallbackFunction); //on http://<ip>/stream you'll get an outadet stream of jpgs
    server.onNotFound(handleNotFound);
    server.begin();
}

void loop(){
  server.handleClient();
}
