#ifndef WAVELET_H
#define WAVELET_H

#include <QSize>
#include <QPoint>
#include <cmath>

namespace Wavelet {

    // Класс 2-х мерной матрицы
    template <class T>
    class Matrix2D {

        T** data;
        QSize size;
    public:
        Matrix2D() : data(NULL) {
        }

        Matrix2D(const QSize& sz) : size(sz) {
            Q_ASSERT(size.width() > 0 && size.height() > 0);
            allocate(size);         // Выделить память
        }

        ~Matrix2D() { clear(); }

        bool isNull(void) const { return data == NULL; }

        // Изменить размер матрицы.
        // Старые данные будут утеряны
        void resize(const QSize& sz) {
            Q_ASSERT(sz.width() > 0 && sz.height() > 0);
            clear();            // Очистить текущую матрицу
            size = sz;          // Присвоить новый размер
            allocate(size);     // Выделить память под новый размер
        }

        // Очистить матрицу
        void clear(void) {
            if (data != NULL && size.width() > 0 && size.height() > 0)
                deallocate(size);   // Освободить память
            size = QSize();
        }

        // Получить рамер
        const QSize& getSize(void) const { return size; }

        // Получить кол-во строк матрицы
        int getWidth(void) const { return size.width(); }

        // Получить кол-во столбцов матрицы
        int getHeight(void) const { return size.height(); }

        // Получить указатель на данные матрицы (следует проверить на null)
        T** getData(void) const { return data; }

    private:
        // Выделить память под data
        void allocate(const QSize& sz) {
            Q_ASSERT(sz.width() > 0 && sz.height() > 0);
            data = new T*[sz.width()];   // Строки
            for (int i = 0; i < sz.width(); ++i)
                data[i] = new T[sz.height()];  // Столбцы
        }

        // Освободить память под data
        void deallocate(const QSize& sz) {
            Q_ASSERT(data);
            Q_ASSERT(sz.width() > 0 && sz.height() > 0);
            for (int i = 0; i < sz.width(); ++i)
                delete [] data[i];
            delete [] data;
            data = NULL;
        }
    };


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


    // Получить квадратную 2-х мерную матрицу и записать её в out для вейвлета fun
    // с указанным типом T,
    // размером матрицы [(size + 1) * 2] х [(size + 1) * 2] (т.е. размер матрицы кратен 2),
    // и масштабирающим коэффициентом ratio, который переводит значения из области
    // {-1.0, 1.0} в {-1.0 * ratio, 1.0 * ratio}.
    //
    // При расчёте значений матрицы, центр 2-х мерного вейвлета находится
    // на пересечении центральных осей Х и Y воображаемой сетки (оси выделены двойной линией):
    // ┌─┬─┬─┬─╥─┬─┬─┬─┐
    // ├─┼─┼─┼─╫─┼─┼─┼─┤
    // ├─┼─┼─┼─╫─┼─┼─┼─┤
    // ├─┼─┼─┼─╫─┼─┼─┼─┤
    // ╞═╪═╪═╪═╬═╪═╪═╪═╡ mSize
    // ├─┼─┼─┼─╫─┼─┼─┼─┤
    // ├─┼─┼─┼─╫─┼─┼─┼─┤
    // ├─┼─┼─┼─╫─┼─┼─┼─┤
    // └─┴─┴─┴─╨─┴─┴─┴─┘
    //  0 1 2 3 4 5 6 7  - индексы элементов матрицы
    //       mSize
    // где mSize - размер стороны матрицы.
    //
    // Т.е. матрицу можно представить как сетку в декартовой системе координат
    // (рёбра сетки разграничивают элементы матрицы),
    // начало системы координат находится в левом верхнем углу.
    //
    // Последовательно перебираются элементы матрицы по осям X и Y,
    // берётся "центр" текущего элемента матрицы: (X + 0.5, Y + 0.5),
    // и находится расстояние от него до центра воображаемой сетки матрицы.
    // Далее, по получившемуся расстоянию вычисляется значение функции вейвлета v = f(t).
    /*template<class T>
    void getWavelet2dMatrix(Matrix2D<T>* out, float (*fun)(float), unsigned int size, float ratio)
    {
        Q_ASSERT (out);
        Q_ASSERT (fun);

        // Центр вейвлета по оси Х находится на границе пикселов:
        // [mSize / 2] и [mSize / 2 - 1], где
        // mSize - размер стороны матрицы

        // Размер стороны матрицы при различных значениях заданного размера size:
        // mSize = (size + 1) * 2, или:
        // 0 - 2 эл.
        // 1 - 4 эл.
        // 2 - 6 эл.
        // 3 - 8 эл.
        // 4 - 10 эл.
        // ...

        const int MSize = (size + 1) * 2; // Размер стороны матрицы

        // Итоговая матрица, которая инициализирована необходимым размером
        out->resize(QSize(MSize, MSize));
        T** data = out->getData();      // Указатель на память данных матрицы

        const int Center = MSize / 2;     // Центр вейвлета по обоим осям

        for (int i = 0; i < MSize; ++i)
            for (int j = 0; j < MSize; ++j) {
                // Берём центр указанного пиксела
                // и считаем расстояние до центрального узла матрицы
                float dx = fabs(Center - i - 0.5);
                float dy = fabs(Center - j - 0.5);
                float d = sqrt(dx * dx + dy *dy);  // расстояние до центра матрицы

                // Найти значение вейвлета (от -1.0 до +1.0)
                float v = fun( d / ((float)(MSize / 2)) );

                // Отмасштабировать и сохранить
                data[i][j] = (T) (v * ratio);
            }
    }*/


    // Возвращает квадратную матрицу out с нечётным числом строк и столбцов.
    // Размер стороны матрицы при различных значениях заданного размера size:
    // mSize = size * 2 + 1, или:
    // при size = 0, mSize = 1 эл.;
    // при size = 1, mSize = 3 эл.;
    // при size = 2, mSize = 5 эл.;
    // и т.д.
    template<class T>
    void getWavelet2dMatrix(Matrix2D<T>* out, float (*fun)(float), unsigned int size, float ratio)
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
    void imposeWavelet(Matrix2D<int>* outMatrix,
                       const Matrix2D<int>& inMatrix,
                       const Matrix2D<int>& wMatrix,
                       int outsideValue = 0,
                       const QPoint& topLeft = QPoint(-1, -1),
                       const QPoint& bottomRight = QPoint(-1, -1));


}   // namespace Wavelet

#endif // WAVELET_H
