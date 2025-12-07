#include "SpectrogramWidget.h"
#include <QPainter>
#include <QMessageBox>
#include <QDebug>
#include <QtMath>
#include <QFile>
#include <algorithm>
#include <sndfile.h>
#include <complex>



// FFT
static void fft(Complex *a, int n)
{
    if (n <= 1)
        return;

    Complex a0[n / 2], a1[n / 2];
    for (int i = 0; i < n / 2; i++)
    {
        a0[i] = a[2 * i];
        a1[i] = a[2 * i + 1];
    }
    fft(a0, n / 2);
    fft(a1, n / 2);
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
      engine("BirdNET_1K_V1.4_Model_FP32.tflite"),
      m_timer(new QTimer(this)),
      m_currentSampleIndex(0),
      m_currentModelIndex(0),
      m_spectrogramImage(SCREEN_WIDTH, HEIGHT, QImage::Format_RGB32),
      m_historyWriteX(0),
      m_currentBirdName("Waiting..."),
      m_currentConfidence(0.0f)
{
    setFixedSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    m_spectrogramImage.fill(Qt::white);
    if (!loadAudioData("soundscape_48k.wav"))
        qDebug() << "ERROR: Failed to load audio";
    qDebug() << "Loaded audio";

    for (int i = 0; i < 5; i++)
        m_lastPredictions.push_back(Prediction{"Waiting...", 0.0f});

    connect(m_timer, &QTimer::timeout, this, &SpectrogramWidget::updateSpectrogram);
}

void SpectrogramWidget::startSimulation()
{
    qDebug() << "Starting";

    if (m_pcmData.isEmpty())
    {
        QMessageBox::warning(this, "No Data", "No audio loaded. Cannot start.");
        return;
    }
    if (!m_timer->isActive())
    {
        m_timer->start(REFRESH_RATE_MS);
    }
}

void SpectrogramWidget::stopSimulation()
{
    m_timer->stop();
    emit analysisFinished();
}

void SpectrogramWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.drawImage(0, 0, m_spectrogramImage);

    // Red cursor line at the right edge (where new data appears)
    painter.setPen(QPen(Qt::red, 1));
    painter.drawLine(SCREEN_WIDTH - 1, 0, SCREEN_WIDTH - 1, HEIGHT);

    // Draws overlay text
    painter.fillRect(0, 0, SCREEN_WIDTH, 30, QColor(255, 255, 255, 200));

    painter.setPen(Qt::black);
    QFont font = painter.font();
    font.setPixelSize(14);
    font.setBold(true);
    painter.setFont(font);

    QString statusText = QString("Bird: %1  (%2%)")
                             .arg(m_currentBirdName)
                             .arg(m_currentConfidence * 100, 0, 'f', 1);

    painter.drawText(10, 20, statusText);
}

int SpectrogramWidget::logarithmicFreq(int y)
{
    double fMin = 275.0;
    double fMax = 48000 / 2.0;                              // We divide by 2 because we need at least 2 frames to get a frequency
    double normalized_y = (double)y / (double)(HEIGHT - 1); // normalize to height
    double freq = fMin * pow(fMax / fMin, normalized_y);
    int new_y = (int)(freq / 48000 * FFT_SIZE);
    if (new_y < 0)
        new_y = 0;
    if (new_y >= FFT_SIZE / 2)
        new_y = FFT_SIZE / 2 - 1;
    return new_y;
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
        return;
    }

    // RUN PREDICTION MODEL
    static int samplesSinceLastPrediction = 0;
    samplesSinceLastPrediction += FFT_SIZE;
    if (samplesSinceLastPrediction >= WINDOW_SIZE)
    {
        samplesSinceLastPrediction = 0;

        int start = m_currentModelIndex;

        if (start + WINDOW_SIZE <= m_pcmData.size())
        {
            float window[WINDOW_SIZE];

            for (int i = 0; i < WINDOW_SIZE; i++)
                window[i] = m_pcmData[start + i];
            float avg = 0;
            for (int i = 0; i < WINDOW_SIZE; i++)
                avg += window[i];
            avg /= WINDOW_SIZE;
            qDebug() << "avg window =" << avg;
            float logits[MODEL_OUTPUT_SIZE];
            float probabilities[MODEL_OUTPUT_SIZE];

            Prediction out[5];

            engine.predict(window, logits);
            engine.softmax(logits, probabilities);
            engine.get_top_results(probabilities, out);

            // Update internal state for the paint event overlay
            m_currentBirdName = QString(out[0].label);
            m_currentConfidence = out[0].score;
            qDebug() << m_currentBirdName << m_currentConfidence;
            m_lastPredictions.clear();
            for (int k = 0; k < 5; k++)
                m_lastPredictions.push_back(out[k]);
            m_currentModelIndex += WINDOW_SIZE;
        }
    }

    // Get window
    for (int i = 0; i < FFT_SIZE; ++i)
    {
        double multiplier = 0.5 * (1 - cos(2 * M_PI * i / (FFT_SIZE - 1)));
        vec[i] = Complex(
            (double)m_pcmData[m_currentSampleIndex + i] * 32768.0 * multiplier);
    }

    m_currentSampleIndex += FFT_SIZE; // Advance
    fft(vec, FFT_SIZE);                         // Process

    // Draw new column
    const double MAX_MAGNITUDE = 32768.0 * FFT_SIZE;
    int x_scroll = SCREEN_WIDTH - 1;

    int x_history = m_historyWriteX;
    if (x_history < m_fullHistoryImage.width())
    {
        m_historyWriteX++;
    }

    for (int y = 0; y < HEIGHT; ++y)
    {

        int new_y = logarithmicFreq(y);
        double magnitude = std::abs(vec[new_y]);
        double dbFS = 20 * log10(magnitude / MAX_MAGNITUDE + 1e-9);

        double shiftedDb = dbFS - NOISE_FLOOR_DB;
        double normalized = shiftedDb / SIGNAL_RANGE_DB;
        normalized = qBound(0.0, normalized, 1.0);

        m_spectrogramImage.setPixel(x_scroll, (HEIGHT - 1) - y, getMerlinColor(normalized));

        if (x_history < m_fullHistoryImage.width())
        {
            m_fullHistoryImage.setPixel(x_history, (HEIGHT - 1) - y, getMerlinColor(normalized));
        }
    }
    update();
}

bool SpectrogramWidget::loadAudioData(const QString &filename)
{
    SF_INFO info = {0};
    SNDFILE *snd = sf_open(filename.toUtf8().constData(), SFM_READ, &info);
    if (!snd)
    {
        return false;
    }
    float *buf = (float *)malloc(sizeof(float) * info.frames * info.channels);
    sf_readf_float(snd, buf, info.frames);
    sf_close(snd);

    float peak = 0.0f;
    for (int i = 0; i < info.frames; i++)
    {
        // Standard find max algorithm to find the loudest part of the audio file
        float v = abs(buf[i]);
        if (v > peak)
            peak = v;
    }

    if (peak < 1e-12f)
        peak = 1.0f;
    m_pcmData.clear();
    m_pcmData.reserve(info.frames);
    for (int i = 0; i < info.frames; i++)
    {
        m_pcmData.append((buf[i] / peak));
    }
    int totalColumns = info.frames / FFT_SIZE;
    m_fullHistoryImage = QImage(totalColumns, HEIGHT, QImage::Format_RGB32);
    m_fullHistoryImage.fill(Qt::white);
    m_historyWriteX = 0;
    return true;
}

void SpectrogramWidget::reset()
{
    m_currentSampleIndex = 0;
    m_currentModelIndex = 0;
    m_historyWriteX = 0;

    // Clear images
    m_spectrogramImage.fill(Qt::white);
    m_fullHistoryImage.fill(Qt::white);

    // Clear predictions
    m_currentBirdName = "Waiting...";
    m_currentConfidence = 0.0f;
    m_lastPredictions.clear();
    for (int i = 0; i < 5; i++)
        m_lastPredictions.push_back(Prediction{"Waiting...", 0.0f});

    update();
}