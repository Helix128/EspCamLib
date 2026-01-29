#ifndef ESPCAMLIB_CAMERA_H
#define ESPCAMLIB_CAMERA_H
#include <Arduino.h>
#include "esp_camera.h"
#include "./BoardDefs.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

namespace EspCam
{
    class Camera
    {

    private:
        BoardDef boardDef;
        camera_config_t config;
        int flashPin;

    public:
        Camera()
        {
            config.xclk_freq_hz = 20000000;
            config.ledc_timer = LEDC_TIMER_0;
            config.ledc_channel = LEDC_CHANNEL_0;
            config.pixel_format = PIXFORMAT_JPEG;
            config.frame_size = FRAMESIZE_VGA;
            config.jpeg_quality = 12;
            config.fb_count = 2;
            config.fb_location = CAMERA_FB_IN_PSRAM;
            config.grab_mode = CAMERA_GRAB_LATEST;
            setBrownout(false);
        }

        Camera(void (*boardDefFunc)(BoardDef *)) : Camera()
        {
            setPinout(boardDefFunc);
        }

        bool begin()
        {
            if (config.pin_pwdn != -1)
            {
                digitalWrite(config.pin_pwdn, LOW);
                delay(10);
            }
            esp_err_t err = esp_camera_init(&config);
            return err == ESP_OK;
        }

        void setPinout(void (*boardDefFunc)(BoardDef *))
        {
            boardDefFunc(&boardDef);
            config.pin_pwdn = boardDef.pwdn;
            config.pin_reset = boardDef.reset;
            config.pin_xclk = boardDef.xclk;
            config.pin_sccb_sda = boardDef.siod;
            config.pin_sccb_scl = boardDef.sioc;
            config.pin_d7 = boardDef.d7;
            config.pin_d6 = boardDef.d6;
            config.pin_d5 = boardDef.d5;
            config.pin_d4 = boardDef.d4;
            config.pin_d3 = boardDef.d3;
            config.pin_d2 = boardDef.d2;
            config.pin_d1 = boardDef.d1;
            config.pin_d0 = boardDef.d0;
            config.pin_vsync = boardDef.vsync;
            config.pin_href = boardDef.href;
            config.pin_pclk = boardDef.pclk;
            flashPin = boardDef.flash;
            pinMode(flashPin, OUTPUT);
            ledcSetup(LEDC_CHANNEL_1, 5000, 8);
            ledcAttachPin(flashPin, LEDC_CHANNEL_1);
        }

        void setBrownout(bool enable)
        {
            if (enable)
            {
                WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
            }
            else
            {
                WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, RTC_CNTL_BROWN_OUT_ENA);
            }
        }

        void setFrameSize(framesize_t frameSize)
        {
            config.frame_size = frameSize;
            sensor_t *s = esp_camera_sensor_get();
            if (s) {
                s->set_framesize(s, frameSize);
            }
        }

        void setPixelFormat(pixformat_t format)
        {
            config.pixel_format = format;
            sensor_t *s = esp_camera_sensor_get();
            if (s) {
                s->set_pixformat(s, format);
            }
        }

        void setJpegQuality(int quality)
        {
            config.jpeg_quality = quality;
            sensor_t *s = esp_camera_sensor_get();
            if (s) {
                s->set_quality(s, quality);
            }
        }

        void setFramebufferCount(size_t count)
        {
            config.fb_count = count;
        }

        void setFlash(int brightness)
        {
            if (flashPin == -1)
            {
                return;
            }
            ledcWrite(LEDC_CHANNEL_1, brightness);
        }

        bool getVFlip()
        {
            sensor_t *s = esp_camera_sensor_get();
            if (s)
            {
                return s->status.vflip;
            }
            return false;
        }

        void setVFlip(bool vflip)
        {
            sensor_t *s = esp_camera_sensor_get();
            if (s)
            {
                s->set_vflip(s, vflip ? 1 : 0);
            }
        }

        bool getHFlip()
        {
            sensor_t *s = esp_camera_sensor_get();
            if (s)
            {
                return s->status.hmirror;
            }
            return false;
        }

        void setHFlip(bool hmirror)
        {
            sensor_t *s = esp_camera_sensor_get();
            if (s)
            {
                s->set_hmirror(s, hmirror ? 1 : 0);
            }
        }

        camera_fb_t *getFrame()
        {
            return esp_camera_fb_get();
        }

        void releaseFrame(camera_fb_t *fb)
        {   
            if(!fb) return;
            esp_camera_fb_return(fb);
        }

        pixformat_t getPixelFormat()
        {
            return config.pixel_format;
        }
    };
}
#endif