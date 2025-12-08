#include <QObject>
#include <QVector>
#include <complex>

#include "bird_identification_engine.h"

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);

public slots:
    void run(const QVector<float> pcm, int startIndex);

signals:
    void done(QVector<Prediction> preds);
private:
    BirdIdentificationEngine engine;
};
