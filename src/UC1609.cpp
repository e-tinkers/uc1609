/*
 * Library Name: UC1609h
 * Description:  This library provides light weight text-only display support on UC1609C LCD controller with Hardware SPI. 
 *               The library should works on any Arduino Core.  
 * Author:       Henry Cheung (E-Tinkers), Tech Studio Design LLP
 * Github:       https://github.com/e-tinkers/uc1609
 */

#include "UC1609.h"

/*
 * Internal function for setting a value at a specific LCD register via SPI. 
 * Command is clock-in to the register at the rising edge of CD pin.
 * param: reg - LCD register
 *        value - the value for configuring the register
 */
void UC1609::_sendCommand(uint8_t reg, uint8_t value) {
  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE0));
  digitalWrite(_cs, LOW);
  digitalWrite(_cd, LOW); 
  SPI.transfer(reg | value);
  digitalWrite(_cd, HIGH); 
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
}

/* Strentch a byte into a 16-bit word, based on the algorithm in
 * Henry S. Warran Jr. Hacker's Delight (2 edition) p. 139-141
 */
uint16_t UC1609::_stretch(uint8_t x) {
  uint16_t d = (uint16_t) x;        // d = 00000000 abcdefgh
  d = (d & 0xF0) << 4 | (d & 0x0F); // d = 0000abcd 0000efgh
  d = (d << 2 | d) & 0x3333;        // d = 00ab00cd 00ef00gh
  d = (d << 1 | d) & 0x5555;        // d = 0a0b0c0d 0e0f0g0h
  return d | d << 1;                // d = aabbccdd eeffgghh
}

void UC1609::_antiAliasing(uint8_t *array) {

  uint8_t temp[24];
  memcpy(temp, array, 24);

 for (int adjL=2; adjL<22; adjL=adjL+2) {
    uint16_t pattern1 = 0x000C;
    uint16_t pattern2 = 0x0003;
    uint16_t patternMask = 0x000F;
    
    uint16_t bitAddedL = 0x0002;
    uint16_t bitAddedR = 0x0004;
    
    uint16_t matching{0};
    uint8_t adjR = adjL + 1;
    
    for (int i=0; i<6; i++) {
      uint16_t adjL16 = (uint16_t) array[adjL+12] << 8 | array[adjL];
      uint16_t adjR16 = (uint16_t) array[adjR+12] << 8 | array[adjR];
      matching = (((adjL16 & patternMask) == pattern1) & ((adjR16  & patternMask) == pattern2));
      if (matching) {
        temp[adjL] |= bitAddedL;
        temp[adjR] |= bitAddedR;
      }     
      matching = (((adjL16 & patternMask) == pattern2) & ((adjR16 & patternMask) == pattern1));
      if (matching) {
        temp[adjL] |= bitAddedR;
        temp[adjR] |= bitAddedL;
      }

      pattern1 <<= 2;
      pattern2 <<= 2;
      patternMask <<= 2;
      bitAddedL <<= 2;
      bitAddedR <<= 2;
    }
  }

  memcpy(array, temp, 24);

}

/*
 * Configure all the pins and SPI interface, it then call Init() to initialize
 * the LCD display.
 * param:  VbiasPOT - Allows user to pass in a value (0x00 - 0xFF) to adjust 
 *                    Vbias Potentiometer value (display contrast). If no 
 *                    parameter is passed in, the default 0x49 would be used.
 * return: void
 */
void UC1609::begin (uint8_t VbiasPot) {

  digitalWrite(_cd, HIGH);
  digitalWrite(_cs, HIGH);  
  
  pinMode(_cd, OUTPUT);
  pinMode(_cs, OUTPUT);

  // if RST pin is actually defined with a pin, set it as OUTPUT, otherwise skip it
  if (_rst != -1) {
    digitalWrite(_rst, HIGH);
    pinMode(_rst, OUTPUT);
  }
  
  SPI.begin();

  _scale = 1;             // Normal font size
  _padding = 1; 
  _antiAliasingEnable = true;
  _VbiasPOT = VbiasPot;   // DEFAULT_VBIAS_POT or user-provided constract value

  resetDisplay();

  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE0));
  digitalWrite(_cs, LOW); 
  digitalWrite(_cd, LOW); 
  SPI.transfer(REG_SYSTEM_RESET | 0);
  SPI.transfer(REG_VBIAS_POT | 0);
  SPI.transfer(REG_VBIAS_POT | _VbiasPOT);
  SPI.transfer(REG_MAPPING_CTRL | NORMAL_ORIENTATION); // can be ROTATED
  SPI.transfer(REG_COL_ADDR_L | 0);
  SPI.transfer(REG_COL_ADDR_H | 0);
  SPI.transfer(REG_PAGE_ADDER | 0);
  SPI.transfer(REG_SCROLL | 0);
  SPI.transfer(REG_INVERSE_DISPLAY | 0);
  SPI.transfer(REG_DISPLAY_ENABLE | DISPLAY_ON);
  digitalWrite(_cd, HIGH); 
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
  if (_rst == -1) return;

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
  _sendCommand(REG_ALL_PIXEL_ON, onOff);
}

/*
 * Clears the LCD display
 * param:  void
 * return: void
 */
void UC1609::clearDisplay() {
  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE0));
  digitalWrite(_cs, LOW);
  uint16_t bytes = _width * (_height / 8); // width * page
  for (uint16_t i = 0; i < bytes; i++) {
    SPI.transfer(0);
  }
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
  _crow = 0;
  _ccol = 0;
}

/*
 * Sets the cursor to col, page location
 * param:  uint8_t col - 0-191 column number
 *         uint8_t page - 0-7 page number (i.e. line number)
 * return: void
 */
void UC1609::setCursor(uint8_t col, uint8_t line) {
  _crow = line;
  _ccol = col;
  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE0));
  digitalWrite(_cs, LOW);
  digitalWrite(_cd, LOW);
  SPI.transfer(REG_COL_ADDR_L | (col & 0x0F)); 
  SPI.transfer(REG_COL_ADDR_H | (col & 0xF0) >> 4);
  SPI.transfer(REG_PAGE_ADDER | line); 
  digitalWrite(_cd, HIGH);
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
}

/*
 * Draw a data pattern (0x01 - 0xff) across a line. 0x80 is a line at the lowest pixel of
 * a 6x8 matrix, and 0x81 will draw two line at both the top and the button of the 6x8 matrix.
 * param: uint8_t line - line 0-7 where the pattern to be drawed
 * param:  uint8_t dataPatern - 0x01-0xff
 * return: void
 */
void UC1609::drawLine(uint8_t line, uint8_t dataPattern) {
  setCursor(0, line);
  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE0));
  digitalWrite(_cs, LOW);
  for (uint16_t i = 0; i < _width; i++) {
    SPI.transfer(dataPattern);
  }
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
  _crow = line * _height;
  _ccol = _width;
}

/*
 * clearLine is a special version of drawLine by calling the drawLine function with 
 * a pattern of 0 (which draw 0 across the line, i.e. clear the line).
 * param: uint8_t line - line 0-7 where the pattern to be clear
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
  _sendCommand(REG_SCROLL, yPixel);
}

/*
 * This function rotates the display orientation based on the rotatevalue
 * provided. 
 * param:  rotatevalue - NORMAL_ORIENTATION(B100),  or ROTATE_UPSIDE_DOWN(0xB010).
 * return: void
 */
void UC1609::rotate(uint8_t rotateValue) {
  _sendCommand(REG_MAPPING_CTRL, rotateValue & B00000110);
}

/*
 * Inverts the display from white text over black background or vice versa
 * Param:  bool invert: true = invert(white text over black background); 
 *                      false = normal
 * return: void
 */
void UC1609::invert(bool invert) {
  _sendCommand(REG_INVERSE_DISPLAY, invert);
}

void UC1609::setFont(const uint8_t * font) {
  _font = font;
}

void UC1609::setFontScale(uint8_t scale) {
  if (scale == 1 || scale == 2) {
    _scale = scale;
    _padding = scale;
  }
}

void UC1609::setAntiAliasing(bool on) {
  _antiAliasingEnable = on;
}

/*
 * Print a printable character 
 * param: const char ch - ASCII value of the character
 * return: 0 - failed, 1 - success
 */
size_t UC1609::write(uint8_t ch) {

  uint8_t fontWidth = readFontByte(_font[0]);
  uint8_t fontStart = readFontByte(_font[2]);

  switch (ch) {
    case '\r':
      _ccol = 0;
      return 1;
    case '\n':
      _crow += _scale;
      if ((_crow * 8) >= _height) {
        clearDisplay();
      }
      setCursor(_ccol, _crow);
      return 1;
    case '\t':
      _ccol += (4 * (fontWidth * _scale + _padding));  // tab = 4 spaces
      setCursor(_ccol, _crow);
      return 1;
    case '\b':
      // move the cursor back one space but does not delete the char in place
      if (_ccol != 0) _ccol -= (fontWidth * _scale + _padding);
      setCursor(_ccol, _crow);
      return 1;
    default:
      if (ch >= fontStart && ch <= readFontByte(_font[3]))
        break;
      return 1;
  }

  // wrap text to next line if the text is wider than screen width
  if (_ccol >= _width) {
    _ccol = 0;
    _crow += _scale;
    setCursor(_ccol, _crow);
  }

  if (_scale == 1) {
    SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    SPI.transfer(0x00); // padding col
    for (uint8_t  col=0; col<fontWidth; col++) {
      SPI.transfer(readFontByte(_font[(ch - fontStart) * fontWidth + col + 4]));
    }
    digitalWrite(_cs, HIGH);
    SPI.endTransaction();
  }
  else {
    uint8_t buf[24]{0};  // each stretched font is 12x2 bytes, 12 bits wide, and 16 bits high

    for (uint8_t x = 0; x < fontWidth; x++) {
      uint16_t stretched = _stretch(readFontByte(_font[(ch - fontStart) * fontWidth + x + 4]));
      buf[x * 2 + 1] = stretched & 0xFF;
      buf[x * 2 + 2] = stretched & 0xFF;
      buf[x * 2 + 13] = (uint8_t) (stretched >> 8);
      buf[x * 2 + 14] = (uint8_t) (stretched >> 8);
    }

    if (_antiAliasingEnable) _antiAliasing(buf);

    setCursor(_ccol, _crow);

    SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    SPI.transfer(&buf[0], 12); // padding at beginning

    digitalWrite(_cd, LOW);
    SPI.transfer(REG_COL_ADDR_L | (_ccol & 0x0F));
    SPI.transfer(REG_COL_ADDR_H | (_ccol & 0xF0) >> 4);
    SPI.transfer(REG_PAGE_ADDER | (_crow + 1));
    digitalWrite(_cd, HIGH);

    SPI.transfer(&buf[12], 12);  // padding at end of each font
    digitalWrite(_cs, HIGH);
    SPI.endTransaction();
  }

  _ccol += ((fontWidth * _scale) + _padding);
  return 1;
}

/*
 * Draws a bitmap image 
 * params: uint8_t x - position in x-axis where the image to be draw
 *         uint8_t y - position in y-axis where the image to be draw
 *         uint8_t w - the width of the image
 *         uint8_t h - the height of the image
 *         uint8_t *data - pointer to the image bitmap array
 * return: void
 */
void UC1609::drawImage(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* data) {

  uint8_t column = x;
  uint8_t page = y >> 3;
  
  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE0));
  digitalWrite(_cs, LOW);
  for (uint8_t ty = 0; ty < h; ty = ty + 8) {
    if (y + ty < 0 || y + ty >= _height)
      continue;

    digitalWrite(_cd, LOW);
    SPI.transfer(REG_COL_ADDR_L | (column & 0x0F)); 
    SPI.transfer(REG_COL_ADDR_H | ((column & 0xF0) >> 4));
    SPI.transfer(REG_PAGE_ADDER | page++); 
    digitalWrite(_cd, HIGH);

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
