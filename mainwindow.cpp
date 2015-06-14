#include "mainwindow.h"

#include <QMouseEvent>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QHBoxLayout>
#include <QCoreApplication>
#include <QPainter>
#include <QFileDialog>
#include <QtConcurrent/QtConcurrent>

#include "imageutils.h"
#include "matrixutils.h"
#include "wavelet.h"
#include "performancetimer.h"

#include <QDebug>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), isSearching(false), searchIter(0)
{
    connect(&watcher, SIGNAL(finished()), this, SLOT(handleExtremumsFinished()));

    createMenus();

    viewer = new ImageViewer(this);
    viewer->resize(300, 300);
    viewer->move(20, 40);

    progressDialog = new QProgressDialog("Search in progress.", QString(), 0, 100, this);

    resize(800, 600);
}


MainWindow::~MainWindow()
{

}


// Создать меню
void MainWindow::createMenus(void)
{
    QAction *openFileAct = new QAction("Open...", this);
    connect(openFileAct, SIGNAL(triggered()), this, SLOT(openFile()));

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openFileAct);


    QAction *findAct = new QAction("Search", this);
    connect(findAct, SIGNAL(triggered()), this, SLOT(find()));

    QMenu *toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(findAct);
}


// Открыть файл
void MainWindow::openFile(void)
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open File"),
                                                    "/home",
                                                    tr("Images (*.png *.bmp *.jpg)"));
    if (!fileName.isEmpty()) {
        filePath = fileName;        // Сохранить путь к файлу
        setWindowTitle(QString("%1 - ImageWavelet").arg(filePath));
        loadImage();                // Загрузить изображение
    }
}


// Обновить изображение с учётом новых размеров и пр.
void MainWindow::loadImage(void)
{
    if (!filePath.isEmpty()) {
        QImage image(filePath);
        if (image.isNull()) {     // Если изображение не открыто
            qWarning() << QString("Image path \"%1\" is incorrect").arg(filePath);
            return;
        }
        viewer->setImage(image);
    }
}


// Найти круглую светлую структуру
void MainWindow::find(void)
{
    if (isSearching) {      // Если поиск активен
        qWarning() << "Searching in progress";
        return;     // Не выполнять перезапуск процедуры поиска
    }

    if (filePath.isEmpty()) {       // Если путь к файлу не задан
        qWarning() << "File path is empty";
        return;
    }

    // 1. Загрузить изображение
    QImage image(filePath);
    if (image.isNull()) {       // Если изображение не открыто
        qWarning() << QString("Image path \"%1\" is incorrect").arg(filePath);
        return;
    }

    // 2. Преобразовать изображение в матрицу
    ImageUtils::imageToMatrix(image, &imageMatrix);

    // 3. Запустить поиск сначала
    execSearch(true);
}


// Обработчик цикла поиска,
// reset = true - сбросить состояние и начать поиск с начала
void MainWindow::execSearch(bool reset)
{
    // Размеры матрицы должны быть ненулевыми
    Q_ASSERT (imageMatrix.getWidth() > 0);
    Q_ASSERT (imageMatrix.getHeight() > 0);

    if (reset) {
        isSearching = true;                         // Установить флаг активности процесса поиска
        searchIter = 0;                             // Итерация поиска
        progressDialog->setValue(0);
        progressDialog->show();

        // Диаметр шара измеряется в относительных единицах от минимальной стороны матрицы
        // 1.0 - Диаметр шара равен минимальной стороне матрицы
        // 0.5 - Диаметр шара равен половине минимальной стороны матрицы
        // Т.е. не важно для какого размера матрица

        // Начальные условия
        float begin = getMinDiameter(imageMatrix.getSize());         // Начать с диаметра
        float end = getMaxDiameter();                                // Закончить диаметром
        float step = (end - begin) / Search_Diameter_Intervals;      // Шаг изменения диаметра

        extrems.clear();    // Очистить старый список экстремумов

        // Заполнить список экстремумов
        // Для всех диаметров
        float diameter = begin;
        while (diameter < end) {
            extrems.append(Extremums(diameter));
            diameter += step;
        }

        // Запустить асинхронное вычисление
        HandleWrapper wrap(this);
        QFuture<void> future = QtConcurrent::map(extrems, wrap);
        watcher.setFuture(future);
    }
    else {
        // Удалить некорректные экстремумы
        int i = 0;
        while (i < extrems.size()) {
            if (extrems.at(i).diameter <= 0)
                extrems.removeAt(i);
            else
                ++i;
        }

        int maxIndex = findIndexMaximum(extrems);       // Найти индекс максимального
        Q_ASSERT (maxIndex >= 0);

        if (searchIter < (Search_Iterations - 1)) {       // Если итерации не завершены
            // Продолжать поиск относительно максимального экстремума
            // слева и справа
            int leftIndex = maxIndex - 1;       // Индекс левой части
            if (leftIndex < 0) leftIndex = 0;
            int rightIndex = maxIndex + 1;      // Индекс правой части
            if (rightIndex >= extrems.size()) rightIndex = extrems.size() - 1;

            float begin = extrems.at(leftIndex).diameter;
            float end = extrems.at(rightIndex).diameter;
            float step = (end - begin) / Search_Diameter_Intervals;

            extrems.clear();    // Очистить старый список экстремумов

            // Заполнить список экстремумов
            // Для всех диаметров
            float diameter = begin;
            while (diameter < end) {
                extrems.append(Extremums(diameter));
                diameter += step;
            }
            extrems.append(Extremums(end));     // Положить последний диаметр

            // Запустить асинхронное вычисление
            HandleWrapper wrap(this);
            QFuture<void> future = QtConcurrent::map(extrems, wrap);
            watcher.setFuture(future);

            ++searchIter;
            progressDialog->setValue( (searchIter * 100) /
                                        Search_Iterations);
        }
        else {                      // Если итерации завершены
            // Поиск завершён
            isSearching = false;

            // Заполнить выходные данные
            int d = extrems.at(maxIndex).diameter * qMin(imageMatrix.getWidth(), imageMatrix.getHeight());
            QPoint center(imageMatrix.getWidth() * extrems.at(maxIndex).maxPoint.x(),
                          imageMatrix.getHeight() * extrems.at(maxIndex).maxPoint.y());

            // Вывести исходную матрицу на экран вместе с результатом измерения
            QRect circleRect(0, 0, d, d);
            circleRect.moveCenter(center);
            QImage image(filePath);
            QPainter painter(&image);
            painter.setPen(QPen(QBrush(Qt::red), 2));
            painter.drawEllipse(circleRect);
            painter.drawPoint(center);
            viewer->setImage(image);
            progressDialog->setValue(100);
        }
    }

}


// Обработка экстремумов завершена
void MainWindow::handleExtremumsFinished(void)
{
    execSearch(false);      // Запустить обработчик поиска
}


// Вычилить экстремумы для указанной матрицы и указанного диаметра
MainWindow::Extremums MainWindow::computeExtremums(const Matrix::Matrix2D<int>& matrix, float diameter) const
{
    // Размеры матрицы должны быть ненулевыми
    Q_ASSERT (matrix.getWidth() > 0);
    Q_ASSERT (matrix.getHeight() > 0);

    // масштабирующий коэффициент для более точного целочисленного вычисления
    const float Wavelet_Ratio = 1000.0;

    // Найти оптимальный размер вейвлета и размер матрицы,
    // исходя из исходного размера матрицы и заданного диаметра.
    QPair<int, QSize> optSizes(getOptimumSizes(matrix.getSize(), diameter));

    // Если размер матрицы вейвлета равен нулю, то исключаем текущий диаметр из поиска
    if (optSizes.first <= 0)
        return Extremums();

    // Получить матрицу вейвлета
    Matrix::Matrix2D<int> wMatrix;
    unsigned int waveletSize = (optSizes.first - 1) >> 1;     // Коэффициент размера вейвлета
    Wavelet::getWavelet2dMatrix<int>(&wMatrix, &Wavelet::getFhat2d, waveletSize, Wavelet_Ratio);

    // Получить уменьшенную матрицу исходной
    Matrix::Matrix2D<int> scaledMatrix;
    Matrix::scaleMatrix(&scaledMatrix, matrix, optSizes.second);

    // Наложить вейвлет на входное изображение
    Matrix::Matrix2D<int> outMatrix;
    Wavelet::imposeWavelet(&outMatrix, scaledMatrix, wMatrix, 255);

    // Найти минимумы и максимумы
    QPoint minPoint, maxPoint;
    int minVal = Wavelet_Ratio * 255, maxVal = -Wavelet_Ratio * 255;

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

    Extremums extrems;
    extrems.diameter = diameter;
    extrems.maxVal = maxVal;
    extrems.minVal = minVal;
    extrems.maxPoint = QPointF((float) maxPoint.x() / outMatrix.getWidth(),
                               (float) maxPoint.y() / outMatrix.getHeight());
    extrems.minPoint = QPointF((float) minPoint.x() / outMatrix.getWidth(),
                               (float) minPoint.y() / outMatrix.getHeight());
    return extrems;
}


// Найти оптимальный размер вейвлета и размер матрицы.
// Диаметр вычисляется по меньшей стороне.
QPair<int, QSize> MainWindow::getOptimumSizes(QSize matrixSize, float diameter) const
{
    // Минимальный размер исходной матрицы, ниже которого нельзя
    // его делать меньше
    const int Min_Matrix_Size = 16;

    Q_ASSERT (Min_Matrix_Size > 0);
    Q_ASSERT (matrixSize.width() >= Min_Matrix_Size);
    Q_ASSERT (matrixSize.height() >= Min_Matrix_Size);
    Q_ASSERT (diameter > 0.0 && diameter <= getMaxDiameter());

    Q_ASSERT (Optimum_Performance_Criteria > 10);
    static const float Optimum_Value = pow(Optimum_Performance_Criteria, 4);

    // Начинаем поиск от самого высокого разрешения (от исходного
    // размера матрицы), а потом начинаем уменьшать его,
    // чтобы достигнуть заданного оптимального значения

    int mWidth = matrixSize.width();     // ширина матрицы
    int mHeight = matrixSize.height();     // высота матрицы
    float mRatio = (float) mWidth / mHeight;    // Коэффициент пропорциональности сторон
    float wSize = 0;            // размер вейвлета
    float diameterSize = 0.0;   // диаметр шарика
    float sizesMult = 0.0;      // произведение квадратов размеров матрицы и вейвлета
    while (mWidth > Min_Matrix_Size &&
           mHeight > Min_Matrix_Size) {
        // Определить размер шарика для текущего размера матрицы
        diameterSize = diameter * qMin(mWidth, mHeight);
        // Определить размер вейвлета (для выбранного вейвлета "Французская шляпа"
        // размер вейвлета будет на sqrt(3) больше диаметра шара)
        wSize = diameterSize * sqrt(3.0);

        // Если размеры вейвлета и матрицы не оптимальны,
        // то уменьшаем размер исходной матрицы вдвое
        sizesMult = wSize * wSize * mWidth * mHeight;
        if ( (sizesMult <= Optimum_Value) ||
             ( (mWidth - 1) < Min_Matrix_Size) ||
             ( ((float) mWidth / mRatio) < Min_Matrix_Size))
            break;
        mWidth--;
        mHeight = (float) mWidth / mRatio;
    }

    return QPair<int, QSize>((int) wSize, QSize(mWidth, mHeight));
}
