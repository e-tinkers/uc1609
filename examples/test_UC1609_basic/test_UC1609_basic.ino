/*
 * Sketch Name:  test_UC1609.ino
 * Description:  Test and demostration example for UC1609-based LCD Arduino library. 
 * Author:       Henry Cheung (E-Tinkers), Tech Studio Design LLP
 * Github:       https://github.com/e-tinkers/uc1609
 */

#include "UC1609.h"

// define control pins to be used for the display
#define CD  PIN_PC0
#define RST PIN_PC2
#define CS  PIN_PC1

// define font to be used for the display

// System default contrast = 0x49, user however can change it with value 0x00 to 0xFE
const uint8_t contrast=0x38;

UC1609  lcd{CD, RST, CS}; 


void setup() {
  lcd.begin();
  lcd.initDisplay(contrast);
  lcd.setFont(font5x7_extended);
  delay(500);
}


void loop() {
    static const unsigned char test0_1[] = "  UC1609 192 x 64 LCD display";
    static const unsigned char test0_2[] = " 32 characters/line (6x8 font)";
  
    static const unsigned char test1_1[] = "This is line(page) 0";
    static const unsigned char test1_2[] = "12345678901234567890123456789012";
    static const unsigned char test1_3[] = "This is line(page) 7";
  
    // Test 0 - printStr and clearDisplay functions
    lcd.printStr(test0_1, 0, 0);  // print string test0_1 at line 0, col 0
    lcd.printStr(test0_2, 0, 3);  // line 3, col 0
    delay(5000);
    lcd.clearDisplay();
    delay(50);
   
    // Test 1 printStr function
    lcd.printStr(test1_1, 0, 0);
    lcd.printStr(test1_2, 0, 4);
    lcd.printStr(test1_3, 0, 7);
    delay(5000);

    // Test - 2 clear line and draw pattern across line
    lcd.clearLine(0);      // clear line 0
    lcd.clearLine(7);      // clear line 7
    lcd.drawLine(3, 0x40); // draw a pattern (0100 0000) to line 3
    lcd.drawLine(5, 0xaa); // draw a pattern (1010 1010) to line 7
    delay(5000);

    // Test 3 - printChar and invertDisplay functions
    lcd.invert(true);           // set display to Inverted mode
    lcd.clearDisplay();
    lcd.printChar('H', 88, 2);  // printChar at line 2, col 88
    lcd.printChar('-');         // printChar at subsequent location
    lcd.printChar('C');
    delay(5000);
    lcd.invert(false);          // set display back to Normal mode
    lcd.clearDisplay();

    // Test 4 - font range
    // Symbols are available in font5x7_symbol and font5x7_extended
    lcd.printStr((const unsigned char*) "Symbols(0x00 - 0x1F)", 0, 0);
    lcd.setCursor(0, 1);
    for(unsigned char i=0; i<32; i++) {
      lcd.printChar(i); 
      delay(10);
    }

    // Those are the fonts available in font5x7
    lcd.printStr((const unsigned char*) "Printable(0x20 - 0x7F)", 0, 3);
    lcd.setCursor(0, 4);
    for(unsigned char i=32; i<128; i++) {
      lcd.printChar(i); 
      delay(10);
    }    
    delay(5000);

    // Full range fonts in font5x7_extended
    lcd.clearDisplay();
    lcd.setCursor(0, 0);
    for(unsigned char i=0; i<255; i++) {
      lcd.printChar(i); 
      delay(10);
    }    
    delay(5000);
    lcd.clearDisplay();

}