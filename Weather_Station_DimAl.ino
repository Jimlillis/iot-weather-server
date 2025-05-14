#include <WiFiS3.h>
#include <ArduinoHttpClient.h>

// WiFi Δίκτυο
const char* ssid = "COSMOTE-966018"; //TO_WIFI
const char* password = "ebbr72nn26ncrr82"; //O_KODIKOS

// Server
char serverAddress[] = "iot-weather-server.onrender.com";
int port = 443;
WiFiSSLClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);

// MQ135
const int mq135Pin = A0;

void setup() {
  Serial.begin(9600);

  while (WiFi.begin(ssid, password) != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("WiFi Συνδεδεμένο!");
}

void loop() {
  int rawValue = analogRead(mq135Pin);
  int air_quality = rawValue / 10; // Κλιμάκωση για να βγει σε "κατανοητή" μονάδα

  Serial.print("Ποιότητα Αέρα: ");
  Serial.println(air_quality);

  // Dummy values για τα υπόλοιπα
  float temperature = 0.0;
  float humidity = 0.0;
  float pressure = 0.0;

  String postData = "{\"temperature\":" + String(temperature) +
                    ",\"humidity\":" + String(humidity) +
                    ",\"pressure\":" + String(pressure) +
                    ",\"air_quality\":" + String(air_quality) + "}";

  client.beginRequest();
  client.post("/data");
  client.sendHeader("Content-Type", "application/json");
  client.sendHeader("Content-Length", postData.length());
  client.endRequest();
  client.print(postData);

  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
  Serial.print("Κατάσταση: "); Serial.println(statusCode);
  Serial.print("Απάντηση: "); Serial.println(response);

  delay(30000); // κάθε 30 δευτερόλεπτα
}
