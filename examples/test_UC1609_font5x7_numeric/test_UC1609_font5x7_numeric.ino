/*
 * Sketch Name:  test_UC1609_FONT5x7_NUMERIC.ino
 * Description:  Test and demostration of using font5x7_numeric which only consitst of 27 chars (ASCII 0x20 - 0x3A).
 * Author:       Henry Cheung (E-Tinkers), Tech Studio Design LLP
 * Github:       https://github.com/e-tinkers/uc1609
 */


#include <UC1609.h>

// define control pins to be used for the display
#define CD  PIN_PC0
#define RST PIN_PC2    // Set it to -1 if share with MCU's NRST pin
#define CS  PIN_PC1

// define font to be used for the display

// System default contrast = 0x49, user however can change it with value 0x00 to 0xFE
const uint8_t contrast=0x38;

UC1609  lcd{CS, CD, RST}; 


void setup() {
  lcd.begin();
  lcd.initDisplay(contrast);
  lcd.setFont(font5x7_numeric);
  delay(500);
}


void loop() {

    // Test FONT_5x7_NUMERIC
    static const unsigned char amount[]{"$1,000,000.00"};
    static const unsigned char timestamp[]{"2022-01-02 12:34:56"};
    static const unsigned char pi[]{"3.1415925"};
    static const unsigned char percentage[]{"+/- 85%"};

    lcd.setCursor(15, 3);
    for (unsigned char c=' '; c<=':'; c++) {
      lcd.printChar(c);
      delay(10);
    }
    delay(5000);

    // right-aligned number display
    lcd.clearDisplay();
    lcd.printStr(amount, 192 - strlen((const char*)amount) * 6, 0);
    lcd.printStr(timestamp, 192 - strlen((const char*)timestamp) * 6, 2);
    lcd.printStr(pi, 192 - strlen((const char*)pi) * 6, 4);
    lcd.printStr(percentage, 192 - strlen((const char*)percentage) * 6, 6);
    delay(5000);
    lcd.clearDisplay();

    // Double-size Numeric Countdown
    lcd.clearDisplay();
    lcd.setFontScale(2);
    for (int i=0; i<24; i++) {
      lcd.printStr(String(2023-i).c_str(), 72, 3);
      delay(1000);
    }
    lcd.setFontScale(1);
    lcd.clearDisplay();

}
