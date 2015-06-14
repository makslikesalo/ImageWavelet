#ifndef MATRIXUTILS_H
#define MATRIXUTILS_H

#include "matrix.h"

namespace Matrix {

    // Изменить размер матрицы с преобразованием информации, имеющейся в исходной матрице
    void scaleMatrix(Matrix2D<int>* out, const Matrix2D<int>& in, const QSize& outSize);

}   // namespace Matrix

#endif // MATRIXUTILS_H
