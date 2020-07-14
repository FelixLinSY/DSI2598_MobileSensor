#include <Adafruit_VEML6070.h>
#include <MAX44009.h>
#include <Sodaq_SHT2x.h>
#include <Wire.h>
#include "bc26.h"

Adafruit_VEML6070 uv = Adafruit_VEML6070();
MAX44009          light;
unsigned long     timer;

#define UPLOAD_INTERVAL     20000       // unit: ms

const char *apn   = "internet.iot";             // APN for CHT
const char *host  = "iiot.ideaschain.com.tw";   // MQTT host
const char *user  = "Q4y4KZVebeQS2DLnIGCp";     // username for MQTT
const char *key   = "";                         // password for MQTT
const char *topic = "v1/devices/me/telemetry";  // sensor data topic on ideaschain

void setup()
{
    // init I2C BUS
    Wire.begin();

    // init serial-USB for debug
    Serial.begin(115200);

    // bc26 init
    BC26Init(BAUDRATE_9600, apn, BAND_8);
    // connect to server
    BC26ConnectMQTTServer(host, user, key, MQTT_PORT_1883);
    // get Signal Quality
    int csq = getBC26CSQ();
    Serial.print(F("BC26 CSQ = "));
    Serial.println(csq);

    // init VEML6070
    uv.begin(VEML6070_HALF_T);     // pass in the integration time constant

    // init MAX44009
    if (light.begin()) {
        Serial.println("Could not find a valid MAX44009 sensor, check wiring!");
        while(1);
    }
}

void loop()
{
    int     uv_index, lux;
    float   temp, humidity;
    char    buff[255];

    Serial.println("##########");

    // read VEML6070 UV sensor
    Serial.print("UV light level: ");
    uv_index = uv.readUV();
    Serial.println(uv_index);

    // read MAX44009 Lux sensor
    Serial.print("Light (lux):    ");
    lux = light.get_lux();
    Serial.println(lux);

    // read from SHT21
    Serial.print("Humidity(%RH): ");
    humidity = SHT2x.GetHumidity();
    Serial.println(humidity);
    Serial.print("Temperature(C): ");
    temp = SHT2x.GetTemperature();
    Serial.println(temp);

    if (millis() >= timer) {
        timer = millis() + UPLOAD_INTERVAL;

        sprintf(buff, "{\"temperature\":%s, \"humidity\":%s, \"uv\":%d, \"lux\":%d}",
                String(temp).c_str(), String(humidity).c_str(), uv_index, lux);

        BC26MQTTPublish(topic, buff, MQTT_QOS0);
    }

    delay(1000);
}
