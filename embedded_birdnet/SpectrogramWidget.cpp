#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QTimer>
#include <QImage>
#include <QPainter>
#include <QColor>
#include <QFile>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDebug>
#include "SpectrogramWidget.h"
#include <QtMath>
#include <complex>
#include <iostream>
#include <algorithm>
#include "bird_identification_engine.h"

// GLOBAL CONSTANTS
const int SCREEN_WIDTH = 480;
const int SCREEN_HEIGHT = 256;
const int FFT_SIZE = 512;
const int HEIGHT = FFT_SIZE / 2;
const int REFRESH_RATE_MS = 33;

const double NOISE_FLOOR_DB = -80.0;
const double SIGNAL_RANGE_DB = 50.0;

typedef std::complex<double> Complex;

// FFT
static void fft(QVector<Complex> &a)
{
    int n = a.size();
    if (n <= 1)
        return;
    QVector<Complex> a0(n / 2), a1(n / 2);
    for (int i = 0; i < n / 2; i++)
    {
        a0[i] = a[2 * i];
        a1[i] = a[2 * i + 1];
    }
    fft(a0);
    fft(a1);
    double ang = 2 * M_PI / n;
    Complex w(1), wn(cos(ang), sin(ang));
    for (int i = 0; i < n / 2; i++)
    {
        a[i] = a0[i] + w * a1[i];
        a[i + n / 2] = a0[i] - w * a1[i];
        w *= wn;
    }
}

// Color mapping
static QRgb getMerlinColor(double normalizedValue)
{
    int grayLevel = (int)((1.0 - normalizedValue) * 255);
    grayLevel = qBound(0, grayLevel, 255);
    return qRgb(grayLevel, grayLevel, grayLevel);
}

// Spectogram widget

SpectrogramWidget::SpectrogramWidget(QWidget *parent)
    : QWidget(parent),
      engine("BirdNET_GLOBAL_6K_V2.4_Model_INT8.tflite"), m_timer(new QTimer(this)),
      m_currentSampleIndex(0),
      m_spectrogramImage(SCREEN_WIDTH, HEIGHT, QImage::Format_RGB32)
{
    setFixedSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    m_spectrogramImage.fill(Qt::white);

    if (!loadAudioData("soundscape_48k.wav"))
        qDebug() << "ERROR: Failed to load audio.wav";

    connect(m_timer, &QTimer::timeout, this, &SpectrogramWidget::updateSpectrogram);
}

void SpectrogramWidget::startSimulation()
{
    if (m_pcmData.isEmpty())
    {
        QMessageBox::warning(this, "No Data", "No audio loaded. Cannot start.");
        return;
    }
    if (!m_timer->isActive())
    {
        m_timer->start(REFRESH_RATE_MS);
        qDebug() << "Simulation STARTED.";
    }
}

void SpectrogramWidget::stopSimulation()
{
    m_timer->stop();
    qDebug() << "Simulation STOPPED.";
}

void SpectrogramWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawImage(0, 0, m_spectrogramImage);

    // Red cursor line at the right edge (where new data appears)
    painter.setPen(QPen(Qt::red, 1));
    painter.drawLine(SCREEN_WIDTH - 1, 0, SCREEN_WIDTH - 1, HEIGHT);
}

void SpectrogramWidget::updateSpectrogram()
{
    if (m_pcmData.isEmpty())
    {
        stopSimulation();
        return;
    }

    QImage shifted = m_spectrogramImage.copy(1, 0, SCREEN_WIDTH - 1, HEIGHT);

    QPainter painter(&m_spectrogramImage);
    painter.drawImage(0, 0, shifted);

    // Clear the new column on the right
    painter.fillRect(SCREEN_WIDTH - 1, 0, 1, HEIGHT, Qt::white);
    painter.end();

    // Check if we reached the end of the audio file
    if (m_currentSampleIndex + FFT_SIZE >= m_pcmData.size())
    {
        stopSimulation();
        m_currentSampleIndex = 0; // Reset so next start plays from beginning
        QMessageBox::information(this, "Analysis Complete", "bird is not identified");
        return;
    }
    // RUN PREDICTION MODEL
    static int samplesSinceLastPrediction = 0;
    samplesSinceLastPrediction += FFT_SIZE;
    if (samplesSinceLastPrediction >= WINDOW_SIZE)
    {
        printf("%d\n", samplesSinceLastPrediction);
        samplesSinceLastPrediction = 0;

        int start = m_currentSampleIndex;
        if (start + WINDOW_SIZE <= m_pcmData.size())
        {

            float window[WINDOW_SIZE];
            for (int i = 0; i < WINDOW_SIZE; i++)
                window[i] = (float)m_pcmData[start + i];

            float scores[MODEL_OUTPUT_SIZE];
            Prediction out[5];

            engine.predict(window, scores);
            engine.get_top_results(scores, out);

            printf("%s\n", out[0].label);
        }
    }
    // Get window
    QVector<Complex> vec(FFT_SIZE);
    for (int i = 0; i < FFT_SIZE; ++i)
    {
        double multiplier = 0.5 * (1 - cos(2 * M_PI * i / (FFT_SIZE - 1)));
        vec[i] = (double)m_pcmData[m_currentSampleIndex + i] * multiplier;
    }

    m_currentSampleIndex += 144000 / 90;; // Advance
    fft(vec);                         // Process

    // Draw new column
    const double MAX_MAGNITUDE = 32768.0 * FFT_SIZE;
    const int x = SCREEN_WIDTH - 1;

    for (int y = 0; y < HEIGHT; ++y)
    {
        double magnitude = std::abs(vec[y]);
        double dbFS = 20 * log10(magnitude / MAX_MAGNITUDE + 1e-9);

        double shiftedDb = dbFS - NOISE_FLOOR_DB;
        double normalized = shiftedDb / SIGNAL_RANGE_DB;
        normalized = qBound(0.0, normalized, 1.0);

        m_spectrogramImage.setPixel(x, (HEIGHT - 1) - y, getMerlinColor(normalized));
    }

    update();
}

bool SpectrogramWidget::loadAudioData(const QString &filename)
{
    QFile audioFile(filename);
    if (!audioFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "Could not open file:" << filename;
        return false;
    }

    audioFile.seek(44);
    QByteArray rawData = audioFile.readAll();
    audioFile.close();

    if (rawData.size() < FFT_SIZE * 2)
        return false;

    int totalSamples = rawData.size() / 2;
    const short *pcmData = reinterpret_cast<const short *>(rawData.constData());
    m_pcmData.reserve(totalSamples);
    for (int i = 0; i < totalSamples; ++i)
    {
        m_pcmData.append(pcmData[i]);
    }

    qDebug() << "Loaded" << totalSamples << "samples.";
    return true;
}
