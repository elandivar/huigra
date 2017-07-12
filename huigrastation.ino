#include <WiFi101.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>

// Web REST API params
String str_response = ""; 
String session_id = "";
int debug = 4;
String servidor = "weatherstation.wunderground.com";

WiFiClient client;
char ssid[] = "YOUR-SSID-HERE";      //  your network SSID (name)
int status = WL_IDLE_STATUS;
char wunder_sid[]  = "YOUR-WUNDERGROUND-STATION-ID";
char wunder_pass[] = "YOUR-STATION-PASSWORD";

#define DHTPIN        1        
#define DHTTYPE       DHT11 
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;

void setup() {
    delay(3000);
    Serial.begin(1200);
    
    // Inicializo los sensores
    dht.begin();
    bmp.begin();
    
    while (status != WL_CONNECTED){
      Serial.println("Attempting to connect to WiFi");
      status = WiFi.begin(ssid, "YOU-WIFI-PASSWORD-HERE");
      printMacAddress();
      listNetworks();
      IPAddress ip = WiFi.localIP();
      Serial.print("IP Address:");
      Serial.println(ip);
    }
    Serial.println("connected to WiFi ");

    //timeClient.begin();
}
  
void loop() {
    Serial.println("loop");

    //timeClient.update();
    //Serial.println(timeClient.getFormattedTime());

    float humi = dht.readHumidity();
    float temp = dht.readTemperature();
    float baro = bmp.readPressure();
    float baro_mbar = baro * 33.86375258 / 101325;

    if (isnan(temp) || isnan(humi)) {
            Serial.println("Fallo al leer del sensor DHT");
    } else {
        Serial.print("La temperatura es de: ");
        Serial.print(temp);  
        float temp_fahr = ((9* temp )/5) + 32;
        Serial.println("C  -->  " + String(temp_fahr) + "F");       
        Serial.print("La humedad es de: ");
        Serial.println(humi);  
        Serial.print("La presion es de: ");
        Serial.println(baro_mbar);  
        
        sendSample(temp_fahr, humi, baro_mbar);
    }

    int interval_mins = 5;
    delay(interval_mins*60*1000);
}

String sendSample(float temp_fahr, float Humidity, float baro_mbar) {
    if(debug>=1) Serial.println("sendMessage: Sending Message");
    String geturl = "/weatherstation/updateweatherstation.php?action=updateraw&ID=" + String(wunder_sid) + "&PASSWORD=" + String(wunder_pass) + "&dateutc=now&humidity=" + String(Humidity) + "&tempf=" + String(temp_fahr) + "&baromin=" + String(baro_mbar);
    Serial.println(geturl);
    String msg_response = sendREST(servidor, 80, geturl, "GET", "");
    Serial.println(msg_response);
}

// HTTP REST FUNCTIONS
String sendREST(String req_server, int req_port, String req_resource, String req_method, String req_params) {
    if(debug>=4) Serial.println("    Entering function sendREST");
    String str_return = "";
    req_method.toLowerCase();

    char charr_server[req_server.length() + 1];
    req_server.toCharArray(charr_server, req_server.length() + 1);
        
    client.connect(charr_server, req_port);

    if(req_method=="post") {
        if(debug>=1) Serial.println("    sendREST: POST method selected. Sending request");
        String postline = "POST " + req_resource + " HTTP/1.1";
        char charr_postline[postline.length() + 1];
        postline.toCharArray(charr_postline, postline.length() + 1);
        client.println(charr_postline);
        client.print("Host: ");
        client.println(req_server);
        client.println("Accept: */*");
        client.println("Content-Type: application/x-www-form-urlencoded");
        client.println("Connection: close");
        // Automated POST data section
        int str_len2 = req_params.length() + 1; 
        char charr_params[str_len2];
        req_params.toCharArray(charr_params, str_len2);
        client.print("Content-Length: ");
        client.println(str_len2);
        client.println();
        client.println(charr_params);
    } else if(req_method=="get") {
        if(debug>=1) Serial.println("    sendREST: GET method selected. Sending request");
        String getline = "GET " + req_resource + " HTTP/1.1";
        int str_len = getline.length() + 1; 
        char charr_getline[str_len];
        getline.toCharArray(charr_getline, str_len);
        client.println(charr_getline);
        client.print("Host: ");
        client.println(req_server);
        client.println("Accept: */*");
        client.println("Connection: close");
        client.println();
    }

    while(client.connected()) {
      while (client.available()) {
        char c = client.read();
        str_return = str_return + String(c);
      }
    }

    client.stop();  
    if(debug>=4) Serial.println("    Leaving function sendREST");
    return str_return;
}

// NETWORK FUNCTIONS
void printMacAddress() {
    // the MAC address of your Wifi shield
    byte mac[6];
  
    // print your MAC address:
    WiFi.macAddress(mac);
    Serial.print("MAC: ");
    Serial.print(mac[5], HEX);
    Serial.print(":");
    Serial.print(mac[4], HEX);
    Serial.print(":");
    Serial.print(mac[3], HEX);
    Serial.print(":");
    Serial.print(mac[2], HEX);
    Serial.print(":");
    Serial.print(mac[1], HEX);
    Serial.print(":");
    Serial.println(mac[0], HEX);
}

void listNetworks() {
    // scan for nearby networks:
    Serial.println("** Scan Networks **");
    int numSsid = WiFi.scanNetworks();
    if (numSsid == -1)
    {
      Serial.println("Couldn't get a wifi connection");
      while (true);
    }

    // print the list of networks seen:
    Serial.print("number of available networks:");
    Serial.println(numSsid);

    // print the network number and name for each network found:
    for (int thisNet = 0; thisNet < numSsid; thisNet++) {
      Serial.print(thisNet);
      Serial.print(") ");
      Serial.print(WiFi.SSID(thisNet));
      Serial.print("\tSignal: ");
      Serial.print(WiFi.RSSI(thisNet));
      Serial.print(" dBm");
      Serial.print("\tEncryption: ");
      printEncryptionType(WiFi.encryptionType(thisNet));
      Serial.flush();
    }
}

void printEncryptionType(int thisType) {
    // read the encryption type and print out the name:
    switch (thisType) {
      case ENC_TYPE_WEP:
        Serial.println("WEP");
        break;
      case ENC_TYPE_TKIP:
        Serial.println("WPA");
        break;
      case ENC_TYPE_CCMP:
        Serial.println("WPA2");
        break;
      case ENC_TYPE_NONE:
        Serial.println("None");
        break;
      case ENC_TYPE_AUTO:
        Serial.println("Auto");
        break;
    }
}
