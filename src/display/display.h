#pragma once
#include <Arduino.h>

namespace Display {
    constexpr uint16_t CLR_BLACK    = 0x0000;
    constexpr uint16_t CLR_WHITE    = 0xFFFF;
    constexpr uint16_t CLR_RED      = 0xF800;
    constexpr uint16_t CLR_GREEN    = 0x07E0;
    constexpr uint16_t CLR_BLUE     = 0x001F;
    constexpr uint16_t CLR_ORANGE   = 0xFD20;
    constexpr uint16_t CLR_YELLOW   = 0xFFE0;
    constexpr uint16_t CLR_CYAN     = 0x07FF;
    constexpr uint16_t CLR_PANEL    = 0x1082;
    constexpr uint16_t CLR_MUTED    = 0x7BEF;
    constexpr uint16_t CLR_DARKGRAY = 0x2104;

    void init();
    void fillScreen(uint16_t color);
    void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
    void drawPixel(uint16_t x, uint16_t y, uint16_t color);
    void drawHLine(uint16_t x, uint16_t y, uint16_t w, uint16_t color);
    void drawVLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color);
    void drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t scale);
    uint16_t drawString(uint16_t x, uint16_t y, const char* str, uint16_t color, uint16_t bg, uint8_t scale);
    void drawBigNumber(uint16_t x, uint16_t y, const char* str, uint16_t color);
}
