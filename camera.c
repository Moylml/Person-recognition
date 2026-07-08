#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_camera.h"

#include "sensor.h"
#include "esp_camera.h"
#include "esp_log.h"
#include "esp_system.h"

#define CAMERA_MODULE_NAME "ESP-S3-EYE"
#define CAMERA_PIN_PWDN -1
#define CAMERA_PIN_RESET -1

#define CAMERA_PIN_VSYNC 6
#define CAMERA_PIN_HREF 7
#define CAMERA_PIN_PCLK 13
#define CAMERA_PIN_XCLK 15

#define CAMERA_PIN_SIOD 4
#define CAMERA_PIN_SIOC 5

#define CAMERA_PIN_D0 11
#define CAMERA_PIN_D1 9
#define CAMERA_PIN_D2 8
#define CAMERA_PIN_D3 10
#define CAMERA_PIN_D4 12
#define CAMERA_PIN_D5 18
#define CAMERA_PIN_D6 17
#define CAMERA_PIN_D7 16
#define XCLK_FREQ_HZ 15000000
#define CAMERA_FRAME_SIZE FRAMESIZE_96X96
#define CAMERA_PIXEL_FORMAT PIXFORMAT_GRAYSCALE



//#define BOARD_WROVER_KIT 1
//#define CAMERA_PIN_PWDN 38

static esp_err_t init_camera(void);
void camera_capture_frame(void);


static esp_err_t init_camera(void)
{

     static camera_config_t camera_config = {
          .pin_pwdn = CAMERA_PIN_PWDN,
          .pin_reset = CAMERA_PIN_RESET,
          .pin_xclk = CAMERA_PIN_XCLK,
          .pin_sccb_sda = CAMERA_PIN_SIOD,
          .pin_sccb_scl = CAMERA_PIN_SIOC,

          .pin_d7 = CAMERA_PIN_D7,
          .pin_d6 = CAMERA_PIN_D6,
          .pin_d5 = CAMERA_PIN_D5,
          .pin_d4 = CAMERA_PIN_D4,
          .pin_d3 = CAMERA_PIN_D3,
          .pin_d2 = CAMERA_PIN_D2,
          .pin_d1 = CAMERA_PIN_D1,
          .pin_d0 = CAMERA_PIN_D0,
          .pin_vsync = CAMERA_PIN_VSYNC,
          .pin_href = CAMERA_PIN_HREF,
          .pin_pclk = CAMERA_PIN_PCLK,

          .xclk_freq_hz = 5000000, //Muy lento 20, Bien 5, No funciona 1 
          .ledc_timer = LEDC_TIMER_0,
          .ledc_channel = LEDC_CHANNEL_0,

          .pixel_format = CAMERA_PIXEL_FORMAT,
          .frame_size = CAMERA_FRAME_SIZE,
          //.frame_size = FRAMESIZE_96X96,

          .jpeg_quality = 10,
          .fb_count = 2,
          .fb_location = CAMERA_FB_IN_PSRAM,
          .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
    };
      sensor_t *s = esp_camera_sensor_get();
  //  s->set_vflip(s, 1); //flip it back 
    esp_err_t err = esp_camera_init(&camera_config);

    return ESP_OK;
}


void camera_capture_frame () {
  camera_fb_t *pic ;
  pic =  esp_camera_fb_get();

  esp_camera_fb_return(pic);
}
void camera_main()
{
    if (  init_camera() != ESP_OK ) {
        return;
    }
    camera_capture_frame ();

}

int app_camera_init() { 

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = CAMERA_PIN_D0;
  config.pin_d1 = CAMERA_PIN_D1;
  config.pin_d2 = CAMERA_PIN_D2;
  config.pin_d3 = CAMERA_PIN_D3;
  config.pin_d4 = CAMERA_PIN_D4;
  config.pin_d5 = CAMERA_PIN_D5;
  config.pin_d6 = CAMERA_PIN_D6;
  config.pin_d7 = CAMERA_PIN_D7;
  config.pin_xclk = CAMERA_PIN_XCLK;
  config.pin_pclk = CAMERA_PIN_PCLK;
  config.pin_vsync = CAMERA_PIN_VSYNC;
  config.pin_href = CAMERA_PIN_HREF;
  config.pin_sscb_sda = CAMERA_PIN_SIOD;
  config.pin_sscb_scl = CAMERA_PIN_SIOC;
  config.pin_pwdn = CAMERA_PIN_PWDN;
  config.pin_reset = CAMERA_PIN_RESET;
  config.xclk_freq_hz = XCLK_FREQ_HZ;
  config.jpeg_quality = 10;
  config.fb_count = 2;
  config.fb_location = CAMERA_FB_IN_PSRAM; 

  // Pixel format and frame size are specific configurations options for this application.
  // Frame size must be 96x96 pixels to match the trained model.
  // Pixel format defaults to grayscale to match the trained model.
  // With display support enabled, the pixel format is RGB565 to match the display. The frame is converted to grayscale before it is passed to the trained model.
  config.pixel_format = CAMERA_PIXEL_FORMAT;
  config.frame_size = CAMERA_FRAME_SIZE;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
    return -1;
  }
  sensor_t *s = esp_camera_sensor_get();
  s->set_vflip(s, 1); //flip it back
  //initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID)
  {
      s->set_brightness(s, 1);  //up the blightness just a bit
      s->set_saturation(s, -2); //lower the saturation
  }
  return 0; 
}
