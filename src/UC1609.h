/*
 * Library Name: UC1609h
 * Description:  This library provides light weight text-only display support on UC1609C LCD controller with Hardware SPI. 
 *               The library should works on any Arduino Core.  
 * Author:       Henry Cheung (E-Tinkers), Tech Stduio Design LLP
 * Github:       https://github.com/e-tinkers/uc1609
 */

#ifndef UC1609_LCD_H
#define UC1609_LCD_H

#include "Arduino.h"
#include <SPI.h>

#ifdef __AVR__
#include <avr/io.h>
#include <avr/pgmspace.h>
#define FlashMem(n) const uint8_t PROGMEM n[]
#define IMAGE(imgN) const uint8_t PROGMEM imgN[]
#define readFontByte(addr) pgm_read_byte(&addr)
#else
#define FlashMem(n) const uint8_t n[]
#define IMAGE(imgN) const uint8_t imgN[]
#define readFontByte(addr) (addr)
#endif

#include "fonts.h"

// UC1609 registers
#define REG_COL_ADDR_L      0x00 // Column Address Set CA [3:0]
#define REG_COL_ADDR_H      0x10 // Column Address Set CA [7:4]
#define REG_TEMP_COMP_REG   0x20 // Temperature Compensation Register
#define REG_POWER_CONTROL   0x28 // default PC[2]: 1 - Interval VLCD(7x charge pump, PC[1:0] B10 - charge current 1.4mA
#define REG_SCROLL          0x40 // scrolls , Set the scroll line number. SL[5:0] 0-64
#define REG_VBIAS_POT       0x81 // Set Vbias Potentiometer (double-byte command)
                                 //                     PM[7:0] default - 0x49, (0x00-0xFF)
#define REG_ADDRESS_CONTROL 0x88 // RAM address control AC[1] 0: col auto-increment, 1: page auto-increment; 
                                 //                     AC[0] 0: no rollover, 1: rollover
#define REG_FRAMERATE_REG   0xA0 // Frame rate FR[1:0] B00: 76 fps, B01: 95 fps(default), B10: 132 fps, B11: 168 fps
#define REG_ALL_PIXEL_ON    0xA4 // sets on all Pixels on, AP[0] 1: ON all SEG drivers, 0(default): OFF all SEG drivers.
#define REG_INVERSE_DISPLAY 0xA6 // inverts display, IV[0] 0(default): no invert, 1: inverse of the data (bit-wise) stored in the display RAM.
#define REG_DISPLAY_ENABLE  0xAE // display enable, DE[0] 0(default): off, 1: on - exit sleep mode, restore power, and turn on COM/SEG drivers.
#define REG_PAGE_ADDER      0xB0 // Page address Set PA[3:0] 0-8
#define REG_MAPPING_CTRL    0xC0 // Mapping control, MY[2], MX[1]
#define REG_SYSTEM_RESET    0xE2
#define REG_BIAS_RATIO      0xE8 // Bias Ratio between V-LCD and V-D. BR[1:0] B00: 6, B01: 7, B10: 8, B11(default): 9

// UC1609 contrast (Vbias potentiometer value)
#define DEFAULT_VBIAS_POT   0x49 // default only used if user does not specify Vbias

// UC1609 Screen Rotation
#define NORMAL_ORIENTATION  0x04
#define ROTATE_UPSIDE_DOWN  0x02

// UC1609 Display Enable
#define DISPLAY_ON          1
#define DISPLAY_OFF         0

// SPI Clock Speed
#ifndef SPI_CLOCK
#define SPI_CLOCK          8000000UL  // 8MHz
#endif

class UC1609 {
  public:
    UC1609(uint8_t cd, uint8_t rst, uint8_t cs);
    ~UC1609(){}; 
    
    void begin(void);
    void initDisplay(uint8_t VbiasPot = DEFAULT_VBIAS_POT);
    void resetDisplay(void);
    void enableDisplay(uint8_t onOff);
    void clearDisplay();
    void setCursor(uint8_t col, uint8_t line);
    void drawLine(uint8_t line, uint8_t dataPattern);    
    void clearLine(uint8_t line);
    void scroll(uint8_t yPixel);
    void rotate(uint8_t rotateValue);
    void invert(bool invert);
    void setFont(const uint8_t * font);
    void setFontScale(uint8_t scale);
    void setAntiAliasing(bool on);
    void printChar(const unsigned char c);
    void printChar(const unsigned char c, uint8_t col, uint8_t line); 
    void printStr(const unsigned char *str, uint8_t col, uint8_t line);
    void printStr(const char *str, uint8_t col, uint8_t line) { printStr((const unsigned char*) str, col, line);};
    void printDoubleChar(const unsigned char c, uint8_t col, uint8_t line);
    void drawImage(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* data);
    void powerDown(void);
           
  private:
    const uint8_t _width{192};
    const uint8_t _height{64};
    uint8_t _cs;
    uint8_t _cd;
    uint8_t _rst;
    uint8_t _VbiasPOT; // Contrast DEFAULT_VBIAS_POT(0x49), datasheet 00-FE
    const uint8_t * _font = font5x7;
    uint8_t _scale;
    bool _antiAliasingEnable;
    
    void _sendCommand(uint8_t reg, uint8_t value);
    uint16_t _stretch(uint8_t x);
    void _antiAliasing(uint8_t *array);
};

#endif
