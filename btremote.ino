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
#include <string.h>
#include "WheelUI.h"
#include "MVC.h"
#include "Icons.h"

#define LEN(x) (sizeof(x) / sizeof(x[0]))

// Software SPI (slower updates, more flexible pin options):
Adafruit_PCD8544 display = Adafruit_PCD8544(
   0, // Serial clock out (SCLK)    
   1, // Serial data out (DIN)
   2, // Data/Command select (D/C)
   3, // LCD chip select (CS)
   5  // LCD reset (RST)
);

/*
 * Characters per line at different font size settings. This assumes
 * the default orientation.
 */
const uint8_t CHARS_PER_LINE[] = {
   14,
   14,
   6,
   5,
};

/*
 * Button IDs
 */
typedef enum {
   ENC_BTN,
   LEFT_BTN,
   RIGHT_BTN,
} ButtonIds;

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
	    ui.put(255, 0);
	 }
      };

   private:
      Adafruit_PCD8544 &m_display;
      uint8_t m_contrast;
      uint8_t m_knob_id;
      uint8_t m_back_button;
};

/*
 * Main screen
 */


class MainScreen : public Screen {
   public:
      MainScreen() :
	 m_playing(false),
	 m_volume(0.5),
	 m_artist_scroll(m_artist),
	 m_track_scroll(m_track),
	 m_source_scroll(m_source),
	 m_play_model(m_playing),
	 m_volume_model(m_volume),
	 m_play_indicator(m_play_model, m_play_icon, m_pause_icon),
	 m_play_controller(m_play_model, ENC_BTN),
	 m_volume_controller(m_volume_model, 0.05, 0, 1.0)
      {
	 strncpy(m_artist, "Phil Collins", sizeof(m_artist));
	 strncpy(m_track, "In the air tonight", sizeof(m_track));
	 strncpy(m_source, "Spotify: Stared", sizeof(m_source));
      };
      
      void draw(Adafruit_GFX &display) {
	 uint8_t w = display.width() - 1;
	 uint8_t h = display.height() - 1;
	 uint8_t pos = (millis() / 100) % w;

	 display.setTextSize(1);
	 display.setTextWrap(false);
	 display.setCursor(-pos, 0);
	 m_artist_scroll.draw(display);
	 m_track_scroll.draw(display);
	 m_source_scroll.draw(display);

	 drawVolumeIndicator(display, w, h);
	 m_play_indicator.draw(display);
      };

      void handle_event(UI &ui, Event &event) {
	 m_play_controller.handle_event(ui, event);
	 m_volume_controller.handle_event(ui, event);
      };

   private:

      void drawVolumeIndicator(Adafruit_GFX &display, uint8_t w, uint8_t h) {
	 m_speaker_icon.draw(display);

	 // Draw a triangle outline that "fills up" according to the
	 // volume level. I.e. at 0 volume, it's just a solid
	 // outline. At full volume it's completely filled.

	 // First, draw the filled triangle.
	 display.fillTriangle(
	    w - 50, h,
	    w - 12, h,
	    w - 12, h - 8,
	    BLACK);

	 // Now, erase the unfilled portion of the triangle.
	 uint8_t tfill = (50 - 12) * m_volume;
	 display.fillRect(
	    w - 50 + tfill, h - 8,
	    50 - 12 - tfill, h,
	    WHITE);

	 // Now, draw the triangle outline.
	 display.drawTriangle(
	    w - 50, h,
	    w - 12, h,
	    w - 12, h - 8,
	    BLACK);
      };

      char m_artist[20];
      char m_track[20];
      char m_source[20];
      boolean m_playing;
      float m_volume;
      ScrolledText<20,  0, 14> m_artist_scroll;
      ScrolledText<20, 10, 14> m_track_scroll;
      ScrolledText<20, 20, 14> m_source_scroll;
      SpeakerIcon<LCDWIDTH - 11, LCDHEIGHT - 8> m_speaker_icon;
      PlayIcon<0, LCDHEIGHT - 9> m_play_icon;
      PauseIcon<0, LCDHEIGHT - 9> m_pause_icon;
      RefModel<boolean> m_play_model;
      RefModel<float> m_volume_model;
      ToggleView m_play_indicator;
      Toggle m_play_controller;
      Knob<float> m_volume_controller;
};


#define DEFINE_BUTTON(pin, id, name)		\
   ButtonSrc<					\
      pin,					\
      INPUT_PULLUP,				\
      id,					\
      BUTTON_PRESS,				\
      BUTTON_RELEASE,				\
      true					\
    > name

DEFINE_BUTTON( 9, ENC_BTN, encBtn);
DEFINE_BUTTON(12, LEFT_BTN, leftBtn);
DEFINE_BUTTON(13, RIGHT_BTN, rightBtn);

EncoderSrc<'a', 10, 11, WHEEL> volume;

/*
 * Configure the screens and the main menus.
 */
MainScreen home;
ContrastAdjustment contrast(display, WHEEL, ENC_BTN);
TestScreen t3;

#if 0
MenuItem main_menu_items[] = {
   {"Home", home},
   {"Contrast", contrast},
   {"Test3", t3},
};
Menu<LEN(main_menu_items)> main_menu(main_menu_items);
#endif

/*
 * Initialize the UI and the main display.
 */
UI ui(display, home);

void loop() {
   static unsigned long next = 0;
   // Poll for events
   volume.poll(ui);
   encBtn.poll(ui);
   leftBtn.poll(ui);
   rightBtn.poll(ui);
  
   if (millis() > next) {
      display.clearDisplay();
      ui.loop();
      next = millis() + 25;
   }
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
