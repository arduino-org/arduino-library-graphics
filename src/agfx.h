#ifndef _AGFX_H_INCLUDED
#define _AGFX_H_INCLUDED

#include <stdint.h>
#include "agfx_ll.h"
#include "agfx_colors.h"
//#include "star_ts.h"

#define ENABLE_DEMOS

class AGFX
{
public:
    AGFX();
    bool begin();
    void setDrawScreen(uint8_t scrIdx);
    uint8_t getDrawScreen(void);
    void showScreen(uint8_t scrIdx);
    void copyScreen(uint8_t fromScrIdx, uint8_t toScrIdx);
    uint32_t color(uint8_t r, uint8_t g, uint8_t b, uint8_t alpha);
    uint32_t color(uint8_t r, uint8_t g, uint8_t b);
    uint32_t color(uint8_t gray, uint8_t alpha);
    uint32_t color(uint8_t gray);
    void background(uint32_t color);
    void stroke(uint32_t color);
    void noStroke();
    void fill(uint32_t color);
    void noFill();
    void point(uint16_t x1, uint16_t y1);
    void line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
    void rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
    void triangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3);
    void quadr(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t x4, uint16_t y4);
    void circle(uint16_t x1, uint16_t y1, uint16_t r);
    void poly(const agfx_point_t *pts, const uint16_t nPts);
    void ellipse(uint16_t x1, uint16_t y1, uint16_t width, uint16_t height);
    void setCursorPos(const uint16_t x1, const uint16_t y1);
    void drawChar(const char c);
    void drawText(const char *text);
    void setFontFace(const GFXfont * const font);
    void setFontMag(const uint8_t mag);

#ifdef ENABLE_DEMOS
    void demoLine(int cnt);
    void demoPolygon();
    void demoCircle();
    void demoEllipse();
    void demoTouch();
    void demoText();
#endif

private:
    uint32_t fillColor;
    uint32_t strokeColor;
    bool fillOn;
    bool strokeOn;
    uint16_t locX;
    uint16_t locY;
    uint8_t fontMag;
};

#endif // _AGFX_H_INCLUDED
