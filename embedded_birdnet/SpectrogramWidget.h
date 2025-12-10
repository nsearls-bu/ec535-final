#include <QWidget>
#include <QTimer>
#include <QImage>
#include <complex>
#include <QVector>
#include <QThread>
#include "bird_identification_engine.h"
#include "worker.h"
#include <queue>
#include <mutex>
#define MODEL_OUTPUT_SIZE 1133
#define WINDOW_SIZE 144000
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 256
#define FFT_SIZE 512
#define HEIGHT 256
#define REFRESH_RATE_MS 30

const double NOISE_FLOOR_DB = -70.0;
const double SIGNAL_RANGE_DB = 50.0;
typedef std::complex<double> Complex;

class SpectrogramWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SpectrogramWidget(QWidget *parent = nullptr, char *audio_path = nullptr);
    void startSimulation();
    void stopSimulation();
    void reset();
    ~SpectrogramWidget();
    QImage getFullHistoryImage() const { return m_fullHistoryImage; }
    std::vector<Prediction> getLastPredictions() const { return m_lastPredictions; }

signals:
    void analysisFinished();
    void runRequest(const QVector<float> &pcm, int startIndex);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateSpectrogram();

private:
    bool loadAudioData(const QString &filename);
    int logarithmicFreq(int y);
    QThread m_workerThread;
    Worker *m_worker;
    QTimer *m_timer;
    QVector<float> m_pcmData;
    Complex vec[FFT_SIZE];
    void handleDone(QVector<Prediction> preds);
    int m_currentSampleIndex;
    int m_currentModelIndex;
    std::mutex m_predictionMutex;
    
    QImage m_spectrogramImage;
    QImage m_fullHistoryImage;
    int m_historyWriteX;
    std::vector<Prediction> m_lastPredictions;
    std::queue<QVector<Prediction>> predictionQueue;
    QString m_currentBirdName;
    float m_currentConfidence;
};