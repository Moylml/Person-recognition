#include "person_detect_model_data.cc"

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;

//constexpr int scratchBufSize = 60 * 1024;
constexpr int scratchBufSize = 0;


// en lugar de 80  habia 100
constexpr int kTensorArenaSize = 130 * 1024 + scratchBufSize;
static uint8_t *tensor_arena;//[kTensorArenaSize]; // Maybe we should move this to external

//  de MODEL SETTINGS h
constexpr int kPersonIndex = 1;
constexpr int kNotAPersonIndex = 0;
constexpr int kNumCols = 96;
constexpr int kNumRows = 96;
constexpr int kNumChannels = 1; 
constexpr int kMaxImageSize = kNumCols * kNumRows * kNumChannels;
constexpr int NumRows = 240;
constexpr int NumCols = 320;

int tflite_init() {

 model = tflite::GetModel(g_person_detect_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    MicroPrintf("Model provided is schema version %d not equal to supported "
                "version %d.", model->version(), TFLITE_SCHEMA_VERSION);
    return -1;
  }

  if (tensor_arena == NULL) {
    tensor_arena = (uint8_t *) heap_caps_malloc(kTensorArenaSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  }
  if (tensor_arena == NULL) {
    printf("Couldn't allocate memory of %d bytes\n", kTensorArenaSize);
    return -1;
  }


  static tflite::MicroMutableOpResolver<5> micro_op_resolver;
  micro_op_resolver.AddAveragePool2D();
  micro_op_resolver.AddConv2D();
  micro_op_resolver.AddDepthwiseConv2D();
  micro_op_resolver.AddReshape();
  micro_op_resolver.AddSoftmax();



  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter; 
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    MicroPrintf("AllocateTensors() failed");
    return -1;
  }

  // Get information about the memory area to use for the model's input.
  input = interpreter->input(0);

  return 0;
}

/*
int pixels2jpg(uint8_t **jbuf, size_t *jbuf_len) {
  uint8_t *image_data ;
  image_data = (uint8_t*) input->data.int8;

  uint8_t *njpg = NULL ;
  size_t nlen ;

  bool rstat = fmt2jpg(image_data, input->bytes, kNumCols, kNumRows,
            PIXFORMAT_GRAYSCALE, 100, &njpg, &nlen);
  printf("Convert to jpg status %d\n", rstat);
  if ( rstat == true ) {
      if ( njpg != NULL ) {
        *jbuf = njpg ;
        //memcpy(jbuf, njpg, nlen); free(njpg);
        printf("Original image len %d\n", *jbuf_len);
        *jbuf_len = nlen ;
        printf("Converted new len %d\n", nlen);
      }
    return 0;
  }

 return -1;
}

esp_err_t jpeg2pixels( uint8_t *buffer, size_t buffer_len, uint16_t **pixels)
{
    esp_err_t ret = ESP_OK;

    *pixels = (uint16_t* ) calloc(NumRows * NumCols, sizeof(uint16_t));


    //if (*pixels != NULL ) goto out;


    //JPEG decode config
    esp_jpeg_image_cfg_t jpeg_cfg = {
        .indata = (uint8_t *) buffer,
        .indata_size = buffer_len,
        .outbuf = (uint8_t *)(* pixels),
        .outbuf_size = NumCols * NumRows * sizeof(uint16_t),
        .out_format = JPEG_IMAGE_FORMAT_RGB565,
        .out_scale = JPEG_IMAGE_SCALE_0,
        .flags = {
            .swap_color_bytes = 1,
        }
    };

    esp_jpeg_image_output_t outimg;
    esp_jpeg_decode(&jpeg_cfg, &outimg);

    ESP_LOGI(TAG, "JPEG image decoded! Size of the decoded image is: %dpx x %dpx", outimg.width, outimg.height);

    return ret;
//out:
    //Something went wrong! Exit cleanly, de-allocating everything we allocated.
    if (*pixels != NULL) {
        free(*pixels);
    }
    return ret;
} 

*/
int convert_image(uint8_t *buffer, size_t buffer_len) {

  uint8_t *image_data ;

  image_data = (uint8_t*) input->data.int8;

  uint16_t *pixels = NULL;
  //jpeg2pixels(buffer, buffer_len, &pixels);


  for (int i = 0; i < kNumRows; i++) {
    for (int j = 0; j < kNumCols; j++) {
      uint16_t pixel = ((uint16_t *) (pixels))[i * NumCols + j];

      // for inference
      uint8_t hb = pixel & 0xFF;
      uint8_t lb = pixel >> 8;
      uint8_t r = (lb & 0x1F) << 3;
      uint8_t g = ((hb & 0x07) << 5) | ((lb & 0xE0) >> 3);
      uint8_t b = (hb & 0xF8);

      /*
       * Gamma corected rgb to greyscale formula: Y = 0.299R + 0.587G + 0.114B
       * for effiency we use some tricks on this + quantize to [-128, 127]
        */
      int8_t grey_pixel = ((305 * r + 600 * g + 119 * b) >> 10) - 128;

      image_data[i * kNumCols + j] = grey_pixel;

    }
  }

  if ( pixels ) free (pixels) ;

 return 0;
}



float person_score_f;
float no_person_score_f ;

int object_detect(uint8_t * jpeg_data, int jpeg_data_len){
  
  //convert_image(jpeg_data, jpeg_data_len);
  uint8_t *image_data ;

  image_data = (uint8_t*) input->data.int8;
  
  for (int i = 0; i < kNumCols * kNumRows; i++) {
    input->data.int8[i] = (jpeg_data)[i] ^ 0x80;
  }

  if (kTfLiteOk != interpreter->Invoke()) {
    MicroPrintf("Invoke failed.");
    return -1;
  }

  TfLiteTensor* output = interpreter->output(0);

  // Process the inference results.
  int8_t person_score = output->data.uint8[kPersonIndex];
  int8_t no_person_score = output->data.uint8[kNotAPersonIndex];

  person_score_f =
      (person_score - output->params.zero_point) * output->params.scale;
  no_person_score_f =
      (no_person_score - output->params.zero_point) * output->params.scale;
  int person_score_int = (person_score_f) * 100 + 0.5;
  vTaskDelay(1); // to avoid watchdog trigger

  if (person_score_int > 60) {  
     printf("Persona\n");
  }
  else{
     printf("%f\n", person_score_f);
  }

  return 0;
} 
 
 