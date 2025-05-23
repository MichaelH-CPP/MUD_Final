#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <string.h>

// WiFi, Socket, and MQTT Broker Initiated
const char* ssid = "SpectrumSetup-E5";
const char* password = "brightroad484";
const char* mqtt_server = "34.94.123.84";
const char* socket_server = "34.94.123.84";
const int socket_port = 8080;

// Initialize the Clients
WiFiClient espClient;
PubSubClient mqttClient(espClient);
WiFiClient socketClient;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Define the LCD Values
#define SDA 14  //Define SDA pins
#define SCL 13  //Define SCL pins
const int buttonPins[4] = {9,10,11,12};
const char* directions[4] = {"north", "east", "south", "west"};


/*
  void lcd_print(char[] str) :
    - Will print strings and scroll them
*/
void lcd_print(const char* str) {
  int length = strlen(str);
  lcd.clear(); // Clear display initially

  // Case 1: Single line (<=16 chars)
  if (length <= 16) {
    lcd.setCursor(0, 0);
    lcd.print(str);
    delay(1000);
    return;
  }

  // Case 2: Two lines (<=32 chars)
  if (length <= 32) {
    lcd.setCursor(0, 0);
    lcd.print(str);
    lcd.setCursor(0, 1);
    lcd.print(str + 16);
    delay(1000);
    return;
  }

  // Case 3: Vertical scrolling (>32 chars)
    else {
      char line1[17] = {0};
      char line2[17] = {0};
      int pos = 0;

      // Print the first 2 lines
      strncpy(line1, str, 16);
      strncpy(line2, str + 16, 16);
      lcd.setCursor(0, 0); 
      lcd.print(line1);
      lcd.setCursor(0, 1); 
      lcd.print(line2);
      delay(1000);

      // Scroll vertically for remaining characters
      for (pos = 32; pos < length; pos += 16) {
        // Move line2 up to line1
        strncpy(line1, line2, 16);
        
        // Fill line2 with next 16 chars (or remaining chars)
        int remaining = length - pos;
        int copy_len = (remaining >= 16) ? 16 : remaining;
        strncpy(line2, str + pos, copy_len);
        
        // Pad with spaces if needed
        if (copy_len < 16) {
          memset(line2 + copy_len, ' ', 16 - copy_len);
        }

        // Update display
        lcd.setCursor(0, 0); 
        lcd.print(line1);
        lcd.setCursor(0, 1); 
        lcd.print(line2);
        delay(1000); // Adjust scroll speed
    }
  }
}

/*
  [Connection Functions]
*/
void connect_WiFi() {
  delay(100);
  Serial.begin(115200);
  while (!Serial);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("WiFi Connected!\n IP Address: ");
  Serial.println(WiFi.localIP());
}
void mqttReconnect() {
  while(!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("ESP32Client")) {
      Serial.println("Connected!");
      mqttClient.subscribe("espRequest");
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println("; Retrying in 3 seconds...");
      delay(3000);
    }
  }
}
bool i2CAddrTest(uint8_t addr) {
  Wire.beginTransmission(addr);
  if (Wire.endTransmission() == 0) {
    return true;
  }
  return false;
}
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  char message[100] = {0};
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  lcd_print(message);
  Serial.print("MQTT Message: ");
  Serial.println(message);
}

/*
  [Socket Functions]
*/
void sendDirection(const char* dir) {
  if (!socketClient.connected()) {
    if (!socketClient.connect(socket_server, socket_port)) {
      Serial.println("Socket connection failed");
      return;
    }
  }
  
  if (socketClient.connected()) {
    socketClient.print(dir);
    Serial.print("Sent direction: ");
    Serial.println(dir);
  }
}

void setup() {
  // Set up the LCD
  Wire.begin(SDA, SCL);  // attach the IIC pin
  if (!i2CAddrTest(0x27)) {
    lcd = LiquidCrystal_I2C(0x3F, 16, 2);
  }
  lcd.init();           // LCD driver initialization
  lcd.backlight();      // Open the backlight
  lcd.setCursor(0, 0);  // Move the cursor to row 0, column 0
  lcd.print("LCD on!");

  for (int i = 0; i < 4; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  // Connect to WiFi and Server
  connect_WiFi();
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(mqttCallback);
  srand(time(NULL));
}

void loop() {
  if(!mqttClient.connected()) {
    mqttReconnect();
  }
  mqttClient.loop();

  // Handle Button Presses
  for (int i = 0; i < 4; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      sendDirection(directions[i]);
      delay(200); // Debounce
      while (digitalRead(buttonPins[i]) == LOW);
    }
  }
}
