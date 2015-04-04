#ifndef ICONS_H
#define ICONS_H

#define DECLARE_ICON_VIEW(NAME, W, H, DATA)			\
   template <uint8_t X,						\
	     uint8_t Y>						\
   class NAME : public IconView<W, H>				\
   {								\
     public:							\
      NAME() : IconView<W, H>(X, Y, DATA) {};			\
   }

static const uint8_t PROGMEM SPEAKER_DATA[] = {
   B00000100, B10000000,
   B00001100, B01000000,
   B00011101, B00100000,
   B11111100, B10100000,
   B11111100, B10100000,
   B00011101, B00100000,
   B00001100, B01000000,
   B00000100, B10000000,
};

DECLARE_ICON_VIEW(SpeakerIcon, 16, 8, SPEAKER_DATA);

static const unsigned char PROGMEM PLAY_DATA[] = {
   B11100000,
   B11110000,
   B11111000,
   B11111100,
   B11111110,
   B11111100,
   B11111000,
   B11110000,
   B11100000,
};

DECLARE_ICON_VIEW(PlayIcon, 8, 9, PLAY_DATA);

static const unsigned char PROGMEM PAUSE_DATA[] = {
   B11100111,
   B11100111,
   B11100111,
   B11100111,
   B11100111,
   B11100111,
   B11100111,
   B11100111,
   B11100111,
};

DECLARE_ICON_VIEW(PauseIcon, 8, 9, PAUSE_DATA);

#endif
