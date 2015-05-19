#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSlider>

#include "imageviewer.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();    

private slots:
    void makeGray(void);
    void reset(void);
    void updateWavelet(void);
    void computeWavelet(void);

    void updateDownSampleLabel(void);
    void updateDownSample(void);
    void updateWaveletLabel(void);

private:
    void createMenus(void);

    QSlider* downSampleSlider;
    QLabel* downSampleLabel;

    QSlider* waveletScaleSlider;
    QLabel* waveletScaleLabel;
    QPushButton* updateWaveletScalePushButton;
    QPushButton* computeWaveletScalePushButton;

    ImageViewer *viewer;
    QString filePath;

    ImageViewer *waveletViewer;
};

#endif // MAINWINDOW_H
