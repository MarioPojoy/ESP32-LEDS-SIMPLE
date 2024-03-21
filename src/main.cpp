#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "credentials.h"

#define ledPin1  GPIO_NUM_16
#define ledPin2  GPIO_NUM_17
#define ledPin3  GPIO_NUM_18

bool ledState1 = 0;
bool ledState2 = 0;
bool ledState3 = 0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
  html {
    font-family: Arial, Helvetica, sans-serif;
    text-align: center;
  }
  h1 {
    font-size: 1.8rem;
    color: white;
  }
  h2{
    font-size: 1.5rem;
    font-weight: bold;
    color: #143642;
  }
  .topnav {
    overflow: hidden;
    background-color: #143642;
  }
  body {
    margin: 0;
  }
  .content {
    padding: 30px;
    max-width: 600px;
    margin: 0 auto;
  }
  .card {
    background-color: #F8F7F9;;
    box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
    padding-top:10px;
    padding-bottom:20px;
  }
  .button {
    padding: 15px 50px;
    font-size: 24px;
    text-align: center;
    outline: none;
    color: #fff;
    background-color: #0f8b8d;
    border: none;
    border-radius: 5px;
    -webkit-touch-callout: none;
    -webkit-user-select: none;
    -khtml-user-select: none;
    -moz-user-select: none;
    -ms-user-select: none;
    user-select: none;
    -webkit-tap-highlight-color: rgba(0,0,0,0);
   }
   /*.button:hover {background-color: #0f8b8d}*/
   .button:active {
     background-color: #0f8b8d;
     box-shadow: 2 2px #CDCDCD;
     transform: translateY(2px);
   }
   .state {
     font-size: 1.5rem;
     color:#8c8c8c;
     font-weight: bold;
   }
  </style>
<title>ESP Web Server</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
</head>
<body>
  <div class="topnav">
    <h1>ESP WebSocket Server</h1>
  </div>
  <div class="content">
    <div class="card">
      <h2>Output - GPIO 16</h2>
      <p class="state">state: <span id="state_1">%STATE1%</span></p>
      <p><button id="button_1" class="button">Toggle</button></p>
    </div>
    <div class="card">
      <h2>Output - GPIO 17</h2>
      <p class="state">state: <span id="state_2">%STATE2%</span></p>
      <p><button id="button_2" class="button">Toggle</button></p>
    </div>
    <div class="card">
      <h2>Output - GPIO 18</h2>
      <p class="state">state: <span id="state_3">%STATE3%</span></p>
      <p><button id="button_3" class="button">Toggle</button></p>
    </div>
  </div>
<script>
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  window.addEventListener('load', onLoad);
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
  }
  function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(event) {
    var state;
    if (event.data == "STATE1_1"){
      state = "ON";
      document.getElementById('state_1').innerHTML = state;
    }
    else if (event.data == "STATE1_0"){
      state = "OFF";
      document.getElementById('state_1').innerHTML = state;
    }
    
    if (event.data == "STATE2_1"){
      state = "ON";
      document.getElementById('state_2').innerHTML = state;
    }
    else if (event.data == "STATE2_0"){
      state = "OFF";
      document.getElementById('state_2').innerHTML = state;
    }

    if (event.data == "STATE3_1"){
      state = "ON";
      document.getElementById('state_3').innerHTML = state;
    }
    else if (event.data == "STATE3_0"){
      state = "OFF";
      document.getElementById('state_3').innerHTML = state;
    }    
  }
  function onLoad(event) {
    initWebSocket();
    initButton();
  }
  function initButton() {
    document.getElementById('button_1').addEventListener('click', toggle_1);
    document.getElementById('button_2').addEventListener('click', toggle_2);
    document.getElementById('button_3').addEventListener('click', toggle_3);
  }
  function toggle_1(){
    websocket.send('toggle1');
  }
  function toggle_2(){
    websocket.send('toggle2');
  }
  function toggle_3(){
    websocket.send('toggle3');
  }

</script>
</body>
</html>
)rawliteral";

void notifyClients(String text) {
  ws.textAll(text);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "toggle1") == 0) {
      ledState1 = !ledState1;
      notifyClients(("STATE1_" + String(ledState1)));
    }
    if (strcmp((char*)data, "toggle2") == 0) {
      ledState2 = !ledState2;
      notifyClients(("STATE2_" + String(ledState2)));
    }
    if (strcmp((char*)data, "toggle3") == 0) {
      ledState3 = !ledState3;
      notifyClients(("STATE3_" + String(ledState3)));
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var){
  Serial.println(var);
  if(var == "STATE1"){
    if (ledState1){
      return "ON";
    }
    else{
      return "OFF";
    }
  }

  if(var == "STATE2"){
    if (ledState2){
      return "ON";
    }
    else{
      return "OFF";
    }
  }

  if(var == "STATE3"){
    if (ledState3){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  digitalWrite(ledPin1, LOW);
  digitalWrite(ledPin2, LOW);
  digitalWrite(ledPin3, LOW);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Start server
  server.begin();
}

void loop() {
  ws.cleanupClients();
  digitalWrite(ledPin1, ledState1);
  digitalWrite(ledPin2, ledState2);
  digitalWrite(ledPin3, ledState3);
}