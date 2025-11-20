#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#include <sndfile.h>
#include <samplerate.h>

#include "litert/c/litert_model.h"
#include "litert/c/litert_model_types.h"

#define WINDOW_SIZE 144000

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("%s", "USAGE: ./birdinference <file.wav> \n");
        return 1;
    }

    const char* model_path  = "BirdNET_GLOBAL_6K_V2.4_Model_FP16.tflite";
    LiteRtModel model = NULL;
    LiteRtStatus status = LiteRtCreateModelFromFile(model_path, &model);

    if (status != 0) {
        printf("Failed to load model\n");
        return 1;
    }
    LiteRtDestroyModel(model);
    return 0;
}