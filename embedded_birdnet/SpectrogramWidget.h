#include <QWidget>
#include <QTimer>
#include <QImage>
#include <QVector>
#include "bird_identification_engine.h"
class SpectrogramWidget : public QWidget
{
    Q_OBJECT
public:
    SpectrogramWidget(QWidget *parent = nullptr);

public slots:
    void startSimulation();
    void stopSimulation();

protected:
    void paintEvent(QPaintEvent *) override;

private slots:
    void updateSpectrogram();

private:
    bool loadAudioData(const QString &filename);
    BirdIdentificationEngine engine;
    QTimer *m_timer;
    QVector<short> m_pcmData;
    int m_currentSampleIndex;
    QImage m_spectrogramImage;
};
