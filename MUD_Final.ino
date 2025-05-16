#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <string.h>



// WiFi and MQTT Broker Initiated
const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";

// Initialize the Clients
WiFiClient espClient;
PubSubClient client(espClient);

// Define the LCD Values
#define SDA 14  //Define SDA pins
#define SCL 13  //Define SCL pins
LiquidCrystal_I2C lcd(0x27, 16, 2);

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
void reconnect() {
  while(!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("Connected!");
      client.subscribe("espRequest");
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
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
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char message[50];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  lcd.print(message);
}

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
  char line1[17] = {0}; // Buffer for row 0 (16 chars + null)
  char line2[17] = {0}; // Buffer for row 1 (16 chars + null)
  int pos = 0;          // Current position in string

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

  // Connect to WiFi and Server
  connect_WiFi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  srand(time(NULL));
}
void loop() {
  if(!client.connected()) {
    reconnect();
  }

  // Display Menu on LCD
  client.loop();
  
}
