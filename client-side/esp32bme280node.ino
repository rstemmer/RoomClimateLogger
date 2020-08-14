#include <WiFi.h>
#include <HTTPClient.h>


const char *ssid     = "SSID";                      // TODO: Insert your WiFi's SSID here
const char *password = "PASSWORD";                  // TODO: Insert your WiFi's access password
const char *roomname = "ROOMNAME";                  // TODO: Name of the room you place the sensor node in
const char *url      = "http://SERVERIP/log.php";   // TODO: Set IP address of the server

IPAddress myipaddr(192,168,1,51);                   // \.
IPAddress gateway(192,168,1,1);                     //  > TODO: Adapt to your network setup
IPAddress subnet(255,255,255,0);                    // /

void setup()
{
    Serial.begin(9600);
    while(!Serial);
    
    auto success = WiFi.config(myipaddr, gateway, subnet);
    if(not success)
    {
        Serial.println("WiFi setup failed!");
        while(1);
    }

    WiFi.begin(ssid, password);
    while(WiFi.status() != WL_CONNECTED)
    {
        delay(500);
    }

}

void loop()
{
    if(WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Disconnected!");
        delay(1000);
        return;
    }

    // Get Data
    float t = 1.0;
    float h = 2.0;

    // Prepare Data
    String postdata;
    postdata =  "r=" + String(roomname)
             + "&t=" + String(t)
             + "&h=" + String(h);
    
    // Send Data
    HTTPClient http;
    int        retval;
    
    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    retval = http.POST(postdata);
    http.end();

    if(retval == 200)
    {
        // Success!
    }
    else if(retval > 0)
    {
        Serial.print("Server Error: ");
        Serial.println(retval);
    }
    else
    {
        Serial.println("Client Error: ");
        Serial.println(retval);
    }

    delay(1000);
    return;
}
