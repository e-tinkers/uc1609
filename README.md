Table of contents
---------------------------

* [Overview](#overview)
* [Installation](#installation)
* [Hardware](##hardware)
* [Software](#software)
* [Datasheet](#datasheet)

![ LCD19264 ](https://github.com/e-tinkers/uc1609/blob/main/image/LCD19264_with_UC1609.jpg)

Overview
--------------------

This Arduino library provides light weight display support for LCD modules that is commonly known as LCD19264 which utilizes UC1609C LCD controller from UltraChip with Hardware SPI interface. The library should works on any Arduino Core. 

The library does not using buffer and access the on-module/chip display RAM directly, can capable to display full-screen or partial images. 

Installation
------------------------------

**Arduino IDE**

As this library is not registered through Arduino Library Manager (yet), to install the library for Arduino IDE, download this github repository and add it to your Arduino library directory.

**PlatformIO**

For those who are using platformIO, add the following line to the `platformio.ini` as `lib_deps`:

```
libs_dep
    https://github.com/e-tinkers/uc1609
```

Hardware
----------------------------

The UC1609 itself is operated at 3.3v, the LCD19264 modules have two versions with operating voltage of 5V or 3.3V. The 5V version comes with an LDO chip soldered on the back of the display module, which bring the 5V input down to 3.3v before feeding into the module. The 3.3V version comes without the LDO chip and allows direct supply of 3.3V to the module. The Backlight should always be connected to 3.3V according to datasheet. 

The LCD19264 has a 9-pin interface on the left of the LCD module.

**Pin Description**

| Identification | Description          |
| -------------- | -------------------- |
| K              | Cathode of backlight |
| A              | Anode of backlight   |
| GND            | Ground               |
| VDD            | Supply Voltage       |
| SCK            | Clock (SCLK)         |
| SDA            | Data (MOSI)          |
| RST            | Reset                |
| C/D            | Command/Data mode    |
| CS             | Chip Select          |

Software
-------------------------

**SPI**

The library only supports Hardware SPI defined by the Arduino Core that the library is running on. 

**buffers**

The library does not use buffer like other libraries found online. As the result the library uses only less than 30 bytes of RAM and limited Flash memory (depeond on the font it is used).

**fonts**

A full 255-character font is carefully implemented and allows user to select a subset to reduce the flash memory footprint. The following diagram ilustrated how the various fonts are overlapping with each other.

```
ASCII Range   Font Name
0x00 ---------------------------------------------------------
 :                                         |                 |
0x1F                                       |                 |
0x20 ------------------------------   font5x7_symbol         |
 :          | font5x7_numeric     |        |                 |
0x3A --------                  font5x7     |          font5x7_extended       
 :                                |        |                 |
0x7F ---------------------------------------                 |
0x80                                                         |
 :                                                           |
0xFF ---------------------------------------------------------
```

Each of these fonts take up different size in flash memory during compilation of the code. 

The `font5x7_numeric` only consists of 27 characters from ASCII 0x20(' ') to ASCII 0x3A(':') with a memory footprint of 135 bytes, it is ideal for applications where you only need some symbols and numeric numbers such as displaying date, time or + and - sign, etc. Refer to `test_UC1609_font5x7_numeric.ino` example for its usage.

The `font5x7` provides *printable* range of ASCII from 0x20 to 0x7F. By default `font5x7` would be used if not explicitly set using `UC1609::setFont()` method. It requires 480 bytes of flash memory.

The `font5x7_symbol` add some emoji-alike icons and symbol between ASCII 0x00 and 0x1F in addition to include the range of `font5x7`. Memory footprint for the font is 640 bytes.

The `font5x7_extended` covers the full range of ASCII 0x00 to 0xFF that consists of all the fonts in `font5x7_symbol` plus the ASCII 0x80-0xFF. It takes up of 1280 bytes of Flash memory.

**bitmaps**

The `test_UC1609_bitmap.ino` consists of an `image.h` file which defines a full screen image as well as a couple of 24x24 pixels icons for demonstration purchase. User can defined their own images or icons using bitmap (BMP) image file with the help of online [image2cpp](https://javl.github.io/image2cpp/) tool.

Image array is saved in flash memory instead of load into RAM. A full screen image will take up 192 x 8 = 1536 bytes of Flash memory.

**API**

*UC1609(uint8_t cd, uint8_t rst, uint8_t cs)*

The UC1609 construct create an instance of UC1609 object and allows user to pass in the GPIO pins to be used by Command/Data(CD), Reset(RST), and Chip Select (CS) pin of  the UC1609 module.

```Cpp
// define the pins to be used based on your Arduino Core
#define CD  PIN_PC0
#define RST PIN_PC2
#define CS  PIN_PC1

UC1609 myLcd{CD, RST, CS}
```

*void begin(void)*

The `UC1609::begin()` setup the pin mode of each pin to be used for controlling the LCD module, it also called the `SPI.begin()` to enable the SPI communication.

*void initDisplay(uint8_t VbiasPot = DEFAULT_VBIAS_POT)*

The `UC1609::initDisplay()` initializes the LCD module through SPI and make it ready to be used. The method also allows to pass in a parameter to set the display contrast with `initDisplay(contract_value)` with a value ranging from 0 to 254, the parameter however is optional as if the user does not pass in a parameter, a default value of `DEFAULT_VBIAS_POT` value (`0x49`) would be used.

*void resetDisplay(void)*

The `resetDispaly()`  performs a hardware reset using RST pin by pulling the RST pin LOW and CD pin HIGH. It is used prior the initialization of the LCD or prior the powering down the LCD.

*void enableDisplay(uint8_t onOff)*

This function turns all pixels on the display on (if `onOff=1`) or off(`onOff=0`).

*void clearDisplay()*

This method set the internal display ram to 0.

*void setCursor(uint8_t col, uint8_t line)*

This method set the cursor location on the display to `col` (0 - 192), `line` (0-7). 

*void drawLine(uint8_t line, uint8_t dataPattern)*

This method draws a line or a data pattern from 0 all the way to the end of the `line`. A line consists of 8 vertical pixels with Most Significate Bit(MSB) at the bottom of the line and the Least Significate Bit(LSB) represented the top of the line. For example, `drawline(0, 0x80)` will draw a line at the bottom of the line 0, `drawline(2, 0x81)` will draw two lines on both the top and the bottom of the line 2. See `test_UC1609_basic.ino` for the demostration.

```
Line      Data Pattern
0         1 (LSB)
1         0
2         0
3         0
4         0
5         0
6         0
7         1 (MSB)
```

*void clearLine(uint8_t line)*

Unlike the `clearDisplay`, this method clear the specific line of the display. The method is a special version of `drawLine()` by calling the drawLine function with a paramter of 0, so it has the same effect of calling `drawline(line, 0)`.

*void scroll(uint8_t yPixel)*

This function scrolls the displayed image up by number of pixels specified by `yPixel`(0-64).

*void rotate(uint8_t rotateValue)*

The `UC1609::rotate()` rotates the display orientation based on the `rotateValue` provided. There are two pre-defined macro can be used as the `rotateValue`, `NORMAL_ORIENTATION` (0x04) for the default normal orientation, or `ROTATE_UPSIDE_DOWN` (0x02) for turning the display 180 degree upside down.

*void invert(bool invert)*

Inverts the display from white text over black background(when `invert=true`) or balck text over white background (when `invert=false`).

*void setFont(const uint8_t * font)*

The `setFont()` method is optional and by default the method is not been called, the `font5x7` font will be used. The following pre-defined value can be used for `font` parameters:

* font5x7_numeric

* font5x7 (default)

* font5x7_symbol

* font5x7_extended

*void printChar(const unsigned char c)*

This method print a character to the LCD module at whatever location previously set.

*void printChar(const unsigned char c, uint8_t col, uint8_t line)*

This method is an overloaded function of `printChar(c)` by calling `setCursor(col, line)` first prior to calling `printChar(c)`.

*void printStr(const unsigned char *str, uint8_t col, uint8_t line)*

This method print a c-style string at location specified by `col` and `line`. The cursor will be automatically advance to next col or next line.

*void drawImage(int16_t x, int16_t y, uint8_t w, uint8_t h, const uint8_t *  data)*

This method draws an image to the LCD display. The `x` and `y` defines the upper-left corner where the image to be drawed, the `w` and `h` values defined the width and height of the image, `data` is the pointer of the image array stored in Flash memory.

*void powerDown(void)*

This method literally performs a hardware reset (`resetDisplay()`) and turn off all the display pixels with `enableDisplay(0)` to put the display in low power consumption mode which is described in page 40 of the UC1609 datasheet.

Datasheets
-----------------------------

[UC1609 Datasheet](https://www.buydisplay.com/download/ic/UC1609_Datasheet.pdf)
