/*
 * Arduino UNO R4 WiFi TFT uae30ubcf8 ud14cuc2a4ud2b8
 * Arduino IDEuc758 uae30ubcf8 ub77cuc774ube0cub7ecub9acub9cc uc0acuc6a9
 */

#include <Arduino_GFX_Library.h>

// TFT ud540 uc815uc758
#define TFT_DC   9
#define TFT_CS   10
#define TFT_RST  A0
#define TFT_BL   A1

// uc0c9uc0c1 uc815uc758
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0
#define WHITE    0xFFFF

// ud45cuc900 Arduino_GFX ub77cuc774ube0cub7ecub9ac uac1duccb4 uc0acuc6a9
Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS);
// 2.8" TFT with ILI9341 controller
Arduino_GFX *gfx = new Arduino_ILI9341(bus, TFT_RST);

void setup() {
  // uc2dcub9acuc5bc ucd08uae30ud654
  Serial.begin(115200);
  delay(1000);  // uc624ub958 uba54uc2dcuc9c0 ucd9cub825uc744 uc704ud574 uc7a0uc2dc ub300uae30
  Serial.println("Arduino UNO R4 WiFi TFT uae30ubcf8 ud14cuc2a4ud2b8 uc2dcuc791");
  
  // ubc31ub77cuc774ud2b8 uc124uc815
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  Serial.println("Backlight ON");
  
  // TFT ucd08uae30ud654
  Serial.println("TFT ucd08uae30ud654 uc2dcuc791...");
  if (!gfx->begin()) {
    Serial.println("TFT ucd08uae30ud654 uc2e4ud328!");
    while (1);
  }
  Serial.println("TFT ucd08uae30ud654 uc131uacf5");
  
  // ud654uba74 ud06cuae30 ucd9cub825
  Serial.print("TFT ud574uc0c1ub3c4: ");
  Serial.print(gfx->width());
  Serial.print(" x ");
  Serial.println(gfx->height());
  
  // ud654uba74 ucd08uae30ud654
  gfx->fillScreen(BLACK);
  gfx->setTextColor(WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(10, 10);
  gfx->println("R4 WiFi TFT Test");
  delay(1000);
  
  // ubaa8ub4e0 ud68cuc804 ubaa8ub4dc ud14cuc2a4ud2b8
  testAllRotations();
  
  // uae30ubcf8 ud14cuc2a4ud2b8 uc2e4ud589
  runBasicTest();
}

void loop() {
  // uc0c9uc0c1 ucc44uc6b0uae30 ud14cuc2a4ud2b8 ubc18ubcf5
  static unsigned long lastTime = 0;
  static uint8_t colorIndex = 0;
  
  if (millis() - lastTime > 3000) { // 3ucd08ub9c8ub2e4 uc0c9uc0c1 ubcc0uacbd
    lastTime = millis();
    
    switch (colorIndex % 7) {
      case 0: 
        Serial.println("RED");
        gfx->fillScreen(RED); 
        break;
      case 1: 
        Serial.println("GREEN");
        gfx->fillScreen(GREEN); 
        break;
      case 2: 
        Serial.println("BLUE");
        gfx->fillScreen(BLUE); 
        break;
      case 3: 
        Serial.println("CYAN");
        gfx->fillScreen(CYAN); 
        break;
      case 4: 
        Serial.println("MAGENTA");
        gfx->fillScreen(MAGENTA); 
        break;
      case 5: 
        Serial.println("YELLOW");
        gfx->fillScreen(YELLOW); 
        break;
      case 6: 
        Serial.println("WHITE");
        gfx->fillScreen(WHITE); 
        break;
    }
    
    // ud604uc7ac ud68cuc804 ubc0f uc0c9uc0c1 uc815ubcf4 ud45cuc2dc
    gfx->setCursor(10, 10);
    gfx->setTextSize(1);
    gfx->setTextColor((colorIndex % 7) == 6 ? BLACK : WHITE);
    gfx->print("Rotation ");
    gfx->println(gfx->getRotation());
    
    colorIndex++;
  }
}

void testAllRotations() {
  for (uint8_t r = 0; r < 4; r++) {
    Serial.print("Rotation ");
    Serial.println(r);
    
    gfx->setRotation(r);
    gfx->fillScreen(BLACK);
    gfx->setTextColor(WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(10, 10);
    gfx->print("Rotation ");
    gfx->println(r);
    
    // ud68cuc804ubcc4 uac00uc2dcuc131 ub192uc740 uac83 uadf8ub9acuae30
    int width = gfx->width();
    int height = gfx->height();
    
    // uac15uc870 uad6cubd84uc744 uc704ud55c ud14cub450ub9ac
    gfx->drawRect(0, 0, width, height, YELLOW);
    
    // ub300uac01uc120
    gfx->drawLine(0, 0, width-1, height-1, CYAN);
    gfx->drawLine(0, height-1, width-1, 0, CYAN);
    
    // uac01 ubaa8uc11cub9acuc5d0 uc6d0 uadf8ub9acuae30
    gfx->fillCircle(5, 5, 5, RED);
    gfx->fillCircle(width-5, 5, 5, GREEN);
    gfx->fillCircle(5, height-5, 5, BLUE);
    gfx->fillCircle(width-5, height-5, 5, WHITE);
    
    delay(2000);
  }
  
  // ucd5cuc885 ud68cuc804 uc124uc815
  gfx->setRotation(1); // uac00ub85cubaa8ub4dc
}

void runBasicTest() {
  // ud654uba74 ucd08uae30ud654
  gfx->fillScreen(BLACK);
  
  // ud14duc2a4ud2b8 ud14cuc2a4ud2b8
  gfx->setTextSize(1);
  gfx->setTextColor(WHITE);
  gfx->setCursor(0, 0);
  gfx->println("Hello R4 WiFi!");
  gfx->setTextColor(YELLOW);
  gfx->setTextSize(2);
  gfx->println("TFT Test");
  gfx->setTextColor(RED);
  gfx->setTextSize(3);
  gfx->println("2023");
  gfx->setTextColor(CYAN);
  gfx->setTextSize(4);
  gfx->print("28");
  
  delay(1500);
  
  // uccb4ucee4ubcf4ub4dc ud14cuc2a4ud2b8 (ub354 uba85ud655ud55c ud45cuc2dc ud6a8uacfc)
  gfx->fillScreen(BLACK);
  int blockSize = 20;
  int width = gfx->width();
  int height = gfx->height();
  
  for (int y = 0; y < height; y += blockSize) {
    for (int x = 0; x < width; x += blockSize) {
      if ((x/blockSize + y/blockSize) % 2) {
        gfx->fillRect(x, y, blockSize, blockSize, RED);
      } else {
        gfx->fillRect(x, y, blockSize, blockSize, BLUE);
      }
    }
  }
  
  delay(2000);
  
  // uadf8ub798ud53d uae30ubcf8 ub3c4ud615 ud14cuc2a4ud2b8
  gfx->fillScreen(BLACK);
  
  // uc0acuac01ud615 uadf8ub9acuae30
  for (int i = 0; i < height/2; i += 5) {
    gfx->drawRect(i, i, width-i*2, height-i*2, YELLOW);
  }
  
  delay(1500);
  
  // uc6d0 uadf8ub9acuae30
  gfx->fillScreen(BLACK);
  for (int i = 0; i < max(width,height)/2; i += 15) {
    gfx->drawCircle(width/2, height/2, i, random(0xFFFF));
  }
  
  delay(1500);
  
  // uc120 ud14cuc2a4ud2b8 - ubc29uc0acud615 ud328ud134
  gfx->fillScreen(BLACK);
  int cx = width/2;
  int cy = height/2;
  for (int i = 0; i < 360; i += 15) {
    float angle = i * PI / 180;
    int x2 = cx + sin(angle) * min(width, height)/2;
    int y2 = cy + cos(angle) * min(width, height)/2;
    gfx->drawLine(cx, cy, x2, y2, random(0xFFFF));
  }
  
  delay(2000);
}
