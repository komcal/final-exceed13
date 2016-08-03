#include <ServerExceed.h>

WiFiServer server(80); // nodeMCU server : port 80
char ssid[] = "eXceed 2G";
char password[] = "";
char host[] = "10.32.176.4";
int port = 80;
String group = "xplusequal13"; 
String name= "cal";
int temp = 25;
ServerExceed mcu(ssid, password, host, port, group, &server);

void setup() {
  Serial.begin(115200);
  mcu.connectServer();
  Serial.print("\n\nIP: ");
  Serial.println(WiFi.localIP());
}


String data = "";

void loop() {
  if(Serial.available()) {
    data = Serial.readStringUntil('\r');
    data.replace("\r","");
    data.replace("\n","");
    Serial.flush();
    if(data != "") mcu.sendDataFromBoardToServer(data);
  }
  mcu.sendDataFromServerToBoard();
}

