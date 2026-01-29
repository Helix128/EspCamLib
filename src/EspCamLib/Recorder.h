#ifndef ESPCAMLIB_RECORDER_H
#define ESPCAMLIB_RECORDER_H

#include <Arduino.h>
#include "Camera.h"
#include <SPI.h>
#include <SD.h>
#include "FS.h"

namespace EspCam
{
    class Recorder
    {
    private:
        Camera *m_camera;
        const char *m_filename;
        int m_frameRate = 30;
        TaskHandle_t m_recordHandle;
        TaskHandle_t m_writeHandle;
        QueueHandle_t m_fbQueue;
        volatile bool m_isRecording;

        static void recordTask(void *param)
        {
            Recorder *self = static_cast<Recorder *>(param);
            const TickType_t frameDelay = pdMS_TO_TICKS(1000 / self->m_frameRate);
            TickType_t lastFrameTime = xTaskGetTickCount();

            while (self->m_isRecording)
            {
                camera_fb_t *fb = self->m_camera->getFrame();

                if (!fb)
                {
                    vTaskDelay(1);
                    continue;
                }

                if (xQueueSend(self->m_fbQueue, &fb, 0) != pdTRUE)
                {
                    self->m_camera->releaseFrame(fb);
                }

                vTaskDelayUntil(&lastFrameTime, frameDelay);
            }

            vTaskDelete(NULL);
        }

        static void writeTask(void *param)
        {
            Recorder *self = static_cast<Recorder *>(param);

            File videoFile = SD.open(self->m_filename, FILE_WRITE);

            if (!videoFile)
            {
                self->m_isRecording = false;
                vTaskDelete(NULL);
                return;
            }

            camera_fb_t *fb = NULL;

            while (self->m_isRecording || uxQueueMessagesWaiting(self->m_fbQueue) > 0)
            {
                if (xQueueReceive(self->m_fbQueue, &fb, pdMS_TO_TICKS(1000)) == pdTRUE)
                {
                    if (fb)
                    {
                        size_t written = videoFile.write(fb->buf, fb->len);
                        self->m_camera->releaseFrame(fb);
                    }
                }
            }

            videoFile.close();
            self->m_writeHandle = NULL;
            vTaskDelete(NULL);
        }

    public:
        Recorder(Camera *camera, int fps = 30) : m_camera(camera), m_recordHandle(NULL), m_writeHandle(NULL), m_fbQueue(NULL), m_isRecording(false), m_frameRate(fps) {}

        ~Recorder()
        {
            stop();
        }

        void setTargetFPS(int fps)
        {
            m_frameRate = fps;
        }

        bool start(const char *filename)
        {
            if (m_isRecording)
                return false;

            if (SD.cardType() == CARD_NONE)
            {
                return false;
            }

            m_filename = filename;
            m_isRecording = true;

            m_fbQueue = xQueueCreate(2, sizeof(camera_fb_t *));

            xTaskCreatePinnedToCore(recordTask, "RecTask", 3072, this, 10, &m_recordHandle, 1);
            xTaskCreatePinnedToCore(writeTask, "WriteTask", 4096, this, 15, &m_writeHandle, 0);

            return true;
        }

        void stop()
        {
            if (!m_isRecording)
                return;

            m_isRecording = false;

            unsigned long startWait = millis();
            while (m_writeHandle != NULL && millis() - startWait < 6000)
            {
                vTaskDelay(10);
            }

            if (m_fbQueue)
            {
                camera_fb_t *fb = NULL;
                while (xQueueReceive(m_fbQueue, &fb, 0) == pdTRUE)
                {
                    if (fb)
                        m_camera->releaseFrame(fb);
                }
                vQueueDelete(m_fbQueue);
                m_fbQueue = NULL;
            }

            m_recordHandle = NULL;
        }
    };
};
#endif