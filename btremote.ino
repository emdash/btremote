/*********************************************************************
btremote.ino - Firmware for BLE remote based on Blend Mirco,
AdaEncoder, and Adafruit_GFX. Based on custom UI framework in this
sketch.
*********************************************************************/

#include <ooPinChangeInt.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <AdaEncoder.h>
#include "WheelUI.h"

#define LEN(x) (sizeof(x) / sizeof(x[0]))

/*
 * Contrast Adjustment.
 */
class ContrastAdjustment : public Screen {
   public:
      ContrastAdjustment(Adafruit_PCD8544 &display,
			 uint8_t knob_id,
			 uint8_t back_button) :
	 m_display(display),
	 m_contrast(55),
	 m_knob_id(knob_id),
	 m_back_button(back_button) {};
      
      void draw(Adafruit_GFX &display) {
	 display.println(m_contrast);
      };

      void handle_event(UI &ui, Event &event) {
	 if (event.source == m_knob_id) {
	    m_contrast = min(255, max(0, m_contrast + (char) event.data));
	    m_display.setContrast(m_contrast);
	 } else if (event.data && event.source == m_back_button) {
	    Serial.println("got here");
	    ui.put(255, 0);
	 }
      };

   private:
      Adafruit_PCD8544 &m_display;
      uint8_t m_contrast;
      uint8_t m_knob_id;
      uint8_t m_back_button;
};


// Software SPI (slower updates, more flexible pin options):
// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(0, 1, 2, 3, 5);

/* 
 * Configure the input sources.
 */
typedef enum {
   ENC_WHEEL = 1,
   ENC_BTN,
   LEFT_BTN,
   RIGHT_BTN,
} EventSources;

EncoderSrc<'a', 10, 11, ENC_WHEEL>           volume;
ButtonSrc< 9, INPUT_PULLUP, ENC_BTN,   true> encBtn;
ButtonSrc<12, INPUT_PULLUP, LEFT_BTN,  true> leftBtn;
ButtonSrc<13, INPUT_PULLUP, RIGHT_BTN, true> rightBtn;

/*
 * Configure the screens and the main menus.
 */
TextScreen t1("Test 1", LEFT_BTN);
ContrastAdjustment contrast(display, ENC_WHEEL, ENC_BTN);
TestScreen t3;

MenuItem main_menu_items[] = {
   {"Test1", t1},
   {"Contrast", contrast},
   {"Test3", t3},
};
Menu<LEN(main_menu_items)> main_menu(main_menu_items);

/*
 * Initialize the UI and the main display.
 */
UI ui(display, main_menu);

void loop() {
   // Poll for events
   volume.poll(ui);
   encBtn.poll(ui);
   leftBtn.poll(ui);
   rightBtn.poll(ui);
  
   display.clearDisplay();
   ui.loop();
   display.display();
}

void setup() {
   Serial.begin(9600);
   display.begin();

   volume.init();
   encBtn.init();
   leftBtn.init();
   rightBtn.init();
  
   // you can change the contrast around to adapt the display
   // for the best viewing!
   display.setContrast(55);
   display.clearDisplay();   // clears the screen and buffer

   // text display tests
   display.setTextSize(1);
   display.setTextColor(BLACK);
   display.display();
}
