#include "worker.h"
#include <QtMath>

Worker::Worker(QObject *parent)
    : QObject(parent),
      engine("BirdNET_1K_V1.4_Model_FP32.tflite")
{
}

void Worker::run(const QVector<float> pcm, int startIndex)
{
    // Runs the prediction model
    float window[WINDOW_SIZE];
    for (int i = 0; i < WINDOW_SIZE; i++)
        window[i] = pcm[startIndex + i];

    float logits[MODEL_OUTPUT_SIZE];
    float probabilities[MODEL_OUTPUT_SIZE];
    Prediction out[5];

    engine.predict(window, logits);
    engine.softmax(logits, probabilities);
    engine.get_top_results(probabilities, out);

    QVector<Prediction> qvector_out;
    qvector_out.reserve(5);
    for (int i = 0; i < 5; i++) {
        qvector_out.push_back(out[i]);
    }
    emit done(qvector_out);
}
