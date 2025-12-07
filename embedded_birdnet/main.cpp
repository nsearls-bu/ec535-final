#include <QApplication>
#include <QMainWindow>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDebug>
#include "SpectrogramWidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QMainWindow window;
    window.setWindowTitle("BeagleBone Spectrogram");
    window.setFixedSize(480, 272);

    QStackedWidget *stackedWidget = new QStackedWidget;
    window.setCentralWidget(stackedWidget);

    // Live spectogram
    QWidget *livePage = new QWidget;
    QVBoxLayout *liveLayout = new QVBoxLayout(livePage);
    liveLayout->setContentsMargins(0, 0, 0, 0);
    liveLayout->setSpacing(0);

    SpectrogramWidget *spectrogram = new SpectrogramWidget;
    liveLayout->addWidget(spectrogram);

    QPushButton *btnStop = new QPushButton("Stop");
    btnStop->setFixedHeight(30);
    liveLayout->addWidget(btnStop);
    QPushButton *btnStart = new QPushButton("Start");
    btnStart->setFixedHeight(30);
    liveLayout->addWidget(btnStart);
    stackedWidget->addWidget(livePage);

    // Results summary
    QWidget *resultsPage = new QWidget;
    QVBoxLayout *resLayout = new QVBoxLayout(resultsPage);
    resLayout->setContentsMargins(5, 5, 5, 5);

    // Full spectogram
    QLabel *lblHistory = new QLabel();
    lblHistory->setScaledContents(true);
    lblHistory->setFixedHeight(100); // Top half of screen
    lblHistory->setStyleSheet("border: 1px solid black; background: white;");

    // Top 5 list
    QLabel *lblPredictions = new QLabel("Loading...");
    lblPredictions->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    lblPredictions->setStyleSheet("font-size: 12px;");

    // Restart Button
    QPushButton *btnRestart = new QPushButton("Restart");

    resLayout->addWidget(new QLabel("<b>Full Session History:</b>"));
    resLayout->addWidget(lblHistory);
    resLayout->addWidget(lblPredictions, 1);
    resLayout->addWidget(btnRestart);

    stackedWidget->addWidget(resultsPage);

    QObject::connect(btnStop, &QPushButton::clicked, spectrogram, &SpectrogramWidget::stopSimulation);
    QObject::connect(btnStart, &QPushButton::clicked, spectrogram, &SpectrogramWidget::startSimulation);
    QObject::connect(spectrogram, &SpectrogramWidget::analysisFinished, [&]()
                     {

        // Get the full image and scale it
        QImage history = spectrogram->getFullHistoryImage();
        lblHistory->setPixmap(QPixmap::fromImage(history));

        // Get predictions and format text
        std::vector<Prediction> preds = spectrogram->getLastPredictions();
        QString html = "<table width='100%'>";
        for(size_t i=0; i<preds.size(); i++) {
            html += QString("<tr><td>%1. <b>%2</b></td><td align='right'>%3%</td></tr>")
            .arg(i+1)
                .arg(preds[i].label)
                .arg(preds[i].score * 100, 0, 'f', 1);
        }
        html += "</table>";
        lblPredictions->setText(html);

        // Switch Screen
        stackedWidget->setCurrentIndex(1); });

    // Restart Logic
    QObject::connect(btnRestart, &QPushButton::clicked, [&]()
                     {
        // Switch back to Live Page
        stackedWidget->setCurrentIndex(0); });

    window.show();

    return a.exec();
}
