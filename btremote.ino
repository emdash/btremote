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

ButtonSrc< 9, INPUT_PULLUP, ENC_BTN, true> encBtn;
ButtonSrc<12, INPUT_PULLUP, LEFT_BTN, true> leftBtn;
ButtonSrc<13, INPUT_PULLUP, RIGHT_BTN, true> rightBtn;
EncoderSrc<'a', 10, 11, WHEEL> encoder;

/*
 * Define a model for the contrast setting on a supported display.
 */
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
 * Views for the main screen.
 */
ScrolledText< 0, 14> g_artist_scroll(g_artist.value());
ScrolledText<10, 14> g_track_scroll(g_track.value());
ScrolledText<20, 14> g_source_scroll(g_source.value());

SpeakerIcon<LCDWIDTH - 11, LCDHEIGHT - 8> g_speaker_icon;
PlayIcon   <0,             LCDHEIGHT - 9> g_play_icon;
PauseIcon  <0,             LCDHEIGHT - 9> g_pause_icon;

RangeView<double> g_volume_indicator(
   g_volume, 0, 1.0,
   LCDWIDTH - 53, LCDHEIGHT - 9,
   40, 8
);
ToggleView g_play_indicator(g_playing, g_play_icon, g_pause_icon);

/*
 * Controllers for the main screen.
 */
Toggle g_play_controller(g_playing, ENC_BTN);
Knob<double> g_volume_controller(g_volume, 0.05, 0, 1.0);

/*
 * Define the main screen
 */
const Layout<6, 2> main_layout = {
   {
      {g_source_scroll},
      {g_artist_scroll}, 
      {g_track_scroll},
      {g_speaker_icon},
      {g_volume_indicator},
      {g_play_indicator},
   },
   {
      {g_play_controller},
      {g_volume_controller},
   }
};

CompositeScreen<6, 2> home(main_layout);

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
   Serial.println("got here");

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
