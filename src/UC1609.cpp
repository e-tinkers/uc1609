/*
 * Library Name: UC1609h
 * Description:  This library provides light weight text-only display support on UC1609C LCD controller with Hardware SPI. 
 *               The library should works on any Arduino Core.  
 * Author:       Henry Cheung (E-Tinkers), Tech Studio Design LLP
 * Github:       https://github.com/e-tinkers/uc1609
 */

#include "UC1609.h"

/*
 * UC1609 constructor initializes the variables for CD, RST and CS pins
 */
UC1609::UC1609(uint8_t cd, uint8_t rst, uint8_t cs) {
    _cd = cd;
    _rst = rst;
    _cs = cs;
}

/*
 * Internal function for setting a value at a specific LCD register via SPI. 
 * Command is clock-in to the register at the rising edge of CD pin.
 * param: reg - LCD register
 *        value - the value for configuring the register
 */
void UC1609::_sendCommand(uint8_t reg, uint8_t value) {
  digitalWrite(_cd, LOW); 
  SPI.transfer(reg | value);
  digitalWrite(_cd, HIGH); 
}

/* Strentch a byte into a 16-bit word, based on the algorithm in
 * Henry S. Warran Jr. Hacker's Delight (2 edition) p. 139-141
 */
uint16_t UC1609::_stretch (uint8_t x) {
  uint16_t d = (uint16_t) x;        // d = 00000000 abcdefgh
  d = (d & 0xF0) << 4 | (d & 0x0F); // d = 0000abcd 0000efgh
  d = (d << 2 | d) & 0x3333;        // d = 00ab00cd 00ef00gh
  d = (d << 1 | d) & 0x5555;        // d = 0a0b0c0d 0e0f0g0h
  return d | d << 1;                // d = aabbccdd eeffgghh
}

/*
 * Configure all the pins and SPI interface, it then call Init() to initialize
 * the LCD display.
 * param:  VbiasPOT - Allows user to pass in a value (0x00 - 0xFF) to adjust 
 *                    Vbias Potentiometer value (display contrast). If no 
 *                    parameter is passed in, the default 0x49 would be used.
 * return: void
 */
void UC1609::begin (void) {
  digitalWrite(_cd, HIGH);
  digitalWrite(_cs, HIGH);
  digitalWrite(_rst, HIGH);
  
  pinMode(_cd, OUTPUT);
  pinMode(_rst, OUTPUT);
  pinMode(_cs, OUTPUT);
  
  SPI.begin();
}

/*
 * Performs a Hard reset and initializes LCD registers and clear the display.
 * param:  void
 * return: void
 */
void UC1609::initDisplay(uint8_t VbiasPOT) {
  resetDisplay();

  _VbiasPOT = VbiasPOT;   // DEFAULT_VBIAS_POT or user-provided constract value
  _scale = 1;             // Normal font size
  
  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE0));
  digitalWrite(_cs, LOW); 
  _sendCommand(REG_VBIAS_POT , 0);
  _sendCommand(REG_VBIAS_POT , _VbiasPOT);
  _sendCommand(REG_MAPPING_CTRL, NORMAL);         // can be ROTATED
  _sendCommand(REG_DISPLAY_ENABLE, DISPLAY_ON);
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();

  clearDisplay();
}

/* 
 *  Performs a hardware reset using RST pin by pulling the RST pin LOW and CD pin
 *  HIGH by at least 3uS. It is used prior the initialization of the LCD or powering
 *  down the LCD.
 *  param:  void
 *  return: void
 */
void UC1609::resetDisplay() {
  digitalWrite(_rst, LOW);
  delayMicroseconds(3);
  digitalWrite(_rst, HIGH);
  delay(5);
}

/*
 * This function turns all pixels on the display on or off
 * param:  onOff: 0 = all off, 1 = all on
 * return: void
 */
void UC1609::enableDisplay(uint8_t onOff) {
  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE0));
  digitalWrite(_cs, LOW);
  _sendCommand(REG_ALL_PIXEL_ON, onOff);
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
}

/*
 * Clears the LCD display
 * param:  void
 * return: void
 */
void UC1609::clearDisplay() {
  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE0));
  digitalWrite(_cs, LOW);
  uint16_t bytes = _width * (_height /8); // width * height
  for (uint16_t i = 0; i < bytes; i++) {
    SPI.transfer(0);
  }
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
}

/*
 * Sets the cursor to col, page location
 * param:  uint8_t col - 0-192 column number
 *         uint8_t page - 0-7 page number (i.e. line number)
 * return: void
 */
void UC1609::setCursor(uint8_t col, uint8_t line) {
  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE0));
  digitalWrite(_cs, LOW);
  _sendCommand(REG_COL_ADDR_L, (col & 0x0F)); 
  _sendCommand(REG_COL_ADDR_H, (col & 0xF0) >> 4);
  _sendCommand(REG_PAGE_ADDER , line++); 
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
}

/*
 * Draw a data pattern (0x01 - 0xff) across a line. 0x80 is a line at the lowest pixel of
 * a 6x8 matrix, and 0x81 will draw two line at both the top and the button of the 6x8 matrix.
 * param:  uint8_t dataPatern - 0x01-0xff
 * return: void
 */
void UC1609::drawLine(uint8_t line, uint8_t dataPattern) {
  setCursor(0, line);
  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE0));
  digitalWrite(_cs, LOW);
  uint16_t bytes = ((_width * (_height/8))/8); // (width * height/8)/8 = 192 bytes
  for (uint16_t i = 0; i < bytes; i++) {
   SPI.transfer(dataPattern);
  }
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
}

/*
 * clearLine is a special version of drawLine by calling the drawLine function with 
 * a paramter of 0 (which draw 0 across the line, which clear the line).
 * param:  void
 * return: void
 */
void UC1609::clearLine(uint8_t line) {
  drawLine(line, 0);
}

/*
 * This function scrolls the displayed image up by number of lines specified by `line`.
 * param:  yPixel: 0-64 pixel along y-axis
 * return: void
 */
void UC1609::scroll(uint8_t yPixel) {
  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE0));
  digitalWrite(_cs, LOW);
  _sendCommand(REG_SCROLL, yPixel);
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
}

/*
 * This function rotates the display orientation based on the rotatevalue
 * provided. 
 * param:  rotatevalue - NORMAL_ORIENTATION(0x04),  or ROTATE_UPSIDE_DOWN(0x02).
 * return: void
 */
void UC1609::rotate(uint8_t rotateValue) {
  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE0));
  digitalWrite(_cs, LOW);
  _sendCommand(REG_MAPPING_CTRL, rotateValue & B00000110);
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
}

/*
 * Inverts the display from white text over black background or vice versa
 * Param:  bool invert: true = invert(white text over black background); false = normal
 * return: void
 */
void UC1609::invert(bool invert) {
  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE0));
  digitalWrite(_cs, LOW);
  _sendCommand(REG_INVERSE_DISPLAY, invert);
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
}

void UC1609::setFont(const uint8_t * font) {
  _font = font;
}

void UC1609::setFontScale(uint8_t scale) {
  if (scale == 1 || scale == 2) {
    _scale = scale;
  }
}

/*
 * Prints a printable character in the ASCII font table to the display
 * param:  const char character - ASCII value of the character
 * return: void
 */
void UC1609::printChar(const unsigned char c) {
  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE0));
  digitalWrite(_cs, LOW);
  SPI.transfer(0x00); // padding col
  uint8_t fontWidth = readFontByte(_font[0]);
  uint8_t fontStart = readFontByte(_font[2]);
  for (uint8_t  col=0; col<fontWidth; col++) {
    SPI.transfer(readFontByte(_font[(c - fontStart) * fontWidth + col + 4]));
  }
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
}

void UC1609::printChar(const unsigned char c, uint8_t col, uint8_t line) {
  setCursor(col, line);
  printChar(c);
}

/*
 * Prints a string to the LCD
 * param:  const char *str - poninter to the str
 * return: void
 */
void UC1609::printStr(const unsigned char *str, uint8_t col, uint8_t line) {
  setCursor(col, line);
  while (*str) {
    printChar(*str++);
  }
}

/*
 * Draws a bitmap image 
 * params: uint16_t x - position in x-axis where the image to be draw
 *         uint16_t y - position in y-axis where the image to be draw
 *         uint8_t w - the width of the image
 *         uint8_t h - the height of the image
 *         uint8_t *data - pointer to the image bitmap array
 * return: void
 */
void UC1609::drawImage(int16_t x, int16_t y, uint8_t w, uint8_t h, const uint8_t* data) {

  uint8_t column = (x < 0) ? 0 : x;
  uint8_t page = (y < 0) ? 0 : y >> 3;
  
  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE0));
  digitalWrite(_cs, LOW);
  for (uint8_t ty = 0; ty < h; ty = ty + 8) {
    if (y + ty < 0 || y + ty >= _height)
      continue;
    _sendCommand(REG_COL_ADDR_L, (column & 0x0F)); 
    _sendCommand(REG_COL_ADDR_H, (column & 0xF0) >> 4);
    _sendCommand(REG_PAGE_ADDER , page++); 

    for (uint8_t tx = 0; tx < w; tx++) {
      if (x + tx < 0 || x + tx >= _width)
        continue;
      SPI.transfer(readFontByte(data[(w * (ty >> 3)) + tx]));
    }
  }
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
  
}

/*
 * This function resets the LCD and turn off the display, see datasheet page 40
 * prarm:  void
 * return: void
 */
void UC1609::powerDown(void) {
  resetDisplay();
  enableDisplay(0);
}
