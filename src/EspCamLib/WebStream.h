#ifndef ESPCAMLIB_WEBSTREAM_H
#define ESPCAMLIB_WEBSTREAM_H
#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"
#include "esp_http_server.h"

#include "Camera.h"

// stream only HTTP server
namespace EspCam
{
    class WebStream
    {
    private:
        int m_port;
        Camera* m_camera;
        httpd_handle_t server = NULL;

        static esp_err_t streamHandler(httpd_req_t *req) {
            WebStream* instance = static_cast<WebStream*>(req->user_ctx);
            if (!instance) {
                return ESP_FAIL;
            }

            httpd_resp_set_type(req, "multipart/x-mixed-replace; boundary=frame");
            httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

            while (true) {
                camera_fb_t* pic = instance->m_camera->getFrame();
                if (!pic) {
                    return ESP_FAIL;
                }

                String partHeader = "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: " + String(pic->len) + "\r\n\r\n";
                httpd_resp_send_chunk(req, partHeader.c_str(), partHeader.length());

                httpd_resp_send_chunk(req, (const char *)pic->buf, pic->len);
                httpd_resp_send_chunk(req, "\r\n", 2);

                instance->m_camera->releaseFrame(pic);
            }

            return ESP_OK;
        }
    public:
        WebStream(Camera* camera, int port = 80) : m_camera(camera), m_port(port) { }

        bool begin(int port = 80) {
            m_port = port;
            pixformat_t format = m_camera->getPixelFormat();
            if (format != PIXFORMAT_JPEG) {
                Serial.println("BasicWebServer ERROR: Camera pixel format must be set to JPEG.");
                return false;
            }

            httpd_config_t config = HTTPD_DEFAULT_CONFIG();
            config.server_port = m_port;

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