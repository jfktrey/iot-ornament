//
// Created by Trey Keown on 12/26/16.
//

#include "Arduino.h"
#include <ESP8266WiFi.h>

#include "ornament.h"

#include "secret.h"
const char* WIFI_SSID     = SECRET_WIFI_SSID;
const char* WIFI_PASSWORD = SECRET_WIFI_PASSWORD;
const char* CHEER_HOST    = SECRET_CHEER_HOST;
const int   CHEER_PORT    = SECRET_CHEER_PORT;

const uint8_t RED_PIN    = D5;
const uint8_t GREEN_PIN  = D6;
const uint8_t BUTTON_PIN = D7;

const uint8_t MAX_TICKS = 50;
uint8_t ticksLeft = MAX_TICKS;

bool checkCheer () {
    WiFiClientSecure client;

    Serial.print("Connecting to ");
    Serial.println(CHEER_HOST);

    if (!client.connect(CHEER_HOST, CHEER_PORT)) {
        Serial.println("Connection failed");
        digitalWrite(BUILTIN_LED, LOW);

        reconnectWifi();

        return false;
    }

    String url = "/status";
    Serial.print("Requesting URL: ");
    Serial.println(url);

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + CHEER_HOST + "\r\n" +
                 "User-Agent: ESP8266\r\n" +
                 "Connection: close\r\n\r\n");

    Serial.println("Request sent");

    unsigned long start_time = millis();
    while (client.available() == 0) {
        if (millis() - start_time > 5000) {
            Serial.println("Client timeout");
            digitalWrite(BUILTIN_LED, LOW);

            client.stop();
            reconnectWifi();

            return false;
        }
    }

    char responseChar = 0;

    String response = client.readString();
    int index = response.indexOf(String("\r\n\r\n"));

    if (index >= 0) {
        String content = response.substring(index + 4);
        int secondLine = content.indexOf("\r\n");
        if (secondLine >= 0) {
            secondLine += 2;
            responseChar = content.charAt(secondLine);
            Serial.println(String("Response: ") + String(responseChar));
        } else {
            Serial.println("Error, no second line found");
        }
    } else {
        Serial.println("Error, no empty line found");
    }

    Serial.println("Closing connection");
    digitalWrite(BUILTIN_LED, HIGH);

    return (responseChar == '1');
}

void sendCheer () {
    WiFiClientSecure client;

    Serial.print("Connecting to ");
    Serial.println(CHEER_HOST);

    if (!client.connect(CHEER_HOST, CHEER_PORT)) {
        Serial.println("Connection failed");
        digitalWrite(BUILTIN_LED, LOW);

        reconnectWifi();

        return;
    }

    String url = "/cheer?from=trey";
    Serial.print("Requesting URL: ");
    Serial.println(url);

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + CHEER_HOST + "\r\n" +
                 "User-Agent: ESP8266\r\n" +
                 "Connection: close\r\n\r\n");

    Serial.println("Request sent");

    unsigned long start_time = millis();
    while (client.available() == 0) {
        if (millis() - start_time > 5000) {
            Serial.println("Client timeout");
            digitalWrite(BUILTIN_LED, LOW);

            client.stop();
            reconnectWifi();

            return;
        }
    }

    String response = client.readString();

    Serial.println("Closing connection");
    digitalWrite(BUILTIN_LED, HIGH);
}

void spreadCheer () {
    for (int i = 0; i < 15; i++) {
        digitalWrite(RED_PIN, HIGH);
        digitalWrite(GREEN_PIN, LOW);
        delay(1000);
        digitalWrite(RED_PIN, LOW);
        digitalWrite(GREEN_PIN, HIGH);
        delay(1000);
    }
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
}

void reconnectWifi () {
    if (WiFi.isConnected()) {
        Serial.println("Disconnecting...");
        WiFi.disconnect();
        delay(10000);
    }

    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void setup () {
    Serial.begin(9600);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    pinMode(BUILTIN_LED, OUTPUT);
    pinMode(RED_PIN,     OUTPUT);
    pinMode(GREEN_PIN,   OUTPUT);

    digitalWrite(BUILTIN_LED, LOW);
    digitalWrite(RED_PIN,     LOW);
    digitalWrite(GREEN_PIN,   LOW);

    reconnectWifi();

    digitalWrite(BUILTIN_LED, HIGH);
}

void loop () {
    if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println("Button pressed!");
        sendCheer();
        spreadCheer();
    } else if (ticksLeft == 0) {
        ticksLeft = MAX_TICKS;
        if (checkCheer()) {
            spreadCheer();
        }
    } else {
        ticksLeft--;
        delay(100);
    }
}