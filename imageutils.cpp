#include "imageutils.h"

using namespace ImageUtils;

// Преобразовать цветное изображение в изображение в оттенках серого
void ImageUtils::colorToGray(QImage* out, const QImage& in)
{
    Q_ASSERT (out);
    Q_ASSERT (!in.isNull());

    *out = QImage(in.size(), QImage::Format_RGB32);

    const int In_Width = in.width();
    const int In_Height = in.height();

    int gray = 0;       // текущее значение серого
    for (int i = 0; i < In_Width; ++i)
        for (int j = 0; j < In_Height; ++j) {
            gray = qGray(in.pixel(i, j));
            out->setPixel(i, j, qRgb(gray, gray, gray));
        }
}


// Уменьшать размер изображения в степенях двойки
void ImageUtils::downSampleImage(QImage* out, const QImage& in, unsigned int value)
{
    Q_ASSERT (out);
    Q_ASSERT (!in.isNull());
    Q_ASSERT (value <= 16);

    int ratio = 1 << value; // Значение прореживания изображения

    int newWidth = in.width() >> value;
    int newHeight = in.height() >> value;

    *out = QImage(newWidth, newHeight, in.format());

    for (int i = 0; i < newWidth; i++)
        for (int j = 0; j < newHeight; j++) {

            int redSum = 0;
            int greenSum = 0;
            int blueSum = 0;
            for (int ix = i * ratio; ix < ((i + 1) * ratio); ++ix)
                for (int iy = j * ratio; iy < ((j + 1) * ratio); ++iy) {
                    QRgb rgb = in.pixel(ix, iy);
                    redSum += qRed(rgb);
                    greenSum += qGreen(rgb);
                    blueSum += qBlue(rgb);
                }

            out->setPixel( i, j, qRgb(redSum / (ratio * ratio),
                                         greenSum  / (ratio * ratio),
                                         blueSum  / (ratio * ratio)) );
        }
}


// Получить матрицу оттенков серого изображения
void ImageUtils::imageToMatrix(const QImage& img, Matrix::Matrix2D<int>* matrix)
{
    Q_ASSERT(matrix);

    if (img.size().isEmpty())   // Если ширина или высота меньше или равна нулю
        return;
    // Преобразовать входное изображение в матрицу оттенков серого
    matrix->resize(img.size());
    int** data = matrix->getData();
    const int Img_Width = img.width();
    const int Img_Height = img.height();
    // Заполнить матрицу
    for (int i = 0; i < Img_Width; ++i)
        for (int j = 0; j < Img_Height; ++j)
            data[i][j] = qGray(img.pixel(i, j));
}

// Получить изображение по матрице
void ImageUtils::matrixToImage(QImage* img, const Matrix::Matrix2D<int>& matrix)
{
    Q_ASSERT(img);
    *img = QImage(matrix.getSize(), QImage::Format_RGB32);
    for (int i = 0; i < matrix.getWidth(); ++i)
        for (int j = 0; j < matrix.getHeight(); ++j) {
            int gray = (matrix.getData())[i][j];
            img->setPixel(i, j, qRgb(gray, gray, gray));
        }
}

