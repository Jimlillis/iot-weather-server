#include <WiFiS3.h>
#include <ArduinoHttpClient.h>
#include <DHT.h>

// ------------ ΡΥΘΜΙΣΕΙΣ WIFI -----------------
const char* ssid = "COSMOTE-566544";
const char* password = "mca2am6h465mxdm7";

char serverAddress[] = "iot-weather-server.onrender.com";
int port = 443;

WiFiSSLClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);

// ------------ ΑΙΣΘΗΤΗΡΕΣ ---------------------
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

const int mq135Pin = A0;
const int waterPin = A5;

// ------------ LED PINS ----------------------
#define TEMP_LED 6
#define HUM_LED 5
#define WATER_LED 4
#define AIR_LED 3

void setup() {
  Serial.begin(9600);

  // Αισθητήρες
  dht.begin();

  // LED ως έξοδοι
  pinMode(TEMP_LED, OUTPUT);
  pinMode(HUM_LED, OUTPUT);
  pinMode(WATER_LED, OUTPUT);
  pinMode(AIR_LED, OUTPUT);

  // Σύνδεση WiFi
  Serial.print("Σύνδεση στο WiFi...");
  while (WiFi.begin(ssid, password) != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println(" Συνδέθηκε!");
}

void loop() {
  // ----------- ΔΙΑΒΑΣΗ ΑΙΣΘΗΤΗΡΩΝ -----------
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int air = analogRead(mq135Pin);
  int water = analogRead(waterPin);

  Serial.println("Αποστολή δεδομένων...");
  Serial.print("Temp: "); Serial.println(temp);
  Serial.print("Humidity: "); Serial.println(hum);
  Serial.print("Water: "); Serial.print(water/10); Serial.println("%");
  Serial.print("Air: "); Serial.println(air);

  // ----------- ΑΠΟΣΤΟΛΗ ΜΕΤΡΗΣΕΩΝ -----------
  String postData = "{\"temperature\":" + String(temp) +
                    ",\"humidity\":" + String(hum) +
                    ",\"pressure\":" + String(water/10) +
                    ",\"air_quality\":" + String(air) + "}";

  client.beginRequest();
  client.post("/data");
  client.sendHeader("Content-Type", "application/json");
  client.sendHeader("Content-Length", postData.length());
  client.endRequest();
  client.print(postData);

  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  Serial.print("Status: "); Serial.println(statusCode);
  Serial.print("Response: "); Serial.println(response);

  client.stop();

  delay(1000);  // μικρό διάλειμμα

  // ----------- ΕΛΕΓΧΟΣ ΕΝΤΟΛΩΝ /command -----------
  client.get("/command");

  int cmdStatus = client.responseStatusCode();
  String cmdResponse = client.responseBody();
  cmdResponse.trim(); // καθαρισμός για σιγουριά

  Serial.print("Command Status: "); Serial.println(cmdStatus);
  Serial.print("Command Response: "); Serial.println(cmdResponse);

  // Σβήσε όλα τα LEDs
  digitalWrite(TEMP_LED, LOW);
  digitalWrite(HUM_LED, LOW);
  digitalWrite(WATER_LED, LOW);
  digitalWrite(AIR_LED, LOW);

  // Άναψε το κατάλληλο LED για 2 δευτερόλεπτα
  if (cmdResponse == "temperature") {
    digitalWrite(TEMP_LED, HIGH);
    delay(2000);
    digitalWrite(TEMP_LED, LOW);
  }
  else if (cmdResponse == "humidity") {
    digitalWrite(HUM_LED, HIGH);
    delay(2000);
    digitalWrite(HUM_LED, LOW);
  }
  else if (cmdResponse == "water") {
    digitalWrite(WATER_LED, HIGH);
    delay(2000);
    digitalWrite(WATER_LED, LOW);
  }
  else if (cmdResponse == "air_quality") {
    digitalWrite(AIR_LED, HIGH);
    delay(2000);
    digitalWrite(AIR_LED, LOW);
  }

  client.stop();

  delay(3000); // νέο loop κάθε 30 δευτερόλεπτα
}
// Σβήσε όλα τα LEDs