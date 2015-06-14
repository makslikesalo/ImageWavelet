#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressDialog>
#include <cmath>
#include <QFutureWatcher>

#include "imageviewer.h"
#include "matrix.h"

/*!
 * \brief The MainWindow класс окна приложения для поиска в изображении
 * светлого шарика на тёмном фоне.
 * В окне имеется компонент просмотра загруженного изображения,
 * а также изображения с результатами поиска.
 * Шарик может иметь как чёткие, так и расплывчатые границы.
 * Предельные размеры шарика:
 * Минимальный диаметр шарика - 16 пикселей
 * Максимальный диаметр шарика - 0.5 пикселов от минимальной стороны изображения
 * (например, w = 100, h = 70, макс. диаметр шарика = 70 * 0.5 = 35 пикселов)
 *
 * По окончании поиска, шарик будет обрисован красной контурной линией,
 * а также указан точкой его центр.
 *
 * \note Длина и ширина изображения могут быть неравными друг другу.
 * \note Рекомендуется выбирать не слишком большой размер изображения (не более 2 МПкс)
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();    

private slots:
    /*!
     * \brief openFile - процедура открытия файла для дальнейшей обработки.
     * При вызове отображается диалоговое окно открытия файла.
     * При утвердительном выборе файла, он становится доступным
     * для обработки.
     * Путь к открытому файлу сохраняется в filePath.
     */
    void openFile(void);


    /*!
     * \brief find - запуск процедуры поиска круглой заполненной светлой структуры
     * на текущем изображении filePath. По окончании процедуры, в просмоторщике
     * будет указан примерный диаметр и положение центра структуры.
     */
    void find(void);


    /*!
     * \brief loadImage - процедура загрузки изображения filePath в просмоторщик viewer.
     */
    void loadImage(void);

private slots:
    void handleExtremumsFinished();     // Обработка экстремумов завершена

private:

    // Кол-во интервалов для поиска диаметра на каждом шаге итерации
    static const int Search_Diameter_Intervals = 5;

    // Кол-во итераций поиска
    static const int Search_Iterations = 5;

    // Критерий оптимальной производительности при поиске оптимального
    // размера матрицы данных и матрицы вейвлета.
    // Для каждой конкретной вычислительной машины может быть индивидуален.
    // Чем больше коэффициент - тем точнее вычисления, но скорость вычислений падает.
    static const int Optimum_Performance_Criteria = 64;

    /*!
     * \brief The Extremums struct - структура с информацией об экстремумах,
     * если diameter = -1.0, значит экстремум не инициализирован
     */
    struct Extremums {
        float diameter;         // Относительный диаметр (от 0 до 1.0)
        QPointF maxPoint;       // Относительная точка максимума (от 0 до 1.0)
        int maxVal;             // Значение максимума
        QPointF minPoint;       // Относительная точка минимума (от 0 до 1.0)
        int minVal;             // Значение минимума
        Extremums() : diameter(-1.0)  {}
        Extremums(float d) : diameter(d)  {}
    };


    /*!
     * \brief The HandleWrapper структура для запуска асинхронного вычисления
     * экстремумов
     */
    struct HandleWrapper {
        MainWindow *instance;
        HandleWrapper(MainWindow *w): instance(w) {}
        void operator()(MainWindow::Extremums& ex) {
            instance->handleExtremums(ex);
        }
    };


    /*!
     * \brief createMenus - создать меню для текущего окна
     */
    void createMenus(void);


    /*!
     * \brief execSearch - выполнить этап поиска (обработчик машины состояния поиска)
     * Данная процедура вызывается в самом начале поиска и каждый раз,
     * как только завершается асинхронное вычисление множества диаметров шарика
     * \param reset - сбросить машину состояний, начать поиск заново
     */
    void execSearch(bool reset);


    /*!
     * \brief computeExtremums - вычилисть экстремумы для матрицы matrix и диаметра diameter
     * \param matrix - матрица значений, для которой выполняется поиск
     * \param diameter - диаметр структуры, для которой будут вычисляться экстремы
     * \return экстремумы. Если возвращает экстремум с diameter = -1.0, то данный
     * экстремум не был определён.
     */
    Extremums computeExtremums(const Matrix::Matrix2D<int>& matrix, float diameter) const;


    /*!
     * \brief getOptimumSizes - получить оптимальный размер вейвлета и размер матрицы данных
     * \param matrixSize - размер матрицы данных
     * \param diameter - диаметр шарика, для которого требуется найти оптимальные
     * размеры матрицы данных и матрицы вейвлета
     * \return пара значений - размер матрицы вейвлета и размер матрицы данных.
     */
    QPair<int, QSize> getOptimumSizes(QSize matrixSize, float diameter) const;


    /*!
     * \brief findIndexMaximum - Найти среди списка экстремумов максимальный, и вернуть его индекс.
     * \param vect - Список экстремумов.
     * \return индекс, если экстремум найден, -1 - если не найден.
     */
    int findIndexMaximum(const QVector<Extremums>& vect) const {
        // Найти максимумальный экстремум
        int maxIndex = -1;
        int maxValue = 0;
        for (int i = 0; i < vect.size(); ++i)
            if (vect.at(i).maxVal > maxValue) {
                maxValue = vect.at(i).maxVal;
                maxIndex = i;
            }
        return maxIndex;
    }


    /*!
     * \brief getMinDiameter - получить минимальный относительный диаметр шарика
     * для заданного размера изображения (матрицы)
     * \param size - размер изображения (или матрицы) для которой будет
     * вычислен минмиальный относительный диаметр шарика
     * \return значение минимального относительного диаметра шарика.
     */
    float getMinDiameter(const QSize& size) const {
        // Минимальный диаметр шарика определяется относительно минимальной стороны изображения

        const float Min_Diameter_Pixels = 16.0;     // Минимальный диаметр в пикселах

        return Min_Diameter_Pixels / qMin(size.width(), size.height());
    }


    /*!
     * \brief getMaxDiameter - получить максимальный относительный диаметр
     * шарика (относительно минимальной стороны изображения)
     * \return
     */
    float getMaxDiameter(void) const {
        // Для вейвлета "Французская шляпа" оптимизированного по двумерные вычисления
        static const float val = 1.0 / sqrt(3.0);
        return val;
    }

    /*!
     * \brief handleExtremums - процедура обработки матрицы изображения
     * и вычисления экстремумов.
     * Данная процедура используется для асинхронного вычисления.
     * \param ex - экстремум, из которого берётся значение диаметра для поиска
     * и в него кладутся вычисленные данные.
     */
    void handleExtremums(MainWindow::Extremums& ex) {
        ex = computeExtremums(imageMatrix, ex.diameter);
    }

    ImageViewer *viewer;        // Просмоторщик изображений
    QProgressDialog *progressDialog;        // Диалоговое окно прогресса

    QString filePath;           // Путь к обрабатываемому и просматриваемому файлу
    Matrix::Matrix2D<int> imageMatrix;      // Матрица значений исходного изображения

    // Список экстремумов, которые асинхронно обрабатываются
    QVector<MainWindow::Extremums> extrems;

    // Наблюдатель за завершением асинхронных вычислений
    QFutureWatcher<void> watcher;
    bool isSearching;       // Активен ли процесс асинхронного поиска
    int searchIter;         // Счётчик итераций поиска
};

#endif // MAINWINDOW_H
