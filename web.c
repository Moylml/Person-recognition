#include <esp_http_server.h>  

#define PART_BOUNDARY "123456789000000000000987654321"

static const char* STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t stream_httpd = NULL;

int object_detect(uint8_t * jpeg_data, int jpeg_data_len);
int pixels2jpg(uint8_t **jbuf, size_t *jbuf_len);

httpd_handle_t start_webserver() {

       httpd_handle_t  server ;

       httpd_config_t config = HTTPD_DEFAULT_CONFIG();
       httpd_start(&server, &config);

     return server;
}



esp_err_t uri_handler_stream(httpd_req_t *req){
     camera_fb_t * fb = NULL;
     esp_err_t res = ESP_OK;
     size_t _jpg_buf_len = 0;
     uint8_t * _jpg_buf = NULL;
     char * part_buf[64];

     res = httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
     if(res != ESP_OK){
       return res;
     }

     while(true){
       fb = esp_camera_fb_get();
       if (!fb) {
         printf("Camera capture failed");
         res = ESP_FAIL;
       } else {
         if(fb->width > 90){
           object_detect(fb->buf, fb->len);
           if(fb->format != PIXFORMAT_JPEG){ 
            
             bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
             esp_camera_fb_return(fb);
             fb = NULL;
             if(!jpeg_converted){
               printf("JPEG compression failed");
               res = ESP_FAIL;
             }
           } else {
             _jpg_buf_len = fb->len;
             _jpg_buf = fb->buf;
           }
           //object_detect(_jpg_buf,_jpg_buf_len);
          //pixels2jpg(&_jpg_buf, &_jpg_buf_len);

         }
       }
       if(res == ESP_OK){
         size_t hlen = snprintf((char *)part_buf, 64, STREAM_PART, _jpg_buf_len);
         res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
       }
       if(res == ESP_OK){
         res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
       }
       if(res == ESP_OK){
         res = httpd_resp_send_chunk(req, STREAM_BOUNDARY, strlen(STREAM_BOUNDARY));
       }
       if(fb){
         esp_camera_fb_return(fb);
         fb = NULL;
         _jpg_buf = NULL;
       } else if(_jpg_buf){
         free(_jpg_buf);
         _jpg_buf = NULL;
       }

       if( _jpg_buf ) free(_jpg_buf);

       if(res != ESP_OK){
         break;
       } 

       //Serial.printf("MJPG: %uB\n",(uint32_t)(_jpg_buf_len));
     }
     return res;
}


esp_err_t uri_handler(httpd_req_t *req)
{ 
    char str[1024];

    strcpy(str, "ESP Device running");

    httpd_resp_send(req, str, strlen(str));
    return ESP_OK;
}



httpd_uri_t get_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = uri_handler_stream,
    .user_ctx  = NULL
};



void web_main(void){

       httpd_handle_t   webserver =   start_webserver();

       httpd_register_uri_handler(webserver, &get_uri);

}