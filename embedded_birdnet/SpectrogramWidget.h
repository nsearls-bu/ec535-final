#include <QWidget>
#include <QTimer>
#include <QImage>
#include <complex>
#include <QVector>
#include "bird_identification_engine.h"
#define MODEL_OUTPUT_SIZE 1133
#define WINDOW_SIZE 144000
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 256
#define FFT_SIZE 512
#define HEIGHT 256
#define REFRESH_RATE_MS 10

const double NOISE_FLOOR_DB = -70.0;
const double SIGNAL_RANGE_DB = 50.0;
typedef std::complex<double> Complex;

class SpectrogramWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SpectrogramWidget(QWidget *parent = nullptr);
    void startSimulation();
    void stopSimulation();
    void reset();

    QImage getFullHistoryImage() const { return m_fullHistoryImage; }
    std::vector<Prediction> getLastPredictions() const { return m_lastPredictions; }

signals:
    void analysisFinished();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateSpectrogram();

private:
    bool loadAudioData(const QString &filename);
    int logarithmicFreq(int y);
    BirdIdentificationEngine engine;
    QTimer *m_timer;
    QVector<float> m_pcmData;
    Complex vec[FFT_SIZE];
    int m_currentSampleIndex;
    int m_currentModelIndex;
    QImage m_spectrogramImage;
    QImage m_fullHistoryImage;
    int m_historyWriteX;
    std::vector<Prediction> m_lastPredictions;
    QString m_currentBirdName;
    float m_currentConfidence;
};