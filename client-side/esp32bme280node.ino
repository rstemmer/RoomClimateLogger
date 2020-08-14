#include <WiFi.h>
#include <HTTPClient.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

const char *ssid     = "SSID";                      // TODO: Insert your WiFi's SSID here
const char *password = "PASSWORD";                  // TODO: Insert your WiFi's access password
const char *roomname = "ROOMNAME";                  // TODO: Name of the room you place the sensor node in
const char *url      = "http://SERVERIP/log.php";   // TODO: Set IP address of the server

IPAddress myipaddr(192,168,1,51);                   // \.
IPAddress gateway(192,168,1,1);                     //  > TODO: Adapt to your network setup
IPAddress subnet(255,255,255,0);                    // /

      unsigned long deadline;
const unsigned long interval = 60000; // send data every 60 seconds

Adafruit_BME280 bme;
const uint8_t bmeaddress = BME280_ADDRESS_ALTERNATE; // or BME280_ADDRESS 

#define ENABLE_GRAVELIGHT     // Comment for better debugging
#define ENABLE_PRINTDATA      // Uncomment for better debuging


void WiFiConnect()
{
    do
    {
        // Try to connect
        WiFi.begin(ssid, password);

        // Wait 10 seconds for the connection
        for(int i = 0; i < 20 and WiFi.status() != WL_CONNECTED; i++)
            delay(500);

        // If still not connected, try to connect again
    }
    while(WiFi.status() != WL_CONNECTED);
    return;
}


// Endless loop letting an LED blink with 1Hz
[[ noreturn ]] void GraveLight()
{
    // My ESP32 has only an UART activity LED.
    // So I spam data to the UART to make this LED flash
    const unsigned long interval = 500;
    unsigned long toggletime;

    // This is a post-mortem function.
    // I do not care about real-time and potential integer overflows.    
    while(true)
    {
        // LED on
        #ifdef ENABLE_GRAVELIGHT
        toggletime = millis() + interval;
        do
        {
            Serial.print("✝");
            delay(10); // Give the LED an eerie flicker.
        }
        while(millis() < toggletime);
        #endif

        // LED off
        delay(500);
    }
}



void setup()
{
    // Setup UART
    Serial.begin(9600);
    while(!Serial);

    // Setup WiFi
    // This feature does not work with ESP32
    #if 0
    if(WiFi.status() == WL_NO_SHIELD)
    {
        Serial.println("No WiFi hardware!");
        GraveLight(); // no return
    }
    #endif
    
    auto success = WiFi.config(myipaddr, gateway, subnet);
    if(not success)
    {
        Serial.println("WiFi setup failed!");
        GraveLight(); // no return
    }

    WiFiConnect();

    // Setup Sensors
    bool sensorready;
    sensorready = bme.begin(bmeaddress);
    if(not sensorready)
    {
        Serial.println("Sensor setup failed!");
        GraveLight(); // no return
    }

    // Initialize deadline for collecting samples
    deadline = millis() + interval; // deadline for data
}



void loop()
{
    // Check WiFi status.
    // In case this node got disconnected, it should try to reconnect.
    if(WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Disconnected!");

        // Try to reconnect.
        // This function is blocking until the connection succeeds.
        WiFiConnect();

        // Due to the connection issues we already lost lots of samples and data.
        // Set new deadline for a fresh restart of logging.
        deadline = millis() + interval;
        return;
    }

    // Collect as many samples as possible until the deadline arrives.
    // This loop consideres overflows of deadline and millis().
    unsigned long timestamp;
    constexpr const unsigned long msbmask = 0x80 << (sizeof(unsigned long) - 1);
    float temp    = 0.0;
    float hum     = 0.0;
    int   samples = 0;
    do
    {
        temp   += bme.readTemperature();
        hum    += bme.readHumidity();
        samples++;

        delay(1000);  // Wait a second for the next sample     
        timestamp = millis();
    }
    while(timestamp < deadline or (deadline^timestamp) & msbmask);
    
    // Calculate average temperature and humidity
    //  observed during the last measurement interval
    deadline += interval;
    temp     /= samples;
    hum      /= samples;

    // Prepare Data
    String postdata;
    postdata =  "r=" + String(roomname)
             + "&t=" + String(temp)
             + "&h=" + String(hum);
#ifdef ENABLE_PRINTDATA
    Serial.println(postdata);
#endif
    
    // Send Data
    HTTPClient http;
    int        retval;
    
    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    retval = http.POST(postdata);
    http.end();

    // Check if sending the data succeeded
    if(retval == HTTP_CODE_OK)
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

    return;
}

