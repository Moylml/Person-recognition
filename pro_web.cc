#include <stdio.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

const char *TAG = "info";

#include "camera.c"
//#include "app_camera_esp.c"
#include "wifi.c"
#include "web.c" 
#include "object.cc" 


extern "C" void app_main() {
  nvs_flash_init();
  tflite_init();
  vTaskDelay(5000 / portTICK_PERIOD_MS); 
  wifi_main();
  vTaskDelay(5000 / portTICK_PERIOD_MS); 
  //camera_main();
  app_camera_init();
  vTaskDelay(5000 / portTICK_PERIOD_MS); 
  web_main();  
}
