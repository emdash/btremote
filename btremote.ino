/*********************************************************************
btremote.ino - Firmware for BLE remote based on Blend Mirco,
AdaEncoder, and Adafruit_GFX. Based on custom UI framework in this
sketch.
*********************************************************************/

/*
 * Defining these macros hopefully reduces the footprint of the
 * ooPinChangeInt library by about 100 bytes. We are trying to squeeze
 * every last drop of ram at this point. This also prevents the
 * library from breaking the bluetooth interface.
 */
#define NO_PORTC_PINCHANGES
#define NO_PORTD_PINCHANGES

/*
 * This is a hack. Let's see if it works.
 */

#include <ooPinChangeInt.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <AdaEncoder.h>
#include <string.h>
#include <boards.h>
#include <RBL_nRF8001.h>

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
class ContrastModel : public ProxyModel<double> {
   public:
      ContrastModel(Adafruit_PCD8544 &display, double value) :
	 ProxyModel<double>::ProxyModel(value),
	 m_display(display)
      {
	 update(value);
      };

      void update(double value) {
	 m_display.setContrast(min_contrast + 
			       value * double(max_contrast - min_contrast));
	 proxy_set(value);
      };

   private:
      Adafruit_PCD8544 &m_display;

      // values determined experimentally and may need adjustment.
      static const uint8_t min_contrast = 48;
      static const uint8_t max_contrast = 70;
};


/*
 * Controllers which make network requests in response to user
 * Define the main screeninput. This is a dummy implementation which
 * fakes network calls over the Serial line.
 */
typedef enum {
   TOGGLE_PLAYBACK,
   NEXT_TRACK,
   PREV_TRACK,
   LIKE_TRACK,
   TOGGLE_ONLINE,
   NEXT_PLAYLIST,
   PREV_PLAYLIST,
} NetworkMessage;

#define BT_CASE(code, name)			\
   case name:					\
   Serial.print('>');				\
   Serial.println(code);			\
   break

class NetworkController : public Command {
   public:
      NetworkController(
	 NetworkMessage message,
	 EventType source,
	 uint8_t id) :
	 Command::Command(source, id),
	 m_message(message) {
      };

      void action(UI &ui) {
	 switch (m_message) {
	    BT_CASE('x', TOGGLE_PLAYBACK);
	    BT_CASE('N', NEXT_TRACK);
	    BT_CASE('P', PREV_TRACK);
	    BT_CASE('L', LIKE_TRACK);
	    BT_CASE('p', PREV_PLAYLIST);
	    BT_CASE('n', NEXT_PLAYLIST);
	    BT_CASE('o', TOGGLE_ONLINE);
	 };
      };
   private:
      NetworkMessage m_message;
};


/*
 * Define the data that we want to display and manipulate.
 */
DirectModel<double>   g_volume(0.5);
DirectModel<boolean>  g_playing(false);
DirectModel<boolean>  g_online(false);
DirectModel<boolean>  g_paired(true);
DirectStringModel<25> g_source("Spotify(Starred)");
DirectStringModel<25> g_artist("Phill Collins");
DirectStringModel<25> g_track("In the air tonight.");
ContrastModel         g_contrast(display, 0.5);

/*
 * Settings Screen.
 */
Label g_contrast_label("Contrast:");
Label g_playlist_label("Playlist:");
RangeView<double> g_contrast_indicator(g_contrast, 0, 1.0);
ToggleView g_network_indicator(g_online, g_online_icon, g_offline_icon);

Knob<double> g_contrast_controller(g_contrast, 0.05, 0, 1.0);
PopController g_back_button(CLICK, ENC_BTN);
NetworkController g_prev_playlist(PREV_PLAYLIST, CLICK, LEFT_BTN);
NetworkController g_next_playlist(NEXT_PLAYLIST, CLICK, RIGHT_BTN);


Layout<3, 4> settings_layout = {
   {
      {{ 0,  0, LCDWIDTH - 1, 10}, g_contrast_label},
      {{ 0,  8, LCDWIDTH - 1,  6}, g_contrast_indicator},
      {{ 0, 20, LCDWIDTH - 1, 10}, g_playlist_label},
   },
   {
      {g_contrast_controller},
      {g_back_button},
      {g_prev_playlist},
      {g_next_playlist}
   }
};

CompositeScreen<3, 4> g_settings(settings_layout);

/*
 * Views for the main screen.
 */
ScrolledText g_artist_scroll(g_artist.value());
ScrolledText g_track_scroll(g_track.value());
ScrolledText g_source_scroll(g_source.value());
RangeView<double> g_volume_indicator(g_volume, 0, 1.0);

ToggleView g_play_indicator(g_playing, g_play_icon, g_pause_icon);

/*
 * Controllers for the main screen.
 */
Toggle g_play_controller(g_playing, ENC_BTN);
Knob<double> g_volume_controller(g_volume, 0.05, 0, 1.0);
PushController g_show_settings(g_settings, HOLD, ENC_BTN);
NetworkController g_prev_controller(PREV_TRACK, CLICK, LEFT_BTN);
NetworkController g_next_controller(NEXT_TRACK, CLICK, RIGHT_BTN);
NetworkController g_like_controller(LIKE_TRACK, HOLD, LEFT_BTN);
Toggle g_online_controller(g_online, RIGHT_BTN, HOLD);

/*
 * Define the main screen
 */
const Layout<7, 7> main_layout = {
   {
      {{0, 0, LCDWIDTH, 10}, g_source_scroll},
      {{0, 10, LCDWIDTH, 10}, g_artist_scroll}, 
      {{0, 20, LCDWIDTH, 10}, g_track_scroll},
      {{LCDWIDTH - 11, LCDHEIGHT - 9, 0, 0}, g_speaker_icon},
      {{30, LCDHEIGHT - 9, 40, 8}, g_volume_indicator},
      {{0, LCDHEIGHT - 9, 0, 0}, g_play_indicator},
      {{10, LCDHEIGHT - 9, 0, 0}, g_network_indicator},
   },
   {
      {g_play_controller},
      {g_volume_controller},
      {g_show_settings},
      {g_prev_controller},
      {g_next_controller},
      {g_like_controller},
      {g_online_controller}
   }
};

CompositeScreen<7, 7> home(main_layout);

/*
 * This screen shows if we are not paired to a phone.
 */
Label g_unpaired_screen("Press any key to pair.");
ToggleView root(g_paired, home, g_unpaired_screen);

/*
 * Initialize the UI with our root screen.
 */

UI ui(display, root);

void handle_bt_char(char c) {
   static enum {
      NORMAL = 0,
      STRING,
      VOLUME_LOW,
      VOLUME_HIGH
   } mode;

   static char *buffer = 0;
   static uint8_t i = 0;
   static int volume = 0;

   Serial.print('<');
   Serial.println(c);

   // update string buffers in place. buffer should point to a valid
   // storage location. no bounds checking, be careful!.
   if (mode == STRING) {
      if (c == '\n') {
	 buffer[i] = 0;
	 mode = NORMAL;
      } else {
	 buffer[i++] = c;
      }
      return;
   // lower nibble of volume byte
   } else if (mode == VOLUME_LOW) {
      if ((c >= '0') && (c <= '9')) {
	 volume |= (c - '0');
      } else if (('a' <= c) && (c <= 'f')) {
	 volume |= 10 + (c - 'a');
      }
      mode = NORMAL;
      g_volume.update(double(volume) / 255);
      return;
   // upper nibble of volume byte.
   } else if (mode == VOLUME_HIGH) {
      if ((c >= '0') && (c <= '9')) {
	 volume |= (c - '0');
      } else if (('a' <= c) && (c <= 'f')) {
	 volume |= 10 + (c - 'a');
      }
      volume = volume << 4;
      mode = VOLUME_LOW;
      return;
   }

   switch (c) {
      case 'x':
	 g_playing.update(false);
	 break;
      case 'X':
	 g_playing.update(true);
	 break;
      case 'o':
	 g_online.update(false);
	 break;
      case 'O':
	 g_online.update(true);
	 break;
      case 's':
	 buffer = (char *) g_source.value();
	 mode = STRING;
	 i = 0;
	 break;
      case 'a':
	 buffer = (char *) g_artist.value();
	 mode = STRING;
	 i = 0;
	 break;
      case 't':
	 buffer = (char *) g_track.value();
	 mode = STRING;
	 i = 0;
	 break;
      case 'v':
	 volume = 0;
	 mode = VOLUME_HIGH;
	 break;
   };
}

void loop() {
   static unsigned long next = 0;

   // Poll input sources for events
   encoder.poll(ui);
   encBtn.poll(ui);
   leftBtn.poll(ui);
   rightBtn.poll(ui);

   while (ble_available()) {
      handle_bt_char(ble_read());
   }
   ble_do_events();

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
   ble_begin();

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
