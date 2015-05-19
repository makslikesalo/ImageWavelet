#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QWidget>
#include <QScrollArea>
#include <QLabel>
#include <QSlider>
#include <QPushButton>


class ImageViewerMarker : public QLabel
{
    Q_OBJECT

public:
    ImageViewerMarker(QWidget *parent = 0);

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

signals:
    void posChanged(QPoint);

private:
    bool pressed;
};


class ImageViewer : public QWidget
{
    Q_OBJECT
public:
    explicit ImageViewer(QWidget *parent = 0);
    ~ImageViewer();

    void setImage(const QImage& im);
    const QImage& getImage(void) const;

signals:

protected:
    virtual void enterEvent(QEvent *event);
    virtual void leaveEvent(QEvent *event);

    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

    virtual void resizeEvent(QResizeEvent * event);

private slots:
    void updateZoomLabel(void);
    void updateImage(void);

    void onMarkerPosChanged(QPoint);
    void onMarkerSizeChanged(QPoint);

private:
    QScrollArea *scrollArea;
    QLabel *imageLabel;
    QSlider *zoomSlider;
    QLabel *zoomLabel;

    QLabel *sizeLabel;

    QImage image;

    ImageViewerMarker *sizeMarker;
    ImageViewerMarker *posMarker;
};

#endif // IMAGEVIEWER_H
