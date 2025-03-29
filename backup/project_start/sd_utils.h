#ifndef SD_UTILS_H
#define SD_UTILS_H

#include <SD.h>
#include <SPI.h>

#define SD_CS 13  // SD 카드 칩 셀렉트 핀

// SD 카드 초기화 함수
void init_sd();

// SD 카드에서 터치 사운드 재생
void play_touch_sound();

#endif
