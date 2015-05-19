#include "imageviewer.h"

#include <QVBoxLayout>
#include <QMouseEvent>
#include <QDebug>

ImageViewer::ImageViewer(QWidget *parent) :
    QWidget(parent)
{
    imageLabel = new QLabel;

    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(scrollArea);
    setLayout(layout);

    zoomSlider = new QSlider(Qt::Horizontal, this);
    zoomSlider->setMaximum(200);
    zoomSlider->setMinimum(1);
    zoomSlider->setValue(100);
    zoomSlider->setVisible(false);
    connect(zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(updateZoomLabel()));
    connect(zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(updateImage()));

    zoomLabel = new QLabel(this);
    zoomLabel->setVisible(false);

    sizeLabel = new QLabel(this);

    QHBoxLayout *zoomSliderHLayout = new QHBoxLayout;
    zoomSliderHLayout->addWidget(sizeLabel);
    zoomSliderHLayout->addSpacing(10);
    zoomSliderHLayout->addWidget(zoomSlider);
    zoomSliderHLayout->addWidget(zoomLabel);
    zoomSliderHLayout->addSpacing(10);    

    QVBoxLayout *zoomSliderLayout = new QVBoxLayout;
    zoomSliderLayout->addLayout(zoomSliderHLayout, 0);
    zoomSliderLayout->addStretch(1);
    scrollArea->setLayout(zoomSliderLayout);

    updateZoomLabel();

    posMarker = new ImageViewerMarker(this);
    posMarker->resize(10, 10);
    posMarker->move(x(), y());
    connect(posMarker, SIGNAL(posChanged(QPoint)), this, SLOT(onMarkerPosChanged(QPoint)));

    sizeMarker = new ImageViewerMarker(this);
    sizeMarker->resize(10, 10);
    connect(sizeMarker, SIGNAL(posChanged(QPoint)), this, SLOT(onMarkerSizeChanged(QPoint)));
}

ImageViewer::~ImageViewer()
{

}


void ImageViewer::onMarkerPosChanged(QPoint p)
{
    move(mapToParent(p));
}


void ImageViewer::onMarkerSizeChanged(QPoint p)
{
    resize(QSize(mapToParent(p).x() - x() + sizeMarker->width(),
                 mapToParent(p).y() - y() + sizeMarker->height()));
}


void ImageViewer::enterEvent(QEvent *event)
{
    zoomSlider->setVisible(true);
    zoomLabel->setVisible(true);
    QWidget::enterEvent(event);
}


void ImageViewer::leaveEvent(QEvent *event)
{
    zoomSlider->setVisible(false);
    zoomLabel->setVisible(false);
    QWidget::leaveEvent(event);
}


void ImageViewer::updateZoomLabel(void)
{
    zoomLabel->setText(QString::number(zoomSlider->value()));
}


void ImageViewer::updateImage(void)
{
    if (image.isNull())
        return;
    QSize oldSize(image.size());
    QSize newSize(oldSize);
    newSize *= ((double) zoomSlider->value()) / 100.0;
    imageLabel->setPixmap(QPixmap::fromImage(image.scaled(newSize)));
}

void ImageViewer::setImage(const QImage& im)
{
    image = im;
    sizeLabel->setText(QString("%1 x %2").arg(image.width()).arg(image.height()));
    updateImage();
}


const QImage& ImageViewer::getImage(void) const
{
    return image;
}


void ImageViewer::mousePressEvent(QMouseEvent *event)
{
}

void ImageViewer::mouseReleaseEvent(QMouseEvent *event)
{
}

void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
}

void ImageViewer::resizeEvent(QResizeEvent * event)
{
    sizeMarker->move(width() - sizeMarker->width(),
                     height() - sizeMarker->height());
}



ImageViewerMarker::ImageViewerMarker(QWidget *parent)
    : QLabel(parent), pressed(false)
{
    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setLineWidth(2);
}

void ImageViewerMarker::mousePressEvent(QMouseEvent *event)
{
    pressed = true;
}

void ImageViewerMarker::mouseReleaseEvent(QMouseEvent *event)
{
    pressed = false;
}

void ImageViewerMarker::mouseMoveEvent(QMouseEvent *event)
{
    if (pressed) {
        QPoint pos(event->pos());
        pos -= QPoint(width() / 2, height() / 2);
        emit posChanged(mapToParent(pos));
    }
}
