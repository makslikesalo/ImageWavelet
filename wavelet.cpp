#include "wavelet.h"

using namespace Wavelet;

void Wavelet::imposeWavelet(Matrix2D<int>* outMatrix,
                   const Matrix2D<int>& inMatrix,
                   const Matrix2D<int>& wMatrix,
                   int outsideValue,
                   const QPoint& topLeft,
                   const QPoint& bottomRight)
{
    Q_ASSERT (outMatrix);
    Q_ASSERT (!inMatrix.isNull());
    Q_ASSERT (!wMatrix.isNull());

    // Константы матрицы вейвлета
    const int wWidth = wMatrix.getWidth();
    const int wHeight = wMatrix.getHeight();
    const int wXCenter = wWidth / 2;
    const int wYCenter = wHeight / 2;
    const int wCount = wWidth * wHeight;    // Кол-во элементов в матрице вейвлета

    const int inWidth = inMatrix.getWidth();
    const int inHeight = inMatrix.getHeight();

    // Обрабатываемые элементы исходного изображения
    int inLeft = 0, inTop = 0;          // Элемент левого верхнего угла
    int inRight = 0, inBottom = 0;      // Элемент правого нижнего угла

    if (topLeft == QPoint(-1, -1) &&
        bottomRight == QPoint(-1, -1)) {
        inLeft = 0;
        inRight = inWidth - 1;
        inTop = 0;
        inBottom = inHeight - 1;
    }
    else {
        if (topLeft.x() < 0)
            inLeft = 0;
        else if (topLeft.x() > inWidth - 1)
            inLeft = inWidth - 1;

        if (topLeft.y() < 0)
            inTop = 0;
        else if (topLeft.y() > inHeight - 1)
            inTop = inHeight - 1;

        if (bottomRight.x() < 0)
            inRight = 0;
        else if (bottomRight.x() > inWidth - 1)
            inRight = inWidth - 1;

        if (bottomRight.y() < 0)
            inBottom = 0;
        else if (bottomRight.y() > inHeight - 1)
            inBottom = inHeight - 1;

        if (inRight < inLeft)
            inRight = inLeft;
        if (inBottom < inTop)
            inBottom = inTop;
    }


    int outWidth = inRight - inLeft + 1;
    int outHeight = inBottom - inTop + 1;

    // Размер выходной матрицы
    outMatrix->resize(QSize(outWidth, outHeight));

    // Указатели на данные матриц
    int** outData = outMatrix->getData();
    int** inData = inMatrix.getData();
    int** wData = wMatrix.getData();
    // Временные переменные
    long long waveletSum = 0;       // Сумма при вычислении свёртки
    int inX = 0, inY = 0;           // Координаты элемента исходной матрицы

    for (int i = inLeft, oi = 0; i <= inRight; ++i, ++oi)
        for (int j = inTop, oj = 0; j <= inBottom; ++j, ++oj) {
            // Вычислить свёртку для текущего элемента
            waveletSum = 0;
            for (int wi = 0; wi < wWidth; ++wi) {
                inX = i - wXCenter + wi;
                for (int wj = 0; wj < wHeight; ++wj) {
                    inY = j - wYCenter + wj;
                    if (inX >= 0 && inX < inWidth &&
                            inY >= 0 && inY < inHeight)
                        waveletSum += ((long long) inData[inX][inY]) * wData[wi][wj];
                    else
                        waveletSum += ((long long) outsideValue) * wData[wi][wj];
                }
            }
            outData[oi][oj] = waveletSum / wCount;
        }
}
