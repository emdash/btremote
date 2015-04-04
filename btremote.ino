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

/*
 * Define the display, including hardware pin-outs.
 */
Adafruit_PCD8544 display = Adafruit_PCD8544(
   0, // Serial clock out (SCLK)    
   1, // Serial data out (DIN)
   2, // Data/Command select (D/C)
   3, // LCD chip select (CS)
   5  // LCD reset (RST)
);

/*
 * Define input sources, and the mapping from hardware inputs map to
 * UI events.
 */
typedef enum {
   ENC_BTN,
   LEFT_BTN,
   RIGHT_BTN,
} ButtonIds;

#define DEFINE_BUTTON(pin, id, name)		\
   ButtonSrc<					\
      pin,					\
      INPUT_PULLUP,				\
      id,					\
      true					\
    > name

DEFINE_BUTTON( 9, ENC_BTN, encBtn);
DEFINE_BUTTON(12, LEFT_BTN, leftBtn);
DEFINE_BUTTON(13, RIGHT_BTN, rightBtn);
EncoderSrc<'a', 10, 11, WHEEL> encoder;

class ContrastModel : public ProxyModel<uint8_t> {
   public:
      ContrastModel(Adafruit_PCD8544 &display, uint8_t value) :
	 ProxyModel<uint8_t>::ProxyModel(value),
	 m_display(display)
      {
	 update(value);
      };

      void update(uint8_t value) {
	 m_display.setContrast(value);
	 proxy_set(value);
      };

   private:
      Adafruit_PCD8544 &m_display;
};

/*
 * Define the data that we want to display and manipulate.
 */
DirectModel<double>   g_volume(0.5);
DirectModel<boolean>  g_playing(false);
DirectModel<boolean>  g_online_mode(false);
DirectStringModel<25> g_source("Spotify(Starred)");
DirectStringModel<25> g_artist("Phill Collins");
DirectStringModel<25> g_track("In the air tonight.");
ContrastModel         g_contrast(display, 65);

/*
 * Contrast Adjustment.
 */
class ContrastAdjustment : public Screen {
   public:
      ContrastAdjustment() :
	 m_knob(g_contrast, 1, 45, 70) {
      };
      
      void draw(Adafruit_GFX &display) {
	 display.println(g_contrast.value());
      };

      void handle_event(UI &ui, Event &event) {
	 m_knob.handle_event(ui, event);
      };

   private:
      Knob<uint8_t> m_knob;
};

ContrastAdjustment contrast;

/*
 * Main screen
 */
class MainScreen : public Screen {
   public:
      MainScreen() :
	 m_artist_scroll(g_artist.value()),
	 m_track_scroll(g_track.value()),
	 m_source_scroll(g_source.value()),
	 m_play_indicator(g_playing, m_play_icon, m_pause_icon),
	 m_play_controller(g_playing, ENC_BTN),
	 m_volume_controller(g_volume, 0.05, 0, 1.0)
      {	 
      };
      
      void draw(Adafruit_GFX &display) {
	 display.setTextSize(1);
	 display.setTextWrap(false);
	 m_artist_scroll.draw(display);
	 m_track_scroll.draw(display);
	 m_source_scroll.draw(display);

	 drawVolumeIndicator(display);
	 m_play_indicator.draw(display);
      };

      void handle_event(UI &ui, Event &event) {
	 m_play_controller.handle_event(ui, event);
	 m_volume_controller.handle_event(ui, event);
      };

   private:

      void drawVolumeIndicator(Adafruit_GFX &display) {
	 uint8_t w = LCDWIDTH - 1;
	 uint8_t h = LCDHEIGHT - 1;

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
	 uint8_t tfill = (50 - 12) * g_volume.value();
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

      ScrolledText<0, 14> m_artist_scroll;
      ScrolledText<10, 14> m_track_scroll;
      ScrolledText<20, 14> m_source_scroll;
      SpeakerIcon<LCDWIDTH - 11, LCDHEIGHT - 8> m_speaker_icon;
      PlayIcon<0, LCDHEIGHT - 9> m_play_icon;
      PauseIcon<0, LCDHEIGHT - 9> m_pause_icon;
      ToggleView m_play_indicator;
      Toggle m_play_controller;
      Knob<double> m_volume_controller;
};

/*
 * Configure the screens and the main menus.
 */
MainScreen home;

/*
 * Initialize the UI with our root screen.
 */
UI ui(display, home);

void loop() {
   static unsigned long next = 0;
   // Poll input sources for events
   encoder.poll(ui);
   encBtn.poll(ui);
   leftBtn.poll(ui);
   rightBtn.poll(ui);
  
   // update screen every 25ms
   // TODO: dirty detection
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

   encoder.init();
   encBtn.init();
   leftBtn.init();
   rightBtn.init();
  
   // todo: load value from persistent storage
   display.setContrast(60);
   display.clearDisplay();

   // text display tests
   display.setTextSize(1);
   display.setTextColor(BLACK);
}
