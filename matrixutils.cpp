#include "matrixutils.h"

#include <QRectF>
#include <QRect>

using namespace Matrix;

// Изменить размер матрицы in на outSize с преобразованием информации, имеющейся в ней
void Matrix::scaleMatrix(Matrix::Matrix2D<int>* out, const Matrix::Matrix2D<int>& in, const QSize& outSize)
{
    // Выходная матрица должна существовать в памяти
    Q_ASSERT (out);
    // Входная матрица должна быть определена
    Q_ASSERT (!in.isNull());
    // Размер входной матрицы должен быть ненулевым
    Q_ASSERT (!in.getSize().isEmpty());
    // Выходные размеры должны быть ненулевыми
    Q_ASSERT (!outSize.isEmpty());
    // Размер входной матрицы должен быть равен или больше выходной
    Q_ASSERT (in.getWidth() >= outSize.width() && in.getHeight() >= outSize.height());

    // Инициализировать размер выходной матрицы
    *out = Matrix2D<int>(outSize);
    int** outData = out->getData();


    // Обратный коэффициент масштабирования
    double scaleX = ((double) in.getWidth()) / outSize.width();
    double scaleY = ((double) in.getHeight()) / outSize.height();

    // По всем элементам выходной матрицы
    for (int i = 0; i < outSize.width(); ++i)
        for (int j = 0; j < outSize.height(); ++j) {
            // Найти координаты границ данного элемента
            // в системе координат исходной матрицы
            QRectF outElement;
            outElement.setLeft(((qreal) i) * scaleX);
            outElement.setRight(((qreal) (i + 1)) * scaleX);
            outElement.setTop(((qreal) j) * scaleY);
            outElement.setBottom(((qreal) (j + 1)) * scaleY);

            // Совокупность элементов входной матрицы,
            // которые попадают в элемент выходной
            QRect inElements;
            inElements.setLeft((int) outElement.left());
            inElements.setRight((int) outElement.right());
            inElements.setTop((int) outElement.top());
            inElements.setBottom((int) outElement.bottom());

            // Ограничить совокупность элементов размерами матрицы
            inElements = inElements.intersected(QRect(0, 0, in.getWidth(), in.getHeight()));

            // Аккумулятор значений элементов входной матрицы
            long long sum = 0;

            // Для всех элементов входной матрицы, которые попадают
            // в границы элемента новой матрицы
            for (int x = inElements.left(); x <= inElements.right(); ++x)
                for (int y = inElements.top(); y <= inElements.bottom(); ++y) {
                    // Область, которая занимает текущий элемент входной матрицы
                    QRectF inElementRect(x, y, 1.0, 1.0);

                    // Найти область пересечения области элемента входной матрицы
                    // и области элемента выходной матрицы
                    QRectF intersectRect(outElement.intersected(inElementRect));

                    // Посчитать площадь области пересечения
                    double s = intersectRect.width() * intersectRect.height();

                    // Найти и прибавить к аккумулятору значение,
                    // которое вкладывает элемент входной матрицы в элемент выходной
                    sum += s * in.getData()[x][y];
                }

            // Найти среднее для элемента выходной матрицы
            Q_ASSERT (i >= 0 && i < out->getWidth());
            Q_ASSERT (j >= 0 && j < out->getHeight());
            outData[i][j] = (int) ( ((double) sum) / (scaleX * scaleY) );
        }
}
