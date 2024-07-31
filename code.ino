#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define ENCODER_CLK 14
#define ENCODER_DT  15
#define ENCODER_SW  16

#define ONE_WIRE_BUS 17
#define DHT_PIN 18
#define DHT_TYPE DHT22

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DHT dht(DHT_PIN, DHT_TYPE);

const char* menu[] = {"Temperature", "Humidity", "Option 3", "Option 4", "Option 5"};
const int menuLength = sizeof(menu) / sizeof(menu[0]);
int currentOption = 0;

int lastClk = HIGH;
bool inTemperatureMode = false;
bool inHumidityMode = false;
bool buttonPressed = false;

void setup() {
  Serial.begin(9600);
  
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
  pinMode(ENCODER_SW, INPUT_PULLUP);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  sensors.begin();
  dht.begin();
  displayMenu();
}

void loop() {
  int newClk = digitalRead(ENCODER_CLK);

  if (newClk != lastClk) {
    lastClk = newClk;
    if (newClk == LOW) {
      int dtValue = digitalRead(ENCODER_DT);
      if (dtValue == HIGH) {
        currentOption = (currentOption + 1) % menuLength;
      } else {
        currentOption = (currentOption - 1 + menuLength) % menuLength;
      }
      if (!inTemperatureMode && !inHumidityMode) {
        displayMenu();
      }
    }
  }

  if (digitalRead(ENCODER_SW) == LOW) {
    if (!buttonPressed) {
      delay(100);
      buttonPressed = true;
      if (inTemperatureMode) {
        inTemperatureMode = false;
        displayMenu();
      } else if (inHumidityMode) {
        inHumidityMode = false;
        displayMenu();
      } else if (currentOption == 0) { 
        inTemperatureMode = true;
        displayTemperature();
      } else if (currentOption == 1) {
        inHumidityMode = true;
        displayHumidity();
      }
    }
  } else {
    buttonPressed = false; 
  }
}

void displayMenu() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("Menu"));
  for (int i = 0; i < menuLength; i++) {
    if (i == currentOption) {
      display.print(F("> "));
    } else {
      display.print(F("  "));
    }
    display.println(menu[i]);
  }
  display.display();
}

void displayTemperature() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("Temperature"));

  while (inTemperatureMode) {
    sensors.requestTemperatures();
    float temperatureC = sensors.getTempCByIndex(0);

    display.setCursor(0, 16);
    display.setTextSize(2);
    display.print(F("Temp: "));
    display.print(temperatureC);
    display.print(F(" C"));
    display.display();

    delay(500); 

    if (digitalRead(ENCODER_SW) == LOW) {
      if (!buttonPressed) {
        delay(100); 
        buttonPressed = true;
        inTemperatureMode = false;
        displayMenu();
      }
    } else {
      buttonPressed = false; 
    }
  }
}

void displayHumidity() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("Humidity"));

  while (inHumidityMode) {
    float humidity = dht.readHumidity();

    display.setCursor(0, 16);
    display.setTextSize(2);
    display.print(F("Humidity: "));
    display.print(humidity);
    display.print(F(" %"));
    display.display();

    delay(500); 

    if (digitalRead(ENCODER_SW) == LOW) {
      if (!buttonPressed) {
        delay(100); 
        buttonPressed = true;
        inHumidityMode = false;
        displayMenu();
      }
    } else {
      buttonPressed = false; 
    }
  }
}
