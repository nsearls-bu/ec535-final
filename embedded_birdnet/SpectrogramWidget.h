#include <QWidget>
#include <QTimer>
#include <QImage>
#include <QVector>
#include "bird_identification_engine.h"
#define MODEL_OUTPUT_SIZE 1133
#define WINDOW_SIZE 144000
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
    QVector<float> m_pcmData;
    int m_currentSampleIndex;
    int m_currentModelIndex;
    QImage m_spectrogramImage;
    QImage m_fullHistoryImage;
    int m_historyWriteX;
    std::vector<Prediction> m_lastPredictions;
    QString m_currentBirdName;
    float m_currentConfidence;

};