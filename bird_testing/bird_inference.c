#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#include <sndfile.h>
#include <samplerate.h>

#include "tensorflow/lite/c/c_api.h"
#define WINDOW_SIZE 144000
#define MAX_LINE_LENGTH 128

#define MODEL_OUTPUT_SIZE 6522

float *load_audio_and_resample(char *file_path, int offset)
{
    SF_INFO info;
    info.format = 0;
    SNDFILE *sndfile = sf_open(file_path, SFM_READ, &info);

    if (!sndfile)
    {
        printf("File not opened");
        return NULL;
    }

    if (info.channels > 1)
    {
        printf("Mono audio is only supported right now");
    }

    // float *buffer = (float *)malloc(sizeof(float) * info.frames);
    float *buffer = (float *)malloc(sizeof(float) * WINDOW_SIZE);

    sf_readf_float(sndfile, buffer, WINDOW_SIZE);
    sf_close(sndfile);

    double sample_ratio = 48000.0 / info.samplerate;
    // The new audio will need to be some ratio longer than the old audio
    int resampled_length = (int)(WINDOW_SIZE * sample_ratio);
    float *resampled_buffer = (float *)malloc(sizeof(float) * resampled_length);

    SRC_DATA res;
    res.data_in = buffer;
    res.data_out = resampled_buffer;
    res.input_frames = WINDOW_SIZE;
    res.output_frames = resampled_length;
    res.src_ratio = sample_ratio;
    res.end_of_input = 1;

    if (src_simple(&res, SRC_SINC_MEDIUM_QUALITY, 1) != 0)
    {
        printf("Resampling error");
        return NULL;
    }
    free(buffer);
    return resampled_buffer;
}

char **load_labels()
{
    FILE *fp = fopen("BirdNET_GLOBAL_6K_V2.4_Labels.txt", "r");
    if (fp == NULL)
    {
        return NULL;
    }
    char **labels = (char **)malloc(sizeof(char *) * MODEL_OUTPUT_SIZE);
    int i = 0;
    char line[128];
    while (fgets(line, MAX_LINE_LENGTH, fp) != NULL)
    {
        line[strcspn(line, "\n")] = 0;
        // Remove new line
        labels[i] = (char *)malloc(MAX_LINE_LENGTH);
        strcpy(labels[i], line);
        i++;
    }
    fclose(fp);
    return labels;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("%s", "USAGE: ./birdinference <file.wav> \n");
        return 1;
    }
    printf("Loading audio\n");
    float *audio_buffer = load_audio_and_resample(argv[1], 0);
    printf("Loading labels\n");
    char **labels = load_labels();

    const char *model_path = "BirdNET_GLOBAL_6K_V2.4_Model_FP16.tflite";
    printf("Loading model\n");
    TfLiteModel *model = TfLiteModelCreateFromFile(model_path);
    // Load and setup the model
    if (model == NULL)
    {
        printf("Failed to load model\n");
        return 1;
    }
    TfLiteInterpreterOptions *options = TfLiteInterpreterOptionsCreate();
    TfLiteInterpreter *interpreter = TfLiteInterpreterCreate(model, options);

    TfLiteInterpreterAllocateTensors(interpreter);
    printf("Input type: %d\n", input->type);
    printf("Input bytes: %d\n", input->bytes);
    printf("Input dims: %d %d\n",
           input->dims->data[0],
           input->dims->data[1]);
    TfLiteTensor *input = TfLiteInterpreterGetInputTensor(interpreter, 0);
    TfLiteTensorCopyFromBuffer(input, audio_buffer, WINDOW_SIZE * sizeof(float));
    // We do the first 3 seconds

    TfLiteInterpreterInvoke(interpreter);
    const TfLiteTensor *output = TfLiteInterpreterGetOutputTensor(interpreter, 0);

    float scores[MODEL_OUTPUT_SIZE];
    TfLiteTensorCopyToBuffer(output, scores, sizeof(scores));

    for (int i = 0; i < MODEL_OUTPUT_SIZE; i++)
    {
        if (scores[i] > 1)
        // Print the scores above 1
        {
            printf("Score %s = %f\n", labels[i], scores[i]);
        }
    }

    TfLiteModelDelete(model);
    return 0;
}