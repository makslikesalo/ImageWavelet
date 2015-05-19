#include "mainwindow.h"

#include <QMouseEvent>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QHBoxLayout>
#include <QCoreApplication>

#include "imageutils.h"
#include "wavelet.h"
#include "performancetimer.h"

#include <QDebug>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), filePath("G:\\TestPhotos\\test.jpg")
{
    createMenus();

    viewer = new ImageViewer(this);
    viewer->resize(300, 300);
    viewer->move(20, 40);

    QImage image(filePath);
    viewer->setImage(image);

    waveletViewer = new ImageViewer(this);
    waveletViewer->resize(300, 300);
    waveletViewer->move(400, 40);

    QWidget *controlWidget = new QWidget(this);
    controlWidget->resize(600, 20);
    controlWidget->move(0, 20);

    QLabel *imageCaptionLabel = new QLabel("Image", controlWidget);

    downSampleSlider = new QSlider(Qt::Horizontal, controlWidget);
    downSampleSlider->setMinimum(0);
    downSampleSlider->setMaximum(4);
    downSampleSlider->setValue(0);
    connect(downSampleSlider, SIGNAL(valueChanged(int)), this, SLOT(updateDownSampleLabel()));
    connect(downSampleSlider, SIGNAL(valueChanged(int)), this, SLOT(updateDownSample()));

    downSampleLabel = new QLabel(controlWidget);

    QLabel *waveletCaptionLabel = new QLabel("Wavelet", controlWidget);

    waveletScaleSlider = new QSlider(Qt::Horizontal, controlWidget);
    waveletScaleSlider->setMinimum(0);
    waveletScaleSlider->setMaximum(127);
    waveletScaleSlider->setValue(0);
    connect(waveletScaleSlider, SIGNAL(valueChanged(int)), this, SLOT(updateWaveletLabel()));

    waveletScaleLabel = new QLabel(controlWidget);

    updateWaveletScalePushButton = new QPushButton("Update");
    connect(updateWaveletScalePushButton, SIGNAL(clicked()), this, SLOT(updateWavelet()));

    computeWaveletScalePushButton = new QPushButton("Compute");
    connect(computeWaveletScalePushButton, SIGNAL(clicked()), this, SLOT(computeWavelet()));

    QHBoxLayout* hBoxLayout = new QHBoxLayout;
    hBoxLayout->setMargin(0);
    hBoxLayout->addWidget(imageCaptionLabel);
    hBoxLayout->addWidget(downSampleSlider);
    hBoxLayout->addWidget(downSampleLabel);
    hBoxLayout->addSpacing(50);
    hBoxLayout->addWidget(waveletCaptionLabel);
    hBoxLayout->addWidget(waveletScaleSlider);
    hBoxLayout->addWidget(waveletScaleLabel);
    hBoxLayout->addWidget(updateWaveletScalePushButton);
    hBoxLayout->addWidget(computeWaveletScalePushButton);
    hBoxLayout->addStretch(1);

    controlWidget->setLayout(hBoxLayout);

    resize(800, 600);

    updateDownSampleLabel();
}

MainWindow::~MainWindow()
{

}


void MainWindow::createMenus(void)
{
    QAction *makeGrayAct = new QAction("Make gray", this);
    connect(makeGrayAct, SIGNAL(triggered()), this, SLOT(makeGray()));

    QAction *resetAct = new QAction("Reset", this);
    connect(resetAct, SIGNAL(triggered()), this, SLOT(reset()));

    QMenu *toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(makeGrayAct);
    toolsMenu->addAction(resetAct);
}


void MainWindow::makeGray(void)
{
    if (viewer->getImage().isNull()) return;
    QImage img;
    ImageUtils::colorToGray(&img, viewer->getImage());
    viewer->setImage(img);
}


void MainWindow::reset(void)
{
    QImage image(filePath);
    viewer->setImage(image);
}


// Обновить изображение вейвлета
void MainWindow::updateWavelet(void)
{
    Wavelet::Matrix2D<int> matrix;
    Wavelet::getWavelet2dMatrix<int>(&matrix, &Wavelet::getFhat2d, waveletScaleSlider->value(), 127.0);

    int sum = 0;
    QImage img(matrix.getSize(), QImage::Format_RGB32);
    for (int i = 0; i < matrix.getWidth(); ++i)
        for (int j = 0; j < matrix.getHeight(); ++j) {
            sum += (matrix.getData())[i][j];
            int gray = (matrix.getData())[i][j] + 127;
            img.setPixel(i, j, qRgb(gray, gray, gray));
        }
    qDebug() << sum;
    waveletViewer->setImage(img);
}


// Пройтись вейвлетом по текущему изображению
void MainWindow::computeWavelet(void)
{
CTimer timer;
timer.Start();

    float ratio = 10000.0;   // Масштабирующе-смещающий коэффициент при расчёте коэффициентов вейвлета

    // Анализируемое изображение
    QImage img(filePath);
    if (img.isNull())
        return;

    QList< QPair<int, int> > downSampleWaveletPairList;

    downSampleWaveletPairList << QPair<int, int>(downSampleSlider->value(), waveletScaleSlider->value());
    //downSampleWaveletPairList << QPair<int, int>(0, 4);
    //downSampleWaveletPairList << QPair<int, int>(0, 8);
    //downSampleWaveletPairList << QPair<int, int>(0, 16);
    //downSampleWaveletPairList << QPair<int, int>(0, 32);
    //downSampleWaveletPairList << QPair<int, int>(1, 32);
    //downSampleWaveletPairList << QPair<int, int>(1, 64);
    //downSampleWaveletPairList << QPair<int, int>(2, 64);
    //downSampleWaveletPairList << QPair<int, int>(2, 128);


    for (int k = 0; k < downSampleWaveletPairList.size(); k++) {

        QImage inImage(img.size(), img.format());
        ImageUtils::downSampleImage(&inImage, img, downSampleWaveletPairList.at(k).first);

        // Преобразовать входное изображение в матрицу оттенков серого
        Wavelet::Matrix2D<int> inMatrix(inImage.size());
        int** inData = inMatrix.getData();
        // Заполнить матрицу
        for (int i = 0; i < inImage.width(); ++i)
            for (int j = 0; j < inImage.height(); ++j)
                inData[i][j] = qGray(inImage.pixel(i, j));

        // Получить матрицу вейвлета
        Wavelet::Matrix2D<int> wMatrix;
        Wavelet::getWavelet2dMatrix<int>(&wMatrix, &Wavelet::getFhat2d, downSampleWaveletPairList.at(k).second, ratio);

        // Наложить вейвлет на входное изображение
        Wavelet::Matrix2D<int> outMatrix;
        Wavelet::imposeWavelet(&outMatrix, inMatrix, wMatrix, 255);

        QPoint minPoint, maxPoint;
        int minVal = ratio * 255, maxVal = 0;

        // Найти максимум и минимум
        int** outData = outMatrix.getData();
        for (int i = 0; i < outMatrix.getWidth(); ++i)
            for (int j = 0; j < outMatrix.getHeight(); ++j) {
                int val = outData[i][j];
                if (val > maxVal) {
                    maxVal = val;
                    maxPoint = QPoint(i, j);
                }
                if (val < minVal) {
                    minVal = val;
                    minPoint = QPoint(i, j);
                }
            }

        // Сместить значение вверх на уровень минимума (чтобы минимум стал нулём)
        int offset = 0;
        if (minVal != 0)
            offset = -minVal;
        // Увеличить так, чтобы с учётом смещения, максимум был самым максимальным значением
        float gain = (2.0 * 255.0 * ratio) / (maxVal + offset);

        // Преобразовать матрицу в выходное изображение
        QImage outImage(outMatrix.getSize(), QImage::Format_RGB32);

        for (int i = 0; i < outMatrix.getWidth(); ++i)
            for (int j = 0; j < outMatrix.getHeight(); ++j) {
                int gray = (gain * (outData[i][j] + offset) / ratio) / 2;
                if (gray > 255)
                    gray = 255;
                outImage.setPixel(i, j, qRgb(gray, gray, gray));
            }


        quint64 ticks = timer.Ticks_after_Start();
        double time = timer.Time();
        qDebug() << "Min:" << minPoint << minVal << "Max:" << maxPoint << maxVal << "ticks:" << QString::number(ticks) + " sec:" + QString::number(time) + " fps:" + QString::number(1.0 / time);

        outImage.setPixel(minPoint, QColor(Qt::red).rgb());
        outImage.setPixel(maxPoint, QColor(Qt::green).rgb());
        waveletViewer->setImage(outImage);
        QCoreApplication::processEvents();
        qDebug() << "K:" << k << "Min:" << minPoint << minVal << "Max:" << maxPoint << maxVal;
    }

}


void MainWindow::updateDownSampleLabel(void)
{
    downSampleLabel->setText( QString::number(1 << downSampleSlider->value()) );
}


void MainWindow::updateDownSample(void)
{
    QImage image(filePath);
    if (image.isNull())
        return;

    QImage img;
    ImageUtils::downSampleImage(&img, image, downSampleSlider->value());
    viewer->setImage(img);
}

void MainWindow::updateWaveletLabel(void)
{
    waveletScaleLabel->setText( QString::number(waveletScaleSlider->value() * 2 + 1) );
}
