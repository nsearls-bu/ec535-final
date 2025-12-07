#include <QWidget>
#include <QTimer>
#include <QImage>
#include <QVector>
#include "bird_identification_engine.h"
class SpectrogramWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SpectrogramWidget(QWidget *parent = nullptr);
    void startSimulation();
    void stopSimulation();

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
    QVector<short> m_pcmData;
    int m_currentSampleIndex;
    QImage m_spectrogramImage;
    QImage m_fullHistoryImage;
    int m_historyWriteX;
    std::vector<Prediction> m_lastPredictions;
    QString m_currentBirdName;
    float m_currentConfidence;
    static const int WINDOW_SIZE = 14400;
    static const int MODEL_OUTPUT_SIZE = 1000;
};