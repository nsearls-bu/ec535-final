#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include "bird_identification_engine.h"
#include <sndfile.h>
#include <samplerate.h>

#include "tensorflow/lite/c/c_api.h"

BirdIdentificationEngine::BirdIdentificationEngine(const char *modelPath)
{
    model = TfLiteModelCreateFromFile(modelPath);
    options = TfLiteInterpreterOptionsCreate();
    interpreter = TfLiteInterpreterCreate(model, options);
    TfLiteInterpreterAllocateTensors(interpreter);
    printf("Loaded model %s\n", modelPath);

    load_labels();
    printf("LABEL[0] = '%s'\n", labels[0]);
}

BirdIdentificationEngine::~BirdIdentificationEngine()
{
    if (interpreter) TfLiteInterpreterDelete(interpreter);
    if (options) TfLiteInterpreterOptionsDelete(options);
    if (model) TfLiteModelDelete(model);
}

int BirdIdentificationEngine::predict(float *window, float *out_scores) {

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


int BirdIdentificationEngine::cmp_scores(const void *a, const void *b)
{
    float diff = ((ScoreLabelPair *)b)->score - ((ScoreLabelPair *)a)->score;
    return (diff > 0) - (diff < 0);
}

void BirdIdentificationEngine::top_N_scores(float *scores, int N, ScoreLabelPair *out)
{
    for (int i = 0; i < MODEL_OUTPUT_SIZE; i++)
    {
        out[i].index = i;
        out[i].score = scores[i];
    }
    qsort(out, MODEL_OUTPUT_SIZE, sizeof(ScoreLabelPair), cmp_scores);
}

int BirdIdentificationEngine::count_labels()
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

void BirdIdentificationEngine::load_labels()
{
    FILE *fp = fopen("BirdNET_GLOBAL_6K_V2.4_Labels.txt", "r");
    if (!fp) {
        return;
    }

    int i = 0;
    char line[MAX_LINE_LENGTH];

    while (i < MODEL_OUTPUT_SIZE && fgets(line, MAX_LINE_LENGTH, fp)) {
        line[strcspn(line, "\n")] = '\0';    // strip newline
        strncpy(labels[i], line, MAX_LINE_LENGTH - 1);
        labels[i][MAX_LINE_LENGTH - 1] = '\0';
        i++;
    }

    fclose(fp);
}


float *BirdIdentificationEngine::get_window(float *audio, int total_samples, int *pos)
{
    if (*pos + WINDOW_SIZE > total_samples)
        return NULL;

    float *win = audio + *pos;
    *pos += WINDOW_SIZE;
    return win;
}


void BirdIdentificationEngine::get_top_results(const float scores[MODEL_OUTPUT_SIZE],
                                        Prediction out[5])
{
    
    ScoreLabelPair ranked[MODEL_OUTPUT_SIZE];

    top_N_scores((float*)scores, 5, ranked);

    for (int i = 0; i < 5; i++) {
        const char* label = labels[(ranked[i].index)];

        strncpy(out[i].label, label, MAX_LINE_LENGTH - 1);
        out[i].label[MAX_LINE_LENGTH - 1] = '\0';

        out[i].score = ranked[i].score;
    }
}