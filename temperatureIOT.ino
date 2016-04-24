/*
  Author: Philip Matuskiewicz - phil@matthouse.us
  #using NodeMCU Amica with ESP8266 chipset
  Modified 4/24/16

  3v DHT22 Chip on pin D5, meant for temperature/humidity collection via wifi
*/

#include <DHT.h>
#include <ESP8266WiFi.h>

#define ID              0 // hard coded id
#define WLAN_SSID       "CHANGEME"
#define WLAN_PASS       "CHANGEME"
#define HOST            "CHANGEME.com"
#define WEBPAGE         "/iot.php"
#define UPLOADPASSWORD  "CHANGEME"
#define TIMEOUT         10

int retrieval_interval_seconds = 600;//600 is 10 minutes

DHT dht(D5, DHT22);

void setup() {
  
  Serial.begin(115200);

  //turn on power light
  pinMode(D0, OUTPUT);
  digitalWrite(D0, LOW);

  Serial.println("Starting DHT Monitor");
  dht.begin();
  

  Serial.println("Connecting to WIFI...");
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000L);
    Serial.print(".");
    i++;
    if (i > 10000L) {
      WiFi.disconnect();
      Serial.println("TIMED OUT CONNECTING TO WIFI!  Waiting 1 minute for a retry.");
      delay(60000L);//1 minute wait
      return;
    }
  }
  while(!RetrievalSetup()){
    delay(10000L);  
  }
  Serial.println("WiFi Connected successfully!");
  
  digitalWrite(D0, HIGH);
  
}

void loop() {

  digitalWrite(D0, LOW);//turn light on

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  double tf = Celcius2Fahrenheit(t);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println("%");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print("C; ");
  Serial.print(tf);
  Serial.println("F");

  upload(h, t, tf);

  digitalWrite(D0, HIGH);//turn light off

  delay(retrieval_interval_seconds * 1000L);
  
}

double Celcius2Fahrenheit(double celsius) {
  return 1.8 * celsius + 32;
}

boolean upload(float humid, float tempc, double tempf){
  String url = WEBPAGE;
  url += "?password=";
  url += UPLOADPASSWORD;
  url += "&device=";
  url += ID;
  url += "&tempc=";
  url += tempc;
  url += "&tempf=";
  url += tempf;
  url += "&humidity=";
  url += humid;

  WiFiClient client;
  client.setTimeout(TIMEOUT * 1000L);
  if (!client.connect(HOST, 80)) {
    Serial.println("HTTP connection failed");
    return false;
  }

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + HOST + "\r\n" +
               "Connection: close\r\n\r\n");

  delay(1000L);

  int start = 0;
  String ret;
  while (client.connected()) {
    if(start > 0){
        ret = client.readStringUntil('\n');
        client.stop();
    }else{
      //otherwise read headers
      String line = client.readStringUntil('\n');
      if(line.equals("\r")){
        start = 1;
      }
    }
  }
  
  return true;
}

boolean RetrievalSetup()
{  
  String url = WEBPAGE;
  url += "?password=";
  url += UPLOADPASSWORD;
  url += "&setup=1";

  WiFiClient client;
  client.setTimeout(TIMEOUT * 1000L);
  if (!client.connect(HOST, 80)) {
    Serial.println("HTTP connection failed");
    return false;
  }
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + HOST + "\r\n" +
               "Connection: close\r\n\r\n");

  delay(1000L);

  int st = 0;
  String conf;
  int value = 600;
  while (client.connected()) {
    if(st > 0){
        conf = client.readStringUntil('\n');
        int pos = conf.indexOf(';');
        if(pos > 0){
          value = conf.substring(0, pos).toInt();
          client.stop();
        }
    }else{
      //otherwise read headers
      String line = client.readStringUntil('\n');
      if(line.equals("\r")){
        st = 1;
      }
    }
  }

  Serial.print("Configured download interval is ");
  Serial.println(value);
  retrieval_interval_seconds = value;
  
  return true;

}
