#ifndef BASICWEBSERVER_H
#define BASICWEBSERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"
#include "esp_http_server.h"

#include "Camera.h"

namespace EspCam
{
    class WebStream
    {
    private:
        Camera m_camera;
        httpd_handle_t server = NULL;

        static esp_err_t streamHandler(httpd_req_t *req) {
            WebStream* instance = static_cast<WebStream*>(req->user_ctx);
            if (!instance) {
                return ESP_FAIL;
            }

            httpd_resp_set_type(req, "multipart/x-mixed-replace; boundary=frame");
            httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

            while (true) {
                CaptureData pic = instance->m_camera.capture();
                if (!pic.success()) {
                    return ESP_FAIL;
                }

                String partHeader = "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: " + String(pic.getLength()) + "\r\n\r\n";
                httpd_resp_send_chunk(req, partHeader.c_str(), partHeader.length());

                httpd_resp_send_chunk(req, pic.getBuffer(), pic.getLength());
                httpd_resp_send_chunk(req, "\r\n", 2);

                pic.dispose();
            }

            return ESP_OK;
        }
    public:
        WebStream(Camera camera) : m_camera(camera) { }

        bool begin(int port = 80) {
            pixformat_t format = m_camera.getPixelFormat();
            if (format != PIXFORMAT_JPEG) {
                Serial.println("BasicWebServer ERROR: Camera pixel format must be set to JPEG.");
                return false;
            }

            httpd_config_t config = HTTPD_DEFAULT_CONFIG();
            config.server_port = port;

            httpd_uri_t streamUri = {
                .uri       = "/",
                .method    = HTTP_GET,
                .handler   = streamHandler,
                .user_ctx  = this
            };

            if (httpd_start(&server, &config) == ESP_OK) {
                httpd_register_uri_handler(server, &streamUri);
            }
            return server != NULL;
        }

        ~WebStream() {
            if (server) {
                httpd_stop(server);
            }
        }
    };
};

#endif