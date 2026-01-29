#ifndef ESPCAMLIB_WEBSERVER_H
#define ESPCAMLIB_WEBSERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"
#include "esp_http_server.h"

#include "Camera.h"
#include "WebServer/Index.h"

// http server with user interactivity and a separate stream endpoint
namespace EspCam
{   
    class WebServer
    {
    private:
        Camera m_camera;
        httpd_handle_t camera_httpd = NULL;
        httpd_handle_t stream_httpd = NULL;
        volatile size_t m_bytes_per_sec = 0;

        static esp_err_t indexHandler(httpd_req_t *req) {
            httpd_resp_set_type(req, "text/html");
            return httpd_resp_send(req, INDEX_HTML, INDEX_HTML_LENGTH);
        }

        static esp_err_t statusHandler(httpd_req_t *req) {
            WebServer* instance = static_cast<WebServer*>(req->user_ctx);
            if (!instance) return ESP_FAIL;

            float kbps = (instance->m_bytes_per_sec * 8.0) / 1024.0;
            instance->m_bytes_per_sec = 0; 

            String json = "{";
            json += "\"heap\":" + String(ESP.getFreeHeap()) + ",";
            json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
            json += "\"kbps\":" + String(kbps, 1) + ",";
            json += "\"ip\":\"" + WiFi.localIP().toString() + "\""; 
            json += "}";

            httpd_resp_set_type(req, "application/json");
            httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
            return httpd_resp_send(req, json.c_str(), json.length());
        }

        static esp_err_t controlHandler(httpd_req_t *req) {
            WebServer* instance = static_cast<WebServer*>(req->user_ctx);
            
            char buf[128];
            size_t buf_len = httpd_req_get_url_query_len(req) + 1;
            
            if (buf_len > 1 && buf_len < 128) {
                if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
                    char var[32] = {0,};
                    char val[32] = {0,};

                    if (httpd_query_key_value(buf, "var", var, sizeof(var)) == ESP_OK &&
                        httpd_query_key_value(buf, "val", val, sizeof(val)) == ESP_OK) {
                        
                        String cmd = String(var);
                        int value = atoi(val);
                    
                        if (cmd == "flash") {
                            instance->m_camera.setFlash(value);
                        }
                        else if (cmd == "vflip") {
                            bool isFlipped = instance->m_camera.getVFlip();
                            instance->m_camera.setVFlip(!isFlipped);
                        }
                        else if (cmd == "hmirror") {
                            bool isMirrored = instance->m_camera.getHFlip();
                            instance->m_camera.setHFlip(!isMirrored);
                        }
                        else if (cmd == "framesize") {
                            instance->m_camera.setFrameSize((framesize_t)value);
                        }
                        else if (cmd == "quality") {
                            instance->m_camera.setJPEGQuality(value);
                        }
                        else if (cmd == "reboot") {
                            ESP.restart();
                        }
                    }
                }
            }
            
            httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
            return httpd_resp_send(req, "OK", 2);
        }

        static esp_err_t streamHandler(httpd_req_t *req) {
            WebServer* instance = static_cast<WebServer*>(req->user_ctx);
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
                esp_err_t res = httpd_resp_send_chunk(req, partHeader.c_str(), partHeader.length());
                
                if (res == ESP_OK) {
                    res = httpd_resp_send_chunk(req, pic.getBuffer(), pic.getLength());
                }
                
                if (res == ESP_OK) {
                    httpd_resp_send_chunk(req, "\r\n", 2);
                }

                if (res == ESP_OK) {
                    instance->m_bytes_per_sec += pic.getLength();
                }

                pic.dispose();
                
                if (res != ESP_OK) {
                    break;
                }
            }

            return ESP_OK;
        }

    public:
        WebServer(Camera camera) : m_camera(camera) { }

        bool begin() {
            pixformat_t format = m_camera.getPixelFormat();
            if (format != PIXFORMAT_JPEG) {
                return false;
            }

            httpd_config_t config = HTTPD_DEFAULT_CONFIG();
            config.server_port = 80;
            config.ctrl_port = 80;

            httpd_uri_t indexUri = {
                .uri       = "/",
                .method    = HTTP_GET,
                .handler   = indexHandler,
                .user_ctx  = NULL
            };

            httpd_uri_t statusUri = {
                .uri       = "/status",
                .method    = HTTP_GET,
                .handler   = statusHandler,
                .user_ctx  = this
            };

            httpd_uri_t controlUri = {
                .uri       = "/control",
                .method    = HTTP_GET,
                .handler   = controlHandler,
                .user_ctx  = this
            };

            if (httpd_start(&camera_httpd, &config) == ESP_OK) {
                httpd_register_uri_handler(camera_httpd, &indexUri);
                httpd_register_uri_handler(camera_httpd, &statusUri);
                httpd_register_uri_handler(camera_httpd, &controlUri);
            }

            config.server_port = 81;
            config.ctrl_port = 81;

            httpd_uri_t streamUri = {
                .uri       = "/stream",
                .method    = HTTP_GET,
                .handler   = streamHandler,
                .user_ctx  = this
            };

            if (httpd_start(&stream_httpd, &config) == ESP_OK) {
                httpd_register_uri_handler(stream_httpd, &streamUri);
            }

            return (camera_httpd != NULL && stream_httpd != NULL);
        }

        ~WebServer() {
            if (camera_httpd) {
                httpd_stop(camera_httpd);
            }
            if (stream_httpd) {
                httpd_stop(stream_httpd);
            }
        }
    };
};
#endif