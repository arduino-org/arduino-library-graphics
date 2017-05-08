#include <agfx.h>

AGFX agfx = AGFX();

void setup()
{
    agfx.begin();
}

void loop()
{
    agfx.showScreen(0);
    agfx.setDrawScreen(0);
    agfx.fill(AGFX_WHITE);

    agfx.setDrawScreen(1);
    agfx.background(AGFX_WHITE);
    agfx.showScreen(1);
    agfx.demoLine(2);
    delay(1000);

    agfx.setDrawScreen(0);
    agfx.background(AGFX_WHITE);
    agfx.demoPolygon();
    agfx.showScreen(0);
    delay(2000);

    agfx.setDrawScreen(1);
    agfx.background(AGFX_WHITE);
    agfx.demoCircle();
    agfx.showScreen(1);
    delay(2000);

    agfx.setDrawScreen(0);
    agfx.background(AGFX_WHITE);
    agfx.demoEllipse();
    agfx.showScreen(0);
    delay(2000);

    // Copy shown screen to hidden screen and add some text
    agfx.setDrawScreen(1);
    agfx.copyScreen(0, 1);
    agfx.demoText();
    agfx.showScreen(1);
    delay(3000);

    agfx.setDrawScreen(0);
    agfx.background(AGFX_WHITE);
    agfx.showScreen(0);

    // Blocking demo
    //agfx.background(AGFX_WHITE);
    //agfx.demoTouch();
}

