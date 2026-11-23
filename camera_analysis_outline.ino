/**
 * @file      MinimalCameraExample.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2022  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2022-09-16
 *
 */
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include "esp_camera.h"
#include <esp_mac.h>

#if __has_include("secrets.h")
#include "secrets.h"
#endif
#ifndef WIFI_SSID1
#define WIFI_SSID1 ""
#define WIFI_SSID_PASSWORD1 ""
#define WIFI_SSID2 ""
#define WIFI_SSID_PASSWORD2 ""
#define WIFI_SSID3 ""
#define WIFI_SSID_PASSWORD3 ""
#endif

#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
#include "utilities.h"
#include "OLED_display.h"
#include "rotary_encoder_control.h"
#include "camera_sample_api.h"

void        startCameraServer();

XPowersPMU  PMU;
WiFiMulti   wifiMulti;
String      hostName = "LilyGo-Cam-";
String      ipAddress = "";
bool        use_ap_mode = USING_AP_MODE;



void setup()
{

    Serial.begin(115200);

    // Allow startup even when no Serial Monitor is connected.
    uint32_t serialStart = millis();
    while (!Serial && millis() - serialStart < 2000) {
        delay(10);
    }

    delay(3000);

    Serial.println();

    /*********************************
     *  step 1 : Initialize power chip,
     *  turn on camera power channel
    ***********************************/
    if (!PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, BOARD_I2C_SDA, BOARD_I2C_SCL)) {
        Serial.println("Failed to initialize power.....");
        while (1) {
            delay(5000);
        }
    }
    //Set the working voltage of the camera, please do not modify the parameters
    PMU.setALDO1Voltage(1800);  // CAM DVDD  1500~1800
    PMU.enableALDO1();
    PMU.setALDO2Voltage(2800);  // CAM DVDD 2500~2800
    PMU.enableALDO2();
    PMU.setALDO4Voltage(3000);  // CAM AVDD 2800~3000
    PMU.enableALDO4();

    // TS Pin detection must be disable, otherwise it cannot be charged
    PMU.disableTSPinMeasure();

    // Reuse the existing I2C bus; OLED failure must not stop camera startup.
    if (!OLED_display::begin()) {
        Serial.println("OLED not detected at I2C address 0x3C; camera will continue.");
    }


    /*********************************
     * step 2 : start network
     * Station mode joins a network configured in secrets.h.
    ***********************************/
    if (use_ap_mode) {

        uint8_t apMac[6];
        esp_read_mac(apMac, ESP_MAC_WIFI_SOFTAP);
        char macSuffix[5];
        snprintf(macSuffix, sizeof(macSuffix), "%02X%02X", apMac[4], apMac[5]);
        hostName += macSuffix;
        WiFi.mode(WIFI_AP);
        WiFi.softAP(hostName.c_str());
        ipAddress = WiFi.softAPIP().toString();
        Serial.print("Started AP mode host name :");
        Serial.println(hostName);
        Serial.print("IP address is :");
        Serial.println(WiFi.softAPIP().toString());

    } else {
        if (WIFI_SSID1[0] == '\0' && WIFI_SSID2[0] == '\0' && WIFI_SSID3[0] == '\0') {
            Serial.println("Set a Wi-Fi SSID and password in secrets.h, then upload again.");
            return;
        }
        WiFi.mode(WIFI_STA);
        if (WIFI_SSID1[0]) wifiMulti.addAP(WIFI_SSID1, WIFI_SSID_PASSWORD1);
        if (WIFI_SSID2[0]) wifiMulti.addAP(WIFI_SSID2, WIFI_SSID_PASSWORD2);
        if (WIFI_SSID3[0]) wifiMulti.addAP(WIFI_SSID3, WIFI_SSID_PASSWORD3);
        
        Serial.println("Connecting Wifi...");
        while (wifiMulti.run(5000) != WL_CONNECTED) {
            Serial.println("Waiting for Wi-Fi. Check credentials and 2.4 GHz network availability.");
            delay(1000);
        }
        ipAddress = WiFi.localIP().toString();
        Serial.println("WiFi connected");
    }



    /*********************************
     *  step 3 : Initialize camera
    ***********************************/
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.frame_size = FRAMESIZE_UXGA;
    config.pixel_format = PIXFORMAT_JPEG; // for streaming
    //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
    //                      for larger pre-allocated frame buffer.
    if (config.pixel_format == PIXFORMAT_JPEG) {
        if (psramFound()) {
            config.jpeg_quality = 10;
            config.fb_count = 2;
            config.grab_mode = CAMERA_GRAB_LATEST;
        } else {
            // Limit the frame size when PSRAM is not available
            config.frame_size = FRAMESIZE_SVGA;
            config.fb_location = CAMERA_FB_IN_DRAM;
        }

    } else {
        // Best option for face detection/recognition
        config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
        config.fb_count = 2;
#endif
    }

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x Please check if the camera is connected well.", err);
        while (1) {
            delay(5000);
        }
    }

    sensor_t *s = esp_camera_sensor_get();
    // initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1); // flip it back
        s->set_brightness(s, 1); // up the brightness just a bit
        s->set_saturation(s, -2); // lower the saturation
    }
    // drop down frame size for higher initial frame rate
    if (config.pixel_format == PIXFORMAT_JPEG) {
        s->set_framesize(s, FRAMESIZE_QVGA);
    }

#if defined(LILYGO_ESP32S3_CAM_PIR_VOICE)
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
#endif



    /*********************************
     *  step 4 : start camera web server
    ***********************************/
    startCameraServer();
    Serial.printf("Camera ready: http://%s/\n", ipAddress.c_str());


    /*********************************
     *  step 5 : set up rotary encoder
    ***********************************/
    if (rotary_encoder_control::begin()) {
        Serial.println("Encoder ready. Rotate to select OLED square 1-4; click remains a test event.");
    } else {
        Serial.println("Encoder not detected; camera and OLED will continue. Check wiring and restart.");
    }
}

void loop()
{
    // Rotation selects a held OLED result. Click sampling and LEDs come later.
    switch (rotary_encoder_control::pollEvent()) {
        case EncoderEvent::Previous: camera_sample_api::selectRelative(-1); break;
        case EncoderEvent::Next: camera_sample_api::selectRelative(1); break;
        case EncoderEvent::SamplePressed: Serial.println("Encoder: SamplePressed (test only)"); break;
        case EncoderEvent::None: break;
    }

    // Keep the existing reconnect behavior, checking once per second rather
    // than on every 10ms input poll. A Wi-Fi reconnect can still briefly block.
    static uint32_t lastNetworkCheck = 0;
    if (millis() - lastNetworkCheck >= 1000) {
        lastNetworkCheck = millis();
        if (!use_ap_mode && (WIFI_SSID1[0] || WIFI_SSID2[0] || WIFI_SSID3[0])) {
            if (wifiMulti.run(5000) == WL_CONNECTED) {
                String currentIp = WiFi.localIP().toString();
                if (currentIp != ipAddress) {
                    ipAddress = currentIp;
                    Serial.printf("Camera address: http://%s/\n", ipAddress.c_str());
                }
            }
        }
    }
    delay(10);
}