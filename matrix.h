#ifndef MATRIX_H
#define MATRIX_H

#include <QSize>

namespace Matrix {

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

        const Matrix2D& operator= (const Matrix2D& right) {
            if (this == &right)
                return *this;
            resize(right.getSize());
            const T** rightData = const_cast<const T**>(right.getData());
            for (int i = 0; i < getWidth(); ++i)
                for (int j = 0; j < getHeight(); ++j)
                    data[i][j] = rightData[i][j];

            return *this;
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

}   // namespace Matrix

#endif // MATRIX_H
