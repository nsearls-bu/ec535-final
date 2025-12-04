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

typedef struct
{
    int index;
    float score;
} ScoreLabelPair;

int cmp_scores(const void *a, const void *b)
{
    float diff = ((ScoreLabelPair *)b)->score - ((ScoreLabelPair *)a)->score;
    return (diff > 0) - (diff < 0);
}

void top_N_scores(float *scores, int N, ScoreLabelPair *out)
{
    for (int i = 0; i < MODEL_OUTPUT_SIZE; i++)
    {
        out[i].index = i;
        out[i].score = scores[i];
    }
    qsort(out, MODEL_OUTPUT_SIZE, sizeof(ScoreLabelPair), cmp_scores);
}

float *load_audio(char *file_path, int *total_frames_out)
{
    SF_INFO info = {0};
    SNDFILE *snd = sf_open(file_path, SFM_READ, &info);
    if (!snd)
        return NULL;

    float *buf = (float *)malloc(sizeof(float) * info.frames);
    sf_readf_float(snd, buf, info.frames);
    sf_close(snd);

    *total_frames_out = info.frames;
    return buf;
}
int count_labels()
{
    FILE *fp = fopen("BirdNET_GLOBAL_6K_V2.4_Labels.txt", "r");
    if (!fp)
        return -1;

    int count = 0;
    char line[512];
    while (fgets(line, sizeof(line), fp))
        count++;

    fclose(fp);
    return count;
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

int predict_window(TfLiteInterpreter *interpreter, float *window, float *out_scores)
{
    TfLiteTensor *input = TfLiteInterpreterGetInputTensor(interpreter, 0);

    if (TfLiteTensorCopyFromBuffer(input, window, WINDOW_SIZE * sizeof(float)) != kTfLiteOk)
        return -1;

    if (TfLiteInterpreterInvoke(interpreter) != kTfLiteOk)
        return -2;

    const TfLiteTensor *output = TfLiteInterpreterGetOutputTensor(interpreter, 0);
    if (TfLiteTensorCopyToBuffer(output, out_scores, MODEL_OUTPUT_SIZE * sizeof(float)) != kTfLiteOk)
        return -3;

    return 0;
}

float *get_window(float *audio, int total_samples, int *pos)
{
    if (*pos + WINDOW_SIZE > total_samples)
        return NULL;

    float *win = audio + *pos;
    *pos += WINDOW_SIZE;
    return win;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("%s", "USAGE: ./birdinference <file.wav> \n");
        return 1;
    }
    printf("Loading audio\n");
    int total_frames;
    float *audio_buffer = load_audio(argv[1], &total_frames);
    printf("Loading labels\n");
    char **labels = load_labels();

    const char *model_path = "BirdNET_GLOBAL_6K_V2.4_Model_INT8.tflite";
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
    int pos = 0;

    float scores[MODEL_OUTPUT_SIZE];
    ScoreLabelPair ranked[MODEL_OUTPUT_SIZE];

    float *win;

    while ((win = get_window(audio_buffer, total_frames, &pos)) != NULL)
    {
        if (predict_window(interpreter, win, scores) != 0)
        {
            printf("Inference error\n");
            break;
        }

        top_N_scores(scores, 5, ranked);

        printf("Window starting at %d:\n", pos - WINDOW_SIZE);
        for (int i = 0; i < 5; i++)
            printf(" %.4f : %s \n", ranked[i].score,labels[ranked[i].index]);

    }

    TfLiteModelDelete(model);
    return 0;
}