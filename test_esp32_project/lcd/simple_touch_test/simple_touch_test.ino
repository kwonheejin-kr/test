// 간단한 보정 코드
bool touched = tft.getTouch(&touchX, &touchY);

if (touched) {
  // 화면 크기에 맞게 스케일링
  int16_t x = map(touchX, 0, 4095, 0, tft.width());
  int16_t y = map(touchY, 0, 4095, 0, tft.height());
  
  // 필요시 X, Y축 반전
  x = tft.width() - x;  // X축 반전
  //y = tft.height() - y;  // Y축 반전 (필요시)
  
  // 표시
  tft.fillCircle(x, y, 3, TFT_YELLOW);
}