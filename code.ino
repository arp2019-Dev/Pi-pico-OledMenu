#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <IRremote.h>
#include <TM1637Display.h>

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

#define IR_RECEIVE_PIN 19
#define POT_PIN 26
#define TM1637_CLK 20
#define TM1637_DIO 21
#define RELAY_PIN 22

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DHT dht(DHT_PIN, DHT_TYPE);
TM1637Display tm1637(TM1637_CLK, TM1637_DIO);

const char* menu[] = {"Temperature", "Humidity", "IR Signal", "Potentiometer", "Option 5"};
const int menuLength = sizeof(menu) / sizeof(menu[0]);
int currentOption = 0;

int lastClk = HIGH;
bool inTemperatureMode = false;
bool inHumidityMode = false;
bool inIRMode = false;
bool inPotentiometerMode = false;
bool buttonPressed = false;

void setup() {
  Serial.begin(115200);
  
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Ensure relay is off

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
  tm1637.setBrightness(0x0f); // Set maximum brightness

  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  
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
      if (!inTemperatureMode && !inHumidityMode && !inIRMode && !inPotentiometerMode) {
        displayMenu();
      }
    }
  }

  if (digitalRead(ENCODER_SW) == LOW) {
    if (!buttonPressed) {
      delay(100); // Debounce delay
      buttonPressed = true;
      if (inTemperatureMode) {
        inTemperatureMode = false;
        displayMenu();
      } else if (inHumidityMode) {
        inHumidityMode = false;
        displayMenu();
      } else if (inIRMode) {
        inIRMode = false;
        displayMenu();
      } else if (inPotentiometerMode) {
        inPotentiometerMode = false;
        displayMenu();
      } else if (currentOption == 0) {
        inTemperatureMode = true;
        displayTemperature();
      } else if (currentOption == 1) {
        inHumidityMode = true;
        displayHumidity();
      } else if (currentOption == 2) {
        inIRMode = true;
        displayIRSignal();
      } else if (currentOption == 3) {
        inPotentiometerMode = true;
        displayPotentiometer();
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

void displayIRSignal() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("IR Signal"));

  while (inIRMode) {
    if (IrReceiver.decode()) {
      display.setCursor(0, 16);
      display.setTextSize(2);
      display.print(F("Signal: YES"));
      display.display();
      IrReceiver.resume();
    } else {
      display.setCursor(0, 16);
      display.setTextSize(2);
      display.print(F("Signal: NO "));
      display.display();
    }

    delay(500);

    if (digitalRead(ENCODER_SW) == LOW) {
      if (!buttonPressed) {
        delay(100); 
        buttonPressed = true;
        inIRMode = false;
        displayMenu();
      }
    } else {
      buttonPressed = false;
    }
  }
}

void displayPotentiometer() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("Potentiometer"));

  while (inPotentiometerMode) {
    int potValue = analogRead(POT_PIN);
    int displayValue = map(potValue, 0, 4095, 0, 1000);

    display.setCursor(0, 16);
    display.setTextSize(2);
    display.print(F("Value: "));
    display.print(displayValue);
    display.display();

    tm1637.showNumberDec(displayValue, false);

    if (displayValue == 1000) {
      digitalWrite(RELAY_PIN, HIGH);
    } else {
      digitalWrite(RELAY_PIN, LOW);
    }

    delay(500);

    if (digitalRead(ENCODER_SW) == LOW) {
      if (!buttonPressed) {
        delay(100); 
        buttonPressed = true;
        inPotentiometerMode = false;
        displayMenu();
      }
    } else {
      buttonPressed = false;
    }
  }
}
