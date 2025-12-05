#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include "SpectrogramWidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QMainWindow window;
    window.setWindowTitle("BeagleBone Spectrogram");

    QWidget *centralWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    SpectrogramWidget *spectrogram = new SpectrogramWidget;
    layout->addWidget(spectrogram, 1);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *startButton = new QPushButton("Start");
    QPushButton *stopButton = new QPushButton("Stop");

    QObject::connect(startButton, &QPushButton::clicked, spectrogram, &SpectrogramWidget::startSimulation);
    QObject::connect(stopButton, &QPushButton::clicked, spectrogram, &SpectrogramWidget::stopSimulation);

    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(stopButton);
    layout->addLayout(buttonLayout);

    window.setCentralWidget(centralWidget);
    window.show();

    return a.exec();
}
