#include "agfx.h"
#include "agfx_ts.h"
#include "star_ts.h"
#include "star_dsi.h"
#include "star_sdram.h"

#define ENABLE_SDRAM_TEST

AGFX::AGFX() :
    strokeOn(true), fillOn(false),
    fillColor(AGFX_WHITE),
    strokeColor(AGFX_BLACK),
    locX(0), locY(0), fontMag(1)
{
}

bool AGFX::begin()
{
    uint8_t res;
    uint16_t width, height;

    // TODO: move SDRAM initialization outside AGFX
    // Initialize sdram
    res = STAR_SDRAM_Init();
    if (res != SDRAM_OK)
        return false;
#ifdef ENABLE_SDRAM_TEST
    res = STAR_SDRAM_Test();
    if (res != SDRAM_OK)
        return false;
#endif

    // Initialize LTDC & DSI bridge
    res = STAR_DSI_Init(LCD_ORIENTATION_LANDSCAPE);
    if (res != LCD_OK)
        return false;
    width = STAR_DSI_PanelWidth();
    height = STAR_DSI_PanelHeight();

    // Initilize low level graphic
    AGFX_LL_Init(width, height);

    // Default font size
    AGFX_LL_SetFontMag(fontMag);

    // Touch Screen init
    uint8_t ts_status = STAR_TS_Init(width, height);
    if (ts_status != TS_OK) {
        // TODO: manage errors
        //TS_ERROR              : generic error
        //TS_TIMEOUT            : I2C timeout
        //TS_DEVICE_NOT_FOUND   : device not found on I2C bus
    }
    else {
#if 0
        // TODO: read/save calibration in EEPROM or elsewhere
        agfx_ts_cal_t calData;
        if (DUMMY_EEPROM_READ_CAL(&calData))
            AGFX_TS_SetCalData(&calData);
        else {
            AGFX_TS_Calibrate(width, height);
            AGFX_TS_GetCalData(&calData);
            DUMMY_EEPROM_WRITE_CAL(&calData);
        }
#else
        // Calibrate always
        AGFX_TS_Calibrate(width, height);
#endif
    }

    return true;
}

void AGFX::setDrawScreen(uint8_t scrIdx)
{
    STAR_DSI_SetDrawScreen(scrIdx);
}

uint8_t AGFX::getDrawScreen(void)
{
    return STAR_DSI_GetDrawScreen();
}

void AGFX::showScreen(uint8_t scrIdx)
{
    STAR_DSI_ShowScreen(scrIdx);
}

void AGFX::copyScreen(uint8_t fromScrIdx, uint8_t toScrIdx)
{
    STAR_DSI_CopyScreen(fromScrIdx, toScrIdx);
}

// Processing stype API ------------------- BEGIN --------------------
uint32_t AGFX::color(uint8_t r, uint8_t g, uint8_t b, uint8_t alpha)
{
    uint32_t col = alpha;

    col = (col << 8) + r;
    col = (col << 8) + g;
    col = (col << 8) + b;

    return col;
}

uint32_t AGFX::color(uint8_t r, uint8_t g, uint8_t b)
{
    return color(r, g, b, 0xFF);
}

uint32_t AGFX::color(uint8_t gray, uint8_t alpha)
{
    return color(gray, gray, gray, alpha);
}

uint32_t AGFX::color(uint8_t gray)
{
    return color(gray, gray, gray, 0xFF);
}

void AGFX::background(uint32_t color)
{
    AGFX_LL_Clear(color);
}

void AGFX::stroke(uint32_t color)
{
    strokeColor = color;
    strokeOn = true;
}

void AGFX::noStroke()
{
    strokeOn = false;
}

void AGFX::fill(uint32_t color)
{
    fillColor = color;
    fillOn = true;
}

void AGFX::noFill()
{
    fillOn = false;
}

void AGFX::point(uint16_t x1, uint16_t y1)
{
    if (strokeOn)
        AGFX_LL_DrawPoint(x1, y1, strokeColor);
}

void AGFX::line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    if (strokeOn)
        AGFX_LL_DrawLine(x1, y1, x2, y2, strokeColor);
}

void AGFX::rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    if (fillOn)
        AGFX_LL_FillRect(x1, y1, x2, y2, fillColor);
    if (strokeOn)
        AGFX_LL_DrawRect(x1, y1, x2, y2, strokeColor);
}

void AGFX::triangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
        uint16_t x3, uint16_t y3)
{
    const agfx_point_t pts[3] = { { x1, y1 }, { x2, y2 }, { x3, y3 } };

    if (fillOn)
        AGFX_LL_FillPolygon(pts, 3, fillColor);
    if (strokeOn)
        AGFX_LL_DrawPolygon(pts, 3, strokeColor);
}

void AGFX::quadr(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
        uint16_t x3, uint16_t y3, uint16_t x4, uint16_t y4)
{
    agfx_point_t pts[4] = { { x1, y1 }, { x2, y2 }, { x3, y3 }, { x4, y4 } };

    if (fillOn)
        AGFX_LL_FillPolygon(pts, 4, fillColor);
    if (strokeOn)
        AGFX_LL_DrawPolygon(pts, 4, strokeColor);
}

void AGFX::circle(uint16_t x1, uint16_t y1, uint16_t r)
{
    if (fillOn)
        AGFX_LL_FillCircle(x1, y1, r, AGFX_QUAD_ALL, fillColor);
    if (strokeOn)
        AGFX_LL_DrawCircle(x1, y1, r, AGFX_QUAD_ALL, strokeColor);
}

void AGFX::ellipse(uint16_t x1, uint16_t y1, uint16_t width, uint16_t height)
{
    width >> 1;
    height >> 1;

    if (fillOn)
        AGFX_LL_FillEllipseInRect(x1-width, y1-height, x1+width, y1+height, fillColor);
    if (strokeOn)
        AGFX_LL_DrawEllipseInRect(x1-width, y1-height, x1+width, y1+height, strokeColor);
}

void AGFX::poly(const agfx_point_t *pts, const uint16_t nPts)
{
    if (fillOn)
        AGFX_LL_FillPolygon(pts, nPts, fillColor);
    if (strokeOn)
        AGFX_LL_DrawPolygon(pts, nPts, fillColor);
}
// Processing stype API ------------------- END ----------------------

// Font API ------------------------------- BEGIN --------------------
void AGFX::setCursorPos(const uint16_t x1, const uint16_t y1)
{
    locX = x1;
    locY = y1;
}

void AGFX::drawChar(const char c)
{
    if (c == '\n') {
        locX  = 0;
        locY += AGFX_LL_GetFontHeight();
    }
    else if (c!='\r' && strokeOn)
        locX += AGFX_LL_DrawChar(locX, locY, strokeColor, c);
}

void AGFX::drawText(const char *text)
{
    while (*text)
        drawChar(*text++);
}

void AGFX::setFontFace(const GFXfont * const font)
{
    AGFX_LL_SetFontFace(font);
}

void AGFX::setFontMag(const uint8_t mag)
{
    AGFX_LL_SetFontMag(mag);
}
// Font API ------------------------------- END ----------------------

// Some demos
#ifdef ENABLE_DEMOS
// -1 == forever
void AGFX::demoLine(int cnt)
{
    uint32_t color = 0;

    while (cnt) {
        for (int s=0; s<24; s+=8) {
            for (int x=0; x<800; x++) {
                stroke((color<<s) | 0xFF000000);
                line(x, 0, 799-x, 479);
                color++;
                color = color % 256;
                if (color==255)
                    color = 0;
            }
            for (int y=0; y<480; y++) {
                stroke((color<<s) | 0xFF000000);
                line(0, 479-y, 799, y);
                color++;
                color = color % 256;
                if (color==255)
                    color = 0;
            }
        }
        if (cnt>0)
            cnt--;
    }
}

void AGFX::demoPolygon()
{
    const agfx_point_t pts1[5] = { { 10, 10 }, { 200, 50 }, { 300, 100 },
        { 150, 300 }, { 50, 250 } };
    const agfx_point_t pts2[5] = { { 310, 10 }, { 550, 300 }, { 500, 50 },
        { 350, 250 }, { 600, 100 } };

    fill(AGFX_GREEN);
    stroke(AGFX_LIGHTRED);
    poly(pts1, 5);

    fill(AGFX_ORANGE);
    stroke(AGFX_BLUE);
    poly(pts2, 5);
}

void AGFX::demoCircle()
{
    stroke(AGFX_RED);
    fill(AGFX_YELLOW);
    circle(10, 10, 5);

    stroke(AGFX_GREEN);
    fill(AGFX_CYAN);
    circle(100, 100, 50);

    stroke(AGFX_BLUE);
    fill(AGFX_ORANGE);
    circle(500, 220, 200);
}

void AGFX::demoEllipse()
{
    noStroke();
    fill(color(255, 255, 0));
    ellipse(50, 50, 30, 20);
    fill(color(255, 0, 0, 100));
    ellipse(300, 300, 200, 150);
    fill(color(230));
    ellipse(500, 220, 100, 200);

    noFill();
    stroke(AGFX_RED);
    ellipse(60, 30, 50, 20);
    stroke(AGFX_GREEN);
    ellipse(300, 200, 200, 150);
    stroke(AGFX_BLUE);
    ellipse(650, 220, 100, 200);
}

void AGFX::demoTouch()
{
    static TS_StateTypeDef state;
    static uint32_t color = AGFX_RED;
    static uint16_t xOld=0, yOld=0;
    static uint8_t r = 0, g = 0, b = 0;
    uint16_t x, y;

    while (1) {
        if (STAR_TS_GetState(&state) == TS_OK) {
            if (state.touchDetected) {
                x = AGFX_TS_FixX(state.touchX[0]);
                y = AGFX_TS_FixY(state.touchY[0]);
                if (x!=xOld || y!=yOld) {
                    xOld = x;
                    yOld = y;
                    color = 0xFF000000 | (r<<16) | (g<<8) | b;
                    stroke(color);
                    fill(color);
                    circle(x, y, 20);
                    if (!b && !g)
                        r++;
                    if (!r && !b)
                        g++;
                    if (!r && !g)
                        b++;
                }
            }
        }
    }
}

// NOTA BENE: temporary font used for testing purposes only
//            replace with own fonts
#include "FreeSerifItalic24pt7b.h"
void AGFX::demoText()
{
    setFontFace(&FreeSerifItalic24pt7b);
    setFontMag(1);

    stroke(AGFX_BLACK);
    setCursorPos(10, 100);
    drawText("Black text");
    drawText(" another text");
    stroke(AGFX_RED);
    setCursorPos(10, 200);
    drawText("Red text\nwith newline");

    stroke(AGFX_BLUE);
    setFontMag(2);
    setCursorPos(10, 400);
    drawText("Big blue text");
}
#endif
