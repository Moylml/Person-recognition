# Person-recognition ESP32-S3

This project is mean to run on an ESP32-S3. Streams the camera feed to a local webpage and runs a neural network to detect whether there are people in the image.

You can create your own project with  ```idf.py create-project <project name> ```. If using a diferent name you need to change ```SRC``` in the CMake file to ```SRCS "pro_web.cc"```. Also you need to change the ```wifi.c``` file in order to add your network.

And then select the target  ```idf.py set-target <target>```

You need to make sure the following components are included in  ```idf_component.yml ```

-   ```espressif/esp_jpeg: '*' ```
-   ```espressif/esp-tflite-micro: '*' ```
-   ```espressif/esp32-camera: ~2.0.5 ```

You can add them with ```idf.py add-dependency <dependency name>```

If everthing is ok you can build and flash your project with ```idf.py flash```
