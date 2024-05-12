/**
 * | Supported ESP SoCs | ESP32-S3 |
 * | ------------------ | -------- |
 *
 * | Supported LCD Controllers | ST7262 |
 * | ------------------------- | ------ |
 *
 * # Waveshare ESP32-S3-Touch-LCD-4.3 RGB Example
 *
 * The example demonstrates how to develop different model LCDs with RGB (without 3-wire SPI) interface using standalone drivers and test them by displaying color bars.
 *
 * ## How to use
 *
 * 1. [Configure drivers](https://github.com/esp-arduino-libs/ESP32_Display_Panel#configuring-drivers) if needed.
 * 2. Modify the macros in the example to match the parameters according to your hardware.
 * 3. Navigate to the `Tools` menu in the Arduino IDE to choose a ESP board and configure its parameters, please refter to [Configuring Supported Development Boards](https://github.com/esp-arduino-libs/ESP32_Display_Panel#configuring-supported-development-boards)
 * 4. Verify and upload the example to your ESP board.
 *
 * ## Serial Output
 *
 * ```
 * ...
 * RGB LCD example start
 * Create RGB LCD bus
 * Create LCD device
 * Draw color bar from top left to bottom right, the order is B - G - R
 * RGB LCD example end
 * RGB refresh rate: 0
 * RGB refresh rate: 0
 * RGB refresh rate: 31
 * RGB refresh rate: 31
 * ...
 * ```
 *
 * ## Troubleshooting
 *
 * Please check the [FAQ](https://github.com/esp-arduino-libs/ESP32_Display_Panel#faq) first to see if the same question exists. If not, please create a [Github issue](https://github.com/esp-arduino-libs/ESP32_Display_Panel/issues). We will get back to you as soon as possible.
 *
 */

#include <Arduino.h>
#include <ESP_Panel_Library.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your LCD spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Currently, the library supports the following RGB (without 3-wire SPI) LCDs:
 *      - ST7262
 */
#define ESP32_S3_LCD_NAME                        ST7262
#define ESP32_S3_LCD_WIDTH                       (800)
#define ESP32_S3_LCD_HEIGHT                      (480)
#define ESP32_S3_LCD_COLOR_BITS                  (24)
#define ESP32_S3_LCD_RGB_DATA_WIDTH              (16)
#define ESP32_S3_LCD_RGB_TIMING_FREQ_HZ          (16 * 1000 * 1000)
#define ESP32_S3_LCD_RGB_TIMING_HPW              (40)
#define ESP32_S3_LCD_RGB_TIMING_HBP              (40)
#define ESP32_S3_LCD_RGB_TIMING_HFP              (48)
#define ESP32_S3_LCD_RGB_TIMING_VPW              (23)
#define ESP32_S3_LCD_RGB_TIMING_VBP              (32)
#define ESP32_S3_LCD_RGB_TIMING_VFP              (13)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your board spec ////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define ESP32_S3_LCD_PIN_NUM_RGB_DISP            (-1)
#define ESP32_S3_LCD_PIN_NUM_RGB_VSYNC           3
#define ESP32_S3_LCD_PIN_NUM_RGB_HSYNC           46
#define ESP32_S3_LCD_PIN_NUM_RGB_DE              5
#define ESP32_S3_LCD_PIN_NUM_RGB_PCLK            7
#define ESP32_S3_LCD_PIN_NUM_RGB_DATA0           1
#define ESP32_S3_LCD_PIN_NUM_RGB_DATA1           2
#define ESP32_S3_LCD_PIN_NUM_RGB_DATA2           42
#define ESP32_S3_LCD_PIN_NUM_RGB_DATA3           41
#define ESP32_S3_LCD_PIN_NUM_RGB_DATA4           40
#define ESP32_S3_LCD_PIN_NUM_RGB_DATA5           14
#define ESP32_S3_LCD_PIN_NUM_RGB_DATA6           38
#define ESP32_S3_LCD_PIN_NUM_RGB_DATA7           18
#if ESP32_S3_LCD_RGB_DATA_WIDTH > 8
#define ESP32_S3_LCD_PIN_NUM_RGB_DATA8           17
#define ESP32_S3_LCD_PIN_NUM_RGB_DATA9           10
#define ESP32_S3_LCD_PIN_NUM_RGB_DATA10          39
#define ESP32_S3_LCD_PIN_NUM_RGB_DATA11          0
#define ESP32_S3_LCD_PIN_NUM_RGB_DATA12          45
#define ESP32_S3_LCD_PIN_NUM_RGB_DATA13          48
#define ESP32_S3_LCD_PIN_NUM_RGB_DATA14          47
#define ESP32_S3_LCD_PIN_NUM_RGB_DATA15          21
#endif
#define ESP32_S3_LCD_PIN_NUM_RST                 (-1)
#define ESP32_S3_LCD_PIN_NUM_BK_LIGHT            (-1)
#define ESP32_S3_LCD_BK_LIGHT_ON_LEVEL           (1)

#define ESP32_S3_LCD_BK_LIGHT_OFF_LEVEL !ESP32_S3_LCD_BK_LIGHT_ON_LEVEL

/* Enable or disable printing RGB refresh rate */
#define EXAMPLE_ENABLE_PRINT_LCD_FPS            (1)

#define _ESP32_S3_LCD_CLASS(name, ...) ESP_PanelLcd_##name(__VA_ARGS__)
#define ESP32_S3_LCD_CLASS(name, ...)  _ESP32_S3_LCD_CLASS(name, ##__VA_ARGS__)

#if EXAMPLE_ENABLE_PRINT_LCD_FPS
#define ESP32_S3_LCD_FPS_COUNT_MAX               (100)

DRAM_ATTR int frame_count = 0;
DRAM_ATTR int fps = 0;
DRAM_ATTR long start_time = 0;

IRAM_ATTR bool onVsyncEndCallback(void *user_data)
{
    long frame_start_time = *(long *)user_data;
    if (frame_start_time == 0) {
        (*(long *)user_data) = millis();

        return false;
    }

    frame_count++;
    if (frame_count >= ESP32_S3_LCD_FPS_COUNT_MAX) {
        fps = ESP32_S3_LCD_FPS_COUNT_MAX * 1000 / (millis() - frame_start_time);
        frame_count = 0;
        (*(long *)user_data) = millis();
    }

    return false;
}
#endif

void setup()
{
    Serial.begin(115200);
    Serial.println("RGB LCD example start");

#if ESP32_S3_LCD_PIN_NUM_BK_LIGHT >= 0
    Serial.println("Initialize backlight control pin and turn it off");
    ESP_PanelBacklight *backlight = new ESP_PanelBacklight(ESP32_S3_LCD_PIN_NUM_BK_LIGHT, ESP32_S3_LCD_BK_LIGHT_ON_LEVEL, true);
    backlight->begin();
    backlight->off();
#endif

    Serial.println("Create RGB LCD bus");
#if ESP32_S3_LCD_RGB_DATA_WIDTH == 8
    ESP_PanelBus_RGB *panel_bus = new ESP_PanelBus_RGB(ESP32_S3_LCD_WIDTH, ESP32_S3_LCD_HEIGHT,
                                                       ESP32_S3_LCD_PIN_NUM_RGB_DATA0, ESP32_S3_LCD_PIN_NUM_RGB_DATA1,
                                                       ESP32_S3_LCD_PIN_NUM_RGB_DATA2, ESP32_S3_LCD_PIN_NUM_RGB_DATA3,
                                                       ESP32_S3_LCD_PIN_NUM_RGB_DATA4, ESP32_S3_LCD_PIN_NUM_RGB_DATA5,
                                                       ESP32_S3_LCD_PIN_NUM_RGB_DATA6, ESP32_S3_LCD_PIN_NUM_RGB_DATA7,
                                                       ESP32_S3_LCD_PIN_NUM_RGB_HSYNC, ESP32_S3_LCD_PIN_NUM_RGB_VSYNC,
                                                       ESP32_S3_LCD_PIN_NUM_RGB_PCLK, ESP32_S3_LCD_PIN_NUM_RGB_DE,
                                                       ESP32_S3_LCD_PIN_NUM_RGB_DISP);
#elif ESP32_S3_LCD_RGB_DATA_WIDTH == 16
    ESP_PanelBus_RGB *panel_bus = new ESP_PanelBus_RGB(ESP32_S3_LCD_WIDTH, ESP32_S3_LCD_HEIGHT,
                                                       ESP32_S3_LCD_PIN_NUM_RGB_DATA0, ESP32_S3_LCD_PIN_NUM_RGB_DATA1,
                                                       ESP32_S3_LCD_PIN_NUM_RGB_DATA2, ESP32_S3_LCD_PIN_NUM_RGB_DATA3,
                                                       ESP32_S3_LCD_PIN_NUM_RGB_DATA4, ESP32_S3_LCD_PIN_NUM_RGB_DATA5,
                                                       ESP32_S3_LCD_PIN_NUM_RGB_DATA6, ESP32_S3_LCD_PIN_NUM_RGB_DATA7,
                                                       ESP32_S3_LCD_PIN_NUM_RGB_DATA8, ESP32_S3_LCD_PIN_NUM_RGB_DATA9,
                                                       ESP32_S3_LCD_PIN_NUM_RGB_DATA10, ESP32_S3_LCD_PIN_NUM_RGB_DATA11,
                                                       ESP32_S3_LCD_PIN_NUM_RGB_DATA12, ESP32_S3_LCD_PIN_NUM_RGB_DATA13,
                                                       ESP32_S3_LCD_PIN_NUM_RGB_DATA14, ESP32_S3_LCD_PIN_NUM_RGB_DATA15,
                                                       ESP32_S3_LCD_PIN_NUM_RGB_HSYNC, ESP32_S3_LCD_PIN_NUM_RGB_VSYNC,
                                                       ESP32_S3_LCD_PIN_NUM_RGB_PCLK, ESP32_S3_LCD_PIN_NUM_RGB_DE,
                                                       ESP32_S3_LCD_PIN_NUM_RGB_DISP);
#endif
    panel_bus->configRgbTimingFreqHz(ESP32_S3_LCD_RGB_TIMING_FREQ_HZ);
    panel_bus->configRgbTimingPorch(ESP32_S3_LCD_RGB_TIMING_HPW, ESP32_S3_LCD_RGB_TIMING_HBP, ESP32_S3_LCD_RGB_TIMING_HFP,
                                    ESP32_S3_LCD_RGB_TIMING_VPW, ESP32_S3_LCD_RGB_TIMING_VBP, ESP32_S3_LCD_RGB_TIMING_VFP);
    // panel_bus->configRgbBounceBufferSize(ESP32_S3_LCD_WIDTH * 10); // Set bounce buffer to avoid screen drift
    panel_bus->begin();

    Serial.println("Create LCD device");
    ESP_PanelLcd *lcd = new ESP32_S3_LCD_CLASS(ESP32_S3_LCD_NAME, panel_bus, ESP32_S3_LCD_COLOR_BITS, ESP32_S3_LCD_PIN_NUM_RST);
    lcd->init();
    lcd->reset();
    lcd->begin();
#if ESP32_S3_LCD_PIN_NUM_RGB_DISP >= 0
    lcd->displayOn();
#endif
#if EXAMPLE_ENABLE_PRINT_LCD_FPS
    lcd->attachRefreshFinishCallback(onVsyncEndCallback, (void *)&start_time);
#endif

    Serial.println("Draw color bar from top left to bottom right, the order is B - G - R");
    lcd->colorBarTest(ESP32_S3_LCD_WIDTH, ESP32_S3_LCD_HEIGHT);

#if ESP32_S3_LCD_PIN_NUM_BK_LIGHT >= 0
    Serial.println("Turn on the backlight");
    backlight->on();
#endif

    Serial.println("RGB LCD example end");
}

void loop()
{
    delay(1000);
#if EXAMPLE_ENABLE_PRINT_LCD_FPS
    Serial.println("RGB refresh rate: " + String(fps));
#else
    Serial.println("IDLE loop");
#endif
}
