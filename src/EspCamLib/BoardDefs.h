#ifndef ESPCAMLIB_BOARDDEFS_H
#define ESPCAMLIB_BOARDDEFS_H
namespace EspCam
{
    struct BoardDef
    {
        int pwdn;
        int reset;
        int xclk;
        int siod;
        int sioc;
        int d7;
        int d6;
        int d5;
        int d4;
        int d3;
        int d2;
        int d1;
        int d0;
        int vsync;
        int href;
        int pclk;
        int flash;

// AI Thinker ESP32-CAM
#define PINOUT_AI_THINKER EspCam::BoardDef::AIThinker
        static void AIThinker(BoardDef *def)
        {
            def->pwdn = 32;
            def->reset = -1;
            def->xclk = 0;
            def->siod = 26;
            def->sioc = 27;
            def->d7 = 35;
            def->d6 = 34;
            def->d5 = 39;
            def->d4 = 36;
            def->d3 = 21;
            def->d2 = 19;
            def->d1 = 18;
            def->d0 = 5;
            def->vsync = 25;
            def->href = 23;
            def->pclk = 22;
            def->flash = 4;
        };
    };
}
#endif