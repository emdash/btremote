/*********************************************************************
This is an example sketch for our Monochrome Nokia 5110 LCD Displays

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/338

These displays use SPI to communicate, 4 or 5 pins are required to
interface

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

#include <ooPinChangeInt.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <AdaEncoder.h>
#include "WheelUI.h"

// Software SPI (slower updates, more flexible pin options):
// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(0, 1, 2, 3, 5);

extern Menu main_menu;

TextScreen t1 = TextScreen("Test 1", main_menu);
TextScreen t2 = TextScreen("Test 2", main_menu);
TestScreen t3;

MenuItem main_menu_items[] = {
  {"Test1", t1},
  {"Test2", t2},
  {"Test3", t3},
  {0, t3}
};

Menu main_menu = Menu(main_menu_items);
UI ui = UI(display, main_menu);
AdaEncoder volume = AdaEncoder('a', 10, 11);

typedef enum {
  ENCODER_CLICK = 1,
  BUTTON_PRESS,
  BUTTON_RELEASE
} EventSources;

typedef enum {
  ENCODER = 1,
  LEFT,
  RIGHT
} Buttons;

void initPins() {
  for (int i = 9; i < 14; i++) {
    pinMode(i, INPUT_PULLUP);
  }
}

void pollButtons() {
  static boolean cur_encoder = 1;
  static boolean cur_left = 1;
  static boolean cur_right = 1;
  static unsigned long debounce = millis() + 100;
  
  if (millis() < debounce) {
    return;
  }
  
  boolean encoder = digitalRead(9);
  boolean left = digitalRead(12);
  boolean right = digitalRead(13);
  
  if (encoder != cur_encoder) {
    ui.put(encoder ? BUTTON_RELEASE : BUTTON_PRESS, ENCODER);
    debounce = millis() + 100;
  }
  
  if (left != cur_left) {
    ui.put(left ? BUTTON_RELEASE : BUTTON_PRESS, LEFT);
    debounce = millis() + 100;
  }
  
  if (right != cur_right) {
    ui.put(right ? BUTTON_RELEASE : BUTTON_PRESS, RIGHT);
    debounce = millis() + 100;
  }
  
  cur_encoder = encoder;
  cur_left = left;
  cur_right = right;
}

void loop() {
  // Poll for encoder clicks
  if (volume.getClicks()) {
    ui.put(ENCODER, volume.query());
  }
  
  pollButtons();  
  
  display.clearDisplay();
  ui.loop();
  display.display();
}

void setup() {
  Serial.begin(9600);
  display.begin();
  
  initPins();

  // you can change the contrast around to adapt the display
  // for the best viewing!
  display.setContrast(55);
  display.clearDisplay();   // clears the screen and buffer

  // text display tests
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.display();
}
