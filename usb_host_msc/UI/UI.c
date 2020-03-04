/*
 * UI.c
 *
 *  Created on: Feb 14, 2020
 *      Author: marka
 */

// *** Includes
//
#include "UI.h"
#include "/Users/margalitnoa/git/USB_Interface_3/usb_host_msc/Memory/usb_host_msc.h"

// *** Variables
//
#pragma DATA_ALIGN(psDMAControlTable, 1024)

bool begin_motor_IT;
bool begin_file_transfer;
bool begin_usb_connect;
uint32_t file_sel_index;

static tDMAControlTable psDMAControlTable[64];
static tContext sContext;
static tRectangle sRect;
extern tCanvasWidget g_psPanelsUI[];


//******************************************************************************
//  Temporary File Names
//******************************************************************************
const char * UI_Filenames[] =
{
     "NULLNULLNULLNULL",
     "NULLNULLNULLNULL",
     "NULLNULLNULLNULL"
};
#define UI_NUM_FILES (sizeof(UI_Filenames) / sizeof(UI_Filenames[0]))

const uint32_t bytesTx = 420;

//*****************************************************************************
// The names for each of the panels, which is displayed at the bottom of the
// screen (The sloppy style is on purpose to properly center the text)
//*****************************************************************************
char *g_pcPanei32NamesUI[] =
{
    "   Introduction   ",
    "   Test Menu   ",
    "   Memory Test   ",
    "   File System   ",
    "   Confirm File   ",
    "   Transferring   ",
    "   Test Complete   ",
    "   Motor Test   ",
    "   In Progress   ",
    "      Error       ",
    "   Test Complete   "
};

//*****************************************************************************
// The panel that is currently being displayed.
//*****************************************************************************
uint32_t g_ui32PanelUI;

//*****************************************************************************
// 0 - Introduction Panel
//*****************************************************************************
Canvas(g_sIntro, g_psPanelsUI, 0, 0, &g_sKentec320x240x16_SSD2119, 0, 24,
       320, 166, CANVAS_STYLE_APP_DRAWN, 0, 0, 0, 0, 0, 0, UI_OnIntroPaint);

//*****************************************************************************
// 1 - Integration Test Menu Panel
//*****************************************************************************
tPushButtonWidget pushButtons_SelectIT[] =
{
    RectangularButtonStruct(g_psPanelsUI + 1, pushButtons_SelectIT + 1, 0,
                            &g_sKentec320x240x16_SSD2119, 60, 60, 200, 50,
                            PB_STYLE_FILL | PB_STYLE_OUTLINE | PB_STYLE_TEXT,
                            ClrDeepSkyBlue, ClrLavender, ClrNavajoWhite, ClrBlack,
                            &g_sFontCm20, "Memory Test", 0, 0, 0, 0, UI_SelectMemTest),
    RectangularButtonStruct(g_psPanelsUI + 1, 0, 0,
                            &g_sKentec320x240x16_SSD2119, 60, 120, 200, 50,
                            PB_STYLE_FILL | PB_STYLE_OUTLINE | PB_STYLE_TEXT,
                            ClrDeepSkyBlue, ClrLavender, ClrNavajoWhite, ClrBlack,
                            &g_sFontCm20, "Motor Test", 0, 0, 0, 0, UI_SelectMotorTest),

};
#define NUM_PUSH_BUTTONS        (sizeof(pushButtons_SelectIT) /   \
                                 sizeof(pushButtons_SelectIT[0]))
uint32_t g_ui32ButtonStateUI = 0;

//*****************************************************************************
// 2 - Memory Test Panel
//*****************************************************************************
Canvas(g_sMemTest, g_psPanelsUI + 2, 0, 0, &g_sKentec320x240x16_SSD2119, 0, 24,
       320, 166, CANVAS_STYLE_APP_DRAWN, 0, 0, 0, 0, 0, 0, UI_OnMemTestPaint);

//*****************************************************************************
// 3 - File System Panel
//*****************************************************************************
Canvas(g_sFileSystemNames, g_psPanelsUI + 3, 0, 0, &g_sKentec320x240x16_SSD2119, 0, 24,
       320, 166, CANVAS_STYLE_APP_DRAWN, 0, 0, 0, 0, 0, 0, UI_OnFilesystemPaint);
tPushButtonWidget pushButtons_MemSelectFile[] =
{
     RectangularButtonStruct(g_psPanelsUI + 3, pushButtons_MemSelectFile + 1, 0,
                             &g_sKentec320x240x16_SSD2119, 60, 30, 50, 50,
                             (PB_STYLE_IMG | PB_STYLE_TEXT |
                              PB_STYLE_AUTO_REPEAT), 0, 0, 0, ClrSilver,
                             &g_sFontCm22, "1", g_pui8Blue50x50,
                             g_pui8Blue50x50Press, 125, 25, UI_MemSelectFile),
     RectangularButtonStruct(g_psPanelsUI + 3, pushButtons_MemSelectFile + 2, 0,
                             &g_sKentec320x240x16_SSD2119, 60, 85, 50, 50,
                             (PB_STYLE_IMG | PB_STYLE_TEXT |
                              PB_STYLE_AUTO_REPEAT), 0, 0, 0, ClrSilver,
                             &g_sFontCm22, "2", g_pui8Blue50x50,
                             g_pui8Blue50x50Press, 125, 25, UI_MemSelectFile),
     RectangularButtonStruct(g_psPanelsUI + 3, &g_sFileSystemNames, 0,
                             &g_sKentec320x240x16_SSD2119, 60, 140, 50, 50,
                             (PB_STYLE_IMG | PB_STYLE_TEXT |
                              PB_STYLE_AUTO_REPEAT), 0, 0, 0, ClrSilver,
                             &g_sFontCm22, "3", g_pui8Blue50x50,
                             g_pui8Blue50x50Press, 125, 25, UI_MemSelectFile),
};

//*****************************************************************************
// 4 - Confirm File Selection Panel
//*****************************************************************************
Canvas(g_sFileSel, g_psPanelsUI + 4, 0, 0, &g_sKentec320x240x16_SSD2119, 0, 24,
       320, 166, CANVAS_STYLE_APP_DRAWN, 0, 0, 0, 0, 0, 0, UI_OnFileSelPaint);
Slider(slider_ConfirmFile ,g_psPanelsUI + 5, &g_sFileSel, 0,
             &g_sKentec320x240x16_SSD2119, 50, 120, 220, 30, 0, 100, 0,
             (SL_STYLE_FILL | SL_STYLE_BACKG_FILL | SL_STYLE_TEXT |
              SL_STYLE_BACKG_TEXT | SL_STYLE_TEXT_OPAQUE |
              SL_STYLE_BACKG_TEXT_OPAQUE),
              ClrGreen, ClrGhostWhite, ClrYellowGreen, ClrYellow, ClrBlack,
             &g_sFontCm18, "Swipe Right to Confirm", 0, 0,
             UI_SliderMemConfrimFile);

//*****************************************************************************
// 5 - Memory Transfer In Progress Panel
//*****************************************************************************
Canvas(g_sTransfer, g_psPanelsUI + 5, 0, 0, &g_sKentec320x240x16_SSD2119, 0, 24,
       320, 166, CANVAS_STYLE_APP_DRAWN, 0, 0, 0, 0, 0, 0, UI_OnTransferPaint);

//*****************************************************************************
// 6 - Memory Test Complete Panel
//*****************************************************************************
Canvas(g_sMemComplete, g_psPanelsUI + 6, 0, 0, &g_sKentec320x240x16_SSD2119, 0, 24,
       320, 166, CANVAS_STYLE_APP_DRAWN, 0, 0, 0, 0, 0, 0, UI_OnMemCompletePaint);

//*****************************************************************************
// 7 - Motor Test Start Panel
//*****************************************************************************
Canvas(g_sMotorTest, g_psPanelsUI + 7, 0, 0, &g_sKentec320x240x16_SSD2119, 0, 24,
       320, 166, CANVAS_STYLE_APP_DRAWN, 0, 0, 0, 0, 0, 0, UI_OnMotorTestPaint);

//*****************************************************************************
// 8 - Motor Test In Progress Panel
//*****************************************************************************
Canvas(g_sMotorGo, g_psPanelsUI + 8, 0, 0, &g_sKentec320x240x16_SSD2119, 0, 24,
       320, 166, CANVAS_STYLE_APP_DRAWN, 0, 0, 0, 0, 0, 0, UI_OnMotorGoPaint);

//*****************************************************************************
// 9 - Motor Collision Panel
//*****************************************************************************
Canvas(g_sMotorCol, g_psPanelsUI + 9, 0, 0, &g_sKentec320x240x16_SSD2119, 0, 24,
       320, 166, CANVAS_STYLE_APP_DRAWN, 0, 0, 0, 0, 0, 0, UI_OnMotorColPaint);

//*****************************************************************************
// 10 - Motor Test Complete Panel
//*****************************************************************************
Canvas(g_sMotorComplete, g_psPanelsUI + 10, 0, 0, &g_sKentec320x240x16_SSD2119, 0, 24,
       320, 166, CANVAS_STYLE_APP_DRAWN, 0, 0, 0, 0, 0, 0, UI_OnMotorCompletePaint);

//*****************************************************************************
// An array of canvas widgets, one per panel.  Each canvas is filled with
// black, overwriting the contents of the previous panel.
//*****************************************************************************
tCanvasWidget g_psPanelsUI[] =
{
//    CanvasStruct(0, 0, &g_sWarmUp, &g_sKentec320x240x16_SSD2119, 0, 24,
//                 320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0),
    CanvasStruct(0, 0, &g_sIntro, &g_sKentec320x240x16_SSD2119, 0, 24,
                 320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0),
    CanvasStruct(0, 0, pushButtons_SelectIT, &g_sKentec320x240x16_SSD2119, 0, 24,
                 320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0),
    CanvasStruct(0, 0, &g_sMemTest, &g_sKentec320x240x16_SSD2119, 0, 24,
                 320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0),
    CanvasStruct(0, 0, pushButtons_MemSelectFile, &g_sKentec320x240x16_SSD2119, 0, 24,
                 320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0),
    CanvasStruct(0, 0, &slider_ConfirmFile, &g_sKentec320x240x16_SSD2119, 0, 24,
                 320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0),
    CanvasStruct(0, 0, &g_sTransfer, &g_sKentec320x240x16_SSD2119, 0, 24,
                 320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0),
    CanvasStruct(0, 0, &g_sMemComplete, &g_sKentec320x240x16_SSD2119, 0, 24,
                 320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0),
    CanvasStruct(0, 0, &g_sMotorTest, &g_sKentec320x240x16_SSD2119, 0, 24,
                 320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0),
    CanvasStruct(0, 0, &g_sMotorGo, &g_sKentec320x240x16_SSD2119, 0, 24,
                 320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0),
    CanvasStruct(0, 0, &g_sMotorCol, &g_sKentec320x240x16_SSD2119, 0, 24,
                 320, 166, CANVAS_STYLE_FILL, ClrDarkRed, 0, 0, 0, 0, 0, 0),
    CanvasStruct(0, 0, &g_sMotorComplete, &g_sKentec320x240x16_SSD2119, 0, 24,
                 320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0)
};

#define UI_NUM_PANELS              (sizeof(g_psPanelsUI) / sizeof(g_psPanelsUI[0]))

//*****************************************************************************
// The buttons and text across the bottom of the screen.
//*****************************************************************************
RectangularButton(g_sPreviousUI, 0, 0, 0, &g_sKentec320x240x16_SSD2119, 0, 190,
                  50, 50, PB_STYLE_FILL, ClrBlack, ClrBlack, 0, ClrSilver,
                  &g_sFontCm20, "H", g_pui8Blue50x50, g_pui8Blue50x50Press, 0, 0,
                  UI_ReturnHome);

Canvas(g_sTitleUI, 0, 0, 0, &g_sKentec320x240x16_SSD2119, 50, 190, 220, 50,
       CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_OPAQUE, 0, 0, ClrSilver,
       &g_sFontCm20, 0, 0, 0);

RectangularButton(g_sNextUI, 0, 0, 0, &g_sKentec320x240x16_SSD2119, 270, 190,
                  50, 50, PB_STYLE_IMG | PB_STYLE_TEXT, ClrBlack, ClrBlack, 0,
                  ClrSilver, &g_sFontCm20, "+", g_pui8Blue50x50,
                  g_pui8Blue50x50Press, 0, 0, UI_OnNext);


//*****************************************************************************
// Initializes User Interface
//*****************************************************************************
void UI_Init(uint32_t ui32SysClock)
{
    begin_motor_IT      = false;
    begin_file_transfer = false;
    begin_usb_connect   = false;

    FPUEnable();
    FPULazyStackingEnable();

    // Set the clock to 40Mhz derived from the PLL and the external oscillator

    // Initialize the display driver.
    Kentec320x240x16_SSD2119Init(ui32SysClock);   //screen goes white

    // Initialize the graphics context.
    GrContextInit(&sContext, &g_sKentec320x240x16_SSD2119);

    // Create banner at the top of the LCD
    UI_GenerateHeader();

    // Configure and enable uDMA
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    SysCtlDelay(10);
    uDMAControlBaseSet(&psDMAControlTable[0]);
    uDMAEnable();

    // Initialize the touch screen driver and have it route its messages to the
    // widget tree.
    TouchScreenInit(ui32SysClock);
    TouchScreenCallbackSet(WidgetPointerMessage);

    // Fun graphic to symbolize system start
    UI_OnWarmUpPaint(&sContext);

    // Initialize the widget tree
    UI_InitWidgetsIntegrationTest();
}

//*****************************************************************************
//
// Initializes Widgets for Integration Testing
//
//*****************************************************************************
void UI_InitWidgetsIntegrationTest(void)
{
    // Add the title block and the previous and next buttons to the widget tree.
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sPreviousUI);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sTitleUI);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sNextUI);

    // Add the first panel to the widget tree.
    g_ui32PanelUI = 0;
    WidgetAdd(WIDGET_ROOT, (tWidget *)g_psPanelsUI);
    CanvasTextSet(&g_sTitleUI, g_pcPanei32NamesUI[0]);

    // Issue the initial paint request to the widgets.
    WidgetPaint(WIDGET_ROOT);
}

//*****************************************************************************
//
// Creates IFFF Header at the top of the LCD
//
//*****************************************************************************
void UI_GenerateHeader(void)
{
    // Fill the top 24 rows of the screen with blue to create the banner.
    sRect.i16XMin = 0;
    sRect.i16YMin = 0;
    sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
    sRect.i16YMax = 23;
    GrContextForegroundSet(&sContext, ClrDarkBlue);
    GrRectFill(&sContext, &sRect);

    // Put a white box around the banner.
    GrContextForegroundSet(&sContext, ClrWhite);
    GrRectDraw(&sContext, &sRect);

    // Put the application name in the middle of the banner.
    GrContextFontSet(&sContext, &g_sFontCm20);
    GrStringDrawCentered(&sContext, "IFFF", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 8, 0);
}

//*****************************************************************************
//
// Handles paint requests for the warm up canvas widget.
//
//*****************************************************************************
void UI_OnWarmUpPaint(tContext *psContext)
{
    uint32_t ui32Idx;

    // Draw a vertical sweep of lines from red to green.
    for(ui32Idx = 0; ui32Idx <= 8; ui32Idx++)
    {
        GrContextForegroundSet(psContext,
                               (((((10 - ui32Idx) * 255) / 10) << ClrRedShift) |
                                (((ui32Idx * 255) / 10) << ClrGreenShift)));
        GrLineDraw(psContext, GrContextDpyWidthGet(psContext) / 2, 120, 5, 120 - (11 * ui32Idx));
        SysCtlDelay(UI_MS_TO_TICKS(UI_WARMUP_DELAY_TIME));
    }

    //
    // Draw a horizontal sweep of lines from green to blue.
    //
    for(ui32Idx = 1; ui32Idx <= 9; ui32Idx++)
    {
        GrContextForegroundSet(psContext,
                               (((((10 - ui32Idx) * 255) / 10) <<
                                 ClrGreenShift) |
                                (((ui32Idx * 255) / 10) << ClrBlueShift)));
        GrLineDraw(psContext, GrContextDpyWidthGet(psContext) / 2, 120, 5 + (ui32Idx * 15), 29);
        SysCtlDelay(UI_MS_TO_TICKS(UI_WARMUP_DELAY_TIME));
    }
    GrContextForegroundSet(psContext, (ClrGreenShift) | (((ui32Idx * 255) / 10) << ClrBlueShift));
    GrLineDraw(psContext, GrContextDpyWidthGet(psContext) / 2, 120, 5 + (150), 29);

    //
    // Draw a horizontal sweep of lines from blue to green.
    //
    for(ui32Idx = 1; ui32Idx <= 10; ui32Idx++)
    {
        GrContextForegroundSet(psContext,
                               (((((10 - ui32Idx) * 255) / 10) <<
                                 ClrBlueShift) |
                                (((ui32Idx * 255) / 10) << ClrGreenShift)));
        GrLineDraw(psContext, GrContextDpyWidthGet(psContext) / 2, 120, 140 + (ui32Idx * 15), 29);
        SysCtlDelay(UI_MS_TO_TICKS(UI_WARMUP_DELAY_TIME));
    }

    //
    // Draw a vertical sweep of lines from green to red.
    //
    for(ui32Idx = 8; ui32Idx > 0; ui32Idx--)
    {
        GrContextForegroundSet(psContext,
                               (((((10 - ui32Idx) * 255) / 10) << ClrRedShift) |
                                (((ui32Idx * 255) / 10) << ClrGreenShift)));
        GrLineDraw(psContext, GrContextDpyWidthGet(psContext) / 2, 120, 305, 120 - (11 * ui32Idx));
        SysCtlDelay(UI_MS_TO_TICKS(UI_WARMUP_DELAY_TIME));
    }

    GrContextForegroundSet(psContext, (255 << ClrRedShift) | (0 << ClrGreenShift));
    GrLineDraw(psContext, GrContextDpyWidthGet(psContext) / 2, 120, 305, 120);
}

//*****************************************************************************
// Handles paint requests for the introduction canvas widget.
//*****************************************************************************
void UI_OnIntroPaint(tWidget *psWidget, tContext *psContext)
{
    // Display the introduction text in the canvas.
    GrContextFontSet(psContext, &g_sFontCm18);
    GrContextForegroundSet(psContext, ClrSilver);

    GrStringDrawCentered(psContext, "~*~*~*~*~ Waveguide Wizards ~*~*~*~*~", -1,
                         GrContextDpyWidthGet(psContext) / 2, 32, 0);
    GrStringDrawCentered(psContext, "Welcome to the incredible fused", -1,
                         GrContextDpyWidthGet(psContext) / 2, 74, 0);
    GrStringDrawCentered(psContext, "filament fabricator (IFFF)", -1,
                         GrContextDpyWidthGet(psContext) / 2, 92, 0);
    GrStringDrawCentered(psContext, "Press the + button at the bottom", -1,
                         GrContextDpyWidthGet(psContext) / 2, 146, 0);
    GrStringDrawCentered(psContext, "to select an integration test: ", -1,
                         GrContextDpyWidthGet(psContext) / 2, 164, 0);
}

//*****************************************************************************
// Handles paint requests for the test menu canvas widget.
//*****************************************************************************
void UI_OnTestMenuPaint(tWidget *psWidget, tContext *psContext)
{
    // This has been replaced by button functionality
    // See UI_SelectMemTest and UI_SelectMotorTest for more information
}

//*****************************************************************************
// Handles paint requests for the memory integration test canvas widget.
//*****************************************************************************
void UI_OnMemTestPaint(tWidget *psWidget, tContext *psContext)
{
    // Display the introduction text in the canvas.
    GrContextFontSet(psContext, &g_sFontCm18);
    GrContextForegroundSet(psContext, ClrSilver);

    GrStringDrawCentered(psContext, "Memory Test", -1,
                         GrContextDpyWidthGet(psContext) / 2, 32, 0);
    GrStringDrawCentered(psContext, "This is an integration test of", -1,
                         GrContextDpyWidthGet(psContext) / 2, 74, 0);
    GrStringDrawCentered(psContext, "the USB file transfer, the Flash", -1,
                         GrContextDpyWidthGet(psContext) / 2, 92, 0);
    GrStringDrawCentered(psContext, "memory peripheral, and the UI.", -1,
                         GrContextDpyWidthGet(psContext) / 2, 110, 0);
    GrStringDrawCentered(psContext, "Continue to select a file from the USB", -1,
                         GrContextDpyWidthGet(psContext) / 2, 164, 0);
}

//*****************************************************************************
//  Paints file names next to filesystem navigation button widgets.
//*****************************************************************************
void UI_OnFilesystemPaint(tWidget *psWidget, tContext *psContext)
{
    GrContextFontSet(psContext, &g_sFontCm18);
    GrContextForegroundSet(psContext, ClrSilver);

    GrStringDrawCentered(psContext, UI_Filenames[0], -1, 140, 50, 0);
    GrStringDrawCentered(psContext, UI_Filenames[1], -1, 140, 105, 0);
    GrStringDrawCentered(psContext, UI_Filenames[2], -1, 140, 160, 0);
}

//*****************************************************************************
// Handles paint requests for the file confirmation canvas widget.
//*****************************************************************************
void UI_OnFileSelPaint(tWidget *psWidget, tContext *psContext)
{
    // Display the introduction text in the canvas.
    GrContextFontSet(psContext, &g_sFontCm18);
    GrContextForegroundSet(psContext, ClrSilver);

    GrStringDrawCentered(psContext, "File Selection:", -1,
                         GrContextDpyWidthGet(psContext) / 2, 32, 0);

    GrStringDrawCentered(psContext, UI_Filenames[file_sel_index], -1,
                         GrContextDpyWidthGet(psContext) / 2, 72, 0);

}

//*****************************************************************************
// Handles paint requests for the file transfer canvas widget.
//*****************************************************************************
void UI_OnTransferPaint(tWidget *psWidget, tContext *psContext)
{
    // Display the introduction text in the canvas.
    GrContextFontSet(psContext, &g_sFontCm18);
    GrContextForegroundSet(psContext, ClrSilver);

    GrStringDrawCentered(psContext, "~*~*~ Transferring ~*~*~", -1,
                         GrContextDpyWidthGet(psContext) / 2, 100, 0);
}

//*****************************************************************************
// Handles paint requests for the memory test completion canvas widget.
//*****************************************************************************
void UI_OnMemCompletePaint(tWidget *psWidget, tContext *psContext)
{
    // Display the introduction text in the canvas.
    GrContextFontSet(psContext, &g_sFontCm18);
    GrContextForegroundSet(psContext, ClrSilver);

    GrStringDrawCentered(psContext, "File Transfer Complete!", -1,
                         GrContextDpyWidthGet(psContext) / 2, 32, 0);

    char transferData[40];
    sprintf(transferData, "Bytes Transferred: %u", bytesTx);

    GrStringDrawCentered(psContext, transferData, -1,
                         GrContextDpyWidthGet(psContext) / 2, 74, 0);
}

//*****************************************************************************
// Handles paint requests for the motor test canvas widget.
//*****************************************************************************
void UI_OnMotorTestPaint(tWidget *psWidget, tContext *psContext)
{
    // Display the introduction text in the canvas.
    GrContextFontSet(psContext, &g_sFontCm18);
    GrContextForegroundSet(psContext, ClrSilver);

    GrStringDrawCentered(psContext, "Motor Test", -1,
                         GrContextDpyWidthGet(psContext) / 2, 32, 0);
    GrStringDrawCentered(psContext, "This is an integration test of", -1,
                         GrContextDpyWidthGet(psContext) / 2, 74, 0);
    GrStringDrawCentered(psContext, "the PWM motor control in the", -1,
                         GrContextDpyWidthGet(psContext) / 2, 92, 0);
    GrStringDrawCentered(psContext, "context of error events while", -1,
                         GrContextDpyWidthGet(psContext) / 2, 110, 0);
    GrStringDrawCentered(psContext, "integrating the UI for control", -1,
                         GrContextDpyWidthGet(psContext) / 2, 128, 0);
    GrStringDrawCentered(psContext, "Continue to begin test", -1,
                         GrContextDpyWidthGet(psContext) / 2, 164, 0);
}

//*****************************************************************************
// Handles paint requests for the motor test underway widget.
//*****************************************************************************
void UI_OnMotorGoPaint(tWidget *psWidget, tContext *psContext)
{
    // Display the introduction text in the canvas.
    GrContextFontSet(psContext, &g_sFontCm18);
    GrContextForegroundSet(psContext, ClrSilver);

    GrStringDrawCentered(psContext, "Test in Progress", -1,
                         GrContextDpyWidthGet(psContext) / 2, 100, 0);
}

//*****************************************************************************
// Handles paint requests for the motor collision widget.
//*****************************************************************************
void UI_OnMotorColPaint(tWidget *psWidget, tContext *psContext)
{
    // Display the introduction text in the canvas.
    GrContextFontSet(psContext, &g_sFontCm18);
    GrContextForegroundSet(psContext, ClrSilver);

    GrStringDrawCentered(psContext, "Motor Collision with Bumper!", -1,
                         GrContextDpyWidthGet(psContext) / 2, 100, 0);
}

//*****************************************************************************
// Handles paint requests for the motor test completion widget.
//*****************************************************************************
void UI_OnMotorCompletePaint(tWidget *psWidget, tContext *psContext)
{
    // Display the introduction text in the canvas.
    GrContextFontSet(psContext, &g_sFontCm18);
    GrContextForegroundSet(psContext, ClrSilver);

    GrStringDrawCentered(psContext, "Motor Integration Test Complete", -1,
                         GrContextDpyWidthGet(psContext) / 2, 100, 0);
}

//*****************************************************************************
// Handles presses of the previous panel button.
//*****************************************************************************
void UI_OnPrevious(tWidget *psWidget)
{
    //
    // There is nothing to be done if the first panel is already being
    // displayed.
    //
    if(g_ui32PanelUI == 0)
    {
        return;
    }

    //
    // Remove the current panel.
    //
    WidgetRemove((tWidget *)(g_psPanelsUI + g_ui32PanelUI));

    //
    // Decrement the panel index.
    //
    g_ui32PanelUI--;

    //
    // Add and draw the new panel.
    //
    WidgetAdd(WIDGET_ROOT, (tWidget *)(g_psPanelsUI + g_ui32PanelUI));
    WidgetPaint((tWidget *)(g_psPanelsUI + g_ui32PanelUI));

    //
    // Set the title of this panel.
    //
    CanvasTextSet(&g_sTitleUI, g_pcPanei32NamesUI[g_ui32PanelUI]);
    WidgetPaint((tWidget *)&g_sTitleUI);

    //
    // See if this is the first panel.
    //
    if(g_ui32PanelUI == 0)
    {
        //
        // Clear the previous button from the display since the first panel is
        // being displayed.
        //
        PushButtonImageOff(&g_sPreviousUI);
        PushButtonTextOff(&g_sPreviousUI);
        PushButtonFillOn(&g_sPreviousUI);
        WidgetPaint((tWidget *)&g_sPreviousUI);
    }

    //
    // See if the previous panel was the last panel.
    //
    if(g_ui32PanelUI == (UI_NUM_PANELS - 2))
    {
        //
        // Display the next button.
        //
        PushButtonImageOn(&g_sNextUI);
        PushButtonTextOn(&g_sNextUI);
        PushButtonFillOff(&g_sNextUI);
        WidgetPaint((tWidget *)&g_sNextUI);
    }

}

//*****************************************************************************
//
// Handles presses of the next panel button.
//
//*****************************************************************************
void UI_OnNext(tWidget *psWidget)
{
    //
    // There is nothing to be done if the last panel is already being
    // displayed.
    //
    if(g_ui32PanelUI == (UI_NUM_PANELS - 1))
    {
        return;
    }

    if(g_ui32PanelUI == 7)
    {
        begin_motor_IT = true;
    }

    // See if the panel should not have a next button
    if(g_ui32PanelUI == 1 || g_ui32PanelUI == 3 ||
       g_ui32PanelUI == 4 || g_ui32PanelUI == 6)
    {
        return;
    }

    //
    // Remove the current panel.
    //
    WidgetRemove((tWidget *)(g_psPanelsUI + g_ui32PanelUI));

    //
    // Increment the panel index.
    //
    g_ui32PanelUI++;

    //
    // Add and draw the new panel.
    //
    WidgetAdd(WIDGET_ROOT, (tWidget *)(g_psPanelsUI + g_ui32PanelUI));
    WidgetPaint((tWidget *)(g_psPanelsUI + g_ui32PanelUI));

    // Set the title of this panel.
    CanvasTextSet(&g_sTitleUI, g_pcPanei32NamesUI[g_ui32PanelUI]);
    WidgetPaint((tWidget *)&g_sTitleUI);

    // See if the previous panel was the first panel.
    if(g_ui32PanelUI == 1)
    {
        //
        // Display the previous button.
        //
        PushButtonImageOn(&g_sPreviousUI);
        PushButtonTextOn(&g_sPreviousUI);
        PushButtonFillOff(&g_sPreviousUI);
        WidgetPaint((tWidget *)&g_sPreviousUI);
    }

    // See if the panel does not need a next button
    if(g_ui32PanelUI == 1 || g_ui32PanelUI == 3 ||
       g_ui32PanelUI == 4 || g_ui32PanelUI == 6)
    {
        // Hide the next button.
        PushButtonImageOff(&g_sNextUI);
        PushButtonTextOff(&g_sNextUI);
        PushButtonFillOn(&g_sNextUI);
        WidgetPaint((tWidget *)&g_sNextUI);
    }
}

//*****************************************************************************
// Handles presses of the memory test button
//*****************************************************************************
void UI_SelectMemTest(tWidget *psWidget)
{
    if(g_ui32PanelUI != 1)
    {
        return;
    }

    begin_usb_connect = true;

    // Remove the current panel.
    WidgetRemove((tWidget *)(g_psPanelsUI + g_ui32PanelUI));

    // Add and draw the new panel.
    g_ui32PanelUI = 3;
    WidgetAdd(WIDGET_ROOT, (tWidget *)(g_psPanelsUI + g_ui32PanelUI));
    WidgetPaint((tWidget *)(g_psPanelsUI + g_ui32PanelUI));

    // Set the title of this panel.
    CanvasTextSet(&g_sTitleUI, g_pcPanei32NamesUI[g_ui32PanelUI]);
    WidgetPaint((tWidget *)&g_sTitleUI);
}

//*****************************************************************************
// Handles presses of the motor test button
//*****************************************************************************
void UI_SelectMotorTest(tWidget *psWidget)
{
    if(g_ui32PanelUI != 1)
    {
        return;
    }

    // Remove the current panel.
    WidgetRemove((tWidget *)(g_psPanelsUI + g_ui32PanelUI));

    // Add and draw the new panel.
    g_ui32PanelUI = 7;
    WidgetAdd(WIDGET_ROOT, (tWidget *)(g_psPanelsUI + g_ui32PanelUI));
    WidgetPaint((tWidget *)(g_psPanelsUI + g_ui32PanelUI));

    // Set the title of this panel.
    CanvasTextSet(&g_sTitleUI, g_pcPanei32NamesUI[g_ui32PanelUI]);
    WidgetPaint((tWidget *)&g_sTitleUI);

    // Display the next button.
    PushButtonImageOn(&g_sNextUI);
    PushButtonTextOn(&g_sNextUI);
    PushButtonFillOff(&g_sNextUI);
    WidgetPaint((tWidget *)&g_sNextUI);
}

//*****************************************************************************
// Handles presses of the file select buttons
//*****************************************************************************
void UI_MemSelectFile(tWidget *psWidget)
{
    if(g_ui32PanelUI != 3)
    {
        return;
    }

    uint32_t ui32Idx;

    // Find the index of this push button.
    for(ui32Idx = 0; ui32Idx < UI_NUM_FILES; ui32Idx++)
    {
        if(psWidget == (tWidget *)(pushButtons_MemSelectFile + ui32Idx))
        {
            break;
        }
    }

    // This will eventually be expanded to allow for more than 3 files
    if(ui32Idx == UI_NUM_FILES)
    {
        return;
    }

    // Set File Index
    file_sel_index = ui32Idx;

    // Remove the current panel.
    WidgetRemove((tWidget *)(g_psPanelsUI + g_ui32PanelUI));

    // Add and draw the new panel.
    g_ui32PanelUI = 4;
    WidgetAdd(WIDGET_ROOT, (tWidget *)(g_psPanelsUI + g_ui32PanelUI));
    WidgetPaint((tWidget *)(g_psPanelsUI + g_ui32PanelUI));

    // Set the title of this panel.
    CanvasTextSet(&g_sTitleUI, g_pcPanei32NamesUI[g_ui32PanelUI]);
    WidgetPaint((tWidget *)&g_sTitleUI);

}

void UI_SliderMemConfrimFile(tWidget * psWidget, int32_t i32Value)
{
    if(g_ui32PanelUI != 4)
    {
        return;
    }

    if(i32Value >= 99)
    {
        // reset to 0
        SliderValueSet(&slider_ConfirmFile, 0);

        begin_file_transfer = true;

        // Display the next button.
        PushButtonImageOn(&g_sNextUI);
        PushButtonTextOn(&g_sNextUI);
        PushButtonFillOff(&g_sNextUI);
        WidgetPaint((tWidget *)&g_sNextUI);

        // Remove the current panel.
        WidgetRemove((tWidget *)(g_psPanelsUI + g_ui32PanelUI));

        // Add and draw the new panel.
        g_ui32PanelUI = 5;
        WidgetAdd(WIDGET_ROOT, (tWidget *)(g_psPanelsUI + g_ui32PanelUI));
        WidgetPaint((tWidget *)(g_psPanelsUI + g_ui32PanelUI));

        // Set the title of this panel.
        CanvasTextSet(&g_sTitleUI, g_pcPanei32NamesUI[g_ui32PanelUI]);
        WidgetPaint((tWidget *)&g_sTitleUI);    }
}

void UI_ReturnHome(tWidget * psWidget)
{
    // There is nothing to be done if the first panel is already being
    // displayed.
    if(g_ui32PanelUI == 0)
    {
        return;
    }

    // Display the next button.
    PushButtonImageOn(&g_sNextUI);
    PushButtonTextOn(&g_sNextUI);
    PushButtonFillOff(&g_sNextUI);
    WidgetPaint((tWidget *)&g_sNextUI);

    // Remove the current panel.
    WidgetRemove((tWidget *)(g_psPanelsUI + g_ui32PanelUI));

    // Decrement the panel index.
    g_ui32PanelUI = 0;

    // Set the title of this panel.
    CanvasTextSet(&g_sTitleUI, g_pcPanei32NamesUI[g_ui32PanelUI]);
    WidgetPaint((tWidget *)&g_sTitleUI);

    // Add and draw the new panel.
    WidgetAdd(WIDGET_ROOT, (tWidget *)(g_psPanelsUI + g_ui32PanelUI));
    WidgetPaint((tWidget *)(g_psPanelsUI + g_ui32PanelUI));
}

void UI_HandleErrors(uint32_t err)
{
    //Error code DNE
    if(err == 0)
        return;

    // Remove the current panel.
    WidgetRemove((tWidget *)(g_psPanelsUI + g_ui32PanelUI));

    // set the panel index.
    g_ui32PanelUI = 9;

    // Set the title of this panel.
    CanvasTextSet(&g_sTitleUI, g_pcPanei32NamesUI[g_ui32PanelUI]);
    WidgetPaint((tWidget *)&g_sTitleUI);

    // Add and draw the new panel.
    WidgetAdd(WIDGET_ROOT, (tWidget *)(g_psPanelsUI + g_ui32PanelUI));
    WidgetPaint((tWidget *)(g_psPanelsUI + g_ui32PanelUI));
}

void UI_UpdateFileNames(void)
{
    //TODO StrCpy to your local buffers
}

void UI_MemTestComplete(uint32_t bytes_written)
{
    //Error code DNE
    if(bytes_written == 0)
        return;

    // Remove the current panel.
    WidgetRemove((tWidget *)(g_psPanelsUI + g_ui32PanelUI));

    // set the panel index.
    g_ui32PanelUI = 6;

    // Set the title of this panel.
    CanvasTextSet(&g_sTitleUI, g_pcPanei32NamesUI[g_ui32PanelUI]);
    WidgetPaint((tWidget *)&g_sTitleUI);

    // Add and draw the new panel.
    WidgetAdd(WIDGET_ROOT, (tWidget *)(g_psPanelsUI + g_ui32PanelUI));
    WidgetPaint((tWidget *)(g_psPanelsUI + g_ui32PanelUI));
}
