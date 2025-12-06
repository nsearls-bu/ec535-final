#ifndef BIRD_IDENTIFICATION_ENGINE_H
#define BIRD_IDENTIFICATION_ENGINE_H

#include <string>
#include "tensorflow/lite/c/c_api.h"

#define MODEL_OUTPUT_SIZE 1133
#define WINDOW_SIZE 144000
#define MAX_LINE_LENGTH 128
#define LABELS_PATH "BirdNET_1K_V1.4_Labels.txt"
typedef struct
{
    int index;
    float score;
} ScoreLabelPair;

struct Prediction
{
    char label[MAX_LINE_LENGTH];
    float score;
};

class BirdIdentificationEngine
{
public:
    BirdIdentificationEngine(const char *modelPath);
    ~BirdIdentificationEngine();

    int predict(float *window, float *out_scores);
    void get_top_results(const float scores[MODEL_OUTPUT_SIZE], Prediction out[5]);

private:
    TfLiteModel *model;
    TfLiteInterpreter *interpreter;
    TfLiteInterpreterOptions *options;

    char labels[MODEL_OUTPUT_SIZE][MAX_LINE_LENGTH];

    void load_labels();
    int count_labels();

    float *get_window(float *audio, int total_samples, int *pos);

    static int cmp_scores(const void *a, const void *b);
    void top_N_scores(float *scores, int N, ScoreLabelPair *out);
};

#endif