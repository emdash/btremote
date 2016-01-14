This is the firmware for my BLE media remote. It is intended to be
compiled as an Arduino sketch, via the Arduino IDE. I do my editing in
EMACS, however.

The BLE remote is based on the Blend Micro from Red Bear Labs, and the
firmware relies on Red Bear Labs' BLE libraries. These must be
installed correctly into the Arduino IDE, as per their
instructions. This involves editing some files within the IDE itself
(pretty sketch).

Other dependencies:
- pcchangeint: An interrupt library (required for AdaEncoder)
- AdaEncoder: library for working with rotary encoders
- AdaGFX: Common code for AdaFruit graphics LCDs
- AdaGFX_PCD8544: Graphics driver code specific to my chosen screen.

I found it necessary to modify the libraries supplied from Red Bear
Labs. They used too many string literals, and the resulting binary
just did not fit into the 28KB of program space remaining (after the
bootloader eats up 4k). I replaced all the string literals in the Red
Bear Labs bluetooth library with single-character constants, removing
some redundant debugging statements in the process.

If I had more time, I would write my own C++ BLE wrapper around the
vendor-supplied ACI library for the NRF8001 BLE chipset. This library
would allow defining custom characteristics, and would be much
cleaner, and probably quite a bit more compact. Unfortunately, I don't
have such time, and so I'm stuck with serial emulation for the time
being. As a result, there's a rather complicated and silly state
machine which parses BLE data character-by-character. I hate writing
code like that, but unfortunately the Arduino doesn't offer much in
the way of string handling. Merely avoiding the use of the string
manipulation functions saves a few hundred bytes of code. And parsing
the character stream this way allows it all do be done in rougly
constant space.

As I said, the right solution is to implement custom characteristics
for each message, and properly advertise them via BLE. Unfortunately,
there is just way too much work involved. Maybe someday. Probably, by
then, the Blend Micro will have been superceded by some much more
powerul (and cheaper) gadget.

Other Notes:

I implemented a small, static UI framework. It's a rough sort of
MVC. It's heavily templated, and has actually proved to be both
compact, memory efficient, and reasonably performant. You allocate all
your UI elements globally (this is a common practice in Arduino-land,
since effectivley a sketch has to be a single translation unit), and
then a UI helper object does the heavy lifting during run-time. For
more, see:

- UserInterface.h
- MVC.h
- WheelUI.h

Icons are pixel-art, defined as arrays of binary literals, which are
then wrapped in a UI View.

- icons.h