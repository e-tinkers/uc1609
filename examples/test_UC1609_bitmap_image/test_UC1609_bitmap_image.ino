/*
 * Sketch Name:  test_UC1609.ino
 * Description:  This sketch test the display of bitmap images.
 * Author:       Henry Cheung (E-Tinkers), Tech Studio Design LLP
 * Github:       https://github.com/e-tinkers/uc1609
 */

#include <UC1609.h>
#include "image.h"  // define bitmap arrays used in the example

// define control pins to be used for the display
#define CD  PIN_PC0
#define RST PIN_PC2
#define CS  PIN_PC1

// System default contrast = 0x49, user however can change it with value 0x00 to 0xFE
const uint8_t contrast=0x38;

UC1609  lcd{CD, RST, CS}; 


void setup() {
  lcd.begin();
  lcd.initDisplay(contrast);
  lcd.setFont(font5x7);
}


void loop() {
  
  lcd.drawImage(0, 0, 192, 64, bitmap);    // full screen image
  delay(5000);
  lcd.clearDisplay();

  // dummy string for demo display
  static const unsigned char temperatureStr[]{"24.8c"};
  static const unsigned char humidityStr[]{"73.4%"};

  lcd.drawImage(70, 0, 24, 24, thermometerIcon); // 24x24 image
  lcd.printStr(temperatureStr, 96, 1);
  lcd.drawImage(70, 32, 24, 24, humidityIcon);   // 24x24 image
  lcd.printStr(humidityStr, 96, 5);

  delay(5000);

}
