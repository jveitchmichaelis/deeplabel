#ifndef IMAGEDISPLAY_H
#define IMAGEDISPLAY_H

#include <QWidget>
#include <QScrollArea>
#include <QScrollBar>
#include <QTemporaryDir>
#include <QApplication>

#include <QtAwesome.h>
#include <opencv2/opencv.hpp>
#include <imagelabel.h>

namespace Ui {
class ImageDisplay;
}

class ImageDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit ImageDisplay(QWidget *parent = nullptr);
    ~ImageDisplay();

public slots:
    void setImagePath(QString path);
    ImageLabel* getImageLabel(void){return imageLabel;}

private:
    cv::Mat display_image;
    QPixmap pixmap;
    Ui::ImageDisplay *ui;
    QString current_imagepath;
    QImage convert16(const cv::Mat &source);
    ImageLabel* imageLabel;
    QScrollArea* scrollArea;
    bool fit_to_window = false;
    double image_scale_factor = 1.0;
    QImage::Format format = QImage::Format_Grayscale8;

private slots:
    void loadPixmap();
    void updateDisplay();
    void resetZoom();
    void scaleImage();

    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    void zoomIn();
    void zoomOut();

signals:
    void image_loaded();
};

#endif // IMAGEDISPLAY_H
