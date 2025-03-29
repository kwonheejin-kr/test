// This file is a bridge between C and C++ code
// It forwards all UI events functions to the C++ implementation in ui_events.cpp
// LVGL과 Arduino 코드 간의 호환성 문제를 해결하기 위해 ui_events.c 파일을 생성합니다. 
// 이 파일은 C와 C++ 코드 사이의 브릿지 역할을 하여 컴파일러가 C 코드에서 C++ 함수를 호출할 수 있게 합니다.

#ifdef __cplusplus
extern "C" {
#endif

#include "ui_events.h"
#include "ui_helpers.h"
#include "ui.h"

// 여기서는 ui_events.cpp에 정의된 함수들을 사용합니다.
// 실제 구현은 ui_events.cpp 파일에 있습니다.

#ifdef __cplusplus
}
#endif