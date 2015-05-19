#ifndef IMAGEUTILS_H
#define IMAGEUTILS_H

#include <QImage>

// Утилиты для работы с изображением
namespace ImageUtils {

    // Преобразовать цветное изображение в изображение в оттенках серого,
    // Процедура сама задаёт размер выходного изображения out.
    void colorToGray(QImage* out, const QImage& in);

    // Уменьшать размер изображения в степенях двойки
    void downSampleImage(QImage* out, const QImage& in, unsigned int value);
}



#endif // IMAGEUTILS_H
