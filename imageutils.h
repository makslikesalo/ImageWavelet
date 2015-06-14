#ifndef IMAGEUTILS_H
#define IMAGEUTILS_H

#include <QImage>
#include "matrix.h"

// Утилиты для работы с изображением
namespace ImageUtils {

    // Преобразовать цветное изображение в изображение в оттенках серого,
    // Процедура сама задаёт размер выходного изображения out.
    void colorToGray(QImage* out, const QImage& in);

    // Уменьшать размер изображения в степенях двойки
    void downSampleImage(QImage* out, const QImage& in, unsigned int value);

    // Получить матрицу оттенков серого изображения
    void imageToMatrix(const QImage& img, Matrix::Matrix2D<int>* matrix);

    // Получить изображение по матрице
    void matrixToImage(QImage* img, const Matrix::Matrix2D<int>& matrix);
}



#endif // IMAGEUTILS_H
