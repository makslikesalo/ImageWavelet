#ifndef WAVELET_H
#define WAVELET_H

#include <QSize>
#include <QPoint>
#include <cmath>

#include "matrix.h"

namespace Wavelet {

    // ФУНКЦИИ ВЕЙВЛЕТОВ:
    // Получить значение вейвлета FHAT ("Французская шляпа")
    // исходня из оси времени в единичним масштабе
    // (т.е. энергия распределена в области |t| <= 1)
    inline float getFhat(float t) {
        float v = 0.0;
        if (fabs(t) < (1.0 / 3.0))
            v = 1.0;
        else if (fabs(t) >= (1.0 / 3.0) && fabs(t) <= 1.0)
            v = -0.5;
        return v;
    }


    // Получить значение вейвлета MHAT ("Мексиканская шляпа")
    inline float getMhat(float t) {
        float t0 = 4.0 * t; // корректировка масштаба
        return (1 - t0 * t0) * exp(- (t0 * t0) / 2.0);
    }

    // Получить значение вейвлета FHAT ("Французская шляпа")
    // скорректированный для двухмерного анализа,
    // исходня из оси времени в единичним масштабе
    // (т.е. энергия распределена в области |t| <= 1)
    inline float getFhat2d(float t) {
        float v = 0.0;
        if (fabs(t) < sqrt(1.0 / 3.0))
            v = 1.0;
        else if (fabs(t) >= sqrt(1.0 / 3.0) && fabs(t) <= 1.0)
            v = -0.5;
        return v;
    }


    // Получить квадратную матрицу out вейвлета fun, размером size и
    // с масштабом значений ratio.
    // Возвращает квадратную матрицу out с нечётным числом строк и столбцов.
    // Размер стороны матрицы при различных значениях заданного размера size:
    // mSize = size * 2 + 1, или:
    // при size = 0, mSize = 1 эл.;
    // при size = 1, mSize = 3 эл.;
    // при size = 2, mSize = 5 эл.;
    // и т.д.
    // ratio - масштабирующий коэффициент результирующих значений вейвлета
    // (результирующие значения вейвлета заданы в интервале от -1.0 до +1.0)
    template<class T>
    void getWavelet2dMatrix(Matrix::Matrix2D<T>* out, float (*fun)(float), unsigned int size, float ratio)
    {
        Q_ASSERT (out);
        Q_ASSERT (fun);

        // Центр вейвлета по оси Х находится на пикселе с индексом:
        // (int) [mSize / 2], где
        // mSize - размер стороны матрицы

        const int MSize = size * 2 + 1; // Размер стороны матрицы

        // Итоговая матрица, которая инициализирована необходимым размером
        out->resize(QSize(MSize, MSize));
        T** data = out->getData();      // Указатель на память данных матрицы

        const float Center = (float) MSize / 2;     // Центр вейвлета по обоим осям

        for (int i = 0; i < MSize; ++i)
            for (int j = 0; j < MSize; ++j) {
                // Берём центр указанного пиксела
                // и считаем расстояние до центрального узла матрицы
                float dx = fabs(Center - i - 0.5);
                float dy = fabs(Center - j - 0.5);
                float d = sqrt(dx * dx + dy *dy);  // расстояние до центра матрицы

                // Найти значение вейвлета (от -1.0 до +1.0)
                float v = fun( d / Center );

                // Отмасштабировать и сохранить
                data[i][j] = (T) (v * ratio);
            }
    }


    // Наложить матрицу вейвлета wMatrix на матрицу данных inMatrix (вычислить свёртку)
    // Результат поместить в outMatrix.
    // Размер итоговой матрицы outMatrix меньше или равен inMatrix
    // (зависит от параметров topLeft и bottomRight).
    // Наложение матрицы вейвлета производится на область матрицы данных inMatrix
    // ограниченная topLeft, и bottomRight.
    // Если topLeft и bottomRight инициализированы по-умолчанию,
    // то вычисляется вся входная матрица данных.
    // Процедура оптимизирована для целочисленного вычисления.
    // Внимание!!!
    // Необходимо следить за тем, чтобы произведение максимальных
    // значений inMatrix и wMatrix не превышало по модулю 2^31 или 2 147 483 648!
    // Например, максимальное значение по модулю в inMatrix = 10000
    // Максимальное значение по модулю в wMatrix = 50000,
    // Их произведение = 10 000 * 50 000 = 500 000 000 не превышает установленного,
    // порога 2^31.
    // outsideValue - значение, которое используется при вычислении свёртки,
    // если вейвлет выходит за пределы матрицы входных данных. По-умолчанию принимается = 0.
    void imposeWavelet(Matrix::Matrix2D<int>* outMatrix,
                       const Matrix::Matrix2D<int>& inMatrix,
                       const Matrix::Matrix2D<int>& wMatrix,
                       int outsideValue = 0,
                       const QPoint& topLeft = QPoint(-1, -1),
                       const QPoint& bottomRight = QPoint(-1, -1));


}   // namespace Wavelet

#endif // WAVELET_H
