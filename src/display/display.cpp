#include "display.h"
#include <SPI.h>
#include "font5x7.h"
#include "pins.h"
#include "defaults.h"

namespace Display {
    static SPIClass* spi = nullptr;

    static void sendCommand(uint8_t cmd) {
        digitalWrite(PIN_TFT_DC, LOW);
        digitalWrite(PIN_TFT_CS, LOW);
        spi->transfer(cmd);
        digitalWrite(PIN_TFT_CS, HIGH);
    }
    
    static void sendData(uint8_t data) {
        digitalWrite(PIN_TFT_DC, HIGH);
        digitalWrite(PIN_TFT_CS, LOW);
        spi->transfer(data);
        digitalWrite(PIN_TFT_CS, HIGH);
    }
    
    static void sendData16(uint16_t data) {
        digitalWrite(PIN_TFT_DC, HIGH);
        digitalWrite(PIN_TFT_CS, LOW);
        spi->write16(data);
        digitalWrite(PIN_TFT_CS, HIGH);
    }
    
    static void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        sendCommand(0x2A);
        sendData16(x);
        sendData16(x + w - 1);
        sendCommand(0x2B);
        sendData16(y);
        sendData16(y + h - 1);
        sendCommand(0x2C);
    }

    void init() {
        pinMode(PIN_TFT_CS, OUTPUT);
        pinMode(PIN_TFT_DC, OUTPUT);
        pinMode(PIN_TFT_RST, OUTPUT);
        pinMode(PIN_TFT_BL, OUTPUT);
        
        digitalWrite(PIN_TFT_CS, HIGH);
        digitalWrite(PIN_TFT_DC, HIGH);
        
        digitalWrite(PIN_TFT_RST, LOW);
        delay(10);
        digitalWrite(PIN_TFT_RST, HIGH);
        delay(120);
        
        spi = new SPIClass(HSPI);
        spi->begin(PIN_SPI_SCK, -1, PIN_SPI_MOSI, PIN_TFT_CS);
        spi->beginTransaction(SPISettings(40000000, MSBFIRST, SPI_MODE0));
        
        sendCommand(0x01); // SWRESET
        delay(150);
        sendCommand(0x11); // SLPOUT
        delay(50);
        
        sendCommand(0x36); // MADCTL
        sendData(0xB0); // Landscape
        
        sendCommand(0x3A); // COLMOD
        sendData(0x55); // 16bit color
        
        sendCommand(0xB2); // PORCTRL
        sendData(0x0C);
        sendData(0x0C);
        sendData(0x00);
        sendData(0x33);
        sendData(0x33);
        
        sendCommand(0xB7); // GCTRL
        sendData(0x35);
        
        sendCommand(0xBB); // VCOMS
        sendData(0x1F);
        
        sendCommand(0xC0); // LCMCTRL
        sendData(0x2C);
        
        sendCommand(0xC2); // VDVVRHEN
        sendData(0x01);
        
        sendCommand(0xC3); // VRHS
        sendData(0x12);
        
        sendCommand(0xC4); // VDVS
        sendData(0x20);
        
        sendCommand(0xC6); // FRCTRL2
        sendData(0x0F);
        
        sendCommand(0xD0); // PWCTRL1
        sendData(0xA4);
        sendData(0xA1);
        
        sendCommand(0x21); // INVON
        
        sendCommand(0x29); // DISPON
        delay(50);
        
        digitalWrite(PIN_TFT_BL, HIGH);
    }

    void drawPixel(uint16_t x, uint16_t y, uint16_t color) {
        if(x >= 320 || y >= 240) return;
        setAddrWindow(x, y, 1, 1);
        digitalWrite(PIN_TFT_DC, HIGH);
        digitalWrite(PIN_TFT_CS, LOW);
        spi->write16(color);
        digitalWrite(PIN_TFT_CS, HIGH);
    }

    void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
        if(x >= 320 || y >= 240 || w == 0 || h == 0) return;
        if(x + w > 320) w = 320 - x;
        if(y + h > 240) h = 240 - y;
        
        setAddrWindow(x, y, w, h);
        digitalWrite(PIN_TFT_DC, HIGH);
        digitalWrite(PIN_TFT_CS, LOW);
        
        uint32_t len = w * h;
        while(len > 0) {
            uint16_t chunk = (len > 32767) ? 32767 : len;
            for(uint16_t i=0; i<chunk; i++) {
                spi->write16(color);
            }
            len -= chunk;
        }
        digitalWrite(PIN_TFT_CS, HIGH);
    }

    void fillScreen(uint16_t color) {
        fillRect(0, 0, 320, 240, color);
    }

    void drawHLine(uint16_t x, uint16_t y, uint16_t w, uint16_t color) {
        fillRect(x, y, w, 1, color);
    }

    void drawVLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color) {
        fillRect(x, y, 1, h, color);
    }

    void drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
        drawHLine(x, y, w, color);
        drawHLine(x, y + h - 1, w, color);
        drawVLine(x, y, h, color);
        drawVLine(x + w - 1, y, h, color);
    }

    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
        int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int16_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1; 
        int16_t err = dx + dy, e2;
        
        while (true) {
            drawPixel(x0, y0, color);
            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }

    void drawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t scale) {
        if (c < 32 || c > 127) return;
        c -= 32;
        
        for (uint8_t i = 0; i < 5; i++) {
            uint8_t line = pgm_read_byte(&font5x7[(uint8_t)c][i]);
            for (uint8_t j = 0; j < 8; j++, line >>= 1) {
                if (line & 1) {
                    if (scale == 1) drawPixel(x + i, y + j, color);
                    else fillRect(x + i * scale, y + j * scale, scale, scale, color);
                } else if (bg != color) {
                    if (scale == 1) drawPixel(x + i, y + j, bg);
                    else fillRect(x + i * scale, y + j * scale, scale, scale, bg);
                }
            }
        }
    }

    uint16_t drawString(uint16_t x, uint16_t y, const char* str, uint16_t color, uint16_t bg, uint8_t scale) {
        uint16_t startX = x;
        while (*str) {
            drawChar(x, y, *str, color, bg, scale);
            x += 6 * scale;
            str++;
        }
        return x - startX;
    }

    void drawBigNumber(uint16_t x, uint16_t y, const char* str, uint16_t color) {
        drawString(x, y, str, color, CLR_BLACK, 4);
    }
}
